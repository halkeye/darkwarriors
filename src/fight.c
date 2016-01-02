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
 *                                SWR Hotboot module                                     *
 *****************************************************************************************
 *                   $Id: fight.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#if defined(__CYGWIN__)
#include <io.h>
#else
#include <sys/dir.h>
#endif

#include "mud.h"
#ifdef ACCOUNT
#include "account.h"
#endif
#include "msp.h"
#include "bounty.h"
#include "races.h"

extern char lastplayercmd[MAX_INPUT_LENGTH];
extern CHAR_DATA *gch_prev;

/* From Skills.c */
int       ris_save(CHAR_DATA * ch, int percent_chance, int ris);

/* From arena.c */
bool arena_can_fight args((CHAR_DATA * ch, CHAR_DATA * victim));
void win_fight args((CHAR_DATA * winner, CHAR_DATA * looser));
bool in_arena args((CHAR_DATA * ch));

/*
 * Local functions.
 */
void dam_message args((CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt));
void group_gain args((CHAR_DATA * ch, CHAR_DATA * victim));
int xp_compute args((CHAR_DATA * gch, CHAR_DATA * victim));
int align_compute args((CHAR_DATA * gch, CHAR_DATA * victim));
ch_ret one_hit args((CHAR_DATA * ch, CHAR_DATA * victim, int dt));
int obj_hitroll args((OBJ_DATA * obj));
bool      get_cover(CHAR_DATA * ch);
bool      dual_flip = FALSE;

DECLARE_DO_FUN(do_draw);
DECLARE_DO_FUN(do_holster);
/* 
 * Check to see if someone is weilding a weapon
 */
                
#define IS_WEAPON(obj) ((obj)->pIndexData->item_type == ITEM_WEAPON || (obj)->pIndexData->item_type == ITEM_GRENADE)

bool is_wielding_weapon(CHAR_DATA * ch)
{
        OBJ_DATA *obj;
        if ((obj = get_eq_char(ch, WEAR_DUAL_WIELD)) != NULL)
                if (IS_WEAPON(obj))
                        return TRUE;
        if ((obj = get_eq_char(ch, WEAR_WIELD)) != NULL)
                if (IS_WEAPON(obj))
                        return TRUE;
        if ((obj = get_eq_char(ch, WEAR_HOLD)) != NULL)
                if (IS_WEAPON(obj))
                        return TRUE;
        if ((obj = get_eq_char(ch, WEAR_MISSILE_WIELD)) != NULL)
                if (IS_WEAPON(obj))
                        return TRUE;
        if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL)
                if (IS_WEAPON(obj))
                        return TRUE;
}
/*
 * Check to see if weapon is poisoned.
 */
bool is_wielding_poisoned(CHAR_DATA * ch)
{
        OBJ_DATA *obj;

        if ((obj = get_eq_char(ch, WEAR_WIELD))
            && (IS_SET(obj->extra_flags, ITEM_POISONED)))
                return TRUE;
        if ((obj = get_eq_char(ch, WEAR_DUAL_WIELD))
            && (IS_SET(obj->extra_flags, ITEM_POISONED)))
                return TRUE;

        return FALSE;

}

/*
 * hunting, hating and fearing code				-Thoric
 */
bool is_hunting(CHAR_DATA * ch, CHAR_DATA * victim)
{
        if (!ch->hunting || ch->hunting->who != victim)
                return FALSE;

        return TRUE;
}

bool is_hating(CHAR_DATA * ch, CHAR_DATA * victim)
{
        if (!ch->hating || ch->hating->who != victim)
                return FALSE;

        return TRUE;
}

bool is_fearing(CHAR_DATA * ch, CHAR_DATA * victim)
{
        if (!ch->fearing || ch->fearing->who != victim)
                return FALSE;

        return TRUE;
}

void stop_hunting(CHAR_DATA * ch)
{
        if (ch->hunting)
        {
                STRFREE(ch->hunting->name);
                DISPOSE(ch->hunting);
                ch->hunting = NULL;
        }
        return;
}

void stop_hating(CHAR_DATA * ch)
{
        if (ch->hating)
        {
                STRFREE(ch->hating->name);
                DISPOSE(ch->hating);
                ch->hating = NULL;
        }
        return;
}

void stop_fearing(CHAR_DATA * ch)
{
        if (ch->fearing)
        {
                STRFREE(ch->fearing->name);
                DISPOSE(ch->fearing);
                ch->fearing = NULL;
        }
        return;
}

void start_hunting(CHAR_DATA * ch, CHAR_DATA * victim)
{
        if (ch->hunting)
                stop_hunting(ch);

        CREATE(ch->hunting, HHF_DATA, 1);
        ch->hunting->name = QUICKLINK(victim->name);
        ch->hunting->who = victim;
        return;
}

void start_hating(CHAR_DATA * ch, CHAR_DATA * victim)
{
        if (ch->hating)
                stop_hating(ch);

        CREATE(ch->hating, HHF_DATA, 1);
        ch->hating->name = QUICKLINK(victim->name);
        ch->hating->who = victim;
        return;
}

void start_fearing(CHAR_DATA * ch, CHAR_DATA * victim)
{
        if (ch->fearing)
                stop_fearing(ch);

        CREATE(ch->fearing, HHF_DATA, 1);
        ch->fearing->name = QUICKLINK(victim->name);
        ch->fearing->who = victim;
        return;
}


int max_fight(CHAR_DATA * ch)
{
        ch = NULL;
        return 8;
}

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * Many hours spent fixing bugs in here by Thoric, as noted by residual
 * debugging checks.  If you never get any of these error messages again
 * in your logs... then you can comment out some of the checks without
 * worry.
 */
void violence_update(void)
{
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *ch;
        CHAR_DATA *lst_ch;
        CHAR_DATA *victim;
        CHAR_DATA *rch, *rch_next;
        AFFECT_DATA *paf, *paf_next;
        TIMER    *timer, *timer_next;
        ch_ret    retcode;
        SKILLTYPE *skill;
        int       sn;

        lst_ch = NULL;
        for (ch = last_char; ch; lst_ch = ch, ch = gch_prev)
        {

                if (ch == first_char && ch->prev)
                {
                        bug("ERROR: first_char->prev != NULL, fixing...", 0);
                        ch->prev = NULL;
                }

                gch_prev = ch->prev;

                if (gch_prev && gch_prev->next != ch)
                {
                        snprintf(buf, MSL,
                                 "FATAL: violence_update: %s->prev->next doesn't point to ch.",
                                 ch->name);
                        bug(buf, 0);
                        bug("Short-cutting here", 0);
                        ch->prev = NULL;
                        gch_prev = NULL;
                        do_shout(ch, "Thoric says, 'Prepare for the worst!'");
                }

                /*
                 * See if we got a pointer to someone who recently died...
                 * if so, either the pointer is bad... or it's a player who
                 * "died", and is back at the healer...
                 * Since he/she's in the char_list, it's likely to be the later...
                 * and should not already be in another fight already
                 */
                if (char_died(ch))
                        continue;

                /*
                 * See if we got a pointer to some bad looking data...
                 */
                if (!ch->in_room || !ch->name)
                {
                        log_string
                                ("violence_update: bad ch record!  (Shortcutting.)");
                        snprintf(buf, MSL,
                                 "ch: %d  ch->in_room: %d  ch->prev: %d  ch->next: %d",
                                 (int) ch, (int) ch->in_room, (int) ch->prev,
                                 (int) ch->next);
                        log_string(buf);
                        log_string(lastplayercmd);
                        if (lst_ch)
                                snprintf(buf, MSL,
                                         "lst_ch: %d  lst_ch->prev: %d  lst_ch->next: %d",
                                         (int) lst_ch, (int) lst_ch->prev,
                                         (int) lst_ch->next);
                        else
                                mudstrlcpy(buf, "lst_ch: NULL", MSL);
                        log_string(buf);
                        gch_prev = NULL;
                        continue;
                }

                /*
                 * Experience gained during battle deceases as battle drags on
                 */
                if (ch->fighting)
                        if ((++ch->fighting->duration % 24) == 0)
                                ch->fighting->xp =
                                        ((ch->fighting->xp * 9) / 10);


                for (timer = ch->first_timer; timer; timer = timer_next)
                {
                        timer_next = timer->next;

                        for (sn = 0; sn < top_sn && skill_table[sn]; sn++)
                                if (*skill_table[sn]->skill_fun ==
                                    timer->do_fun)
                                        break;

                        /*
                         * There's a bug here, about sn not being valid, but i can't think of how its explitable 
                         */
                        if (--timer->count <= 0
                            || (!IS_NPC(ch) &&
                                skill_table[sn] && skill_table[sn]->guild ==
                                ENGINEERING_ABILITY
                                && IS_SET(ch->pcdata->flags,
                                          PCFLAG_FASTENGINEER)))
                        {
                                if (timer->type == TIMER_DO_FUN)
                                {
                                        int       tempsub;

                                        tempsub = ch->substate;
                                        ch->substate = timer->value;
                                        (timer->do_fun) (ch, "");
                                        if (char_died(ch))
                                                break;
                                        ch->substate = tempsub;
                                }
                                /*
                                 * if ( timer->count == 0 ) 
                                 */
                                extract_timer(ch, timer);
                        }
                }

                if (char_died(ch))
                        continue;

                /*
                 * We need spells that have shorter durations than an hour.
                 * So a melee round sounds good to me... -Thoric
                 */
                for (paf = ch->first_affect; paf; paf = paf_next)
                {
                        paf_next = paf->next;
                        if (paf->duration > 0)
                                paf->duration--;
                        else if (paf->duration < 0)
                                ;
                        else
                        {
                                if (!paf_next
                                    || paf_next->type != paf->type
                                    || paf_next->duration > 0)
                                {
                                        skill = get_skilltype(paf->type);
                                        if (paf->type > 0 && skill
                                            && skill->msg_off)
                                        {
                                                set_char_color(AT_WEAROFF,
                                                               ch);
                                                send_to_char(skill->msg_off,
                                                             ch);
                                                send_to_char("\n\r", ch);
                                        }
                                }
                                if (paf->type == gsn_possess)
                                {
                                        ch->desc->character =
                                                ch->desc->original;
                                        ch->desc->original = NULL;
                                        ch->desc->character->desc = ch->desc;
                                        ch->desc->character->switched = NULL;
                                        ch->desc = NULL;
                                }
                                affect_remove(ch, paf);
                        }
                }

                if ((victim = who_fighting(ch)) == NULL
                    || IS_AFFECTED(ch, AFF_PARALYSIS))
                        continue;

                retcode = rNONE;

                if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
                {
                        snprintf(buf, MSL,
                                 "violence_update: %s fighting %s in a SAFE room.",
                                 ch->name, victim->name);
                        log_string(buf);
                        stop_fighting(ch, TRUE);
                }
                else if (IS_AWAKE(ch) && ch->in_room == victim->in_room)
                        retcode = multi_hit(ch, victim, TYPE_UNDEFINED);
                else
                        stop_fighting(ch, FALSE);

                if (char_died(ch))
                        continue;

                if (retcode == rCHAR_DIED
                    || (victim = who_fighting(ch)) == NULL)
                        continue;

                /*
                 *  Mob triggers
                 */
                rprog_rfight_trigger(ch);
                if (char_died(ch))
                        continue;
                mprog_hitprcnt_trigger(ch, victim);
                if (char_died(ch))
                        continue;
                mprog_fight_trigger(ch, victim);
                if (char_died(ch))
                        continue;

                /*
                 * Fun for the whole family!
                 */
                for (rch = ch->in_room->first_person; rch; rch = rch_next)
                {
                        rch_next = rch->next_in_room;

                        if (IS_AWAKE(rch) && !rch->fighting)
                        {
                                /*
                                 * PC's auto-assist others in their group.
                                 */
                                if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
                                {
                                        if ((!IS_NPC(rch)
                                             || IS_AFFECTED(rch, AFF_CHARM))
                                            && is_same_group(ch, rch))
                                                multi_hit(rch, victim,
                                                          TYPE_UNDEFINED);
                                        continue;
                                }

                                /*
                                 * NPC's assist NPC's of same type or 12.5% chance regardless.
                                 */
                                if (IS_NPC(rch)
                                    && !IS_AFFECTED(rch, AFF_CHARM)
                                    && !IS_SET(rch->act, ACT_NOASSIST))
                                {
                                        if (char_died(ch))
                                                break;
                                        if (rch->pIndexData == ch->pIndexData
                                            || number_bits(3) == 0)
                                        {
                                                CHAR_DATA *vch;
                                                CHAR_DATA *target;
                                                int       number;

                                                target = NULL;
                                                number = 0;
                                                for (vch =
                                                     ch->in_room->
                                                     first_person; vch;
                                                     vch = vch->next_in_room)
                                                {
                                                        if (can_see(rch, vch)
                                                            &&
                                                            is_same_group(vch,
                                                                          victim)
                                                            && number_range(0,
                                                                            number)
                                                            == 0)
                                                        {
                                                                target = vch;
                                                                number++;
                                                        }
                                                }

                                                if (target)
                                                        multi_hit(rch, target,
                                                                  TYPE_UNDEFINED);
                                        }
                                }
                        }
                }
        }

        return;
}



/*
 * Do one group of attacks.
 */
ch_ret multi_hit(CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
        int       percent_chance;
        int       dual_bonus;
        int       chance = 0;
        ch_ret    retcode;

        /*
         * add timer if player is attacking another player 
         */
        if (!IS_NPC(ch) && !IS_NPC(victim))
                add_timer(ch, TIMER_RECENTFIGHT, 20, NULL, 0);

        if (!IS_NPC(ch) && IS_SET(ch->act, PLR_NICE) && !IS_NPC(victim))
                return rNONE;


        if (!arena_can_fight(ch, victim))
        {
                send_to_char
                        ("You can't do that yet. Wait for betting to close.\n\r",
                         ch);
                return rNONE;
        }
        /*
         *  Check if either of the combatants are holding someone, and if so
         *  there is a chance the fight will release them. This will allow for
         *  even weak characters to become martyrs, or the toughest combatants
         *  to one-round their enemy without losing their quarry.
         *                                                       -Ulthrax
         */
        if (ch->holding != NULL)
        {
                chance = IS_NPC(victim) ? victim->top_level : number_range(10,
                                                                           80);
                if (number_percent() < chance)
                {
                        CHAR_DATA *holding = NULL;

                        if ((holding = ch->holding) == NULL)
                        {
                                bug("%s with null holding structure in multi_hit", ch->name);
                                return rNONE;
                        }
                        send_to_char
                                ("The fight distracts you, and you lose hold of your victim.\n\r",
                                 ch);
                        act(AT_ACTION,
                            "In the midst of the fight, $n lets go of $N!",
                            ch, NULL, holding, TO_NOTVICT);
                        do_release(ch, "");
                }

        }
        if (victim->holding != NULL)
        {
                chance = IS_NPC(victim) ? victim->top_level : number_range(10,
                                                                           80);
                if (number_percent() < chance)
                {
                        CHAR_DATA *holding = NULL;

                        if ((holding = victim->holding) == NULL)
                        {
                                bug("%s with null holding structure in multi_hit", victim->name);
                                return rNONE;
                        }
                        send_to_char
                                ("The fight distracts you, and you lose hold of your victim.\n\r",
                                 ch);
                        act(AT_ACTION,
                            "In the midst of the fight, $n lets go of $N!",
                            victim, NULL, holding, TO_NOTVICT);
                        do_release(victim, "");
                }

        }
        if (!IS_NPC(ch) && IS_SET(ch->pcdata->flags, PCFLAG_AUTODRAW))
        {
                OBJ_DATA *holster1 = get_eq_char(ch, WEAR_HOLSTER_L),
                         *holster2 = get_eq_char(ch, WEAR_HOLSTER_R);
                if ((holster1 && holster1->item_type == ITEM_HOLSTER && holster1->first_content) ||
                                (holster2 && holster2->item_type == ITEM_HOLSTER && holster2->first_content))
                        do_draw(ch, "");
        }
        if ((retcode = one_hit(ch, victim, dt)) != rNONE)
                return retcode;

        if (who_fighting(ch) != victim || dt == gsn_backstab
            || dt == gsn_circle || dt == gsn_jab)
                return rNONE;

        /*
         * Very high percent_chance of hitting compared to percent_chance of going berserk 
         */
        /*
         * 40% or higher is always hit.. don't learn anything here though. 
         */
        /*
         * -- Altrag 
         */
        percent_chance =
                IS_NPC(ch) ? 100 : (ch->pcdata->learned[gsn_berserk] * 5 / 2);
        if (IS_AFFECTED(ch, AFF_BERSERK) && number_percent() < percent_chance)
                if ((retcode = one_hit(ch, victim, dt)) != rNONE ||
                    who_fighting(ch) != victim)
                        return retcode;

        if (get_eq_char(ch, WEAR_DUAL_WIELD))
        {
                dual_bonus =
                        IS_NPC(ch) ? (ch->skill_level[COMBAT_ABILITY] /
                                      10) : (ch->pcdata->
                                             learned[gsn_dual_wield] / 10);
                percent_chance =
                        IS_NPC(ch) ? ch->top_level : ch->pcdata->
                        learned[gsn_dual_wield];
                if (number_percent() < percent_chance)
                {
                        learn_from_success(ch, gsn_dual_wield);
                        retcode = one_hit(ch, victim, dt);
                        if (retcode != rNONE || who_fighting(ch) != victim)
                                return retcode;
                }
                else
                        learn_from_failure(ch, gsn_dual_wield);
        }
        else
                dual_bonus = 0;

        if (ch->endurance < 10)
                dual_bonus = -20;

        /*
         * NPC predetermined number of attacks          -Thoric
         */
        if (IS_NPC(ch) && ch->numattacks > 0)
        {
                for (percent_chance = 0; percent_chance <= ch->numattacks;
                     percent_chance++)
                {
                        retcode = one_hit(ch, victim, dt);
                        if (retcode != rNONE || who_fighting(ch) != victim)
                                return retcode;
                }
                return retcode;
        }

        percent_chance = IS_NPC(ch) ? ch->top_level
                : (int) ((ch->pcdata->learned[gsn_second_attack] +
                          dual_bonus) / 1.5);
        if (number_percent() < percent_chance)
        {
                learn_from_success(ch, gsn_second_attack);
                retcode = one_hit(ch, victim, dt);
                if (retcode != rNONE || who_fighting(ch) != victim)
                        return retcode;
        }
        else
                learn_from_failure(ch, gsn_second_attack);

        percent_chance = IS_NPC(ch) ? ch->top_level
                : (int) ((ch->pcdata->learned[gsn_third_attack] +
                          (dual_bonus * 1.5)) / 2);
        if (number_percent() < percent_chance)
        {
                learn_from_success(ch, gsn_third_attack);
                retcode = one_hit(ch, victim, dt);
                if (retcode != rNONE || who_fighting(ch) != victim)
                        return retcode;
        }
        else
                learn_from_failure(ch, gsn_third_attack);

        retcode = rNONE;

        percent_chance = IS_NPC(ch) ? (int) (ch->top_level / 4) : 0;
        if (number_percent() < percent_chance)
                retcode = one_hit(ch, victim, dt);

        if (retcode == rNONE)
        {
                int       endurance;

                if (!IS_AFFECTED(ch, AFF_FLYING)
                    && !IS_AFFECTED(ch, AFF_FLOATING))
                        endurance =
                                encumbrance(ch,
                                            movement_loss[UMIN
                                                          (SECT_MAX - 1,
                                                           ch->in_room->
                                                           sector_type)]);
                else
                        endurance = encumbrance(ch, 1);
                if (ch->endurance)
                        ch->endurance = UMAX(0, ch->endurance - endurance);
        }

        return retcode;
}


/*
 * Weapon types, haus
 */
int weapon_prof_bonus_check(CHAR_DATA * ch, OBJ_DATA * wield, int *gsn_ptr)
{
        int       bonus;

        bonus = 0;
        *gsn_ptr = -1;
        if (!IS_NPC(ch) && wield)
        {
                switch (wield->value[3])
                {
                default:
                        *gsn_ptr = -1;
                        break;
                case 3:
                        *gsn_ptr = gsn_lightsabers;
                        break;
                case 2:
                        *gsn_ptr = gsn_vibro_blades;
                        break;
                case 4:
                        *gsn_ptr = gsn_flexible_arms;
                        break;
                case 5:
                        *gsn_ptr = gsn_knives;
                        break;
                case 6:
                        *gsn_ptr = gsn_blasters;
                        break;
                case 8:
                        *gsn_ptr = gsn_bludgeons;
                        break;
                case 9:
                        *gsn_ptr = gsn_bowcasters;
                        break;
                case 11:
                        *gsn_ptr = gsn_force_pikes;
                        break;

                }
                if (*gsn_ptr != -1)
                        bonus = (int) (ch->pcdata->learned[*gsn_ptr]);

        }
        if (IS_NPC(ch) && wield)
                bonus = get_trust(ch);
        return bonus;
}

/*
 * Calculate the tohit bonus on the object and return RIS values.
 * -- Altrag
 */
int obj_hitroll(OBJ_DATA * obj)
{
        int       tohit = 0;
        AFFECT_DATA *paf;

        for (paf = obj->pIndexData->first_affect; paf; paf = paf->next)
                if (paf->location == APPLY_HITROLL)
                        tohit += paf->modifier;
        for (paf = obj->first_affect; paf; paf = paf->next)
                if (paf->location == APPLY_HITROLL)
                        tohit += paf->modifier;
        return tohit;
}

/*
 * Offensive shield level modifier
 */
sh_int off_shld_lvl(CHAR_DATA * ch, CHAR_DATA * victim)
{
        sh_int    lvl;

        if (!IS_NPC(ch))    /* players get much less effect */
        {
                lvl = UMAX(1, (ch->skill_level[FORCE_ABILITY]));
                if (number_percent() +
                    (victim->skill_level[COMBAT_ABILITY] - lvl) < 35)
                        return lvl;
                else
                        return 0;
        }
        else
        {
                lvl = ch->top_level;
                if (number_percent() +
                    (victim->skill_level[COMBAT_ABILITY] - lvl) < 70)
                        return lvl;
                else
                        return 0;
        }
}

/*
 * Hit one guy once.
 */
ch_ret one_hit(CHAR_DATA * ch, CHAR_DATA * victim, int dt)
{
        OBJ_DATA *wield;
        int       victim_ac;
        int       thac0;
        int       thac0_00;
        int       thac0_32;
        int       plusris;
        int       dam, x;
        int       diceroll;
        int       attacktype, cnt;
        int       prof_bonus;
        int       prof_gsn;
        ch_ret    retcode = 0;
        int       percent_chance;
        bool      fail;
        AFFECT_DATA af;


        /*
         * Can't beat a dead char!
         * Guard against weird room-leavings.
         */
        if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
                return rVICT_DIED;
        if ((get_eq_char(ch, WEAR_BINDING)) != NULL)
        {
                send_to_char("You are bound, and cannot fight!\n\r", ch);
                return rNONE;
        }

        /*
         * Figure out the weapon doing the damage           -Thoric
         */
        if ((wield = get_eq_char(ch, WEAR_DUAL_WIELD)) != NULL)
        {
                if (dual_flip == FALSE)
                {
                        dual_flip = TRUE;
                        wield = get_eq_char(ch, WEAR_WIELD);
                }
                else
                        dual_flip = FALSE;
        }
        else
                wield = get_eq_char(ch, WEAR_WIELD);

        prof_bonus = weapon_prof_bonus_check(ch, wield, &prof_gsn);

        if (ch->fighting    /* make sure fight is already started */
            && dt == TYPE_UNDEFINED && IS_NPC(ch) && ch->attacks != 0)
        {
                cnt = 0;
                for (;;)
                {
                        x = number_range(0, 6);
                        attacktype = 1 << x;
                        if (IS_SET(ch->attacks, attacktype))
                                break;
                        if (cnt++ > 16)
                        {
                                attacktype = 0;
                                break;
                        }
                }
                if (attacktype == ATCK_BACKSTAB)
                        attacktype = 0;
                if (wield && number_percent() > 25)
                        attacktype = 0;
                switch (attacktype)
                {
                default:
                        break;
                case ATCK_BITE:
                        do_bite(ch, "");
                        retcode = global_retcode;
                        break;
                case ATCK_CLAWS:
                        do_claw(ch, "");
                        retcode = global_retcode;
                        break;
                case ATCK_TAIL:
                        do_tail(ch, "");
                        retcode = global_retcode;
                        break;
                case ATCK_STING:
                        do_sting(ch, "");
                        retcode = global_retcode;
                        break;
                case ATCK_PUNCH:
                        do_punch(ch, "");
                        retcode = global_retcode;
                        break;
                case ATCK_KICK:
                        do_kick(ch, "");
                        retcode = global_retcode;
                        break;
                case ATCK_TRIP:
                        attacktype = 0;
                        break;
                }
                if (attacktype)
                        return retcode;
        }

        if (dt == TYPE_UNDEFINED)
        {
                dt = TYPE_HIT;
                if (wield && wield->item_type == ITEM_WEAPON)
                        dt += wield->value[3];
        }

        /*
         * Calculate to-hit-armor-class-0 versus armor.
         */
        thac0_00 = 20;
        thac0_32 = 10;
        thac0 = interpolate(ch->skill_level[COMBAT_ABILITY], thac0_00,
                            thac0_32) - GET_HITROLL(ch);
        victim_ac = (int) (GET_AC(victim) / 10);

        /*
         * if you can't see what's coming... 
         */
        if (wield && !can_see_obj(victim, wield))
                victim_ac += 1;
        if (!can_see(ch, victim))
                victim_ac -= 4;

        if (!str_cmp(ch->race->name(), "defel"))
                victim_ac += 2;

        if (!IS_AWAKE(victim))
                victim_ac += 5;

        /*
         * Weapon proficiency bonus 
         */
        victim_ac += prof_bonus / 20;

        /*
         * The moment of excitement!
         */
        diceroll = number_range(1, 20);

        if (diceroll == 1 || (diceroll < 20 && diceroll < thac0 - victim_ac))
        {
                /*
                 * Miss. 
                 */
                if (prof_gsn != -1)
                        learn_from_failure(ch, prof_gsn);
                damage(ch, victim, 0, dt);
                tail_chain();
                return rNONE;
        }

        /*
         * Hit.
         * Calc damage.
         */

        if (!wield) /* dice formula fixed by Thoric */
                dam = number_range(ch->barenumdie,
                                   ch->baresizedie * ch->barenumdie);
        else
                dam = number_range(wield->value[1], wield->value[2]);

        /*
         * Bonuses.
         */

        dam += GET_DAMROLL(ch);

        if (IS_SET(ch->affected_by, AFF_RESTRAINED))
        {
                dam /= 100;
        }

        if (prof_bonus)
                dam *= (1 + prof_bonus / 100);


        if (!IS_NPC(ch) && ch->pcdata->learned[gsn_enhanced_damage] > 0)
        {
                dam += (int) (dam * ch->pcdata->learned[gsn_enhanced_damage] /
                              120);
                learn_from_success(ch, gsn_enhanced_damage);
        }


        if (!IS_AWAKE(victim))
                dam *= 2;
        if (dt == gsn_backstab)
                dam *= (2 +
                        URANGE(2,
                               ch->skill_level[HUNTING_ABILITY] -
                               (victim->skill_level[COMBAT_ABILITY] / 4),
                               30) / 16);

        if (dt == gsn_circle)
                dam *= (2 +
                        URANGE(2,
                               ch->skill_level[HUNTING_ABILITY] -
                               (victim->skill_level[COMBAT_ABILITY] / 4),
                               30) / 32);

        if (dt == gsn_jab)
                dam *= (2 +
                        URANGE(2,
                               ch->skill_level[HUNTING_ABILITY] -
                               (victim->skill_level[COMBAT_ABILITY] / 4),
                               30) / 16);

        plusris = 0;

        if (wield)
        {
                if (IS_SET(wield->extra_flags, ITEM_MAGIC))
                        dam = ris_damage(victim, dam, RIS_MAGIC);
                else
                        dam = ris_damage(victim, dam, RIS_NONMAGIC);

                /*
                 * Handle PLUS1 - PLUS6 ris bits vs. weapon hitroll   -Thoric
                 */
                plusris = obj_hitroll(wield);
        }
        else
                dam = ris_damage(victim, dam, RIS_NONMAGIC);

        /*
         * check for RIS_PLUSx                  -Thoric 
         */
        if (dam)
        {
                int       res, imm, sus, mod;

                if (plusris)
                        plusris = RIS_PLUS1 << UMIN(plusris, 7);

                /*
                 * initialize values to handle a zero plusris 
                 */
                imm = res = -1;
                sus = 1;

                /*
                 * find high ris 
                 */
                for (x = RIS_PLUS1; x <= RIS_PLUS6; x <<= 1)
                {
                        if (IS_SET(victim->immune, x))
                                imm = x;
                        if (IS_SET(victim->resistant, x))
                                res = x;
                        if (IS_SET(victim->susceptible, x))
                                sus = x;
                }
                mod = 10;
                if (imm >= plusris)
                        mod -= 10;
                if (res >= plusris)
                        mod -= 2;
                if (sus <= plusris)
                        mod += 2;

                /*
                 * check if immune 
                 */
                if (mod <= 0)
                        dam = -1;
                if (mod != 10)
                        dam = (dam * mod) / 10;
        }

        /*
         * race modifier 
         */

        if (!str_cmp(victim->race->name(), "duinuogwuin"))
                dam /= 5;

        /*
         * check to see if weapon is charged 
         */

        if (dt == (TYPE_HIT + WEAPON_BLASTER) && wield
            && wield->item_type == ITEM_WEAPON)
        {
                if (wield->value[4] < 1)
                {
                        act(AT_YELLOW,
                            "$n points their blaster at you but nothing happens.",
                            ch, NULL, victim, TO_VICT);
                        act(AT_YELLOW,
                            "*CLICK* ... your blaster needs a new ammunition cell!",
                            ch, NULL, victim, TO_CHAR);
                        if (IS_NPC(ch))
                        {
                                do_remove(ch, wield->name);
                        }
                        return rNONE;
                }
                else if (wield->blaster_setting == BLASTER_FULL
                         && wield->value[4] >= 5)
                {
                        dam *= (sh_int) 1.5;
                        wield->value[4] -= 5;
                }
                else if (wield->blaster_setting == BLASTER_HIGH
                         && wield->value[4] >= 4)
                {
                        dam *= (sh_int) 1.25;
                        wield->value[4] -= 4;
                }
                else if (wield->blaster_setting == BLASTER_NORMAL
                         && wield->value[4] >= 3)
                {
                        wield->value[4] -= 3;
                }
                else if (wield->blaster_setting == BLASTER_STUN
                         && wield->value[4] >= 5)
                {
                        dam /= 10;
                        wield->value[4] -= 3;
                        fail = FALSE;
                        percent_chance =
                                ris_save(victim,
                                         ch->skill_level[COMBAT_ABILITY],
                                         RIS_PARALYSIS);
                        if (percent_chance == 1000)
                                fail = TRUE;
                        else
                                fail = saves_para_petri(percent_chance,
                                                        victim);
                        if (victim->was_stunned > 0)
                        {
                                fail = TRUE;
                                victim->was_stunned--;
                        }
                        percent_chance =
                                100 - get_curr_con(victim) -
                                victim->skill_level[COMBAT_ABILITY] / 2;
                        /*
                         * harder for player to stun another player 
                         */
                        if (!IS_NPC(ch) && !IS_NPC(victim))
                                percent_chance -= sysdata.stun_plr_vs_plr;
                        else
                                percent_chance -= sysdata.stun_regular;
                        percent_chance = URANGE(5, percent_chance, 95);
                        if (!fail && number_percent() < percent_chance)
                        {
                                WAIT_STATE(victim, PULSE_VIOLENCE);
                                act(AT_BLUE,
                                    "Blue rings of energy from &R$N's &Bblaster knock you down leaving you stunned!",
                                    victim, NULL, ch, TO_CHAR);
                                act(AT_BLUE,
                                    "Blue rings of energy from your blaster strike &R$N&B, leaving $M &Bstunned!",
                                    ch, NULL, victim, TO_CHAR);
                                act(AT_BLUE,
                                    "Blue rings of energy from &R$n's &Bblaster hit &R$N, &Bleaving $M stunned!",
                                    ch, NULL, victim, TO_NOTVICT);
                                stop_fighting(victim, TRUE);
                                if (!IS_AFFECTED(victim, AFF_PARALYSIS))
                                {
                                        af.type = gsn_stun;
                                        af.location = APPLY_AC;
                                        af.modifier = 20;
                                        af.duration = 7;
                                        af.bitvector = AFF_PARALYSIS;
                                        affect_to_char(victim, &af);
                                        update_pos(victim);
                                        if (IS_NPC(victim))
                                        {
                                                start_hating(victim, ch);
                                                start_hunting(victim, ch);
                                                victim->was_stunned = 10;
                                        }
                                }
                        }
                        else
                        {
                                act(AT_BLUE,
                                    "Blue rings of energy from &R$N's &Bblaster hit you but have little effect",
                                    victim, NULL, ch, TO_CHAR);
                                act(AT_BLUE,
                                    "Blue rings of energy from your blaster hit &R$N,&B but nothing seems to happen!",
                                    ch, NULL, victim, TO_CHAR);
                                act(AT_BLUE,
                                    "Blue rings of energy from $n's blaster hit &R$N,&B but nothing seems to happen!",
                                    ch, NULL, victim, TO_NOTVICT);

                        }
                }
                else if (wield->blaster_setting == BLASTER_HALF
                         && wield->value[4] >= 2)
                {
                        dam *= (sh_int) 0.75;
                        wield->value[4] -= 2;
                }
                else
                {
                        dam *= (sh_int) 0.5;
                        wield->value[4] -= 1;
                }

        }
        else if (dt == (TYPE_HIT + WEAPON_VIBRO_BLADE)
                 && wield && wield->item_type == ITEM_WEAPON)
        {
                if (wield->value[4] < 1)
                {
                        act(AT_YELLOW,
                            "Your vibro-blade needs recharging ...", ch, NULL,
                            victim, TO_CHAR);
                        dam /= 3;
                }
        }
        else if (dt == (TYPE_HIT + WEAPON_FORCE_PIKE)
                 && wield && wield->item_type == ITEM_WEAPON)
        {
                if (wield->value[4] < 1)
                {
                        act(AT_YELLOW, "Your force-pike needs recharging ...",
                            ch, NULL, victim, TO_CHAR);
                        dam /= 2;
                }
                else
                        wield->value[4]--;
        }
        else if (dt == (TYPE_HIT + WEAPON_LIGHTSABER)
                 && wield && wield->item_type == ITEM_WEAPON)
        {
                if (wield->value[4] < 1)
                {
                        act(AT_YELLOW,
                            "$n waves a dead hand grip around in the air.",
                            ch, NULL, victim, TO_VICT);
                        act(AT_YELLOW,
                            "You need to recharge your lightsaber ... it seems to be lacking a blade.",
                            ch, NULL, victim, TO_CHAR);
                        if (IS_NPC(ch))
                        {
                                do_remove(ch, wield->name);
                        }
                        return rNONE;
                }
        }
        else if (dt == (TYPE_HIT + WEAPON_BOWCASTER) && wield
                 && wield->item_type == ITEM_WEAPON)
        {
                if (wield->value[4] < 1)
                {
                        act(AT_YELLOW,
                            "$n points their bowcaster at you but nothing happens.",
                            ch, NULL, victim, TO_VICT);
                        act(AT_YELLOW,
                            "*CLICK* ... your bowcaster needs a new bolt cartridge!",
                            ch, NULL, victim, TO_CHAR);
                        if (IS_NPC(ch))
                        {
                                do_remove(ch, wield->name);
                        }
                        return rNONE;
                }
                else
                        wield->value[4]--;
        }

        if (dam <= 0)
                dam = 1;

        if (prof_gsn != -1)
        {
                if (dam > 0)
                        learn_from_success(ch, prof_gsn);
                else
                        learn_from_failure(ch, prof_gsn);
        }

        /*
         * immune to damage 
         */
        if (dam == -1)
        {
                if (dt >= 0 && dt < top_sn)
                {
                        SKILLTYPE *skill = skill_table[dt];
                        bool      found = FALSE;

                        if (skill->imm_char && skill->imm_char[0] != '\0')
                        {
                                act(AT_HIT, skill->imm_char, ch, NULL, victim,
                                    TO_CHAR);
                                found = TRUE;
                        }
                        if (skill->imm_vict && skill->imm_vict[0] != '\0')
                        {
                                act(AT_HITME, skill->imm_vict, ch, NULL,
                                    victim, TO_VICT);
                                found = TRUE;
                        }
                        if (skill->imm_room && skill->imm_room[0] != '\0')
                        {
                                act(AT_ACTION, skill->imm_room, ch, NULL,
                                    victim, TO_NOTVICT);
                                found = TRUE;
                        }
                        if (found)
                                return rNONE;
                }
                dam = 0;
        }
        if ((retcode = damage(ch, victim, dam, dt)) != rNONE)
                return retcode;
        if (char_died(ch))
                return rCHAR_DIED;
        if (char_died(victim))
                return rVICT_DIED;

        retcode = rNONE;
        if (dam == 0)
                return retcode;

/* weapon spells	-Thoric */
        if (wield
            && !IS_SET(victim->immune, RIS_MAGIC)
            && !xIS_SET(victim->in_room->room_flags, ROOM_NO_MAGIC))
        {
                AFFECT_DATA *aff;

                for (aff = wield->pIndexData->first_affect; aff;
                     aff = aff->next)
                        if (aff->location == APPLY_WEAPONSPELL
                            && IS_VALID_SN(aff->modifier)
                            && skill_table[aff->modifier]->spell_fun)
                                retcode =
                                        (*skill_table[aff->modifier]->
                                         spell_fun) (aff->modifier,
                                                     (wield->level + 3) / 3,
                                                     ch, victim);
                if (retcode != rNONE || char_died(ch) || char_died(victim))
                        return retcode;
                for (aff = wield->first_affect; aff; aff = aff->next)
                        if (aff->location == APPLY_WEAPONSPELL
                            && IS_VALID_SN(aff->modifier)
                            && skill_table[aff->modifier]->spell_fun)
                                retcode =
                                        (*skill_table[aff->modifier]->
                                         spell_fun) (aff->modifier,
                                                     (wield->level + 3) / 3,
                                                     ch, victim);
                if (retcode != rNONE || char_died(ch) || char_died(victim))
                        return retcode;
        }

        /*
         * magic shields that retaliate             -Thoric
         */
        if (IS_AFFECTED(victim, AFF_FIRESHIELD)
            && !IS_AFFECTED(ch, AFF_FIRESHIELD))
                retcode =
                        spell_fireball(gsn_fireball, off_shld_lvl(victim, ch),
                                       victim, ch);
        if (retcode != rNONE || char_died(ch) || char_died(victim))
                return retcode;

        if (retcode != rNONE || char_died(ch) || char_died(victim))
                return retcode;

        if (IS_AFFECTED(victim, AFF_SHOCKSHIELD)
            && !IS_AFFECTED(ch, AFF_SHOCKSHIELD))
                retcode =
                        spell_lightning_bolt(gsn_lightning_bolt,
                                             off_shld_lvl(victim, ch), victim,
                                             ch);
        if (retcode != rNONE || char_died(ch) || char_died(victim))
                return retcode;

        /*
         *   folks with blasters move and snipe instead of getting neatin up in one spot.
         */
        if (IS_NPC(victim) && !IS_SET(victim->act, ACT_NORUNSNIPE))
        {
                OBJ_DATA *blaster_wield;

                blaster_wield = get_eq_char(victim, WEAR_WIELD);
                if (blaster_wield != NULL
                    && blaster_wield->value[3] == WEAPON_BLASTER
                    && get_cover(victim) == TRUE)
                {
                        start_hating(victim, ch);
                        start_hunting(victim, ch);
                }
        }

        tail_chain();
        return retcode;
}

/*
 * Calculate damage based on resistances, immunities and suceptibilities
 *					-Thoric
 */
sh_int ris_damage(CHAR_DATA * ch, sh_int dam, int ris)
{
        sh_int    modifier;

        modifier = 10;
        if (IS_SET(ch->immune, ris))
                modifier -= 10;
        if (IS_SET(ch->resistant, ris))
                modifier -= 2;
        if (IS_SET(ch->susceptible, ris))
                modifier += 2;
        if (modifier <= 0)
                return -1;
        if (modifier == 10)
                return dam;
        return (dam * modifier) / 10;
}


/*
 * Inflict damage from a hit.
 */
ch_ret damage(CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt)
{
        char      buf1[MAX_STRING_LENGTH];
        sh_int    dameq;
        bool      npcvict;
        bool      loot;
        int       xp_gain;
        OBJ_DATA *damobj;
        ch_ret    retcode;
        sh_int    dampmod;
        CHAR_DATA *gch;

        int       init_gold, new_gold, gold_diff;

        retcode = rNONE;

        if (!ch)
        {
                bug("Damage: null ch!", 0);
                return rERROR;
        }
        if (!victim)
        {
                bug("Damage: null victim!", 0);
                return rVICT_DIED;
        }

        if (victim->position == POS_DEAD)
                return rVICT_DIED;

        npcvict = IS_NPC(victim);

        /*
         * Check damage types for RIS               -Thoric
         */
        if (dam && dt != TYPE_UNDEFINED)
        {
                if (IS_FIRE(dt))
                        dam = ris_damage(victim, dam, RIS_FIRE);
                else if (IS_COLD(dt))
                        dam = ris_damage(victim, dam, RIS_COLD);
                else if (IS_ACID(dt))
                        dam = ris_damage(victim, dam, RIS_ACID);
                else if (IS_ELECTRICITY(dt))
                        dam = ris_damage(victim, dam, RIS_ELECTRICITY);
                else if (IS_ENERGY(dt))
                        dam = ris_damage(victim, dam, RIS_ENERGY);
                else if (IS_DRAIN(dt))
                        dam = ris_damage(victim, dam, RIS_DRAIN);
                else if (dt == gsn_poison || IS_POISON(dt))
                        dam = ris_damage(victim, dam, RIS_POISON);
                else if (dt == (TYPE_HIT + 7) || dt == (TYPE_HIT + 8))
                        dam = ris_damage(victim, dam, RIS_BLUNT);
                else if (dt == (TYPE_HIT + 2) || dt == (TYPE_HIT + 11)
                         || dt == (TYPE_HIT + 10))
                        dam = ris_damage(victim, dam, RIS_PIERCE);
                else if (dt == (TYPE_HIT + 1) || dt == (TYPE_HIT + 3)
                         || dt == (TYPE_HIT + 4) || dt == (TYPE_HIT + 5))
                        dam = ris_damage(victim, dam, RIS_SLASH);

                if (dam == -1)
                {
                        if (dt >= 0 && dt < top_sn)
                        {
                                bool      found = FALSE;
                                SKILLTYPE *skill = skill_table[dt];

                                if (skill->imm_char
                                    && skill->imm_char[0] != '\0')
                                {
                                        act(AT_HIT, skill->imm_char, ch, NULL,
                                            victim, TO_CHAR);
                                        found = TRUE;
                                }
                                if (skill->imm_vict
                                    && skill->imm_vict[0] != '\0')
                                {
                                        act(AT_HITME, skill->imm_vict, ch,
                                            NULL, victim, TO_VICT);
                                        found = TRUE;
                                }
                                if (skill->imm_room
                                    && skill->imm_room[0] != '\0')
                                {
                                        act(AT_ACTION, skill->imm_room, ch,
                                            NULL, victim, TO_NOTVICT);
                                        found = TRUE;
                                }
                                if (found)
                                        return rNONE;
                        }
                        dam = 0;
                }
        }

        if (dam && npcvict && ch != victim)
        {
                if (!IS_SET(victim->act, ACT_SENTINEL))
                {
                        if (victim->hunting)
                        {
                                if (victim->hunting->who != ch)
                                {
                                        STRFREE(victim->hunting->name);
                                        victim->hunting->name =
                                                QUICKLINK(ch->name);
                                        victim->hunting->who = ch;
                                }
                        }
                        else
                                start_hunting(victim, ch);
                }

                if (victim->hating)
                {
                        if (victim->hating->who != ch)
                        {
                                STRFREE(victim->hating->name);
                                victim->hating->name = QUICKLINK(ch->name);
                                victim->hating->who = ch;
                        }
                }
                else
                        start_hating(victim, ch);
        }

        if (victim != ch)
        {
                /*
                 * Certain attacks are forbidden.
                 * Most other attacks are returned.
                 */
                if (is_safe(ch, victim))
                        return rNONE;


                if (victim->position > POS_STUNNED)
                {
                        if (!victim->fighting)
                                set_fighting(victim, ch);
                        if (victim->fighting)
                                victim->position = POS_FIGHTING;
                }

                if (victim->position > POS_STUNNED)
                {
                        if (!ch->fighting)
                                set_fighting(ch, victim);

                        /*
                         * If victim is charmed, ch might attack victim's master.
                         */
                        if (IS_NPC(ch)
                            && npcvict
                            && IS_AFFECTED(victim, AFF_CHARM)
                            && victim->master
                            && victim->master->in_room == ch->in_room
                            && number_bits(3) == 0)
                        {
                                stop_fighting(ch, FALSE);
                                retcode =
                                        multi_hit(ch, victim->master,
                                                  TYPE_UNDEFINED);
                                return retcode;
                        }
                }


                /*
                 * More charm stuff.
                 */
                if (victim->master == ch)
                        stop_follower(victim);


                /*
                 * Inviso attacks ... not.
                 */
                if (IS_AFFECTED(ch, AFF_INVISIBLE)
                    && str_cmp(ch->race->name(), "defel"))
                {
                        affect_strip(ch, gsn_invis);
                        affect_strip(ch, gsn_mass_invis);
                        REMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
                        act(AT_MAGIC, "$n fades into existence.", ch, NULL,
                            NULL, TO_ROOM);
                }

                if (IS_AFFECTED(ch, AFF_SECRETIVE))
                {
                        affect_strip(ch, gsn_slight);
                        REMOVE_BIT(ch->affected_by, AFF_SECRETIVE);
                        REMOVE_BIT(ch->act, PLR_SECRETIVE);
                        act(AT_MAGIC, "$n's action become noticable!", ch,
                            NULL, NULL, TO_ROOM);
                }

                /*
                 * Take away Hide 
                 */
                if (IS_AFFECTED(ch, AFF_HIDE))
                {
                        affect_strip(ch, gsn_hide);
                        REMOVE_BIT(ch->affected_by, AFF_HIDE);
                        act(AT_MAGIC, "$n's appears out of no where!", ch,
                            NULL, NULL, TO_ROOM);
                }

                /*
                 * Damage modifiers.
                 */
                if (IS_AFFECTED(victim, AFF_SANCTUARY))
                        dam /= 2;

                if (IS_AFFECTED(victim, AFF_PROTECT) && IS_EVIL(ch))
                        dam -= (int) (dam / 4);

                if (dam < 0)
                        dam = 0;

                /*
                 * Check for disarm, trip, parry, and dodge.
                 */
                if (dt >= TYPE_HIT)
                {
                        if (IS_NPC(ch)
                            && IS_SET(ch->attacks, ATCK_TRIP)
                            && number_percent() <
                            ch->skill_level[COMBAT_ABILITY])
                                trip(ch, victim);

                        if (check_parry(ch, victim))
                                return rNONE;
                        if (check_dodge(ch, victim))
                                return rNONE;
                }



                /*
                 * Check control panel settings and modify damage
                 */
                if (IS_NPC(ch))
                {
                        if (npcvict)
                                dampmod = sysdata.dam_mob_vs_mob;
                        else
                                dampmod = sysdata.dam_mob_vs_plr;
                }
                else
                {
                        if (npcvict)
                                dampmod = sysdata.dam_plr_vs_mob;
                        else
                                dampmod = sysdata.dam_plr_vs_plr;
                }
                if (dampmod > 0)
                        dam = (dam * dampmod) / 100;

                dam_message(ch, victim, dam, dt);
        }


        /*
         * Code to handle equipment getting damaged, and also support  -Thoric
         * bonuses/penalties for having or not having equipment where hit
         */
        if (dam > 10 && dt != TYPE_UNDEFINED &&
            !xIS_SET(victim->in_room->room_flags, ROOM_ARENA))
        {
                /*
                 * get a random body eq part 
                 */
                dameq = number_range(WEAR_LIGHT, WEAR_EYES);
                damobj = get_eq_char(victim, dameq);
                if (damobj)
                {
                        if (dam > get_obj_resistance(damobj))
                        {
                                set_cur_obj(damobj);
                                damage_obj(damobj);
                        }
                        dam -= 5;   /* add a bonus for having something to block the blow */
                }
                else
                        dam += 5;   /* add penalty for bare skin! */
        }

        /*
         * Hurt the victim.
         * Inform the victim of his new state.
         */

        victim->hit -= dam;

        /*
         * Get experience based on % of damage done         -Thoric
         */
        if (dam && ch != victim
            && !IS_NPC(ch) && ch->fighting && ch->fighting->xp)
        {
                xp_gain =
                        (int) (xp_compute(ch, victim) * 0.1 * dam) /
                        victim->max_hit;
                gain_exp(ch, xp_gain, COMBAT_ABILITY);
        }

        if (xIS_SET(ch->in_room->room_flags, ROOM_ARENA) && victim->hit < 1
            && !IS_NPC(victim))
        {
                stop_fighting(ch, TRUE);
                stop_fighting(victim, TRUE);
                win_fight(ch, victim);
                return rNONE;
        }

        if (!IS_NPC(victim)
            && victim->top_level >= LEVEL_IMMORTAL && victim->hit < 1)
                victim->hit = 1;

        /*
         * Make sure newbies dont die 
         */

        if (!IS_NPC(victim) && NOT_AUTHED(victim) && victim->hit < 1)
                victim->hit = 1;

        if (dam > 0 && dt > TYPE_HIT
            && !IS_AFFECTED(victim, AFF_POISON)
            && is_wielding_poisoned(ch)
            && !IS_SET(victim->immune, RIS_POISON)
            && !saves_poison_death(ch->skill_level[COMBAT_ABILITY], victim))
        {
                AFFECT_DATA af;

                af.type = gsn_poison;
                af.duration = 20;
                af.location = APPLY_STR;
                af.modifier = -2;
                af.bitvector = AFF_POISON;
                affect_join(victim, &af);
                victim->mental_state =
                        URANGE(20, victim->mental_state + 2, 100);
        }

        if (!npcvict
            && get_trust(victim) >= LEVEL_IMMORTAL
            && get_trust(ch) >= LEVEL_IMMORTAL && victim->hit < 1)
                victim->hit = 1;
        update_pos(victim);

        switch (victim->position)
        {
        case POS_MORTAL:
                act(AT_DYING,
                    "$n is mortally wounded, and will die soon, if not aided.",
                    victim, NULL, NULL, TO_ROOM);
                send_to_char
                        ("&RYou are mortally wounded, and will die soon, if not aided.",
                         victim);
                break;

        case POS_INCAP:
                act(AT_DYING,
                    "$n is incapacitated and will slowly die, if not aided.",
                    victim, NULL, NULL, TO_ROOM);
                send_to_char
                        ("&RYou are incapacitated and will slowly die, if not aided.",
                         victim);
                break;

        case POS_STUNNED:
                if (!IS_AFFECTED(victim, AFF_PARALYSIS))
                {
                        act(AT_ACTION,
                            "$n is stunned, but will probably recover.",
                            victim, NULL, NULL, TO_ROOM);
                        send_to_char
                                ("&RYou are stunned, but will probably recover.",
                                 victim);
                }
                break;

        case POS_DEAD:
                if (dt >= 0 && dt < top_sn)
                {
                        SKILLTYPE *skill = skill_table[dt];

                        if (skill->die_char && skill->die_char[0] != '\0')
                                act(AT_DEAD, skill->die_char, ch, NULL,
                                    victim, TO_CHAR);
                        if (skill->die_vict && skill->die_vict[0] != '\0')
                                act(AT_DEAD, skill->die_vict, ch, NULL,
                                    victim, TO_VICT);
                        if (skill->die_room && skill->die_room[0] != '\0')
                                act(AT_DEAD, skill->die_room, ch, NULL,
                                    victim, TO_NOTVICT);
                }
                if (IS_NPC(victim) && IS_SET(victim->act, ACT_NOKILL))
                        act(AT_YELLOW,
                            "$n flees for $s life ... barely escaping certain death!",
                            victim, 0, 0, TO_ROOM);
                else if (IS_NPC(victim) && IS_SET(victim->act, ACT_DROID))
                        act(AT_DEAD, "$n EXPLODES into many small pieces!",
                            victim, 0, 0, TO_ROOM);
                else if (str_cmp(victim->in_room->area->filename, "arena.are")
                         || !xIS_SET(victim->in_room->room_flags, ROOM_ARENA))
                        act(AT_DEAD, "$n is DEAD!", victim, 0, 0, TO_ROOM);
                send_to_char("&WYou have been KILLED!\n\r", victim);
                break;

        default:
                if (dam > victim->max_hit / 4)
                {
                        act(AT_HURT, "That really did HURT!", victim, 0, 0,
                            TO_CHAR);
                        if (number_bits(3) == 0)
                                worsen_mental_state(victim, 1);
                }
                if (victim->hit < victim->max_hit / 4)

                {
                        act(AT_DANGER,
                            "You wish that your wounds would stop BLEEDING so much!",
                            victim, 0, 0, TO_CHAR);
                        if (number_bits(2) == 0)
                                worsen_mental_state(victim, 1);
                }
                break;
        }

        /*
         * Sleep spells and extremely wounded folks.
         */
        if (!IS_AWAKE(victim)   /* lets make NPC's not slaughter PC's */
            && !IS_AFFECTED(victim, AFF_PARALYSIS))
        {
                if (victim->fighting
                    && victim->fighting->who->hunting
                    && victim->fighting->who->hunting->who == victim)
                        stop_hunting(victim->fighting->who);

                if (victim->fighting
                    && victim->fighting->who->hating
                    && victim->fighting->who->hating->who == victim)
                        stop_hating(victim->fighting->who);

                stop_fighting(victim, TRUE);
        }

        if (victim->hit <= 0 && !IS_NPC(victim))
        {
                OBJ_DATA *obj;
                OBJ_DATA *obj_next;
                int       cnt = 0;

                REMOVE_BIT(victim->act, PLR_ATTACKER);

                stop_fighting(victim, TRUE);

                if ((obj = get_eq_char(victim, WEAR_DUAL_WIELD)) != NULL)
                        unequip_char(victim, obj);
                if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
                        unequip_char(victim, obj);
                if ((obj = get_eq_char(victim, WEAR_HOLD)) != NULL)
                        unequip_char(victim, obj);
                if ((obj = get_eq_char(victim, WEAR_MISSILE_WIELD)) != NULL)
                        unequip_char(victim, obj);
                if ((obj = get_eq_char(victim, WEAR_LIGHT)) != NULL)
                        unequip_char(victim, obj);

                for (obj = victim->first_carrying; obj; obj = obj_next)
                {
                        obj_next = obj->next_content;

                        if (obj->wear_loc == WEAR_NONE)
                        {
                                if (obj->pIndexData->progtypes & DROP_PROG
                                    && obj->count > 1)
                                {
                                        ++cnt;
                                        separate_obj(obj);
                                        obj_from_char(obj);
                                        if (!obj_next)
                                                obj_next =
                                                        victim->
                                                        first_carrying;
                                }
                                else
                                {
                                        cnt += obj->count;
                                        obj_from_char(obj);
                                }
                                act(AT_ACTION, "$n drops $p.", victim, obj,
                                    NULL, TO_ROOM);
                                act(AT_ACTION, "You drop $p.", victim, obj,
                                    NULL, TO_CHAR);
                                obj = obj_to_room(obj, victim->in_room);
                        }
                }

                if (IS_NPC(ch) && !IS_NPC(victim))
                {
                        long      lose_exp;

                        lose_exp =
                                UMAX((victim->experience[COMBAT_ABILITY] -
                                      exp_level(victim->
                                                skill_level[COMBAT_ABILITY])),
                                     0);
                        ch_printf(victim, "You lose %ld experience.\n\r",
                                  lose_exp);
                        victim->experience[COMBAT_ABILITY] -= lose_exp;
                }

                add_timer(victim, TIMER_RECENTFIGHT, 100, NULL, 0);

        }

        /*
         * Payoff for killing things.
         */
        if (victim->position == POS_DEAD)
        {
                group_gain(ch, victim);

                if (!npcvict)
                {
                        if (!xIS_SET(victim->in_room->room_flags, ROOM_ARENA))
                        {
                                snprintf(log_buf, MSL,
                                         "%s killed by %s at %d",
                                         victim->name,
                                         (IS_NPC(ch) ? ch->short_descr : ch->
                                          name), victim->in_room->vnum);
                                log_string(log_buf);
                        }
                }
                else if (!IS_NPC(ch) && IS_NPC(victim)) /* keep track of mob vnum killed */
                {
                        add_kill(ch, victim);

                        /*
                         * Add to kill tracker for grouped chars, as well. -Halcyon
                         */
                        for (gch = ch->in_room->first_person; gch;
                             gch = gch->next_in_room)
                                if (is_same_group(gch, ch) && !IS_NPC(gch)
                                    && gch != ch)
                                        add_kill(gch, victim);
                }

                check_killer(ch, victim);

                if (!IS_NPC(victim) || !IS_SET(victim->act, ACT_NOKILL))
                        loot = legal_loot(ch, victim);
                else
                        loot = FALSE;

                raw_kill(ch, victim);
                victim = NULL;

                if (!IS_NPC(ch) && loot)
                {
                        /*
                         * Autogold by Scryn 8/12 
                         */
                        if (IS_SET(ch->act, PLR_AUTOGOLD))
                        {
                                init_gold = ch->gold;
                                do_get(ch, "credits corpse");
                                new_gold = ch->gold;
                                gold_diff = (new_gold - init_gold);
                                if (gold_diff > 0)
                                {
                                        snprintf(buf1, MSL, "%d", gold_diff);
                                        do_split(ch, buf1);
                                }
                        }
                        if (IS_SET(ch->act, PLR_AUTOLOOT))
                                do_get(ch, "all corpse");
                        else
                                do_look(ch, "in corpse");

                }

                if (IS_SET(sysdata.save_flags, SV_KILL))
                        save_char_obj(ch);
                return rVICT_DIED;
        }

        if (victim == ch)
                return rNONE;

        /*
         * Take care of link dead people.
         */
        if (!npcvict && !victim->desc && !victim->switched)
        {
                if (number_range(0, victim->wait) == 0)
                {
                        do_flee(victim, "");
                        do_flee(victim, "");
                        do_flee(victim, "");
                        do_flee(victim, "");
                        do_flee(victim, "");
                        do_hail(victim, "");
                        do_quit(victim, "");
                        return rNONE;
                }
        }

        /*
         * Wimp out?
         */
        if (npcvict && dam > 0)
        {
                if ((IS_SET(victim->act, ACT_WIMPY) && number_bits(1) == 0
                     && victim->hit < victim->max_hit / 2)
                    || (IS_AFFECTED(victim, AFF_CHARM) && victim->master
                        && victim->master->in_room != victim->in_room))
                {
                        start_fearing(victim, ch);
                        stop_hunting(victim);
                        do_flee(victim, "");
                }
        }

        if (!npcvict
            && victim->hit > 0
            && victim->hit <= victim->wimpy && victim->wait == 0)
                do_flee(victim, "");
        else if (!npcvict && IS_SET(victim->act, PLR_FLEE))
                do_flee(victim, "");

        tail_chain();
        return rNONE;
}

bool is_safe(CHAR_DATA * ch, CHAR_DATA * victim)
{
        if (!victim)
                return FALSE;

        /*
         * Thx Josh! 
         */
        if (who_fighting(ch) == ch)
                return FALSE;

        if (xIS_SET(victim->in_room->room_flags, ROOM_SAFE))
        {
                set_char_color(AT_MAGIC, ch);
                send_to_char("You'll have to do that elswhere.\n\r", ch);
                return TRUE;
        }

        if (get_trust(ch) > LEVEL_HERO)
                return FALSE;

        if (IS_NPC(ch) || IS_NPC(victim))
                return FALSE;


        return FALSE;

}

/* checks is_safe but without the output 
   cuts out imms and safe rooms as well 
   for info only */

bool is_safe_nm(CHAR_DATA * ch, CHAR_DATA * victim)
{
        ch = NULL;
        victim = NULL;
        return FALSE;
}


/*
 * just verify that a corpse looting is legal
 */
bool legal_loot(CHAR_DATA * ch, CHAR_DATA * victim)
{
        victim = NULL;
        /*
         * pc's can now loot .. why not .. death is pretty final 
         */
        if (!IS_NPC(ch))
                return TRUE;
        /*
         * non-charmed mobs can loot anything 
         */
        if (IS_NPC(ch) && !ch->master)
                return TRUE;

        return FALSE;
}

/*
see if an attack justifies a KILLER flag --- edited so that none do but can't 
murder a no pk person. --- edited again for planetary wanted flags -- well will be soon :p
 */

void check_killer(CHAR_DATA * ch, CHAR_DATA * victim)
{

        /*
         * Charm-o-rama.
         */
        if (IS_SET(ch->affected_by, AFF_CHARM))
        {
                if (!ch->master)
                {
                        char      buf[MAX_STRING_LENGTH];

                        snprintf(buf, MSL, "Check_killer: %s bad AFF_CHARM",
                                 IS_NPC(ch) ? ch->short_descr : ch->name);
                        bug(buf, 0);
                        affect_strip(ch, gsn_charm_person);
                        REMOVE_BIT(ch->affected_by, AFF_CHARM);
                        return;
                }

                /*
                 * stop_follower( ch ); 
                 */
                if (ch->master)
                        check_killer(ch->master, victim);
        }

        if (IS_NPC(victim))
        {
                if (!IS_NPC(ch))
                {
                        if (IS_SET(victim->act, ACT_CITIZEN))
                        {
                                if (victim->in_room && victim->in_room->area
                                    && victim->in_room->area->planet)
                                {
                                        PLANET_DATA *planet =
                                                victim->in_room->area->planet;
                                        add_wanted(ch, planet);
                                }
                        }
                        if (ch->pcdata->clan)
                                ch->pcdata->clan->mkills++;
                        ch->pcdata->mkills++;
                        ch->in_room->area->mkills++;
                }
                return;
        }

        if (!IS_NPC(ch) && !IS_NPC(victim))
        {
                if (ch->pcdata->clan)
                        ch->pcdata->clan->pkills++;
                ch->pcdata->pkills++;
                update_pos(victim);
                if (victim->pcdata->clan)
                        victim->pcdata->clan->pdeaths++;
        }


        if (IS_NPC(ch))
                if (!IS_NPC(victim))
                        victim->in_room->area->mdeaths++;

        return;
}



/*
 * Set position of a victim.
 */
void update_pos(CHAR_DATA * victim)
{
        if (!victim)
        {
                bug("update_pos: null victim", 0);
                return;
        }

        if (victim->hit > 0)
        {
                if (victim->position <= POS_STUNNED
                    && !IS_AFFECTED(victim, AFF_PARALYSIS))
                        victim->position = POS_STANDING;
                if (IS_AFFECTED(victim, AFF_PARALYSIS))
                {
                        victim->position = POS_STUNNED;
                        stop_hating(victim);
                        stop_hunting(victim);
                }
                return;
        }

        if (IS_NPC(victim) || victim->hit <= -500)
        {
                if (victim->mount)
                {
                        act(AT_ACTION, "$n falls from $N.",
                            victim, NULL, victim->mount, TO_ROOM);
                        REMOVE_BIT(victim->mount->act, ACT_MOUNTED);
                        victim->mount = NULL;
                }
                victim->position = POS_DEAD;
                return;
        }

        if (victim->hit <= -400)
                victim->position = POS_MORTAL;
        else if (victim->hit <= -200)
                victim->position = POS_INCAP;
        else
                victim->position = POS_STUNNED;

        if (victim->position > POS_STUNNED
            && IS_AFFECTED(victim, AFF_PARALYSIS))
                victim->position = POS_STUNNED;

        if (victim->mount)
        {
                act(AT_ACTION, "$n falls unconscious from $N.",
                    victim, NULL, victim->mount, TO_ROOM);
                REMOVE_BIT(victim->mount->act, ACT_MOUNTED);
                victim->mount = NULL;
        }
        return;
}


/*
 * Start fights.
 */
void set_fighting(CHAR_DATA * ch, CHAR_DATA * victim)
{
        FIGHT_DATA *fight;

        if (ch->fighting)
        {
                char      buf[MAX_STRING_LENGTH];

                snprintf(buf, MSL,
                         "Set_fighting: %s -> %s (already fighting %s)",
                         ch->name, victim->name, ch->fighting->who->name);
                bug(buf, 0);
                return;
        }

        if (IS_AFFECTED(ch, AFF_SLEEP))
                affect_strip(ch, gsn_sleep);

        /*
         * Limit attackers -Thoric 
         */
        if (victim->num_fighting > max_fight(victim))
        {
                send_to_char
                        ("There are too many people fighting for you to join in.\n\r",
                         ch);
                return;
        }

        CREATE(fight, FIGHT_DATA, 1);
        fight->who = victim;
        fight->xp = (int) xp_compute(ch, victim);
        fight->align = align_compute(ch, victim);
        if (!IS_NPC(ch) && IS_NPC(victim))
                fight->timeskilled = times_killed(ch, victim);
        ch->num_fighting = 1;
        ch->fighting = fight;
        ch->position = POS_FIGHTING;
        victim->num_fighting++;
        if (victim->switched && IS_AFFECTED(victim->switched, AFF_POSSESS))
        {
                send_to_char("You are disturbed!\n\r", victim->switched);
                do_return(victim->switched, "");
        }
        return;
}


CHAR_DATA *who_fighting(CHAR_DATA * ch)
{
        if (!ch)
        {
                bug("who_fighting: null ch", 0);
                return NULL;
        }
        if (!ch->fighting)
                return NULL;
        return ch->fighting->who;
}

void free_fight(CHAR_DATA * ch)
{
        if (!ch)
        {
                bug("Free_fight: null ch!", 0);
                return;
        }
        if (ch->fighting)
        {
                if (!char_died(ch->fighting->who))
                        --ch->fighting->who->num_fighting;
                DISPOSE(ch->fighting);
        }
        ch->fighting = NULL;
        if (ch->mount)
                ch->position = POS_MOUNTED;
        else
                ch->position = POS_STANDING;
        /*
         * Berserk wears off after combat. -- Altrag 
         */
        if (IS_AFFECTED(ch, AFF_BERSERK))
        {
                affect_strip(ch, gsn_berserk);
                set_char_color(AT_WEAROFF, ch);
                send_to_char(skill_table[gsn_berserk]->msg_off, ch);
                send_to_char("\n\r", ch);
        }
        return;
}


/*
 * Stop fights.
 */
void stop_fighting(CHAR_DATA * ch, bool fBoth)
{
        CHAR_DATA *fch;

        free_fight(ch);
        update_pos(ch);

        if (!IS_NPC(ch) && IS_SET(ch->pcdata->flags, PCFLAG_AUTODRAW))
        {
                OBJ_DATA *holster1 = get_eq_char(ch, WEAR_HOLSTER_L),
                         *holster2 = get_eq_char(ch, WEAR_HOLSTER_R);
                if ((holster1 && holster1->item_type == ITEM_HOLSTER && !holster1->first_content) ||
                                (holster2 && holster2->item_type == ITEM_HOLSTER && !holster2->first_content))
                        do_holster(ch, "");
        }

        if (!fBoth) /* major short cut here by Thoric */
                return;

        for (fch = first_char; fch; fch = fch->next)
        {
                if (who_fighting(fch) == ch)
                {
                        /* cleaner than copying code */
                        stop_fighting(fch, FALSE);
                        /* free_fight(fch);
                        update_pos(fch); */
                }
        }
        return;
}



void death_cry(CHAR_DATA * ch)
{
        ch = NULL;
        return;
}



void raw_kill(CHAR_DATA * ch, CHAR_DATA * victim)
{

        CHAR_DATA *victmp;

        char      buf[MAX_STRING_LENGTH];
        char      buf2[MAX_STRING_LENGTH];
        char      arg[MAX_STRING_LENGTH];
        OBJ_DATA *obj, *obj_next;
        SHIP_DATA *ship;

        if (!victim)
        {
                bug("raw_kill: null victim!", 0);
                return;
        }

        mudstrlcpy(arg, victim->name, MSL);

        stop_fighting(victim, TRUE);

        if (!str_cmp(victim->in_room->area->filename, "arena.are")
            || xIS_SET(victim->in_room->room_flags, ROOM_ARENA))
        {
                victim->hit = 1;
                update_pos(victim);
                return;
        }

        if (ch && !IS_NPC(ch) && !IS_NPC(victim))
                claim_disintigration(ch, victim);

/* Take care of polymorphed chars */
        if (IS_NPC(victim) && IS_SET(victim->act, ACT_POLYMORPHED))
        {
                char_from_room(victim->desc->original);
                char_to_room(victim->desc->original, victim->in_room);
                victmp = victim->desc->original;
                do_revert(victim, "");
                raw_kill(ch, victmp);
                return;
        }

        if (victim->in_room && IS_NPC(victim)
            && IS_SET(victim->act, ACT_CITIZEN) && victim->in_room->area
            && victim->in_room->area->planet)
        {
                victim->in_room->area->planet->population--;
                victim->in_room->area->planet->population =
                        UMAX(victim->in_room->area->planet->population, 0);
                victim->in_room->area->planet->pop_support -=
                        (1 +
                         1 / (victim->in_room->area->planet->population + 1));
                if (victim->in_room->area->planet->pop_support < -100)
                        victim->in_room->area->planet->pop_support = -100;
        }

        if (!IS_NPC(victim) || !IS_SET(victim->act, ACT_NOKILL))
                mprog_death_trigger(ch, victim);
        if (char_died(victim))
                return;

        if (!IS_NPC(victim) || !IS_SET(victim->act, ACT_NOKILL))
                rprog_death_trigger(ch, victim);
        if (char_died(victim))
                return;

        if (!IS_NPC(victim))
        {
                snprintf(buf, MSL, "%s%c/%s", PLAYER_DIR, tolower(arg[0]),
                         capitalize(arg));
                snprintf(buf2, MSL, "%s%c/%s", BACKUP_DIR, tolower(arg[0]),
                         capitalize(arg));

                rename(buf, buf2);
        }

        if (!IS_NPC(victim)
            || (!IS_SET(victim->act, ACT_NOKILL)
                && !IS_SET(victim->act, ACT_NOCORPSE)))
                make_corpse(victim, ch);
        else
        {
                for (obj = victim->last_carrying; obj; obj = obj_next)
                {
                        obj_next = obj->prev_content;
                        obj_from_char(obj);
                        extract_obj(obj);
                }
        }


/*    make_blood( victim ); */

        if (IS_NPC(victim))
        {
                victim->pIndexData->killed++;
                extract_char(victim, TRUE);
                victim = NULL;
                return;
        }

        set_char_color(AT_DIEMSG, victim);
        do_help(victim, "_DIEMSG_");


/* swreality chnages begin here */

/* Check if they have an ongoing auction */
        if (auction->item && (auction->seller == victim))
        {
                talk_auction("Auction has been halted.");
                obj_to_char(auction->item, auction->seller);
                auction->item = NULL;

                if (auction->buyer != NULL
                    && auction->buyer != auction->seller)
                {
                        auction->buyer->gold += auction->bet;
                        send_to_char("&YYour money has been returned.\n\r",
                                     auction->buyer);
                }
        }
        for (ship = first_ship; ship; ship = ship->next)
        {
                if (!str_cmp(ship->owner, victim->name))
                {
                        STRFREE(ship->owner);
                        ship->owner = STRALLOC("");
                        STRFREE(ship->pilot);
                        ship->pilot = STRALLOC("");
                        STRFREE(ship->copilot);
                        ship->copilot = STRALLOC("");

                        save_ship(ship);
                }

        }


        if (victim->plr_home)
        {
                ROOM_INDEX_DATA *room = victim->plr_home;

                STRFREE(room->name);
                room->name = STRALLOC("An Empty Apartment");

                xREMOVE_BIT(room->room_flags, ROOM_PLR_HOME);
                xSET_BIT(room->room_flags, ROOM_EMPTY_HOME);

                fold_area(room->area, room->area->filename, FALSE, TRUE);
        }

        if (victim->pcdata && victim->pcdata->clan)
        {
                if (!str_cmp(victim->name, victim->pcdata->clan->leader))
                {
                        STRFREE(victim->pcdata->clan->leader);
                        if (victim->pcdata->clan->number1)
                        {
                                victim->pcdata->clan->leader =
                                        STRALLOC(victim->pcdata->clan->
                                                 number1);
                                STRFREE(victim->pcdata->clan->number1);
                                victim->pcdata->clan->number1 = STRALLOC("");
                        }
                        else if (victim->pcdata->clan->number2)
                        {
                                victim->pcdata->clan->leader =
                                        STRALLOC(victim->pcdata->clan->
                                                 number2);
                                STRFREE(victim->pcdata->clan->number2);
                                victim->pcdata->clan->number2 = STRALLOC("");
                        }
                        else
                                victim->pcdata->clan->leader = STRALLOC("");
                }

                if (!str_cmp(victim->name, victim->pcdata->clan->number1))
                {
                        STRFREE(victim->pcdata->clan->number1);
                        if (victim->pcdata->clan->number2)
                        {
                                victim->pcdata->clan->number1 =
                                        STRALLOC(victim->pcdata->clan->
                                                 number2);
                                STRFREE(victim->pcdata->clan->number2);
                                victim->pcdata->clan->number2 = STRALLOC("");
                        }
                        else
                                victim->pcdata->clan->number1 = STRALLOC("");
                }

                if (!str_cmp(victim->name, victim->pcdata->clan->number2))
                {
                        STRFREE(victim->pcdata->clan->number2);
                        victim->pcdata->clan->number1 = STRALLOC("");
                }

                victim->pcdata->clan->members--;
                if (victim->pcdata->clan->roster)
                        if (hasname
                            (victim->pcdata->clan->roster, victim->name))
                                removename(&victim->pcdata->clan->roster,
                                           victim->name);
        }

        snprintf(buf, MSL, "%s%c/%s.clone", PLAYER_DIR, tolower(arg[0]),
                 capitalize(arg));
        snprintf(buf2, MSL, "%s%c/%s", PLAYER_DIR, tolower(arg[0]),
                 capitalize(arg));
#ifdef ACCOUNT
        if (rename(buf, buf2) != 0)
        {
                if (victim && victim->pcdata && victim->pcdata->account)
                {
                        del_from_account(victim->pcdata->account, victim);
                }
        }
#else
        rename(buf, buf2);
#endif
        if (!victim)
        {
                DESCRIPTOR_DATA *d;

                /*
                 * Make sure they aren't halfway logged in. 
                 */
                for (d = first_descriptor; d; d = d->next)
                        if ((victim = d->character) && !IS_NPC(victim))
                                break;
                if (d)
                        close_socket(d, TRUE);
        }
        else
        {
                int       x, y;

                quitting_char = victim;
                /*
                 * save_char_obj( victim );
                 */
                saving_char = NULL;
                extract_char(victim, TRUE);
                for (x = 0; x < MAX_WEAR; x++)
                        for (y = 0; y < MAX_LAYERS; y++)
                                save_equipment[x][y] = NULL;
        }
        return;


/* original player kill started here
    
    extract_char( victim, FALSE );
    if ( !victim )
    {
      bug( "oops! raw_kill: extract_char destroyed pc char", 0 );
      return;
    }
    while ( victim->first_affect )
	affect_remove( victim, victim->first_affect );
    victim->affected_by	= race_table[victim->race].affected;
    victim->resistant   = 0;
    victim->susceptible = 0;
    victim->immune      = 0;
    victim->carry_weight= 0;
    victim->armor	= 100;
    victim->mod_str	= 0;
    victim->mod_dex	= 0;
    victim->mod_wis	= 0;
    victim->mod_int	= 0;
    victim->mod_con	= 0;
    victim->mod_cha	= 0;
    victim->mod_lck   	= 0;
    victim->damroll	= 0;
    victim->hitroll	= 0;
    victim->mental_state = -10;
    victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
    victim->saving_spell_staff = 0;
    victim->position	= POS_RESTING;
    victim->hit		= UMAX( 1, victim->hit  );
    victim->endurance	= UMAX( 1, victim->endurance );
    victim->endurance	= UMAX( 1, victim->endurance );
    
    victim->pcdata->condition[COND_FULL]   = 12;
    victim->pcdata->condition[COND_THIRST] = 12;
    
    if ( IS_SET( sysdata.save_flags, SV_DEATH ) )
	save_char_obj( victim );
    return;

*/

}



void group_gain(CHAR_DATA * ch, CHAR_DATA * victim)
{
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *gch;
        CHAR_DATA *lch;
        int       xp;
        int       members;

        /*
         * Monsters don't get kill xp's or alignment changes.
         * Dying of mortal wounds or poison doesn't give xp to anyone!
         */
        if (IS_NPC(ch) || victim == ch)
                return;

        members = 0;

        for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
        {
                if (is_same_group(gch, ch))
                        members++;
        }

        if (members == 0)
        {
                bug("Group_gain: members.", members);
                members = 1;
        }

        lch = ch->leader ? ch->leader : ch;

        if (IS_SET(ch->act, PLR_QUESTOR) && IS_NPC(victim))
        {
                if (ch->questmob == victim->pIndexData->vnum)
                {
                        send_to_char
                                ("You have almost completed your QUEST!\n\r",
                                 ch);
                        send_to_char
                                ("Return to the questmaster before your time runs out!\n\r",
                                 ch);
                        ch->questmob = -1;
                }
        }


        for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
        {
                OBJ_DATA *obj;
                OBJ_DATA *obj_next;

                if (!is_same_group(gch, ch))
                        continue;

                xp = (int) (xp_compute(gch, victim) / members);

                gch->alignment = align_compute(gch, victim);

                if (!IS_NPC(gch) && IS_NPC(victim) && gch->pcdata
                    && gch->pcdata->clan
                    && !str_cmp(gch->pcdata->clan->name, victim->mob_clan))
                {
                        xp = 0;
                        snprintf(buf, MSL,
                                 "You receive no experience for killing your organizations resources.\n\r");
                        send_to_char(buf, gch);
                }
                else
                {
                        snprintf(buf, MSL,
                                 "You receive %d combat experience.\n\r", xp);
                        send_to_char(buf, gch);
                }

                gain_exp(gch, xp, COMBAT_ABILITY);

                if (lch == gch && members > 1)
                {
                        xp = URANGE(members, xp * members,
                                    (exp_level
                                     (gch->skill_level[LEADERSHIP_ABILITY] +
                                      1) -
                                     exp_level(gch->
                                               skill_level
                                               [LEADERSHIP_ABILITY]) / 10));
                        snprintf(buf, MSL,
                                 "You get %d leadership experience for leading your group to victory.\n\r",
                                 xp);
                        send_to_char(buf, gch);
                        gain_exp(gch, xp, LEADERSHIP_ABILITY);
                }


                for (obj = ch->first_carrying; obj; obj = obj_next)
                {
                        obj_next = obj->next_content;
                        if (obj->wear_loc == WEAR_NONE)
                                continue;

                        if ((IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch))
                            || (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)
                                && IS_GOOD(ch))
                            || (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL)
                                && IS_NEUTRAL(ch)))
                        {
                                act(AT_MAGIC, "You are zapped by $p.", ch,
                                    obj, NULL, TO_CHAR);
                                act(AT_MAGIC, "$n is zapped by $p.", ch, obj,
                                    NULL, TO_ROOM);

                                obj_from_char(obj);
                                obj = obj_to_room(obj, ch->in_room);
                                oprog_zap_trigger(ch, obj); /* mudprogs */
                                if (char_died(ch))
                                        return;
                        }
                }
        }

        return;
}


int align_compute(CHAR_DATA * gch, CHAR_DATA * victim)
{

/* never cared much for this system

    int align, newalign;

    align = gch->alignment - victim->alignment;

    if ( align >  500 )
	newalign  = UMIN( gch->alignment + (align-500)/4,  1000 );
    else
    if ( align < -500 )
	newalign  = UMAX( gch->alignment + (align+500)/4, -1000 );
    else
	newalign  = gch->alignment - (int) (gch->alignment / 4);
    
    return newalign;

make it simple instead */

        return URANGE(-1000,
                      (int) (gch->alignment - victim->alignment / 5), 1000);

}


/*
 * Calculate how much XP gch should gain for killing victim
 * Lots of redesigning for new exp system by Thoric
 */
int xp_compute(CHAR_DATA * gch, CHAR_DATA * victim)
{
        int       align;
        int       xp;

        xp = (get_exp_worth(victim)
              * URANGE(1,
                       (victim->skill_level[COMBAT_ABILITY] -
                        gch->skill_level[COMBAT_ABILITY]) + 10, 20)) / 10;
        align = gch->alignment - victim->alignment;

        /*
         * bonus for attacking opposite alignment 
         */
        if (align > 990 || align < -990)
                xp = (xp * 5) >> 2;
        else
                /*
                 * penalty for good attacking same alignment 
                 */
        if (gch->alignment > 300 && align < 250)
                xp = (xp * 3) >> 2;

        xp = number_range((xp * 3) >> 2, (xp * 5) >> 2);

        /*
         * reduce exp for killing the same mob repeatedly       -Thoric 
         */
        if (!IS_NPC(gch) && IS_NPC(victim))
        {
                int       times = times_killed(gch, victim);

                if (times >= 3)
                        xp = 0;
                else if (times)
                        xp = (xp * (3 - times)) / 3;
        }

        /*
         * new xp cap for swreality 
         */

        return URANGE(1, xp,
                      (exp_level(gch->skill_level[COMBAT_ABILITY] + 1) -
                       exp_level(gch->skill_level[COMBAT_ABILITY])) / 5);
}


/*
 * Revamped by Thoric to be more realistic
 */
void dam_message(CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt)
{
        char      buf1[256], buf2[256], buf3[256];
        const char *vs;
        const char *vp;
        const char *attack;
        char      punct;
        sh_int    dampc;
        struct skill_type *skill = NULL;
        bool      gcflag = FALSE;
        bool      gvflag = FALSE;

        if (!dam)
                dampc = 0;
        else
                dampc = ((dam * 1000) / victim->max_hit) +
                        (50 - ((victim->hit * 50) / victim->max_hit));

        /*
         * 10 * percent                  
         */
        if (dam == 0)
        {
                vs = "&Rmiss&R&W";
                vp = "&Rmisses&R&W";
        }
        else if (dampc <= 5)
        {
                vs = "&Rbarely scratch&R&W";
                vp = "&Rbarely scratches&R&W";
        }
        else if (dampc <= 10)
        {
                vs = "&Rscratch&R&W";
                vp = "&Rscratches&R&W";
        }
        else if (dampc <= 20)
        {
                vs = "&Rnick&R&W";
                vp = "&Rnicks&R&W";
        }
        else if (dampc <= 30)
        {
                vs = "&Rgraze&R&W";
                vp = "&Rgrazes&R&W";
        }
        else if (dampc <= 40)
        {
                vs = "&Rbruise&R&W";
                vp = "&Rbruises&R&W";
        }
        else if (dampc <= 50)
        {
                vs = "&Rhit&R&W";
                vp = "&Rhits&R&W";
        }
        else if (dampc <= 60)
        {
                vs = "&Rinjure&R&W";
                vp = "&Rinjures&R&W";
        }
        else if (dampc <= 75)
        {
                vs = "&Rthrash&R&W";
                vp = "&Rthrashes&R&W";
        }
        else if (dampc <= 80)
        {
                vs = "&Rwound&R&W";
                vp = "&Rwounds&R&W";
        }
        else if (dampc <= 90)
        {
                vs = "&Rmaul&R&W";
                vp = "&Rmauls&R&W";
        }
        else if (dampc <= 125)
        {
                vs = "&Rdecimate&R&W";
                vp = "&Rdecimates&R&W";
        }
        else if (dampc <= 150)
        {
                vs = "&Rdevastate&R&W";
                vp = "&Rdevastates&R&W";
        }
        else if (dampc <= 200)
        {
                vs = "&Rmaim&R&W";
                vp = "&Rmaims&R&W";
        }
        else if (dampc <= 300)
        {
                vs = "&Rmutilate&R&W";
                vp = "&Rmutilates&R&W";
        }
        else if (dampc <= 400)
        {
                vs = "&Rdisembowels&R&W";
                vp = "&Rdisembowels&R&W";
        }
        else if (dampc <= 500)
        {
                vs = "&Rmassacre&R&W";
                vp = "&Rmassacres&R&W";
        }
        else if (dampc <= 600)
        {
                vs = "&Rpulverizes&R&W";
                vp = "&Rpulverizes&R&W";
        }
        else if (dampc <= 750)
        {
                vs = "&Rerviscerate&R&W";
                vp = "&Reviscerate&R&W";
        }
        else if (dampc <= 990)
        {
                vs = "&Robliterte&R&W";
                vp = "&Robliterates&R&W";
        }
        else
        {
                vs = "&Rannihilate&R&W";
                vp = "&Rannihilates&R&W";
        }
        punct = (dampc <= 30) ? '.' : '!';

        if (dam == 0 && (!IS_NPC(ch) &&
                         (IS_SET(ch->pcdata->flags, PCFLAG_GAG))))
                gcflag = TRUE;

        if (dam == 0 && (!IS_NPC(victim) &&
                         (IS_SET(victim->pcdata->flags, PCFLAG_GAG))))
                gvflag = TRUE;

        if (dt >= 0 && dt < top_sn)
                skill = skill_table[dt];

        if (dt == (TYPE_HIT + WEAPON_BLASTER))
        {
                char      sound[MAX_STRING_LENGTH];
                int       vol = number_range(20, 80);

                /*
                 * FIXME 
                 */
                snprintf(sound, MSL, "!!SOUND(blaster V=%d)", vol);
                sound_to_room(ch->in_room, "blaster");
        }

        if (dt == TYPE_HIT || dam == 0)
        {
                snprintf(buf1, MSL, "&c&C$n %s &c&C$N%c", vp, punct);
                snprintf(buf2, MSL, "&c&CYou %s &c&C$N%c", vs, punct);
                snprintf(buf3, MSL, "&c&C$n %s &c&Cyou%c", vp, punct);
        }
        else if (dt > TYPE_HIT && is_wielding_poisoned(ch))
        {
                if (dt <
                    TYPE_HIT +
                    (int) sizeof(attack_table) /
                    (int) sizeof(attack_table[0]))
                        attack = attack_table[dt - TYPE_HIT];
                else
                {
                        bug("Dam_message: bad dt %d.", dt);
                        dt = TYPE_HIT;
                        attack = attack_table[0];
                }

                snprintf(buf1, MSL, "$n's poisoned %s %s $N%c", attack, vp,
                         punct);
                snprintf(buf2, MSL, "Your poisoned %s %s $N%c", attack, vp,
                         punct);
                snprintf(buf3, MSL, "$n's poisoned %s %s you%c", attack, vp,
                         punct);
        }
        else
        {
                if (skill)
                {
                        attack = skill->noun_damage;
                        if (dam == 0)
                        {
                                bool      found = FALSE;

                                if (skill->miss_char
                                    && skill->miss_char[0] != '\0')
                                {
                                        act(AT_HIT, skill->miss_char, ch,
                                            NULL, victim, TO_CHAR);
                                        found = TRUE;
                                }
                                if (skill->miss_vict
                                    && skill->miss_vict[0] != '\0')
                                {
                                        act(AT_HITME, skill->miss_vict, ch,
                                            NULL, victim, TO_VICT);
                                        found = TRUE;
                                }
                                if (skill->miss_room
                                    && skill->miss_room[0] != '\0')
                                {
                                        act(AT_ACTION, skill->miss_room, ch,
                                            NULL, victim, TO_NOTVICT);
                                        found = TRUE;
                                }
                                if (found)  /* miss message already sent */
                                        return;
                        }
                        else
                        {
                                if (skill->hit_char
                                    && skill->hit_char[0] != '\0')
                                        act(AT_HIT, skill->hit_char, ch, NULL,
                                            victim, TO_CHAR);
                                if (skill->hit_vict
                                    && skill->hit_vict[0] != '\0')
                                        act(AT_HITME, skill->hit_vict, ch,
                                            NULL, victim, TO_VICT);
                                if (skill->hit_room
                                    && skill->hit_room[0] != '\0')
                                        act(AT_ACTION, skill->hit_room, ch,
                                            NULL, victim, TO_NOTVICT);
                        }
                }
                else if (dt >= TYPE_HIT
                         && dt <
                         TYPE_HIT +
                         (int) sizeof(attack_table) /
                         (int) sizeof(attack_table[0]))
                        attack = attack_table[dt - TYPE_HIT];
                else
                {
                        bug("Dam_message: bad dt %d.", dt);
                        dt = TYPE_HIT;
                        attack = attack_table[0];
                }

                snprintf(buf1, MSL, "&c&C$n's %s %s &c&C$N%c&R&W", attack, vp,
                         punct);
                snprintf(buf2, MSL, "&c&CYour %s %s &c&C$N%c&R&W", attack, vp,
                         punct);
                snprintf(buf3, MSL, "&c&C$n's %s %s &c&Cyou%c&R&W", attack,
                         vp, punct);
        }

        if (ch->skill_level[COMBAT_ABILITY] >= 100)
        {
                char      tmp_buf[MSL];

                mudstrlcpy(tmp_buf, buf2, MSL);
                snprintf(buf2, MSL,
                         "%s &WYou do &R%d &R&Wpoints of damage.&R&W",
                         tmp_buf, dam);
        }
        act(AT_ACTION, buf1, ch, NULL, victim, TO_NOTVICT);
        if (!gcflag)
                act(AT_HIT, buf2, ch, NULL, victim, TO_CHAR);
        if (!gvflag)
                act(AT_HITME, buf3, ch, NULL, victim, TO_VICT);

        return;
}


CMDF do_kill(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Kill whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (!IS_NPC(victim))
        {
                send_to_char("You must MURDER a player.\n\r", ch);
                return;
        }

        /*
         *
         else
         {
         if ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
         {
         send_to_char( "You must MURDER a charmed creature.\n\r", ch );
         return;
         }
         }
         *
         */

        if (victim == ch)
        {
                send_to_char("You hit yourself.  Ouch!\n\r", ch);
                multi_hit(ch, ch, TYPE_UNDEFINED);
                return;
        }

        if (is_safe(ch, victim))
                return;

        if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
        {
                act(AT_PLAIN, "$N is your beloved master.", ch, NULL, victim,
                    TO_CHAR);
                return;
        }

        if (ch->position == POS_FIGHTING)
        {
                send_to_char("You do the best you can!\n\r", ch);
                return;
        }

        if (IS_SET(victim->act, ACT_CITIZEN))
                ch->alignment -= 10;

        WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
        multi_hit(ch, victim, TYPE_UNDEFINED);
        return;
}



CMDF do_murde(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        send_to_char("If you want to MURDER, spell it out.\n\r", ch);
        return;
}



CMDF do_murder(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Murder whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("Suicide is a mortal sin.\n\r", ch);
                return;
        }

        if (is_safe(ch, victim))
                return;

        if (IS_AFFECTED(ch, AFF_CHARM))
        {
                if (ch->master == victim)
                {
                        act(AT_PLAIN, "$N is your beloved master.", ch, NULL,
                            victim, TO_CHAR);
                        return;
                }
        }

        if (ch->position == POS_FIGHTING)
        {
                send_to_char("You do the best you can!\n\r", ch);
                return;
        }

        if (!IS_NPC(victim) && IS_SET(ch->act, PLR_NICE))
        {
                send_to_char("You feel too nice to do that!\n\r", ch);
                return;
        }

        ch->alignment -= 10;

        WAIT_STATE(ch, 1 * PULSE_VIOLENCE);
        multi_hit(ch, victim, TYPE_UNDEFINED);
        return;
}


CMDF do_flee(CHAR_DATA * ch, char *argument)
{
        ROOM_INDEX_DATA *was_in;
        ROOM_INDEX_DATA *now_in;
        char      buf[MAX_STRING_LENGTH];
        int       attempt;
        sh_int    door;
        EXIT_DATA *pexit;

        argument = NULL;

        if (!who_fighting(ch))
        {
                if (ch->position == POS_FIGHTING)
                {
                        if (ch->mount)
                                ch->position = POS_MOUNTED;
                        else
                                ch->position = POS_STANDING;
                }
                send_to_char("You aren't fighting anyone.\n\r", ch);
                return;
        }

        if (ch->endurance <= 0)
        {
                send_to_char("You're too exhausted to flee from combat!\n\r",
                             ch);
                return;
        }

        /*
         * No fleeing while stunned. - Narn 
         */
        if (ch->position < POS_FIGHTING)
                return;

        was_in = ch->in_room;
        for (attempt = 0; attempt < 8; attempt++)
        {

                door = number_door();
                if ((pexit = get_exit(was_in, door)) == NULL
                    || !pexit->to_room
                    || (IS_SET(pexit->exit_info, EX_CLOSED)
                        && !IS_AFFECTED(ch, AFF_PASS_DOOR))
                    || (IS_NPC(ch)
                        && xIS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)))
                        continue;

                affect_strip(ch, gsn_sneak);
                REMOVE_BIT(ch->affected_by, AFF_SNEAK);
                if (ch->mount && ch->mount->fighting)
                        stop_fighting(ch->mount, TRUE);
                move_char(ch, pexit, 0, FALSE);
                if ((now_in = ch->in_room) == was_in)
                        continue;

                ch->in_room = was_in;
                act(AT_FLEE, "$n runs for cover!", ch, NULL, NULL, TO_ROOM);
                ch->in_room = now_in;
                act(AT_FLEE, "$n glances around for signs of pursuit.", ch,
                    NULL, NULL, TO_ROOM);
                snprintf(buf, MSL, "You run for cover!");
                send_to_char(buf, ch);

                stop_fighting(ch, TRUE);
                return;
        }

        snprintf(buf, MSL, "You attempt to run for cover!");
        send_to_char(buf, ch);
        return;
}

bool get_cover(CHAR_DATA * ch)
{
        ROOM_INDEX_DATA *was_in;
        ROOM_INDEX_DATA *now_in;
        int       attempt;
        sh_int    door;
        EXIT_DATA *pexit;

        if (!who_fighting(ch))
                return FALSE;

        if (ch->position < POS_FIGHTING)
                return FALSE;

        was_in = ch->in_room;
        for (attempt = 0; attempt < 10; attempt++)
        {

                door = number_door();
                if ((pexit = get_exit(was_in, door)) == NULL
                    || !pexit->to_room
                    || (IS_SET(pexit->exit_info, EX_CLOSED)
                        && !IS_AFFECTED(ch, AFF_PASS_DOOR))
                    || (IS_NPC(ch)
                        && xIS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)))
                        continue;

                affect_strip(ch, gsn_sneak);
                REMOVE_BIT(ch->affected_by, AFF_SNEAK);
                if (ch->mount && ch->mount->fighting)
                        stop_fighting(ch->mount, TRUE);
                move_char(ch, pexit, 0, FALSE);
                if ((now_in = ch->in_room) == was_in)
                        continue;

                ch->in_room = was_in;
                act(AT_FLEE, "$n sprints for cover!", ch, NULL, NULL,
                    TO_ROOM);
                ch->in_room = now_in;
                act(AT_FLEE, "$n spins around and takes aim.", ch, NULL, NULL,
                    TO_ROOM);

                stop_fighting(ch, TRUE);

                return TRUE;
        }

        return FALSE;
}



CMDF do_sla(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        send_to_char("If you want to SLAY, spell it out.\n\r", ch);
        return;
}



CMDF do_slay(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];

        argument = one_argument(argument, arg);
        one_argument(argument, arg2);
        if (arg[0] == '\0')
        {
                send_to_char("Slay whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (ch == victim)
        {
                send_to_char("Suicide is a mortal sin.\n\r", ch);
                return;
        }

        if (!IS_NPC(victim)
            && (get_trust(victim) == 103 || get_trust(ch) < 103))
        {
                send_to_char("You failed.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "immolate"))
        {
                act(AT_FIRE, "Your fireball turns $N into a blazing inferno.",
                    ch, NULL, victim, TO_CHAR);
                act(AT_FIRE,
                    "$n releases a searing fireball in your direction.", ch,
                    NULL, victim, TO_VICT);
                act(AT_FIRE,
                    "$n points at $N, who bursts into a flaming inferno.", ch,
                    NULL, victim, TO_NOTVICT);
        }

        else if (!str_cmp(arg2, "shatter"))
        {
                act(AT_LBLUE,
                    "You freeze $N with a glance and shatter the frozen corpse into tiny shards.",
                    ch, NULL, victim, TO_CHAR);
                act(AT_LBLUE,
                    "$n freezes you with a glance and shatters your frozen body into tiny shards.",
                    ch, NULL, victim, TO_VICT);
                act(AT_LBLUE,
                    "$n freezes $N with a glance and shatters the frozen body into tiny shards.",
                    ch, NULL, victim, TO_NOTVICT);
        }

        else if (!str_cmp(arg2, "demon"))
        {
                act(AT_IMMORT,
                    "You gesture, and a slavering demon appears.  With a horrible grin, the",
                    ch, NULL, victim, TO_CHAR);
                act(AT_IMMORT,
                    "foul creature turns on $N, who screams in panic before being eaten alive.",
                    ch, NULL, victim, TO_CHAR);
                act(AT_IMMORT,
                    "$n gestures, and a slavering demon appears.  The foul creature turns on",
                    ch, NULL, victim, TO_VICT);
                act(AT_IMMORT,
                    "you with a horrible grin.   You scream in panic before being eaten alive.",
                    ch, NULL, victim, TO_VICT);
                act(AT_IMMORT,
                    "$n gestures, and a slavering demon appears.  With a horrible grin, the",
                    ch, NULL, victim, TO_NOTVICT);
                act(AT_IMMORT,
                    "foul creature turns on $N, who screams in panic before being eaten alive.",
                    ch, NULL, victim, TO_NOTVICT);
        }

        else if (!str_cmp(arg2, "pounce") && get_trust(ch) >= LEVEL_ASCENDANT)
        {
                act(AT_BLOOD,
                    "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...",
                    ch, NULL, victim, TO_CHAR);
                act(AT_BLOOD,
                    "In a heartbeat, $n rips $s fangs through your throat!  Your blood sprays and pours to the ground as your life ends...",
                    ch, NULL, victim, TO_VICT);
                act(AT_BLOOD,
                    "Leaping suddenly, $n sinks $s fangs into $N's throat.  As blood sprays and gushes to the ground, $n tosses $N's dying body away.",
                    ch, NULL, victim, TO_NOTVICT);
        }

        else if (!str_cmp(arg2, "slit") && get_trust(ch) >= LEVEL_ASCENDANT)
        {
                act(AT_BLOOD, "You calmly slit $N's throat.", ch, NULL,
                    victim, TO_CHAR);
                act(AT_BLOOD,
                    "$n reaches out with a clawed finger and calmly slits your throat.",
                    ch, NULL, victim, TO_VICT);
                act(AT_BLOOD, "$n calmly slits $N's throat.", ch, NULL,
                    victim, TO_NOTVICT);
        }

        else
        {
                act(AT_IMMORT, "You slay $N in cold blood!", ch, NULL, victim,
                    TO_CHAR);
                act(AT_IMMORT, "$n slays you in cold blood!", ch, NULL,
                    victim, TO_VICT);
                act(AT_IMMORT, "$n slays $N in cold blood!", ch, NULL, victim,
                    TO_NOTVICT);
        }

        raw_kill(ch, victim);
        return;
}

/* In fight.c */
CMDF do_trip(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;

        argument = NULL;
        if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
        {
                send_to_char("You can't concentrate enough for that.\n\r",
                             ch);
                return;
        }

        if ((victim = who_fighting(ch)) == NULL)
        {
                send_to_char("You aren't fighting anyone.\n\r", ch);
                return;
        }

        WAIT_STATE(ch, skill_table[gsn_trip]->beats);
        if (IS_NPC(ch) || number_percent() < ch->pcdata->learned[gsn_trip])
        {
                act(AT_HIT, "You pull $N's legs from underneath $M.", ch,
                    NULL, victim, TO_CHAR);
                act(AT_HITME, "$n pulls your legs from underneath you.", ch,
                    NULL, victim, TO_VICT);
                act(AT_ACTION, "$n pulls $N's legs from underneath $M.", ch,
                    NULL, victim, TO_NOTVICT);
                learn_from_success(ch, gsn_trip);
                global_retcode =
                        damage(ch, victim,
                               number_range(1,
                                            ch->skill_level[COMBAT_ABILITY]),
                               gsn_trip);
        }
        else
        {
                act(AT_HIT, "You swing your foot but connect with nothing.",
                    ch, NULL, victim, TO_CHAR);
                act(AT_HITME, "$n swing $s foot but connect with nothing.",
                    ch, NULL, NULL, TO_ROOM);
                learn_from_failure(ch, gsn_trip);
                global_retcode = damage(ch, victim, 0, gsn_trip);
        }

        if (number_percent() <= 25 && !IS_IMMORTAL(ch))
        {
                if (!IS_NPC(victim))
                {
                        if (!IS_SET(victim->bodyparts, BODY_L_LEG))
                        {
                                act(AT_HITME, "You hit $N's legs!", ch, NULL,
                                    victim, TO_CHAR);
                                act(AT_HITME, "Ouch!", ch, NULL, victim,
                                    TO_VICT);
                                SET_BIT(victim->bodyparts, BODY_L_LEG);
                        }
                        else if (!IS_SET(victim->bodyparts, BODY_R_LEG))
                        {
                                act(AT_HITME, "You hit $N's legs!", ch, NULL,
                                    victim, TO_CHAR);
                                act(AT_HITME, "Ouch!", ch, NULL, victim,
                                    TO_VICT);
                                SET_BIT(victim->bodyparts, BODY_R_LEG);
                        }
                }
        }
        return;
}
