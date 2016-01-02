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
 *                $Id: update.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "mud.h"
#include "homes.h"
#include "olc-shuttle.h"
#include "account.h"
#include "races.h"
#include "space2.h"
#include "installations.h"

/* from swskills.c
 * Local functions.
 */
int hit_gain args((CHAR_DATA * ch));
int mana_gain args((CHAR_DATA * ch));
int move_gain args((CHAR_DATA * ch));
void gain_addiction args((CHAR_DATA * ch));
void mobile_update args((void));
void weather_update args((void));
void update_taxes args((void));
void update_salaries args((void));
void      update_baccounts();

void update_orbit args((void));

void battalion_update args((void));
void char_update args((void));
void obj_update args((void));
void aggr_update args((void));
void room_act_update args((void));
void obj_act_update args((void));
void char_check args((void));
void bacta_update args((void));
void quest_update args((void)); /* Vassago - quest.c */
void drunk_randoms args((CHAR_DATA * ch));
void halucinations args((CHAR_DATA * ch));
void subtract_times args((struct timeval * etime, struct timeval * systime));
void init_crashover args((void));
void arena_update args((void));

/*
 * Global Variables
 */
CHAR_DATA *gch_prev;
OBJ_DATA *gobj_prev;

CHAR_DATA *timechar;

char     *corpse_descs[] = {
        "The corpse of %s will soon be gone.",
        "The corpse of %s lies here.",
        "The corpse of %s lies here.",
        "The corpse of %s lies here.",
        "The corpse of %s lies here."
};

char     *d_corpse_descs[] = {
        "The shattered remains %s will soon be gone.",
        "The shattered remains %s are here.",
        "The shattered remains %s are here.",
        "The shattered remains %s are here.",
        "The shattered remains %s are here."
};

extern int top_exit;

/*
 * Advancement stuff.
 */
int max_level(CHAR_DATA * ch, int ability)
{
        int       level = 0;

        if (IS_NPC(ch))
                return 100;

        if (IS_IMMORTAL(ch))
                return 200;

        if (ability == COMBAT_ABILITY)
        {
                if (ch->main_ability == COMBAT_ABILITY)
                        level = 100;
                if (ch->main_ability == FORCE_ABILITY)
                        level = 25;
                if (ch->main_ability == HUNTING_ABILITY)
                        level = 25;
                if (ch->main_ability == FORCE_ABILITY)
                        level = 50;
                if (ch->sex == SEX_MALE)
                        level += 5;
                if (ch->sex == SEX_FEMALE)
                        level -= 5;
                level += ch->perm_con + ch->perm_dex + ch->perm_str;
        }
        if (ability == PILOTING_ABILITY)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == ENGINEERING_ABILITY)
                        level = 25;
                if (ch->main_ability == HUNTING_ABILITY)
                        level = 25;
                if (ch->main_ability == SMUGGLING_ABILITY)
                        level = 50;
                if (ch->main_ability == FORCE_ABILITY)
                        level = 30;
                if (ch->main_ability == PIRACY_ABILITY)
                        level = 50;
                if (ch->sex == SEX_MALE)
                        level -= 10;
                if (ch->sex == SEX_FEMALE)
                        level += 10;
                level += ch->perm_dex * 2;
        }
        if (ability == ENGINEERING_ABILITY)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == PILOTING_ABILITY)
                        level = 20;
                if (ch->main_ability == FORCE_ABILITY)
                        level = 30;
                if (ch->sex == SEX_MALE)
                        level -= 5;
                if (ch->sex == SEX_FEMALE)
                        level += 5;
                level += ch->perm_int * 2;
        }
        if (ability == HUNTING_ABILITY)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == FORCE_ABILITY)
                        level = 40;
                if (ch->sex == SEX_MALE)
                        level += 10;
                if (ch->sex == SEX_FEMALE)
                        level -= 10;
        }
        if (ability == SMUGGLING_ABILITY)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == PILOTING_ABILITY)
                        level = 20;
                if (ch->main_ability == ENGINEERING_ABILITY)
                        level = 25;
                if (ch->main_ability == PIRACY_ABILITY)
                        level = 40;
                if (ch->main_ability == FORCE_ABILITY)
                        level = 50;
                level += ch->perm_lck * 2;
        }
        if (ability == LEADERSHIP_ABILITY)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == COMBAT_ABILITY)
                        level = 50;
                if (ch->main_ability == DIPLOMACY_ABILITY)
                        level = 50;
                if (ch->main_ability == FORCE_ABILITY)
                        level = 30;
                if (ch->sex == SEX_MALE)
                        level += 5;
                if (ch->sex == SEX_FEMALE)
                        level -= 5;
                level += ch->perm_wis + ch->perm_cha + ch->perm_int;
        }
        if (ability == DIPLOMACY_ABILITY)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == PILOTING_ABILITY)
                        level = 10;
                if (ch->main_ability == LEADERSHIP_ABILITY)
                        level = 50;
                if (ch->main_ability == FORCE_ABILITY)
                        level = 30;
                if (ch->sex == SEX_MALE)
                        level -= 5;
                if (ch->sex == SEX_FEMALE)
                        level += 5;
                level += ch->perm_cha * 3;
        }
        if (ability == OCCUPATION_ABILITY)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == COMBAT_ABILITY)
                        level = 60;
                if (ch->main_ability == PILOTING_ABILITY)
                        level = 50;
                if (ch->main_ability == ENGINEERING_ABILITY)
                        level = 80;
                if (ch->main_ability == HUNTING_ABILITY)
                        level = 30;
                if (ch->main_ability == SMUGGLING_ABILITY)
                        level = 40;
                if (ch->main_ability == DIPLOMACY_ABILITY)
                        level = 100;
                if (ch->main_ability == LEADERSHIP_ABILITY)
                        level = 90;
                if (ch->main_ability == FORCE_ABILITY)
                        level = 70;
                if (ch->main_ability == PIRACY_ABILITY)
                        level = 40;
                if (ch->main_ability == MEDIC_ABILITY)
                        level = 90;
        }
        if (ability == PIRACY_ABILITY)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == PILOTING_ABILITY)
                        level = 20;
                if (ch->main_ability == HUNTING_ABILITY)
                        level = 30;
                if (ch->main_ability == SMUGGLING_ABILITY)
                        level = 40;
                if (ch->main_ability == FORCE_ABILITY)
                        level = 35;
                level += ch->perm_dex + ch->perm_lck;
        }
        if (ability == MEDIC_ABILITY)
        {
                if (ch->main_ability == ability)
                        level = 100;
                if (ch->main_ability == COMBAT_ABILITY)
                        level = 20;
                if (ch->main_ability == FORCE_ABILITY)
                        level = 50;
                level += ch->perm_dex + ch->perm_int + ch->perm_lck;
        }
        level += ch->race->class_modifier(ability);
        level = URANGE(1, level, 150);
        if (ability == FORCE_ABILITY)
        {
                level = ch->perm_frc * 5;
        }
        return level;
}

void advance_level(CHAR_DATA * ch, int ability)
{

        if (ch->top_level < ch->skill_level[ability] && ch->top_level < 100)
        {
                ch->top_level = URANGE(1, ch->skill_level[ability], 100);
        }

        if (!IS_NPC(ch))
                REMOVE_BIT(ch->act, PLR_BOUGHT_PET);
        return;
}

void gain_exp_new(CHAR_DATA * ch, int gain, int ability, bool outtext)
{
        /*
         * why were these global variables? - Gavin 
         */
        int       hpgain;
        int       mvgain;

        if (IS_NPC(ch))
                return;

        if (IS_SET(ch->act, PLR_FORSAKEN))
        {
                send_to_char
                        ("You bare the mark of the forsaken, and will not grow until you have repenced.\n\r",
                         ch);
                return;
        }

        ch->experience[ability] = UMAX(0, ch->experience[ability] + gain);

        if (NOT_AUTHED(ch)
            && ch->experience[ability] >=
            exp_level(ch->skill_level[ability] + 1))
        {
                send_to_char
                        ("You can not ascend to a higher level until you are authorized.\n\r",
                         ch);
                ch->experience[ability] =
                        (exp_level(ch->skill_level[ability] + 1) - 1);
                return;
        }

        while (ch->experience[ability] >=
               exp_level(ch->skill_level[ability] + 1))
        {
                if (ch->skill_level[ability] >= max_level(ch, ability))
                {
                        ch->experience[ability] =
                                (exp_level(ch->skill_level[ability] + 1) - 1);
                        return;
                }
                ++ch->skill_level[ability];
                if (outtext)
                {
                        set_char_color(AT_WHITE + AT_BLINK, ch);
                        ch_printf(ch,
                                  "You have now obtained %s level %d!\n\r",
                                  ability_name[ability],
                                  ch->skill_level[ability]);
                }
                advance_level(ch, ability);

                if (ability == COMBAT_ABILITY)
                {
                        hpgain = (int) (1 + (ch->perm_con * 0.25));
                        ch->max_hit += hpgain;
                        if (outtext)
                                send_to_char
                                        ("Your maximum hit points have increased!.\n\r",
                                         ch);
                }

                if (ability == HUNTING_ABILITY)
                {
                        mvgain = 0 + (ch->perm_dex);
                        ch->max_endurance += mvgain;
                        if (outtext)
                                send_to_char
                                        ("Your maximum movement points have increased!.\n\r",
                                         ch);
                }

                if (ability == FORCE_ABILITY)
                {
                        ch->max_endurance += 20;
                        if (outtext)
                                send_to_char
                                        ("Your maximum endurance has increased!.\n\r",
                                         ch);
                }

                if (ch->skill_level[PILOTING_ABILITY] == 100)
                {
                        ch->bonus_dex = 1;
                        if (outtext)
                                send_to_char
                                        ("Your dexterity has increased by one point!\n\r",
                                         ch);
                }

                if (ch->skill_level[PILOTING_ABILITY] == 140)
                {
                        ch->bonus_dex = 4;
                        if (outtext)
                                send_to_char
                                        ("Your dexterity has increased by three point!\n\r",
                                         ch);
                }

                if (ch->skill_level[SMUGGLING_ABILITY] == 50)
                {
                        ch->bonus_lck = 1;
                        if (outtext)
                                send_to_char("Your luck has increased!\n\r",
                                             ch);
                }

                if (ch->skill_level[SMUGGLING_ABILITY] == 100)
                {
                        ch->bonus_lck = 2;
                        if (outtext)
                                send_to_char("Your luck has increased!\n\r",
                                             ch);
                }

                if (ch->skill_level[SMUGGLING_ABILITY] == 150)
                {
                        ch->bonus_lck = 3;
                        if (outtext)
                                send_to_char("Your luck has increased!\n\r",
                                             ch);
                }

                if (ch->skill_level[ENGINEERING_ABILITY] == 100)
                {
                        ch->bonus_int = 1;
                        if (outtext)
                                send_to_char
                                        ("Your intelligence has increased by one point!\n\r",
                                         ch);
                }

                if (ch->skill_level[ENGINEERING_ABILITY] == 140)
                {
                        ch->bonus_int = 4;
                        if (outtext)
                                send_to_char
                                        ("Your intelligence has increased by three point!\n\r",
                                         ch);
                }

                if (ch->skill_level[DIPLOMACY_ABILITY] == 100)
                {
                        ch->bonus_cha = 1;
                        if (outtext)
                                send_to_char
                                        ("Your charisma has increased by one point!\n\r",
                                         ch);
                }

                if (ch->skill_level[DIPLOMACY_ABILITY] == 140)
                {
                        ch->bonus_cha = 4;
                        if (outtext)
                                send_to_char
                                        ("Your charisma has increased by three point!\n\r",
                                         ch);
                }

                if (ch->skill_level[PIRACY_ABILITY] == 100)
                {
                        ch->bonus_con = 1;
                        if (outtext)
                                send_to_char
                                        ("Your constitution has increased by one point!\n\r",
                                         ch);
                }

                if (ch->skill_level[PIRACY_ABILITY] == 150)
                {
                        ch->bonus_con = 3;
                        if (outtext)
                                send_to_char
                                        ("Your constitution has increased by three point!\n\r",
                                         ch);
                }

                if (ch->skill_level[MEDIC_ABILITY] == 100)
                {
                        ch->bonus_wis = 1;
                        if (outtext)
                                send_to_char
                                        ("Your wisdom has increased by one point!\n\r",
                                         ch);
                }

                if (ch->skill_level[MEDIC_ABILITY] == 150)
                {
                        ch->bonus_wis = 3;
                        if (outtext)
                                send_to_char
                                        ("Your wisdom has increased by one point!\n\r",
                                         ch);
                }
        }
        return;
}

void gain_exp(CHAR_DATA * ch, int gain, int ability)
{
        gain_exp_new(ch, gain, ability, TRUE);
        return;
}

/*
 * Regeneration stuff.
 */
int hit_gain(CHAR_DATA * ch)
{
        int       gain;

        if (IS_NPC(ch))
        {
                gain = ch->top_level;
        }
        else
        {
                if (IS_SET(ch->act, PLR_FORSAKEN))
                {
                        send_to_char
                                ("You bare the mark of the forsaken, and will not grow until you have repenced.\n\r",
                                 ch);
                        return 0;
                }

                gain = UMIN(5, ch->top_level);

                switch (ch->position)
                {
                case POS_DEAD:
                        return 0;
                case POS_MORTAL:
                        return -25;
                case POS_INCAP:
                        return -20;
                case POS_STUNNED:
                        return get_curr_con(ch) * 2;
                case POS_SLEEPING:
                        gain += (int) (get_curr_con(ch) * 1.5);
                        break;
                case POS_RESTING:
                        gain += get_curr_con(ch);
                        break;
                }

                if (ch->pcdata->condition[COND_FULL] == 0)
                        gain /= 2;

                if (ch->pcdata->condition[COND_THIRST] == 0)
                        gain /= 2;

        }

        if (IS_AFFECTED(ch, AFF_POISON))
                gain /= 4;

        if (ch->race && ch->race->death_age() != -1)
        {
                if (get_age(ch) > ch->race->death_age())
                {
                        send_to_char
                                ("You are very old.\n\rYou are becoming weaker with every moment.\n\rSoon you will die.\n\r",
                                 ch);
                        return -10;
                }
                else if (get_age(ch) > (ch->race->death_age() / 1.6))
                        gain /= 10;
                else if (get_age(ch) > (ch->race->death_age() / 2))
                        gain /= 5;
                else if (get_age(ch) > (ch->race->death_age() / 2.6))
                        gain /= 2;
        }

        /*
         * FIXME - shouldn't be hardcoded 
         */
        if (ch->race && !str_cmp(ch->race->name(), "trandoshan"))
                gain *= 4;

        return UMIN(gain, ch->max_hit - ch->hit);
}


int mana_gain(CHAR_DATA * ch)
{
        int       gain;

        if (IS_NPC(ch))
        {
                gain = ch->top_level;
        }
        else
        {
                if (IS_SET(ch->act, PLR_FORSAKEN))
                {
                        send_to_char
                                ("You bare the mark of the forsaken, and will not grow until you have repenced.\n\r",
                                 ch);
                        return 0;
                }
                if (ch->skill_level[FORCE_ABILITY] <= 1)
                        return (0);

                gain = UMIN(5, ch->skill_level[FORCE_ABILITY] / 2);

                if (ch->position < POS_SLEEPING)
                        return 0;
                switch (ch->position)
                {
                case POS_SLEEPING:
                        gain += get_curr_int(ch) * 3;
                        break;
                case POS_RESTING:
                        gain += (int) (get_curr_int(ch) * 1.5);
                        break;
                }

                if (ch->pcdata->condition[COND_FULL] == 0)
                        gain /= 2;

                if (ch->pcdata->condition[COND_THIRST] == 0)
                        gain /= 2;

        }

        if (IS_AFFECTED(ch, AFF_POISON))
                gain /= 4;

        return UMIN(gain, ch->max_endurance - ch->endurance);
}

int move_gain(CHAR_DATA * ch)
{
        int       gain;

        if (IS_NPC(ch))
        {
                gain = ch->top_level;
        }
        else
        {
                if (IS_SET(ch->act, PLR_FORSAKEN))
                {
                        send_to_char
                                ("You bare the mark of the forsaken, and will not grow until you have repenced.\n\r",
                                 ch);
                        return 0;
                }
                gain = UMAX(15, 2 * ch->top_level);
                switch (ch->position)
                {
                case POS_DEAD:
                        return 0;
                case POS_MORTAL:
                        return -1;
                case POS_INCAP:
                        return -1;
                case POS_STUNNED:
                        return 1;
                case POS_SLEEPING:
                        gain += get_curr_dex(ch) * 2;
                        break;
                case POS_RESTING:
                        gain += get_curr_dex(ch);
                        break;
                }
                if (ch->pcdata->condition[COND_FULL] == 0)
                        gain /= 2;
                if (ch->pcdata->condition[COND_THIRST] == 0)
                        gain /= 2;
        }
        if (IS_SET(ch->bodyparts, BODY_R_LEG))
                gain -= 10;
        if (ch->race && ch->race->death_age() != -1)
        {
                if (get_age(ch) > ch->race->death_age())
                {
                        send_to_char
                                ("You are very old.\n\rYou are becoming weaker with every moment.\n\rSoon you will die.\n\r",
                                 ch);
                        return -10;
                }
                else if (get_age(ch) > (ch->race->death_age() / 1.6))
                        gain /= 10;
                else if (get_age(ch) > (ch->race->death_age() / 2))
                        gain /= 5;
                else if (get_age(ch) > (ch->race->death_age() / 2.6))
                        gain /= 2;

        }
        if (IS_SET(ch->bodyparts, BODY_L_LEG))
                gain -= 10;
        if (IS_SET(ch->bodyparts, BODY_R_LEG))
                gain -= 10;

        /*
         * Both are broken? HAHAHAHA okay, screw them over more 
         */
        if (IS_SET(ch->bodyparts, BODY_L_LEG)
            && IS_SET(ch->bodyparts, BODY_R_LEG))
                gain -= 10;

        return UMIN(gain, ch->max_endurance - ch->endurance);
}

void gain_addiction(CHAR_DATA * ch)
{
        short     drug;
        ch_ret    retcode;
        AFFECT_DATA af;

        for (drug = 0; drug <= 9; drug++)
        {

                if (ch->pcdata->addiction[drug] <
                    ch->pcdata->drug_level[drug])
                        ch->pcdata->addiction[drug]++;

                if (ch->pcdata->addiction[drug] >
                    ch->pcdata->drug_level[drug] + 150)
                {
                        switch (ch->pcdata->addiction[drug])
                        {
                        default:
                        case SPICE_GLITTERSTIM:
                                if (!IS_AFFECTED(ch, AFF_BLIND))
                                {
                                        af.type = gsn_blindness;
                                        af.location = APPLY_AC;
                                        af.modifier = 10;
                                        af.duration =
                                                ch->pcdata->addiction[drug];
                                        af.bitvector = AFF_BLIND;
                                        affect_to_char(ch, &af);
                                }
                        case SPICE_CARSANUM:
                                if (!IS_AFFECTED(ch, AFF_WEAKEN))
                                {
                                        af.type = -1;
                                        af.location = APPLY_DAMROLL;
                                        af.modifier = -10;
                                        af.duration =
                                                ch->pcdata->addiction[drug];
                                        af.bitvector = AFF_WEAKEN;
                                        affect_to_char(ch, &af);
                                }
                        case SPICE_RYLL:
                                if (!IS_AFFECTED(ch, AFF_WEAKEN))
                                {
                                        af.type = -1;
                                        af.location = APPLY_DEX;
                                        af.modifier = -5;
                                        af.duration =
                                                ch->pcdata->addiction[drug];
                                        af.bitvector = AFF_WEAKEN;
                                        affect_to_char(ch, &af);
                                }
                        case SPICE_ANDRIS:
                                if (!IS_AFFECTED(ch, AFF_WEAKEN))
                                {
                                        af.type = -1;
                                        af.location = APPLY_CON;
                                        af.modifier = -5;
                                        af.duration =
                                                ch->pcdata->addiction[drug];
                                        af.bitvector = AFF_WEAKEN;
                                        affect_to_char(ch, &af);
                                }
                        }
                }

                if (ch->pcdata->addiction[drug] >
                    ch->pcdata->drug_level[drug] + 200)
                {
                        ch_printf(ch,
                                  "You feel like you are going to die. You NEED %s\n\r.",
                                  spice_table[drug]);
                        worsen_mental_state(ch, 2);
                        retcode = damage(ch, ch, 5, TYPE_UNDEFINED);
                }
                else if (ch->pcdata->addiction[drug] >
                         ch->pcdata->drug_level[drug] + 100)
                {
                        ch_printf(ch, "You need some %s.\n\r",
                                  spice_table[drug]);
                        worsen_mental_state(ch, 2);
                }
                else if (ch->pcdata->addiction[drug] >
                         ch->pcdata->drug_level[drug] + 50)
                {
                        ch_printf(ch, "You really crave some %s.\n\r",
                                  spice_table[drug]);
                        worsen_mental_state(ch, 1);
                }
                else if (ch->pcdata->addiction[drug] >
                         ch->pcdata->drug_level[drug] + 25)
                {
                        ch_printf(ch,
                                  "Some more %s would feel quite nice.\n\r",
                                  spice_table[drug]);
                }
                else if (ch->pcdata->addiction[drug] <
                         ch->pcdata->drug_level[drug] - 50)
                {
                        act(AT_POISON, "$n bends over and vomits.\n\r", ch,
                            NULL, NULL, TO_ROOM);
                        act(AT_POISON, "You vomit.\n\r", ch, NULL, NULL,
                            TO_CHAR);
                        ch->pcdata->drug_level[drug] -= 10;
                }

                if (ch->pcdata->drug_level[drug] > 1)
                        ch->pcdata->drug_level[drug] -= 2;
                else if (ch->pcdata->drug_level[drug] > 0)
                        ch->pcdata->drug_level[drug] -= 1;
                else if (ch->pcdata->addiction[drug] > 0
                         && ch->pcdata->drug_level[drug] <= 0)
                        ch->pcdata->addiction[drug]--;
        }

}

void gain_condition(CHAR_DATA * ch, int iCond, int value)
{
        int       condition;
        ch_ret    retcode = rNONE;

        if (value == 0 || IS_NPC(ch) || get_trust(ch) >= LEVEL_IMMORTAL ||
            NOT_AUTHED(ch) || !str_cmp("droid", ch->race->name())
            || IS_SET(ch->pcdata->flags, PCFLAG_NOHUNGER))
                return;

        condition = ch->pcdata->condition[iCond];
		/* For decreasing checks */
		if (value <= 0 && 
			(iCond == COND_FULL || iCond == COND_THIRST) &&
			get_implant_affect(ch,IMPLANT_HUNGER))
				return;
        ch->pcdata->condition[iCond] = URANGE(0, condition + value, 48);

        if (ch->pcdata->condition[iCond] == 0)
        {
                switch (iCond)
                {
                case COND_FULL:
                        if (ch->top_level <= LEVEL_AVATAR)
                        {
                                set_char_color(AT_HUNGRY, ch);
                                send_to_char("You are STARVING!\n\r", ch);
                                act(AT_HUNGRY, "$n is starved half to death!",
                                    ch, NULL, NULL, TO_ROOM);
                                worsen_mental_state(ch, 1);
                                retcode = damage(ch, ch, 5, TYPE_UNDEFINED);
                        }
                        break;

                case COND_THIRST:
                        if (ch->top_level < LEVEL_AVATAR)
                        {
                                set_char_color(AT_THIRSTY, ch);
                                send_to_char("You are DYING of THIRST!\n\r",
                                             ch);
                                act(AT_THIRSTY, "$n is dying of thirst!", ch,
                                    NULL, NULL, TO_ROOM);
                                worsen_mental_state(ch, 2);
                                retcode = damage(ch, ch, 5, TYPE_UNDEFINED);
                        }
                        break;

                case COND_DRUNK:
                        if (condition != 0)
                        {
                                set_char_color(AT_SOBER, ch);
                                send_to_char("You are sober.\n\r", ch);
                        }
                        retcode = rNONE;
                        break;
                default:
                        bug("Gain_condition: invalid condition type %d",
                            iCond);
                        retcode = rNONE;
                        break;
                }
        }

        if (retcode != rNONE)
                return;

        if (ch->pcdata->condition[iCond] == 1)
        {
                switch (iCond)
                {
                case COND_FULL:
                        if (ch->top_level < LEVEL_AVATAR)
                        {
                                set_char_color(AT_HUNGRY, ch);
                                send_to_char("You are really hungry.\n\r",
                                             ch);
                                act(AT_HUNGRY,
                                    "You can hear $n's stomach growling.", ch,
                                    NULL, NULL, TO_ROOM);
                                if (number_bits(1) == 0)
                                        worsen_mental_state(ch, 1);
                        }
                        break;

                case COND_THIRST:
                        if (ch->top_level < LEVEL_AVATAR)
                        {
                                set_char_color(AT_THIRSTY, ch);
                                send_to_char("You are really thirsty.\n\r",
                                             ch);
                                worsen_mental_state(ch, 1);
                                act(AT_THIRSTY, "$n looks a little parched.",
                                    ch, NULL, NULL, TO_ROOM);
                        }
                        break;

                case COND_DRUNK:
                        if (condition != 0)
                        {
                                set_char_color(AT_SOBER, ch);
                                send_to_char
                                        ("You are feeling a little less light headed.\n\r",
                                         ch);
                        }
                        break;
                }
        }
        if (ch->pcdata->condition[iCond] == 2)
        {
                switch (iCond)
                {
                case COND_FULL:
                        if (ch->top_level < LEVEL_AVATAR)
                        {
                                set_char_color(AT_HUNGRY, ch);
                                send_to_char("You are hungry.\n\r", ch);
                        }
                        break;

                case COND_THIRST:
                        if (ch->top_level < LEVEL_AVATAR)
                        {
                                set_char_color(AT_THIRSTY, ch);
                                send_to_char("You are thirsty.\n\r", ch);
                        }
                        break;

                }
        }

        if (ch->pcdata->condition[iCond] == 3)
        {
                switch (iCond)
                {
                case COND_FULL:
                        if (ch->top_level < LEVEL_AVATAR)
                        {
                                set_char_color(AT_HUNGRY, ch);
                                send_to_char("You are a mite peckish.\n\r",
                                             ch);
                        }
                        break;

                case COND_THIRST:
                        if (ch->top_level < LEVEL_AVATAR)
                        {
                                set_char_color(AT_THIRSTY, ch);
                                send_to_char
                                        ("You could use a sip of something refreshing.\n\r",
                                         ch);
                        }
                        break;

                }
        }
        return;
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Mud cpu time.
 */
void mobile_update(void)
{
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *ch;
        EXIT_DATA *pexit;
        int       door;
        ch_ret    retcode;

        retcode = rNONE;

        /*
         * Examine all mobs. 
         */
        for (ch = last_char; ch; ch = gch_prev)
        {

                if (ch == first_char && ch->prev)
                {
                        bug("mobile_update: first_char->prev != NULL... fixed", 0);
                        ch->prev = NULL;
                }

                gch_prev = ch->prev;

                if (gch_prev && gch_prev->next != ch)
                {
                        snprintf(buf, MSL,
                                 "FATAL: Mobile_update: %s->prev->next doesn't point to ch.",
                                 ch->name);
                        bug(buf, 0);
                        bug("Short-cutting here", 0);
                        gch_prev = NULL;
                        ch->prev = NULL;
                        do_shout(ch, "Thoric says, 'Prepare for the worst!'");
                }

                if (!IS_NPC(ch))
                {
                        drunk_randoms(ch);
                        halucinations(ch);
                        continue;
                }

                if (!ch->in_room
                    || IS_AFFECTED(ch, AFF_CHARM)
                    || IS_AFFECTED(ch, AFF_PARALYSIS))
                        continue;

/* Clean up 'animated corpses' that are not charmed' - Scryn */

                if (ch->pIndexData->vnum == 5 && !IS_AFFECTED(ch, AFF_CHARM))
                {
                        if (ch->in_room->first_person)
                                act(AT_MAGIC,
                                    "$n returns to the dust from whence $e came.",
                                    ch, NULL, NULL, TO_ROOM);

                        if (IS_NPC(ch)) /* Guard against purging switched? */
                                extract_char(ch, TRUE);
                        continue;
                }

                if (!IS_SET(ch->act, ACT_SENTINEL)
                    && !ch->fighting && ch->hunting)
                {
                        if (ch->top_level < 20)
                                WAIT_STATE(ch, 6 * PULSE_PER_SECOND);
                        else if (ch->top_level < 40)
                                WAIT_STATE(ch, 5 * PULSE_PER_SECOND);
                        else if (ch->top_level < 60)
                                WAIT_STATE(ch, 4 * PULSE_PER_SECOND);
                        else if (ch->top_level < 80)
                                WAIT_STATE(ch, 3 * PULSE_PER_SECOND);
                        else if (ch->top_level < 100)
                                WAIT_STATE(ch, 2 * PULSE_PER_SECOND);
                        else
                                WAIT_STATE(ch, 1 * PULSE_PER_SECOND);
                        hunt_victim(ch);
                        continue;
                }
                else if (!ch->fighting && !ch->hunting
                         && !IS_SET(ch->act, ACT_RUNNING)
                         && ch->was_sentinel && ch->position >= POS_STANDING)
                {
                        act(AT_ACTION, "$n leaves.", ch, NULL, NULL, TO_ROOM);
                        char_from_room(ch);
                        char_to_room(ch, ch->was_sentinel);
                        act(AT_ACTION, "$n arrives.", ch, NULL, NULL,
                            TO_ROOM);
                        SET_BIT(ch->act, ACT_SENTINEL);
                        ch->was_sentinel = NULL;
                }

                /*
                 * Examine call for special procedure 
                 */
                if (!IS_SET(ch->act, ACT_RUNNING) && ch->spec_fun)
                {
                        if ((*ch->spec_fun) (ch))
                                continue;
                        if (char_died(ch))
                                continue;
                }

                if (!IS_SET(ch->act, ACT_RUNNING) && ch->spec_2)
                {
                        if ((*ch->spec_2) (ch))
                                continue;
                        if (char_died(ch))
                                continue;
                }

                /*
                 * Check for mudprogram script on mob 
                 */
                if (IS_SET(ch->pIndexData->progtypes, SCRIPT_PROG))
                {
                        mprog_script_trigger(ch);
                        continue;
                }
                /*
                 * That's all for sleeping / busy monster 
                 */
                if (ch->position != POS_STANDING)
                        continue;


                if (IS_SET(ch->act, ACT_MOUNTED))
                {
                        if (IS_SET(ch->act, ACT_AGGRESSIVE))
                                do_emote(ch, "snarls and growls.");
                        continue;
                }

                if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE)
                    && IS_SET(ch->act, ACT_AGGRESSIVE))
                        do_emote(ch, "glares around and snarls.");  /* MOBprogram random trigger */
                if (ch->in_room->area->nplayer > 0)
                {
                        mprog_random_trigger(ch);
                        if (char_died(ch))
                                continue;
                        if (ch->position < POS_STANDING)
                                continue;
                }

                /*
                 * MOBprogram hour trigger: do something for an hour 
                 */
                mprog_hour_trigger(ch);

                if (char_died(ch))
                        continue;

                rprog_hour_trigger(ch);
                if (char_died(ch))
                        continue;

                if (ch->position < POS_STANDING)
                        continue;

                /*
                 * Scavenge 
                 */
                if (IS_SET(ch->act, ACT_SCAVENGER)
                    && ch->in_room->first_content && number_bits(2) == 0)
                {
                        OBJ_DATA *obj;
                        OBJ_DATA *obj_best;
                        int       max;

                        max = 1;
                        obj_best = NULL;
                        for (obj = ch->in_room->first_content; obj;
                             obj = obj->next_content)
                        {
                                if (CAN_WEAR(obj, ITEM_TAKE)
                                    && obj->cost > max
                                    && !IS_OBJ_STAT(obj, ITEM_BURRIED))
                                {
                                        obj_best = obj;
                                        max = obj->cost;
                                }
                        }

                        if (obj_best)
                        {
                                obj_from_room(obj_best);
                                obj_to_char(obj_best, ch);
                                if (auction && auction->item
                                    && auction->item->item_type ==
                                    ITEM_BEACON)
                                        auction->item->value[0] = 0;
                                act(AT_ACTION, "$n gets $p.", ch, obj_best,
                                    NULL, TO_ROOM);
                        }
                }

                /*
                 * Wander 
                 */ if (!IS_SET(ch->act, ACT_SENTINEL)
                        && !IS_SET(ch->act, ACT_PROTOTYPE)
                        && !ch->held
                        && (door = number_bits(5)) <= 9
                        && (pexit = get_exit(ch->in_room, door)) != NULL
                        && pexit->to_room
                        && !IS_SET(pexit->exit_info, EX_CLOSED)
                        && !xIS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)
                        && (!IS_SET(ch->act, ACT_STAY_AREA)
                            || pexit->to_room->area == ch->in_room->area))
                {
                        retcode = move_char(ch, pexit, 0, FALSE);
                        /*
                         * If ch changes position due
                         * to it's or someother mob's
                         * movement via MOBProgs,
                         * continue - Kahn 
                         */
                        if (char_died(ch))
                                continue;
                        if (retcode != rNONE || IS_SET(ch->act, ACT_SENTINEL)
                            || ch->position < POS_STANDING)
                                continue;
                }

                /*
                 * Flee 
                 */
                if (ch->hit < ch->max_hit / 2
                    && !ch->held
                    && (door = number_bits(4)) <= 9
                    && (pexit = get_exit(ch->in_room, door)) != NULL
                    && pexit->to_room
                    && !IS_SET(pexit->exit_info, EX_CLOSED)
                    && !xIS_SET(pexit->to_room->room_flags, ROOM_NO_MOB))
                {
                        CHAR_DATA *rch;
                        bool      found;

                        found = FALSE;
                        for (rch = ch->in_room->first_person;
                             rch; rch = rch->next_in_room)
                        {
                                if (is_fearing(ch, rch))
                                {
                                        switch (number_bits(2))
                                        {
                                        case 0:
                                                snprintf(buf, MSL,
                                                         "yell Get away from me, %s!",
                                                         rch->name);
                                                break;
                                        case 1:
                                                snprintf(buf, MSL,
                                                         "yell Leave me be, %s!",
                                                         rch->name);
                                                break;
                                        case 2:
                                                snprintf(buf, MSL,
                                                         "yell %s is trying to kill me!  Help!",
                                                         rch->name);
                                                break;
                                        case 3:
                                                snprintf(buf, MSL,
                                                         "yell Someone save me from %s!",
                                                         rch->name);
                                                break;
                                        }
                                        interpret(ch, buf);
                                        found = TRUE;
                                        break;
                                }
                        }
                        if (found)
                                retcode = move_char(ch, pexit, 0, FALSE);
                }
        }

        return;
}

void update_salaries(void)
{
        CHAR_DATA *ch;
        CLAN_DATA *clan;
        DESCRIPTOR_DATA *d;
        int       amount;

        for (d = first_descriptor; d; d = d->next)
        {
                if (d->connected != CON_PLAYING)
                        continue;
                ch = d->character;
                /*
                 * Not as likely this way, but can be switched 
                 */
                if (IS_NPC(ch))
                        continue;   /* Not likely, but just in case */
                if (ch->pcdata && ch->pcdata->clan)
                {
                        clan = ch->pcdata->clan;
                        amount = clan->salary[ch->pcdata->clanrank];
                        if (amount <= 0)
                                continue;
                        ch->gold += amount;
                        ch->pcdata->clan->funds -= amount;
                        act(AT_ACTION,
                            "Someone quickly appears and hands $n a package.",
                            ch, NULL, NULL, TO_ROOM);
                        act(AT_ACTION,
                            "A clan messenger appears and hands you a salary package.",
                            ch, NULL, NULL, TO_CHAR);
                }
        }
}

void update_taxes(void)
{
        PLANET_DATA *planet;
        int       i, iv, ev;
        CLAN_DATA *clan;
        char      buf[MAX_STRING_LENGTH];

        for (planet = first_planet; planet; planet = planet->next)
        {
                for (i = 1; i < CARGO_MAX; i++)
                {
                        /*
                         * Set resources value (iv)
                         */
                        iv = 0;
                        ev = 0;
                        if (planet->resource[i] > 1
                            && planet->resource[i] < 10)
                        {
                                iv = 10;
                                ev = 1;
                        }
                        if (planet->resource[i] > 10
                            && planet->resource[i] < 100)
                        {
                                iv = 9;
                                ev = 2;
                        }
                        if (planet->resource[i] > 100
                            && planet->resource[i] < 1000)
                        {
                                iv = 8;
                                ev = 3;
                        }
                        if (planet->resource[i] > 1000
                            && planet->resource[i] < 10000)
                        {
                                iv = 7;
                                ev = 4;
                        }
                        if (planet->resource[i] > 10000
                            && planet->resource[i] < 50000)
                        {
                                iv = 6;
                                ev = 5;
                        }
                        if (planet->resource[i] > 50000
                            && planet->resource[i] < 100000)
                        {
                                iv = 5;
                                ev = 6;
                        }
                        if (planet->resource[i] > 100000
                            && planet->resource[i] < 500000)
                        {
                                iv = 4;
                                ev = 7;
                        }
                        if (planet->resource[i] > 500000
                            && planet->resource[i] < 1000000)
                        {
                                iv = 3;
                                ev = 8;
                        }
                        if (planet->resource[i] > 1000000)
                        {
                                iv = 2;
                                ev = 9;
                        }
                        if (iv > ev)
                        {
                                /*
                                 * Adjust import and export costs
                                 */
                                planet->cargoimport[i] =
                                        ((10 * i) * (iv)) +
                                        (int) (number_percent() * (0.1));
                                planet->cargoexport[i] = 0;
                                /*
                                 * Adjust production and consumtion
                                 */
                                planet->produces[i] =
                                        (int) (1.6 * number_percent() *
                                               (iv - ev)) +
                                        (number_percent());
                                planet->consumes[i] =
                                        (number_percent() * (iv - ev)) +
                                        (number_percent());
                        }
                        else    /* if ( iv < ev ) */
                        {
                                /*
                                 * Adjust import and export costs
                                 */
                                planet->cargoexport[i] =
                                        ((10 * i) * (iv)) +
                                        (int) (number_percent() * (0.1));
                                planet->cargoimport[i] = 0;
                                /*
                                 * Adjust production and consumtion
                                 */
                                planet->produces[i] =
                                        (number_percent() * (ev - iv)) +
                                        (number_percent());
                                planet->consumes[i] =
                                        (int) (1.6 * number_percent() *
                                               (ev - iv)) +
                                        (number_percent());
                        }
                        /*
                         * Implement production and consumtion values 
                         */

                        planet->resource[i] += planet->produces[i];

                        planet->resource[i] -= planet->consumes[i];
                        /*
                         * Broadcast urgent import messages 
                         */
                        if (planet->resource[i] < 10000
                            && planet->resource[i] > 0)
                        {
                                snprintf(buf, MSL,
                                         "Trade Alert: %s is in need of imports!",
                                         planet->name);
                                echo_to_all(AT_GOLD, buf, 0);
                        }
                        /*
                         * Reset negative values to 1 
                         */
                        /*
                         * and do emergency import stuff 
                         */
                        if (planet->resource[i] < 0)
                        {
                                planet->resource[i] = 1;
                                planet->cargoimport[i] =
                                        ((100 * i) +
                                         (int) (number_percent() * (0.1)));
                                planet->cargoexport[i] = 0;
                                snprintf(buf, MSL,
                                         "Trade Alert: %s is in need of emergency imports!",
                                         planet->name);
                                echo_to_all(AT_GOLD, buf, 0);
                        }
                        /*
                         * Save the planet file 
                         */
                        save_planet(planet, FALSE);
                }
                clan = planet->governed_by;
                if (clan)
                {
                        clan->funds += get_taxes(planet) / 360;
                        save_clan(clan);
                        save_planet(planet, FALSE);
                }
        }
}

/*
 * Update the weather.
 */
void weather_update(void)
{
        char      buf[MAX_STRING_LENGTH];
        DESCRIPTOR_DATA *d;
        int       diff;
        sh_int    AT_TEMP = AT_PLAIN;

        buf[0] = '\0';

        switch (++time_info.hour)
        {
        case 5:
                weather_info.sunlight = SUN_LIGHT;
                mudstrlcat(buf, "The day has begun.", MSL);
                AT_TEMP = AT_YELLOW;
                break;

        case 6:
                weather_info.sunlight = SUN_RISE;
                mudstrlcat(buf, "The sun rises in the east.", MSL);
                AT_TEMP = AT_ORANGE;
                break;

        case 12:
                weather_info.sunlight = SUN_LIGHT;
                mudstrlcat(buf, "It's noon.", MSL);
                AT_TEMP = AT_YELLOW;
                break;

        case 19:
                weather_info.sunlight = SUN_SET;
                mudstrlcat(buf, "The sun slowly disappears in the west.",
                           MSL);
                AT_TEMP = AT_BLOOD;
                break;

        case 20:
                weather_info.sunlight = SUN_DARK;
                mudstrlcat(buf, "The night has begun.", MSL);
                AT_TEMP = AT_DGREY;
                break;

        case 24:
                time_info.hour = 0;
                time_info.day++;
                break;
        }

        if (time_info.day >= 29)
        {
                time_info.day = 0;
                time_info.month++;
        }

        if (time_info.month >= 17)
        {
                time_info.month = 0;
                time_info.year++;
        }

        if (buf[0] != '\0')
        {
                for (d = first_descriptor; d; d = d->next)
                {
                        if (d->connected == CON_PLAYING
                            && IS_OUTSIDE(d->character)
                            && IS_AWAKE(d->character)
                            && d->character->in_room
                            && d->character->in_room->sector_type !=
                            SECT_UNDERWATER
                            && d->character->in_room->sector_type !=
                            SECT_OCEANFLOOR
                            && d->character->in_room->sector_type !=
                            SECT_UNDERGROUND)
                                act(AT_TEMP, buf, d->character, 0, 0,
                                    TO_CHAR);
                }
                buf[0] = '\0';
        }
        /*
         * Weather change.
         */
        if (time_info.month >= 9 && time_info.month <= 16)
                diff = weather_info.mmhg > 985 ? -2 : 2;
        else
                diff = weather_info.mmhg > 1015 ? -2 : 2;

        weather_info.change += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
        weather_info.change = UMAX(weather_info.change, -12);
        weather_info.change = UMIN(weather_info.change, 12);

        weather_info.mmhg += weather_info.change;
        weather_info.mmhg = UMAX(weather_info.mmhg, 960);
        weather_info.mmhg = UMIN(weather_info.mmhg, 1040);

        AT_TEMP = AT_GREY;
        switch (weather_info.sky)
        {
        default:
                bug("Weather_update: bad sky %d.", weather_info.sky);
                weather_info.sky = SKY_CLOUDLESS;
                break;

        case SKY_CLOUDLESS:
                if (weather_info.mmhg < 990
                    || (weather_info.mmhg < 1010 && number_bits(2) == 0))
                {
                        mudstrlcat(buf, "The sky is getting cloudy.", MSL);
                        weather_info.sky = SKY_CLOUDY;
                        AT_TEMP = AT_GREY;
                }
                break;

        case SKY_CLOUDY:
                if (weather_info.mmhg < 970
                    || (weather_info.mmhg < 990 && number_bits(2) == 0))
                {
                        mudstrlcat(buf, "It starts to rain.", MSL);
                        weather_info.sky = SKY_RAINING;
                        AT_TEMP = AT_BLUE;
                }

                if (weather_info.mmhg > 1030 && number_bits(2) == 0)
                {
                        mudstrlcat(buf, "The clouds disappear.", MSL);
                        weather_info.sky = SKY_CLOUDLESS;
                        AT_TEMP = AT_WHITE;
                }
                break;

        case SKY_RAINING:
                if (weather_info.mmhg < 970 && number_bits(2) == 0)
                {
                        mudstrlcat(buf, "Lightning flashes in the sky.", MSL);
                        weather_info.sky = SKY_LIGHTNING;
                        AT_TEMP = AT_YELLOW;
                }

                if (weather_info.mmhg > 1030
                    || (weather_info.mmhg > 1010 && number_bits(2) == 0))
                {
                        mudstrlcat(buf, "The rain stopped.", MSL);
                        weather_info.sky = SKY_CLOUDY;
                        AT_TEMP = AT_WHITE;
                }
                break;

        case SKY_LIGHTNING:
                if (weather_info.mmhg > 1010
                    || (weather_info.mmhg > 990 && number_bits(2) == 0))
                {
                        mudstrlcat(buf, "The lightning has stopped.", MSL);
                        weather_info.sky = SKY_RAINING;
                        AT_TEMP = AT_GREY;
                        break;
                }
                break;
        }

        if (buf[0] != '\0')
        {
                for (d = first_descriptor; d; d = d->next)
                {
                        if (d->connected == CON_PLAYING
                            && IS_OUTSIDE(d->character)
                            && IS_AWAKE(d->character))
                                act(AT_TEMP, buf, d->character, 0, 0,
                                    TO_CHAR);
                }
        }

        return;
}

/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
void char_update(void)
{
        CHAR_DATA *ch;

        char      buf[MSL];

        CHAR_DATA *ch_save;
        int       value;
        sh_int    save_count = 0;
        OBJ_DATA *binding = NULL;

        ch_save = NULL;
        for (ch = last_char; ch; ch = gch_prev)
        {
                if (ch == first_char && ch->prev)
                {
                        bug("char_update: first_char->prev != NULL... fixed",
                            0);
                        ch->prev = NULL;
                }
                gch_prev = ch->prev;
                if (gch_prev && gch_prev->next != ch)
                {
                        bug("char_update: ch->prev->next != ch", 0);
                        return;
                }
                /*
                 * erflink Erflink ERFLINK did you add the player fields for it? If so, I dunno where they are, gotta look.
                 */
                if (ch->pcdata)
                {
/*
     value = number_range( 1,10000 );
     if ( ( value >= 1 ) && ( value <= 3 ) && ( ch->pcdata->illness < 15 ) )
        ch->pcdata->illness = 15;
     if ( ( value >= 4 ) && ( value <= 7 ) && ( ch->pcdata->illness < 14 ) )
        ch->pcdata->illness = 14;
     if ( ( value >= 8 ) && ( value <= 20 ) && ( ch->pcdata->illness < 13 ) )
        ch->pcdata->illness = 13;
     if ( ( value >= 21 ) && ( value <= 31 ) && ( ch->pcdata->illness < 12 ) )
        ch->pcdata->illness = 12;
     if ( ( value >= 32 ) && ( value <= 42 ) && ( ch->pcdata->illness < 11 ) )
        ch->pcdata->illness = 11;
     if ( ( value >= 43 ) && ( value <= 53 ) && ( ch->pcdata->illness < 10 ) )
        ch->pcdata->illness = 10;
     if ( ( value >= 54 ) && ( value <= 64 ) && ( ch->pcdata->illness < 9 ) )
        ch->pcdata->illness = 9;
     if ( ( value >= 65 ) && ( value <= 75 ) && ( ch->pcdata->illness < 8 ) )
        ch->pcdata->illness = 8;
     if ( ( value >= 76 ) && ( value <= 80 ) && ( ch->pcdata->illness < 7 ) )
        ch->pcdata->illness = 7;
     if ( ( value >= 81 ) && ( value <= 99 ) && ( ch->pcdata->illness < 6 ) )
        ch->pcdata->illness = 6;
     if ( ( value >= 100 ) && ( value <= 119 ) && ( ch->pcdata->illness < 5 ) )
        ch->pcdata->illness = 5;
     if ( ( value >= 120 ) && ( value <= 149 ) && ( ch->pcdata->illness < 4 ) )
        ch->pcdata->illness = 4;
     if ( ( value >= 150 ) && ( value <= 189 ) && ( ch->pcdata->illness < 3 ) )
        ch->pcdata->illness = 3;
     if ( ( value >= 190 ) && ( value <= 239 ) && ( ch->pcdata->illness < 2 ) )
        ch->pcdata->illness = 2;
     if ( ( value >= 240 ) && ( value <= 300 ) && ( ch->pcdata->illness < 1 ) )
        ch->pcdata->illness = 1;
*/
                        if (ch->pcdata->illness > 0)
                        {
                                switch (ch->pcdata->illness)
                                {
                                case 1:
                                        ch->endurance -= 140;
                                        send_to_char
                                                ("&P&WYour nose is runny\n\r",
                                                 ch);
                                        break;

                                case 2:
                                        ch->endurance -= 170;
                                        send_to_char
                                                ("&P&WYou have bad indigestion\n\r",
                                                 ch);
                                        interpret(ch, "burp");
                                        break;

                                case 3:
                                        ch->endurance -= 200;
                                        send_to_char
                                                ("&P&WYour throat is kind of scratchy.\n\r",
                                                 ch);
                                        interpret(ch, "cough");
                                        break;

                                case 4:
                                        ch->endurance -= 200;
                                        send_to_char
                                                ("&P&WYour sinuses are quite stuffy.\n\r",
                                                 ch);
                                        interpret(ch, "sneeze");
                                        break;

                                case 5:
                                        ch->endurance -= 300;
                                        ch->hit -= 10;
                                        send_to_char
                                                ("&P&WYou feel very nauseaus\n\r",
                                                 ch);
                                        if (number_percent() >= 25)
                                        {
                                                interpret(ch, "puke");
                                                ch->hit -= 30;
                                                gain_condition(ch, COND_FULL,
                                                               -2);
                                        }
                                        if (ch->position == POS_RESTING)
                                        {
                                                ch->endurance += 100;
                                                ch->hit += 5;
                                        }
                                        if (ch->position == POS_SLEEPING)
                                        {
                                                ch->endurance += 250;
                                                ch->hit += 10;
                                        }
                                        break;

                                case 6:
                                        ch->endurance -= 350;
                                        ch->hit -= 20;
                                        send_to_char
                                                ("&P&WYour throat burns like fire.\n\r",
                                                 ch);
                                        interpret(ch, "cough");
                                        gain_condition(ch, COND_THIRST, -3);
                                        if (ch->position == POS_RESTING)
                                        {
                                                ch->endurance += 100;
                                                ch->hit += 5;
                                        }
                                        if (ch->position == POS_SLEEPING)
                                        {
                                                ch->endurance += 250;
                                                ch->hit += 10;
                                        }
                                        break;

                                case 7:
                                        ch->endurance -= 400;
                                        ch->hit -= 40;
                                        send_to_char
                                                ("&P&WYour head aches and your sinuses feel ready to explode.\n\r",
                                                 ch);
                                        send_to_char
                                                ("&P&WYour stomach growls and your nausea is barely controlable.\n\r",
                                                 ch);
                                        if (number_percent() >= 50)
                                        {
                                                interpret(ch, "puke");
                                                ch->hit -= 10;
                                                gain_condition(ch, COND_FULL,
                                                               -1);
                                        }
                                        gain_condition(ch, COND_FULL, -1);
                                        if (ch->position == POS_RESTING)
                                        {
                                                ch->endurance += 100;
                                                ch->hit += 10;
                                        }
                                        if (ch->position == POS_SLEEPING)
                                        {
                                                ch->endurance += 250;
                                                ch->hit += 20;
                                        }
                                        break;

                                case 8:
                                        ch->endurance -= 200;
                                        ch->hit -= 5;
                                        send_to_char
                                                ("&P&WThere must be something in your eye, it burns.\n\r",
                                                 ch);
                                        break;

                                case 9:
                                        ch->endurance -= 200;
                                        ch->hit -= 5;
                                        send_to_char
                                                ("&P&WYour ears feel funny, and you find it hard to balance.\n\r",
                                                 ch);
                                        break;

                                case 10:
                                        ch->endurance -= 200;
                                        ch->hit -= 5;
                                        send_to_char
                                                ("&P&WYour throat feels like it is full of phlem.\n\r",
                                                 ch);
                                        break;

                                case 11:
                                        ch->endurance -= 400;
                                        ch->hit -= 90;
                                        send_to_char
                                                ("&P&WYour whole body feels week, as if you had been beaten.\n\r",
                                                 ch);
                                        gain_condition(ch, COND_FULL, -2);
                                        gain_condition(ch, COND_THIRST, -2);
                                        worsen_mental_state(ch, 1);
                                        if (ch->position == POS_RESTING)
                                        {
                                                ch->endurance += 100;
                                                ch->hit += 20;
                                        }
                                        if (ch->position == POS_SLEEPING)
                                        {
                                                ch->endurance += 250;
                                                ch->hit += 40;
                                        }
                                        if ((number_percent() <= 15)
                                            && (ch->position != POS_SLEEPING))
                                        {
                                                interpret(ch, "faint");
                                                ch->hit -= 10;
                                                ch->position = POS_SLEEPING;
                                        }
                                        break;

                                case 12:
                                        ch->endurance -= 200;
                                        ch->hit -= 40;
                                        send_to_char
                                                ("&P&WYou feel a sharp pain.\n\r",
                                                 ch);
                                        worsen_mental_state(ch, 1);
                                        value = (number_percent());
                                        if (value <= 15)
                                        {
                                        }
                                        if ((value >= 16) && (value <= 30))
                                        {
                                        }
                                        if ((value >= 31) && (value <= 45))
                                        {
                                        }
                                        if ((value >= 46) && (value <= 60))
                                        {
                                        }
                                        if ((value >= 61) && (value <= 75))
                                        {
                                        }
                                        if ((value >= 76) && (value <= 100))
                                        {
                                        }
                                        break;

                                case 13:
                                        ch->endurance -= 200;
                                        send_to_char
                                                ("&P&WYou don't feel like doing much.\n\r",
                                                 ch);
                                        worsen_mental_state(ch, 2);
                                        if (ch->position == POS_SLEEPING)
                                        {
                                                better_mental_state(ch, 2);
                                        }
                                        if (ch->position == POS_RESTING)
                                        {
                                                better_mental_state(ch, 1);
                                        }
                                        break;

                                case 14:
                                        ch->endurance -= 400;
                                        ch->hit -= 100;
                                        send_to_char
                                                ("&P&WYour whole body feels tired.\n\r",
                                                 ch);
                                        send_to_char
                                                ("&P&WPain jolts through your body for no aparent reason.\n\r",
                                                 ch);
                                        worsen_mental_state(ch, 2);
                                        gain_condition(ch, COND_FULL, -2);
                                        gain_condition(ch, COND_THIRST, -2);
                                        if (ch->position == POS_SLEEPING)
                                        {
                                                better_mental_state(ch, 2);
                                                gain_condition(ch, COND_FULL,
                                                               +2);
                                                gain_condition(ch,
                                                               COND_THIRST,
                                                               +2);
                                                ch->hit += 30;
                                                ch->endurance += 100;
                                        }
                                        if (ch->position == POS_RESTING)
                                        {
                                                gain_condition(ch, COND_FULL,
                                                               +1);
                                                gain_condition(ch,
                                                               COND_THIRST,
                                                               +1);
                                                better_mental_state(ch, 1);
                                                ch->hit += 15;
                                                ch->endurance += 50;
                                        }
                                        break;

                                case 15:
                                        ch->endurance -= 400;
                                        ch->hit -= 120;
                                        worsen_mental_state(ch, 2);
                                        gain_condition(ch, COND_FULL, -2);
                                        gain_condition(ch, COND_THIRST, -2);
                                        send_to_char
                                                ("&P&WYou feel as if your body is burning up.\n\r",
                                                 ch);
                                        send_to_char
                                                ("&P&WThe pain is excrutiating.\n\r",
                                                 ch);
                                        if (ch->position == POS_SLEEPING)
                                        {
                                                better_mental_state(ch, 2);
                                                gain_condition(ch, COND_FULL,
                                                               +2);
                                                gain_condition(ch,
                                                               COND_THIRST,
                                                               +2);
                                                ch->hit += 30;
                                                ch->endurance += 100;
                                        }
                                        if (ch->position == POS_RESTING)
                                        {
                                                gain_condition(ch, COND_FULL,
                                                               +1);
                                                gain_condition(ch,
                                                               COND_THIRST,
                                                               +1);
                                                better_mental_state(ch, 1);
                                                ch->hit += 15;
                                                ch->endurance += 50;
                                        }
                                        break;

                                default:
                                        break;
                                }
                        }
                }
                /*
                 * if (!IS_NPC(ch))
                 * fix_char(ch);
                 */

                /*
                 *  Do a room_prog rand check right off the bat
                 *   if ch disappears (rprog might wax npc's), continue
                 */
                if (!IS_NPC(ch))
                        rprog_random_trigger(ch);

                if (char_died(ch))
                        continue;

                if (IS_NPC(ch))
                        mprog_time_trigger(ch);

                if (char_died(ch))
                        continue;

                rprog_time_trigger(ch);

                if (char_died(ch))
                        continue;
                /*
                 * Check to ensure you don't magically free yourself from bindings. -Ulthrax 
                 */

                if ((binding = get_eq_char(ch, WEAR_BINDING)) != NULL
                    && !ch->held)
                {
                        ch->held = TRUE;
                        bug("%s is bound, but not held. Re-applying held state.\n\r", ch->name);
                }
                /*
                 * See if player should be auto-saved.
                 */
                if (!IS_NPC(ch)
                    && !NOT_AUTHED(ch)
                    && current_time - ch->save_time >
                    (sysdata.save_frequency * 60))
                        ch_save = ch;
                else
                        ch_save = NULL;

                if (ch->position >= POS_STUNNED)
                {
                        if (ch->hit < ch->max_hit)
                                ch->hit += hit_gain(ch);

                        /*
                         * Psst, wtf is this checking for force ability to be.. 1? that makes no sense... should ignore that - gavin 
                         */
                        if (ch->endurance < ch->max_endurance
                            /*
                             * || ch->skill_level[FORCE_ABILITY] == 1 
                             */ )
                                ch->endurance += mana_gain(ch);

                        if (ch->endurance < ch->max_endurance)
                                ch->endurance += move_gain(ch);
                }

                if (ch->position == POS_STUNNED)
                        update_pos(ch);

                if (ch->pcdata)
                        gain_addiction(ch);


                if (!IS_NPC(ch) && ch->top_level < LEVEL_IMMORTAL)
                {
                        OBJ_DATA *obj;

                        if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL
                            && obj->item_type == ITEM_LIGHT
                            && obj->value[2] > 0)
                        {
                                if (--obj->value[2] == 0 && ch->in_room)
                                {
                                        ch->in_room->light -= obj->count;
                                        act(AT_ACTION, "$p goes out.", ch,
                                            obj, NULL, TO_ROOM);
                                        act(AT_ACTION, "$p goes out.", ch,
                                            obj, NULL, TO_CHAR);

                                        extract_obj(obj);
                                }
                        }

                        if (str_cmp("droid", ch->race->name()))
                        {
                                if (ch->pcdata->condition[COND_DRUNK] > 8)
                                        worsen_mental_state(ch,
                                                            ch->pcdata->
                                                            condition
                                                            [COND_DRUNK] / 8);
                                if (ch->pcdata->condition[COND_FULL] > 1)
                                {
                                        switch (ch->position)
                                        {
                                        case POS_SLEEPING:
                                                better_mental_state(ch, 4);
                                                break;
                                        case POS_RESTING:
                                                better_mental_state(ch, 3);
                                                break;
                                        case POS_SITTING:
                                        case POS_MOUNTED:
                                                better_mental_state(ch, 2);
                                                break;
                                        case POS_STANDING:
                                                better_mental_state(ch, 1);
                                                break;
                                        case POS_FIGHTING:
                                                if (number_bits(2) == 0)
                                                        better_mental_state
                                                                (ch, 1);
                                                break;
                                        }
                                }
                                if (ch->pcdata->condition[COND_THIRST] > 1)
                                {
                                        switch (ch->position)
                                        {
                                        case POS_SLEEPING:
                                                better_mental_state(ch, 5);
                                                break;
                                        case POS_RESTING:
                                                better_mental_state(ch, 3);
                                                break;
                                        case POS_SITTING:
                                        case POS_MOUNTED:
                                                better_mental_state(ch, 2);
                                                break;
                                        case POS_STANDING:
                                                better_mental_state(ch, 1);
                                                break;
                                        case POS_FIGHTING:
                                                if (number_bits(2) == 0)
                                                        better_mental_state
                                                                (ch, 1);
                                                break;
                                        }
                                }
                                gain_condition(ch, COND_DRUNK, -1);
                                /*
                                 * OLC Races - some races will be better tan others now :D 
                                 */
                                gain_condition(ch, COND_FULL,-1 + (ch->race ? ch->race->hunger_mod() : 0));

                                if (ch->in_room) {
                                        switch (ch->in_room->sector_type)
                                        {
                                        default:
                                                gain_condition(ch,COND_THIRST,-1 +(ch->race ? ch->race->thirst_mod() : 0));
                                                break;
                                        case SECT_DESERT:
                                                gain_condition(ch,COND_THIRST,-2 +(ch->race ? ch->race->thirst_mod() : 0));
                                                break;
                                        case SECT_UNDERWATER:
                                        case SECT_OCEANFLOOR:
                                                if (number_bits(1) == 0)
                                                        gain_condition(ch,COND_THIRST,-1 +(ch->race ?ch->race->thirst_mod() : 0));
                                                break;
                                         }  
								}

                        }
                }

                if (!char_died(ch))
                {
                        /*
                         * Careful with the damages here,
                         *   MUST NOT refer to ch after damage taken,
                         *   as it may be lethal damage (on NPC).
                         */
                        if (IS_AFFECTED(ch, AFF_POISON))
                        {
                                act(AT_POISON, "$n shivers and suffers.", ch,
                                    NULL, NULL, TO_ROOM);
                                act(AT_POISON, "You shiver and suffer.", ch,
                                    NULL, NULL, TO_CHAR);
                                ch->mental_state =
                                        URANGE(20, ch->mental_state + 4, 100);
                                damage(ch, ch, 6, gsn_poison);
                        }
                        else if (ch->position == POS_INCAP)
                                damage(ch, ch, 1, TYPE_UNDEFINED);
                        else if (ch->position == POS_MORTAL)
                                damage(ch, ch, 4, TYPE_UNDEFINED);
                        if (char_died(ch))
                                continue;
                        if (ch->mental_state >= 30)
                                switch ((ch->mental_state + 5) / 10)
                                {
                                case 3:
                                        send_to_char("You feel feverish.\n\r",
                                                     ch);
                                        act(AT_ACTION,
                                            "$n looks kind of out of it.", ch,
                                            NULL, NULL, TO_ROOM);
                                        break;
                                case 4:
                                        send_to_char
                                                ("You do not feel well at all.\n\r",
                                                 ch);
                                        act(AT_ACTION,
                                            "$n doesn't look too good.", ch,
                                            NULL, NULL, TO_ROOM);
                                        break;
                                case 5:
                                        send_to_char("You need help!\n\r",
                                                     ch);
                                        act(AT_ACTION,
                                            "$n looks like $e could use your help.",
                                            ch, NULL, NULL, TO_ROOM);
                                        break;
                                case 6:
                                        send_to_char
                                                ("Seekest thou a cleric.\n\r",
                                                 ch);
                                        act(AT_ACTION,
                                            "Someone should fetch a healer for $n.",
                                            ch, NULL, NULL, TO_ROOM);
                                        break;
                                case 7:
                                        send_to_char
                                                ("You feel reality slipping away...\n\r",
                                                 ch);
                                        act(AT_ACTION,
                                            "$n doesn't appear to be aware of what's going on.",
                                            ch, NULL, NULL, TO_ROOM);
                                        break;
                                case 8:
                                        send_to_char
                                                ("You begin to understand... everything.\n\r",
                                                 ch);
                                        act(AT_ACTION,
                                            "$n starts ranting like a madman!",
                                            ch, NULL, NULL, TO_ROOM);
                                        break;
                                case 9:
                                        send_to_char
                                                ("You are ONE with the universe.\n\r",
                                                 ch);
                                        act(AT_ACTION,
                                            "$n is ranting on about 'the answer', 'ONE' and other mumbo-jumbo...",
                                            ch, NULL, NULL, TO_ROOM);
                                        break;
                                case 10:
                                        send_to_char
                                                ("You feel the end is near.\n\r",
                                                 ch);
                                        act(AT_ACTION,
                                            "$n is muttering and ranting in tongues...",
                                            ch, NULL, NULL, TO_ROOM);
                                        break;
                                }
                        if (ch->mental_state <= -30)
                                switch ((abs(ch->mental_state) + 5) / 10)
                                {
                                case 10:
                                        if (ch->position > POS_SLEEPING)
                                        {
                                                if ((ch->position ==
                                                     POS_STANDING
                                                     || ch->position <
                                                     POS_FIGHTING)
                                                    && number_percent() + 10 <
                                                    abs(ch->mental_state))
                                                        do_sleep(ch, "");
                                                else
                                                        send_to_char
                                                                ("You're barely conscious.\n\r",
                                                                 ch);
                                        }
                                        break;
                                case 9:
                                        if (ch->position > POS_SLEEPING)
                                        {
                                                if ((ch->position ==
                                                     POS_STANDING
                                                     || ch->position <
                                                     POS_FIGHTING)
                                                    && (number_percent() +
                                                        20) <
                                                    abs(ch->mental_state))
                                                        do_sleep(ch, "");
                                                else
                                                        send_to_char
                                                                ("You can barely keep your eyes open.\n\r",
                                                                 ch);
                                        }
                                        break;
                                case 8:
                                        if (ch->position > POS_SLEEPING)
                                        {
                                                if (ch->position < POS_SITTING
                                                    && (number_percent() +
                                                        30) <
                                                    abs(ch->mental_state))
                                                        do_sleep(ch, "");
                                                else
                                                        send_to_char
                                                                ("You're extremely drowsy.\n\r",
                                                                 ch);
                                        }
                                        break;
                                case 7:
                                        if (ch->position > POS_RESTING)
                                                send_to_char
                                                        ("You feel very unmotivated.\n\r",
                                                         ch);
                                        break;
                                case 6:
                                        if (ch->position > POS_RESTING)
                                                send_to_char
                                                        ("You feel sedated.\n\r",
                                                         ch);
                                        break;
                                case 5:
                                        if (ch->position > POS_RESTING)
                                                send_to_char
                                                        ("You feel sleepy.\n\r",
                                                         ch);
                                        break;
                                case 4:
                                        if (ch->position > POS_RESTING)
                                                send_to_char
                                                        ("You feel tired.\n\r",
                                                         ch);
                                        break;
                                case 3:
                                        if (ch->position > POS_RESTING)
                                                send_to_char
                                                        ("You could use a rest.\n\r",
                                                         ch);
                                        break;
                                }

                        if (ch->backup_wait > 0)
                        {
                                --ch->backup_wait;
                                /*
                                 * This is why you want to make sure to use brackets 
                                 */
                                if (IS_IMMORTAL(ch))
                                {
                                        snprintf(buf, MSL,
                                                 "time left for reins: %d",
                                                 ch->backup_wait);
                                        send_to_char(buf, ch);
                                }
                                if (ch->backup_wait == 0)
                                        add_reinforcements(ch);
                        }

                        if (!IS_NPC(ch))
                        {
                                if (++ch->timer > 15 && !ch->desc)
                                {
                                        EXT_BV    room_flags;
                                        ROOM_INDEX_DATA *room = ch->in_room;

                                        if (NOT_AUTHED(ch)
                                            && ch->pcdata->account)
                                        {
                                                del_from_account(ch->pcdata->
                                                                 account, ch);
                                        }
                                        if (room)
                                        {
                                                room_flags = room->room_flags;
                                                /*
                                                 * char_from_room(ch); 
                                                 */
                                        }
                                        else
                                        {
                                                char_to_room(ch,
                                                             get_room_index
                                                             (ROOM_PLUOGUS_QUIT));
                                        }
                                        if (!IS_IMMORTAL(ch))
                                                ch->position = POS_RESTING;
                                        ch->hit = UMAX(1, ch->hit);
                                        save_char_obj(ch);
                                        if (room)
                                                xSET_BIT(room->room_flags,
                                                         ROOM_HOTEL);
                                        do_quit(ch, "");
                                        if (room)
                                                room->room_flags = room_flags;
                                }
                                else if (ch == ch_save && IS_SET(sysdata.save_flags, SV_AUTO) && ++save_count < 10) /* save max of 10 per tick */
                                        save_char_obj(ch);
                        }
                }

        }

        return;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update(void)
{
        OBJ_DATA *obj;
        sh_int    AT_TEMP = 0;

        for (obj = last_object; obj; obj = gobj_prev)
        {
                CHAR_DATA *rch;
                char     *message;

                if (obj == first_object && obj->prev)
                {
                        bug("obj_update: first_object->prev != NULL... fixed",
                            0);
                        obj->prev = NULL;
                }
                gobj_prev = obj->prev;
                if (gobj_prev && gobj_prev->next != obj)
                {
                        bug("obj_update: obj->prev->next != obj", 0);
                        return;
                }
                set_cur_obj(obj);
                if (obj->carried_by)
                        oprog_random_trigger(obj);
                else if (obj->in_room && obj->in_room->area->nplayer > 0)
                        oprog_random_trigger(obj);

                if (obj_extracted(obj))
                        continue;

                if (obj->item_type == ITEM_WEAPON && obj->carried_by &&
                    (obj->wear_loc == WEAR_WIELD
                     || obj->wear_loc == WEAR_DUAL_WIELD)
                    && obj->value[3] != WEAPON_BLASTER && obj->value[4] > 0
                    && obj->value[3] != WEAPON_BOWCASTER
                    && obj->value[3] != WEAPON_FORCE_PIKE)
                {
                        obj->value[4]--;
                        if (obj->value[4] <= 0)
                        {
                                if (obj->value[3] == WEAPON_LIGHTSABER)
                                {
                                        act(AT_PLAIN, "$p fizzles and dies.",
                                            obj->carried_by, obj, NULL,
                                            TO_CHAR);
                                        act(AT_PLAIN,
                                            "$n's lightsaber fizzles and dies.",
                                            obj->carried_by, NULL, NULL,
                                            TO_ROOM);
                                }
                                else if (obj->value[3] == WEAPON_VIBRO_BLADE)
                                {
                                        act(AT_PLAIN, "$p stops vibrating.",
                                            obj->carried_by, obj, NULL,
                                            TO_CHAR);
                                }
                        }
                }

                if (obj->item_type == ITEM_PIPE)
                {
                        if (IS_SET(obj->value[3], PIPE_LIT))
                        {
                                if (--obj->value[1] <= 0)
                                {
                                        obj->value[1] = 0;
                                        REMOVE_BIT(obj->value[3], PIPE_LIT);
                                }
                                else if (IS_SET(obj->value[3], PIPE_HOT))
                                        REMOVE_BIT(obj->value[3], PIPE_HOT);
                                else
                                {
                                        if (IS_SET
                                            (obj->value[3], PIPE_GOINGOUT))
                                        {
                                                REMOVE_BIT(obj->value[3],
                                                           PIPE_LIT);
                                                REMOVE_BIT(obj->value[3],
                                                           PIPE_GOINGOUT);
                                        }
                                        else
                                                SET_BIT(obj->value[3],
                                                        PIPE_GOINGOUT);
                                }
                                if (!IS_SET(obj->value[3], PIPE_LIT))
                                        SET_BIT(obj->value[3],
                                                PIPE_FULLOFASH);
                        }
                        else
                                REMOVE_BIT(obj->value[3], PIPE_HOT);
                }   /* Corpse decay (npc corpses decay at 8 times the rate of pc corpses) - Narn */

                if (obj->item_type == ITEM_CORPSE_PC
                    || obj->item_type == ITEM_CORPSE_NPC
                    || obj->item_type == ITEM_DROID_CORPSE)
                {
                        sh_int    timerfrac = UMAX(1, obj->timer - 1);

                        if (obj->item_type == ITEM_CORPSE_PC)
                                timerfrac = (int) (obj->timer / 8 + 1);

                        if (obj->timer > 0 && obj->value[2] > timerfrac)
                        {
                                char      buf[MAX_STRING_LENGTH];
                                char      name[MAX_STRING_LENGTH];
                                char     *bufptr;

                                bufptr = one_argument(obj->short_descr, name);
                                bufptr = one_argument(bufptr, name);
                                bufptr = one_argument(bufptr, name);

                                separate_obj(obj);
                                obj->value[2] = timerfrac;
                                if (obj->item_type == ITEM_DROID_CORPSE)
                                        snprintf(buf, MSL,
                                                 d_corpse_descs[UMIN
                                                                (timerfrac -
                                                                 1, 4)],
                                                 bufptr);
                                else
                                        snprintf(buf, MSL,
                                                 corpse_descs[UMIN
                                                              (timerfrac - 1,
                                                               4)],
                                                 capitalize(bufptr));

                                STRFREE(obj->description);
                                obj->description = STRALLOC(buf);
                        }
                }

                /*
                 * don't let inventory decay 
                 */
                if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
                        continue;

                if (obj->timer > 0 && obj->timer < 5
                    && obj->item_type == ITEM_ARMOR)
                {
                        if (obj->carried_by)
                        {
                                act(AT_TEMP, "$p is almost dead.",
                                    obj->carried_by, obj, NULL, TO_CHAR);
                        }
                }

                if ((obj->timer <= 0 || --obj->timer > 0))
                        continue;


                /*
                 * if we get this far, object's timer has expired. 
                 */

                AT_TEMP = AT_PLAIN;
                switch (obj->item_type)
                {
                default:
                        message = "$p has depleted itself.";
                        AT_TEMP = AT_PLAIN;
                        break;

                case ITEM_BEACON:
                        make_scraps(obj);
                        message = "$p runs out of power and self destructs.";
                        AT_TEMP = AT_OBJECT;
                        break;
                case ITEM_GRENADE:
                        explode(obj);
                        return;
                        break;

                case ITEM_PORTAL:
                        message = "$p winks out of existence.";
                        remove_portal(obj);
                        obj->item_type = ITEM_TRASH;    /* so extract_obj    */
                        AT_TEMP = AT_MAGIC; /* doesn't remove_portal */
                        break;
                case ITEM_FOUNTAIN:
                        message = "$p dries up.";
                        AT_TEMP = AT_BLUE;
                        break;
                case ITEM_CORPSE_NPC:
                        message = "$p decays into dust and blows away.";
                        AT_TEMP = AT_OBJECT;
                        break;
                case ITEM_DROID_CORPSE:
                        message = "$p rusts away into oblivion.";
                        AT_TEMP = AT_OBJECT;
                        break;
                case ITEM_CORPSE_PC:
                        message = "$p decays into dust and is blown away...";
                        AT_TEMP = AT_MAGIC;
                        break;
                case ITEM_FOOD:
                        message = "$p is devoured by a swarm of maggots.";
                        AT_TEMP = AT_HUNGRY;
                        break;
                case ITEM_BLOOD:
                        message = "$p slowly seeps into the ground.";
                        AT_TEMP = AT_BLOOD;
                        break;
                case ITEM_BLOODSTAIN:
                        message = "$p dries up into flakes and blows away.";
                        AT_TEMP = AT_BLOOD;
                        break;
                case ITEM_SCRAPS:
                        message = "$p crumbles and decays into nothing.";
                        AT_TEMP = AT_OBJECT;
                        break;
                case ITEM_FIRE:
                        if (obj->in_room)
                                --obj->in_room->light;
                        message = "$p burns out.";
                        AT_TEMP = AT_FIRE;
                }

                if (obj->carried_by)
                {
                        act(AT_TEMP, message, obj->carried_by, obj, NULL,
                            TO_CHAR);
                }
                else if (obj->in_room
                         && (rch = obj->in_room->first_person) != NULL
                         && !IS_OBJ_STAT(obj, ITEM_BURRIED))
                {
                        act(AT_TEMP, message, rch, obj, NULL, TO_ROOM);
                        act(AT_TEMP, message, rch, obj, NULL, TO_CHAR);
                }
                if (obj->item_type != ITEM_BEACON)
                        extract_obj(obj);
        }
        return;
}   /*
     * Function to check important stuff happening to a player
     * This function should take about 5% of mud cpu time
     */
void char_check(void)
{
        CHAR_DATA *ch, *ch_next;
        EXIT_DATA *pexit;
        static int cnt = 0;
        int       door, retcode;

        cnt = (cnt + 1) % 2;

        for (ch = first_char; ch; ch = ch_next)
        {
                ch_next = ch->next;
                will_fall(ch, 0);

                if (char_died(ch))
                        continue;

                if (IS_NPC(ch))
                {
                        if (cnt != 0)
                                continue;

                        /*
                         * running mobs -Thoric 
                         */
                        if (!IS_SET(ch->act, ACT_SENTINEL)
                            && !ch->fighting && ch->hunting)
                        {
                                WAIT_STATE(ch, 2 * PULSE_VIOLENCE);
                                hunt_victim(ch);
                                continue;
                        }

                        if (ch->spec_fun)
                        {
                                if ((*ch->spec_fun) (ch))
                                        continue;
                                if (char_died(ch))
                                        continue;
                        }
                        if (ch->spec_2)
                        {
                                if ((*ch->spec_2) (ch))
                                        continue;
                                if (char_died(ch))
                                        continue;
                        }

                        if (!IS_SET(ch->act, ACT_SENTINEL)
                            && !IS_SET(ch->act, ACT_PROTOTYPE)
                            && !ch->held
                            && (door = number_bits(4)) <= 9
                            && (pexit = get_exit(ch->in_room, door)) != NULL
                            && pexit->to_room
                            && !IS_SET(pexit->exit_info, EX_CLOSED)
                            && !xIS_SET(pexit->to_room->room_flags,
                                        ROOM_NO_MOB)
                            && (!IS_SET(ch->act, ACT_STAY_AREA)
                                || pexit->to_room->area == ch->in_room->area))
                        {
                                retcode = move_char(ch, pexit, 0, FALSE);
                                if (char_died(ch))
                                        continue;
                                if (retcode != rNONE
                                    || IS_SET(ch->act, ACT_SENTINEL)
                                    || ch->position < POS_STANDING)
                                        continue;
                        }
                        continue;
                }
                else
                {
                        if (ch->mount && ch->in_room != ch->mount->in_room)
                        {
                                REMOVE_BIT(ch->mount->act, ACT_MOUNTED);
                                ch->mount = NULL;
                                ch->position = POS_STANDING;
                                send_to_char
                                        ("No longer upon your mount, you fall to the ground...\n\rOUCH!\n\r",
                                         ch);
                        }

                        if ((ch->in_room
                             && ch->in_room->sector_type == SECT_UNDERWATER)
                            || (ch->in_room
                                && ch->in_room->sector_type ==
                                SECT_OCEANFLOOR))
                        {
                                if (!IS_AFFECTED(ch, AFF_AQUA_BREATH))
                                {
                                        if (get_trust(ch) < LEVEL_IMMORTAL)
                                        {
                                                int       dam;


                                                dam = number_range(ch->
                                                                   max_hit /
                                                                   50,
                                                                   ch->
                                                                   max_hit /
                                                                   30);
                                                dam = UMAX(1, dam);
                                                if (ch->hit <= 0)
                                                        dam = UMIN(10, dam);
                                                if (number_bits(3) == 0)
                                                        send_to_char
                                                                ("You cough and choke as you try to breathe water!\n\r",
                                                                 ch);
                                                damage(ch, ch, dam,
                                                       TYPE_UNDEFINED);
                                        }
                                }
                        }

                        if (char_died(ch))
                                continue;

                        if (ch->in_room
                            &&
                            ((ch->in_room->sector_type == SECT_WATER_NOSWIM)
                             || (ch->in_room->sector_type ==
                                 SECT_WATER_SWIM)))
                        {
                                if (!IS_AFFECTED(ch, AFF_FLYING)
                                    && !IS_AFFECTED(ch, AFF_FLOATING)
                                    && !IS_AFFECTED(ch, AFF_AQUA_BREATH)
                                    && !ch->mount)
                                {
                                        if (get_trust(ch) < LEVEL_IMMORTAL)
                                        {
                                                int       dam;

                                                if (ch->endurance > 0)
                                                        ch->endurance--;
                                                else
                                                {
                                                        dam = number_range
                                                                (ch->max_hit /
                                                                 50,
                                                                 ch->max_hit /
                                                                 30);
                                                        dam = UMAX(1, dam);
                                                        if (ch->hit <= 0)
                                                                dam = UMIN(10,
                                                                           dam);
                                                        if (number_bits(3) ==
                                                            0)
                                                                send_to_char
                                                                        ("Struggling with exhaustion, you choke on a mouthful of water.\n\r",
                                                                         ch);
                                                        damage(ch, ch, dam,
                                                               TYPE_UNDEFINED);
                                                }
                                        }
                                }
                        }

                }
        }
}   /*
     * Aggress.
     *
     * for each descriptor
     *     for each mob in room
     *         aggress on some random PC
     *
     * This function should take 5% to 10% of ALL mud cpu time.
     * Unfortunately, checking on each PC move is too tricky,
     *   because we don't the mob to just attack the first PC
     *   who leads the party into the room.
     *
     */
void aggr_update(void)
{
        DESCRIPTOR_DATA *d, *dnext;
        CHAR_DATA *wch;
        CHAR_DATA *ch;
        CHAR_DATA *ch_next;
        CHAR_DATA *victim;
        struct act_prog_data *apdtmp;

#ifdef UNDEFD
        /*
         *  GRUNT!  To do
         *
         */
        if (IS_NPC(wch) && wch->mpactnum > 0
            && wch->in_room->area->nplayer > 0)
        {
                MPROG_ACT_LIST *tmp_act, *tmp2_act;

                for (tmp_act = wch->mpact; tmp_act; tmp_act = tmp_act->next)
                {
                        oprog_wordlist_check(tmp_act->buf, wch, tmp_act->ch,
                                             tmp_act->obj, tmp_act->vo,
                                             ACT_PROG);
                        DISPOSE(tmp_act->buf);
                }
                for (tmp_act = wch->mpact; tmp_act; tmp_act = tmp2_act)
                {
                        tmp2_act = tmp_act->next;
                        DISPOSE(tmp_act);
                }
                wch->mpactnum = 0;
                wch->mpact = NULL;
        }
#endif

        /*
         * check mobprog act queue 
         */
        while ((apdtmp = mob_act_list) != NULL)
        {
                wch = (CHAR_DATA *) mob_act_list->vo;
                if (!char_died(wch) && wch->mpactnum > 0)
                {
                        MPROG_ACT_LIST *tmp_act;

                        while ((tmp_act = wch->mpact) != NULL)
                        {
                                if (tmp_act->obj
                                    && obj_extracted(tmp_act->obj))
                                        tmp_act->obj = NULL;
                                if (tmp_act->ch && !char_died(tmp_act->ch))
                                        mprog_wordlist_check(tmp_act->buf,
                                                             wch, tmp_act->ch,
                                                             tmp_act->obj,
                                                             tmp_act->vo,
                                                             ACT_PROG);
                                wch->mpact = tmp_act->next;
                                DISPOSE(tmp_act->buf);
                                DISPOSE(tmp_act);
                        }
                        wch->mpactnum = 0;
                        wch->mpact = NULL;
                }
                mob_act_list = apdtmp->next;
                DISPOSE(apdtmp);
        }   /*
             * Just check descriptors here for victims to aggressive mobs
             * We can check for linkdead victims to mobile_update  -Thoric
             */
        for (d = first_descriptor; d; d = dnext)
        {
                dnext = d->next;
                if (d->connected != CON_PLAYING
                    || (wch = d->character) == NULL)
                        continue;

                if (char_died(wch)
                    || IS_NPC(wch)
                    || wch->top_level >= LEVEL_IMMORTAL || !wch->in_room)
                        continue;

                for (ch = wch->in_room->first_person; ch; ch = ch_next)
                {
                        int       count = 0;

                        ch_next = ch->next_in_room;

                        if (!IS_NPC(ch)
                            || ch->fighting
                            || IS_AFFECTED(ch, AFF_CHARM)
                            || !IS_AWAKE(ch)
                            || (IS_SET(ch->act, ACT_WIMPY))
                            || !can_see(ch, wch) || ch->position != 8)
                                continue;

                        if (is_hating(ch, wch) && ch->position == 8)
                        {
                                found_prey(ch, wch);
                                continue;
                        }

                        if (!IS_SET(ch->act, ACT_AGGRESSIVE)
                            || IS_SET(ch->act, ACT_MOUNTED)
                            || xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
                                continue;

                        victim = wch;

                        if (!victim)
                        {
                                bug("Aggr_update: null victim.", count);
                                continue;
                        }

                        if (get_timer(victim, TIMER_RECENTFIGHT) > 0)
                                continue;

                        if (IS_NPC(ch) && IS_SET(ch->attacks, ATCK_BACKSTAB))
                        {
                                OBJ_DATA *obj;

                                if (!ch->mount
                                    && (obj =
                                        get_eq_char(ch, WEAR_WIELD)) != NULL
                                    && obj->value[3] == 11
                                    && !victim->fighting
                                    && victim->hit >= victim->max_hit)
                                {
                                        WAIT_STATE(ch,
                                                   skill_table[gsn_backstab]->
                                                   beats);
                                        if (!IS_AWAKE(victim)
                                            || number_percent() + 5 <
                                            ch->top_level)
                                        {
                                                global_retcode =
                                                        multi_hit(ch, victim,
                                                                  gsn_backstab);
                                                continue;
                                        }
                                        else
                                        {
                                                global_retcode =
                                                        damage(ch, victim, 0,
                                                               gsn_backstab);
                                                continue;
                                        }
                                }
                        }
                        global_retcode =
                                multi_hit(ch, victim, TYPE_UNDEFINED);
                }
        }

        return;
}

/* From interp.c */
bool check_social args((CHAR_DATA * ch, char *command, char *argument));

/*
 * drunk randoms	- Tricops
 * (Made part of mobile_update	-Thoric)
 */
void drunk_randoms(CHAR_DATA * ch)
{
        CHAR_DATA *rvch = NULL;
        CHAR_DATA *vch;
        sh_int    drunk;
        sh_int    position;

        if (IS_NPC(ch) || ch->pcdata->condition[COND_DRUNK] <= 0)
                return;

        if (number_percent() < 30)
                return;

        drunk = ch->pcdata->condition[COND_DRUNK];
        position = ch->position;
        ch->position = POS_STANDING;

        if (number_percent() < (2 * drunk / 20))
                check_social(ch, "burp", "");
        else if (number_percent() < (2 * drunk / 20))
                check_social(ch, "hiccup", "");
        else if (number_percent() < (2 * drunk / 20))
                check_social(ch, "drool", "");
        else if (number_percent() < (2 * drunk / 20))
                check_social(ch, "fart", "");
        else if (drunk > (10 + (get_curr_con(ch) / 5))
                 && number_percent() < (2 * drunk / 18))
        {
                for (vch = ch->in_room->first_person; vch;
                     vch = vch->next_in_room)
                        if (number_percent() < 10)
                                rvch = vch;
                check_social(ch, "puke", (rvch ? rvch->name : (char *) ""));
        }

        ch->position = position;
        return;
}

void halucinations(CHAR_DATA * ch)
{
        if (ch->mental_state >= 30
            && number_bits(5 - (ch->mental_state >= 50) -
                           (ch->mental_state >= 75)) == 0)
        {
                char     *t;

                switch (number_range(1, UMIN(20, (ch->mental_state + 5) / 5)))
                {
                default:
                case 1:
                        t = "You feel very restless... you can't sit still.\n\r";
                        break;
                case 2:
                        t = "You're tingling all over.\n\r";
                        break;
                case 3:
                        t = "Your skin is crawling.\n\r";
                        break;
                case 4:
                        t = "You suddenly feel that something is terribly wrong.\n\r";
                        break;
                case 5:
                        t = "Those damn little fairies keep laughing at you!\n\r";
                        break;
                case 6:
                        t = "You can hear your mother crying...\n\r";
                        break;
                case 7:
                        t = "Have you been here before, or not?  You're not sure...\n\r";
                        break;
                case 8:
                        t = "Painful childhood memories flash through your mind.\n\r";
                        break;
                case 9:
                        t = "You hear someone call your name in the distance...\n\r";
                        break;
                case 10:
                        t = "Your head is pulsating... you can't think straight.\n\r";
                        break;
                case 11:
                        t = "The ground... seems to be squirming...\n\r";
                        break;
                case 12:
                        t = "You're not quite sure what is real anymore.\n\r";
                        break;
                case 13:
                        t = "It's all a dream... or is it?\n\r";
                        break;
                case 14:
                        t = "They're coming to get you... coming to take you away...\n\r";
                        break;
                case 15:
                        t = "You begin to feel all powerful!\n\r";
                        break;
                case 16:
                        t = "You're light as air... the heavens are yours for the taking.\n\r";
                        break;
                case 17:
                        t = "Your whole life flashes by... and your future...\n\r";
                        break;
                case 18:
                        t = "You are everywhere and everything... you know all and are all!\n\r";
                        break;
                case 19:
                        t = "You feel immortal!\n\r";
                        break;
                case 20:
                        t = "Ahh... the power of a Supreme Entity... what to do...\n\r";
                        break;
                }
                send_to_char(t, ch);
        }
        return;
}

void auth_update(void)
{
        CHAR_DATA *victim;
        DESCRIPTOR_DATA *d;
        char      buf[MAX_INPUT_LENGTH], log_buffer[MAX_INPUT_LENGTH];
        bool      found_hit = FALSE;    /* was at least one found? */

        mudstrlcpy(log_buffer, "Pending authorizations:\n\r", MIL);
        for (d = first_descriptor; d; d = d->next)
        {
                if ((victim = d->character) && IS_WAITING_FOR_AUTH(victim))
                {
                        found_hit = TRUE;
                        snprintf(buf, MSL, " %s@%s new %s %s %s\n\r",
                                 victim->name, victim->desc->host,
                                 victim->sex ==
                                 SEX_MALE ? "Male" : victim->sex ==
                                 SEX_FEMALE ? "Female" : "Neutra",
                                 victim->race->name(),
                                 ability_name[victim->main_ability]);
                        mudstrlcat(log_buffer, buf, MIL);
                }
        }
        if (found_hit)
        {
                log_string(log_buffer);
        }
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler(void)
{
        static int pulse_crashover;
        static int pulse_auth;
        struct timeval systime;
        struct timeval etime;

        if (timechar)
        {
                set_char_color(AT_PLAIN, timechar);
                send_to_char("Starting update timer.\n\r", timechar);
                gettimeofday(&systime, NULL);
        }

        if (--sysdata.pulse_area <= 0)  /* > */
        {
                sysdata.pulse_area =
                        number_range(PULSE_AREA / 2, 3 * PULSE_AREA / 2);
                area_update();
                quest_update();
                bacta_update();
                battalion_update();
        }

        if (--sysdata.pulse_taxes <= 0)
        {
                sysdata.pulse_taxes = PULSE_TAXES;
                update_orbit();
                update_taxes();
                update_salaries();
                update_baccounts();
        }

        if (--sysdata.pulse_mobile <= 0)
        {
                sysdata.pulse_mobile = PULSE_MOBILE;
                mobile_update();
        }

        if (--sysdata.pulse_space <= 0)
        {
                sysdata.pulse_space = PULSE_SPACE;
                update_shuttle();
                update_space();
                do_who(NULL, "");
        }

        if (--sysdata.pulse_recharge <= 0)
        {
                sysdata.pulse_recharge = PULSE_SPACE / 3;
                recharge_ships();
        }

        if (--sysdata.pulse_ship <= 0)
        {
                sysdata.pulse_ship = PULSE_SPACE / 10;
                move_ships();
        }

        if (--sysdata.pulse_violence <= 0)
        {
                sysdata.pulse_violence = PULSE_VIOLENCE;
                arena_update();
                violence_update();
        }

        if (--sysdata.pulse_point <= 0)
        {
                sysdata.pulse_point =
                        number_range((int) (PULSE_TICK * 0.75),
                                     (int) (PULSE_TICK * 1.25));
                weather_update();
                char_update();
                obj_update();
        }

        /*
         * This seems wrong to me, but i can't figure out why - Gavin 
         */
        if (--pulse_crashover <= 0)
        {
                pulse_crashover = PULSE_CRASHOVER;
        }
        else if (--pulse_crashover <= 0)
        {
                pulse_crashover = PULSE_CRASHOVER;
                init_crashover();
        }

        if (--pulse_auth <= 0)
        {
                pulse_auth = PULSE_AUTH;
                auth_update();  /* Gorog */
        }

        if (--sysdata.pulse_second <= 0)
        {
                sysdata.pulse_second = PULSE_PER_SECOND;
                char_check();
#ifdef OLC_HOMES
                save_homes_check();
#endif
                check_pfiles(0);
                check_dns();

                /*
                 * reboot_check( "" ); Disabled to check if its lagging a lot - Scryn
                 */
                /*
                 * Much faster version enabled by Altrag..
                 * * although I dunno how it could lag too much, it was just a bunch
                 * * of comparisons.. 
                 */
                reboot_check(0);

                fireplanet_update();
        }

        if (auction->item && --auction->pulse <= 0)
        {
                auction->pulse = PULSE_AUCTION;
                auction_update();
        }

        mpsleep_update();   /* Check for sleeping mud progs -rkb */
        aggr_update();
        obj_act_update();
        room_act_update();
        clean_obj_queue();  /* dispose of extracted objects */
        clean_char_queue(); /* dispose of dead mobs/quitting chars */
        if (timechar)
        {
                gettimeofday(&etime, NULL);
                set_char_color(AT_PLAIN, timechar);
                send_to_char("Update timing complete.\n\r", timechar);
                subtract_times(&etime, &systime);
                ch_printf(timechar, "Timing took %d.%06d seconds.\n\r",
                          etime.tv_sec, etime.tv_usec);
                timechar = NULL;
        }
        tail_chain();
        return;
}


void remove_portal(OBJ_DATA * portal)
{
        ROOM_INDEX_DATA *fromRoom, *toRoom;
        CHAR_DATA *ch;
        EXIT_DATA *pexit;
        bool      found;

        if (!portal)
        {
                bug("remove_portal: portal is NULL", 0);
                return;
        }

        fromRoom = portal->in_room;
        found = FALSE;
        if (!fromRoom)
        {
                bug("remove_portal: portal->in_room is NULL", 0);
                return;
        }

        for (pexit = fromRoom->first_exit; pexit; pexit = pexit->next)
                if (IS_SET(pexit->exit_info, EX_PORTAL))
                {
                        found = TRUE;
                        break;
                }

        if (!found)
        {
                bug("remove_portal: portal not found in room %d!",
                    fromRoom->vnum);
                return;
        }

        if (pexit->vdir != DIR_PORTAL)
                bug("remove_portal: exit in dir %d != DIR_PORTAL",
                    pexit->vdir);

        if ((toRoom = pexit->to_room) == NULL)
                bug("remove_portal: toRoom is NULL", 0);

        extract_exit(fromRoom, pexit);
        /*
         * rendunancy 
         */
        /*
         * send a message to fromRoom 
         */
        /*
         * ch = fromRoom->first_person; 
         */
        /*
         * if(ch!=NULL) 
         */
        /*
         * act( AT_PLAIN, "A magical portal below winks from existence.", ch, NULL, NULL, TO_ROOM ); 
         */

        /*
         * send a message to toRoom 
         */
        if (toRoom && (ch = toRoom->first_person) != NULL)
                act(AT_PLAIN, "A magical portal above winks from existence.",
                    ch, NULL, NULL, TO_ROOM);

        /*
         * remove the portal obj: looks better to let update_obj do this 
         */
        /*
         * extract_obj(portal);  
         */

        return;
}

void reboot_check(time_t reset)
{
        static char *tmsg[] = { "SYSTEM: Reboot in 10 seconds.",
                "SYSTEM: Reboot in 30 seconds.",
                "SYSTEM: Reboot in 1 minute.",
                "SYSTEM: Reboot in 2 minutes.",
                "SYSTEM: Reboot in 3 minutes.",
                "SYSTEM: Reboot in 4 minutes.",
                "SYSTEM: Reboot in 5 minutes.",
                "SYSTEM: Reboot in 10 minutes.",
        };
        static const int times[] = { 10, 30, 60, 120, 180, 240, 300, 600 };
        static const int timesize = UMIN(sizeof(times) / sizeof(*times),
                                         sizeof(tmsg) / sizeof(*tmsg));
        char      buf[MAX_STRING_LENGTH];
        static int trun;
        static bool init;

        if (!init || reset >= current_time)
        {
                for (trun = timesize - 1; trun >= 0; trun--)
                        if (reset >= current_time + times[trun])
                                break;
                init = TRUE;
                return;
        }

        if ((current_time % 1800) == 0)
        {
                snprintf(buf, MSL, "%.24s: %d players", ctime(&current_time),
                         num_descriptors);
                append_to_file(USAGE_FILE, buf);
        }

        if (new_boot_time_t - boot_time < 60 * 60 * 18 &&
            !set_boot_time->manual)
                return;

        if (new_boot_time_t <= current_time)
        {
                CHAR_DATA *vch;
                extern bool mud_down;

                if (auction->item)
                {
                        snprintf(buf, MSL,
                                 "Sale of %s has been stopped by mud.",
                                 auction->item->short_descr);
                        talk_auction(buf);
                        obj_to_char(auction->item, auction->seller);
                        if (auction->item->item_type == ITEM_BEACON)
                                auction->item->value[0] = 0;
                        auction->item = NULL;
                        if (auction->buyer
                            && auction->buyer != auction->seller)
                        {
                                auction->buyer->gold += auction->bet;
                                send_to_char
                                        ("Your money has been returned.\n\r",
                                         auction->buyer);
                        }
                }
                echo_to_all(AT_YELLOW,
                            "You are forced from these realms by a strong "
                            "presence\n\ras life here is reconstructed.",
                            ECHOTAR_ALL);

                for (vch = first_char; vch; vch = vch->next)
                        if (!IS_NPC(vch))
                                save_char_obj(vch);
                mud_down = TRUE;
                return;
        }

        if (trun != -1 && new_boot_time_t - current_time <= times[trun])
        {
                echo_to_all(AT_YELLOW, tmsg[trun], ECHOTAR_ALL);
                if (trun <= 5)
                        sysdata.DENY_NEW_PLAYERS = TRUE;
                --trun;
                return;
        }
        return;
}


/* the auction update*/

void auction_update(void)
{
        int       tax, pay;
        char      buf[MAX_STRING_LENGTH];

        switch (++auction->going)   /* increase the going state */
        {
        case 1:    /* going once */
        case 2:    /* going twice */
                if (auction->bet > auction->starting)
                        snprintf(buf, MSL, "%s: going %s for %d.",
                                 auction->item->short_descr,
                                 ((auction->going == 1) ? "once" : "twice"),
                                 auction->bet);
                else
                        snprintf(buf, MSL,
                                 "%s: going %s (bid not received yet).",
                                 auction->item->short_descr,
                                 ((auction->going == 1) ? "once" : "twice"));

                talk_auction(buf);
                break;

        case 3:    /* SOLD! */
                if (!auction->buyer && auction->bet)
                {
                        bug("Auction code reached SOLD, with NULL buyer, but %d gold bid", auction->bet);
                        auction->bet = 0;
                }
                if (auction->bet > 0 && auction->buyer != auction->seller)
                {
                        snprintf(buf, MSL, "%s sold to %s for %d.",
                                 auction->item->short_descr,
                                 IS_NPC(auction->buyer) ? auction->buyer->
                                 short_descr : auction->buyer->name,
                                 auction->bet);
                        talk_auction(buf);

                        act(AT_ACTION,
                            "The auctioneer materializes before you, and hands you $p.",
                            auction->buyer, auction->item, NULL, TO_CHAR);
                        act(AT_ACTION,
                            "The auctioneer materializes before $n, and hands $m $p.",
                            auction->buyer, auction->item, NULL, TO_ROOM);

                        if ((auction->buyer->carry_weight
                             + get_obj_weight(auction->item))
                            > can_carry_w(auction->buyer))
                        {
                                act(AT_PLAIN,
                                    "$p is too heavy for you to carry with your current inventory.",
                                    auction->buyer, auction->item, NULL,
                                    TO_CHAR);
                                act(AT_PLAIN,
                                    "$n is carrying too much to also carry $p, and $e drops it.",
                                    auction->buyer, auction->item, NULL,
                                    TO_ROOM);
                                obj_to_room(auction->item,
                                            auction->buyer->in_room);
                        }
                        else
                                obj_to_char(auction->item, auction->buyer);
                        pay = (int) (auction->bet * 0.9);
                        tax = (int) (auction->bet * 0.1);
                        boost_economy(auction->seller->in_room->area, tax);
                        auction->seller->gold += pay;   /* give him the money, tax 10 % */
                        snprintf(buf, MSL,
                                 "The auctioneer pays you %d gold, charging an auction fee of %d.\n\r",
                                 pay, tax);
                        send_to_char(buf, auction->seller);
                        auction->item = NULL;   /* reset item */
                        if (IS_SET(sysdata.save_flags, SV_AUCTION))
                        {
                                save_char_obj(auction->buyer);
                                save_char_obj(auction->seller);
                        }
                }
                else    /* not sold */
                {
                        snprintf(buf, MSL,
                                 "No bids received for %s - object has been removed from auction\n\r.",
                                 auction->item->short_descr);
                        talk_auction(buf);
                        act(AT_ACTION,
                            "The auctioneer appears before you to return $p to you.",
                            auction->seller, auction->item, NULL, TO_CHAR);
                        act(AT_ACTION,
                            "The auctioneer appears before $n to return $p to $m.",
                            auction->seller, auction->item, NULL, TO_ROOM);
                        if ((auction->seller->carry_weight +
                             get_obj_weight(auction->item)) >
                            can_carry_w(auction->seller))
                        {
                                act(AT_PLAIN,
                                    "You drop $p as it is just too much to carry"
                                    " with everything else you're carrying.",
                                    auction->seller, auction->item, NULL,
                                    TO_CHAR);
                                act(AT_PLAIN,
                                    "$n drops $p as it is too much extra weight"
                                    " for $m with everything else.",
                                    auction->seller, auction->item, NULL,
                                    TO_ROOM);
                                obj_to_room(auction->item,
                                            auction->seller->in_room);
                        }
                        else
                                obj_to_char(auction->item, auction->seller);
                        tax = (int) (auction->item->cost * 0.05);
                        boost_economy(auction->seller->in_room->area, tax);
                        snprintf(buf, MSL,
                                 "The auctioneer charges you an auction fee of %d.\n\r",
                                 tax);
                        send_to_char(buf, auction->seller);
                        if ((auction->seller->gold - tax) < 0)
                                auction->seller->gold = 0;
                        else
                                auction->seller->gold -= tax;
                        if (IS_SET(sysdata.save_flags, SV_AUCTION))
                                save_char_obj(auction->seller);
                }   /* else */
                auction->item = NULL;   /* clear auction */
        }   /* switch */
}   /* func */

void subtract_times(struct timeval *etime, struct timeval *systime)
{
        etime->tv_sec -= systime->tv_sec;
        etime->tv_usec -= systime->tv_usec;
        while (etime->tv_usec < 0)
        {
                etime->tv_usec += 1000000;
                etime->tv_sec--;
        }
        return;
}

void bacta_update(void)
{
        CHAR_DATA *ch;

        for (ch = first_char; ch != NULL; ch = ch->next)
        {
                if (IS_NPC(ch))
                        continue;

                if (xIS_SET(ch->in_room->room_flags, ROOM_BACTA))
                {
                        send_to_char
                                ("You feel the soothing bubbles of the bacta!\n\rYou are healed.\n\r",
                                 ch);

                        ch->hit += 100;
                        if (ch->hit > ch->max_hit)
                        {
                                ch->hit = ch->max_hit;
                        }
                }

                if (xIS_SET(ch->in_room->room_flags, ROOM_BACTA_CHARGE))
                {

                        if (ch->gold < 25)
                        {
                                send_to_char
                                        ("You do not have the money to pay for the bacta.\n\r",
                                         ch);
                                continue;
                        }
                        send_to_char
                                ("You pay the fee for the bacta tank.\n\rYou feel the soothing bubbles of the bacta!\n\rYou are healed.\n\r",
                                 ch);

                        ch->hit += 100;

                        ch->gold -= 25;
                        if (ch->hit > ch->max_hit)
                                ch->hit = ch->max_hit;
                }
        }
        return;
}

void battalion_update(void)
{
        PLANET_DATA *planet;
        int       attchange, defchange, attmult, defmult;
        CLAN_DATA *clan;
        CLAN_DATA *aclan;
        CLAN_DATA *attacker;
        AREA_DATA *area;
        CHAR_DATA *ch;
        char      buf[MAX_STRING_LENGTH];
        INSTALLATION_DATA *installation;

        for (planet = first_planet; planet; planet = planet->next)
        {
                if (planet->attgovern && (planet->attbattalions > 0))
                {
                        clan = planet->governed_by;
                        attacker = planet->attgovern;
                        attchange = 0;
                        defchange = 0;
                        attmult = 1;
                        defmult = 1;

                        if (!clan)
                        {
                                planet->defbattalions = planet->attbattalions;
                                planet->attbattalions = 0;
                                planet->attgovern = NULL;
                                planet->governed_by = attacker;
                                snprintf(buf, MSL,
                                         "%s has been conquered by %s!",
                                         planet->name, attacker->name);
                                echo_to_all(AT_RED, buf, 0);
                                continue;
                        }
                        for (area = planet->first_area; area;
                             area = area->next_on_planet)
                        {
                                for (ch = area->first_person; ch;
                                     ch = ch->next_in_area)
                                {
                                        aclan = NULL;
                                        if (ch->pcdata->clan)
                                        {
                                                if (ch->pcdata->clan->
                                                    mainclan)
                                                        aclan = ch->pcdata->
                                                                clan->
                                                                mainclan;
                                                else
                                                        aclan = ch->pcdata->
                                                                clan;
                                        }

                                        if (aclan == attacker)
                                        {
                                                defmult = defmult + 1;
                                                send_to_char
                                                        ("You help your clan attack in battle!",
                                                         ch);
                                        }
                                        if (aclan == clan)
                                        {
                                                attmult = attmult + 1;
                                                send_to_char
                                                        ("You help your clan defend in battle!",
                                                         ch);
                                        }
                                }
                        }
                        attmult += number_range(1, 3);
                        attmult -= number_range(1, 3);

                        defmult += number_range(1, 3);
                        defmult -= number_range(1, 3);

                        if (planet->pop_support < 0)
                                defmult = defmult * 2;
                        else
                                attmult = attmult * 2;

                        if (attmult < 0)
                                attmult = 0;
                        if (defmult < 0)
                                defmult = 0;
                        attchange = attmult + 1;
                        defchange = defmult;
                        planet->attbattalions -= attchange;
                        planet->defbattalions -= defchange;

                        if (0 >= planet->attbattalions)
                        {
                                if (planet->defbattalions < 0)
                                        planet->defbattalions = 0;
                                planet->attbattalions = 0;
                                planet->attgovern = NULL;
                                /*
                                 * If they are happy with the current gov, defending will increase pop support 
                                 */
                                if (planet->pop_support > 50)
                                        planet->pop_support += 5;
                                snprintf(buf, MSL,
                                         "%s has been defended against %s!",
                                         planet->name, attacker->name);
                                echo_to_all(AT_RED, buf, 0);
                        }

                        if ((0 >= planet->defbattalions)
                            && (planet->attbattalions > 0))
                        {
                                planet->defbattalions = planet->attbattalions;
                                planet->attbattalions = 0;
                                planet->attgovern = NULL;
                                planet->governed_by = attacker;
                                snprintf(buf, MSL,
                                         "%s has been conquered by %s!",
                                         planet->name, attacker->name);
                                echo_to_all(AT_RED, buf, 0);

                                planet->pop_support = 50;
                                for (installation = planet->first_install;
                                     installation;
                                     installation =
                                     installation->next_on_planet)
                                        installation->clan =
                                                planet->governed_by;
                        }
                }

        }
}

/* Can I remove this? */
void logsize_update(void)
{
        char      buf[MAX_STRING_LENGTH];

        snprintf(buf, MSL, "%s%s", SYSTEM_DIR, BUG_FILE);
        if (file_size(buf) > sysdata.log_size)
        {
                FILE     *fp = fopen(buf, "w");

                if (fp)
                        FCLOSE(fp);
        }
        snprintf(buf, MSL, "%s%s", SYSTEM_DIR, LOG_FILE);
        if (file_size(buf) > sysdata.log_size)
        {
                FILE     *fp = fopen(buf, "w");

                if (fp)
                        FCLOSE(fp);
        }
        snprintf(buf, MSL, "%s%s", SYSTEM_DIR, IDEA_FILE);
        if (file_size(buf) > sysdata.log_size)
        {
                FILE     *fp = fopen(buf, "w");

                if (fp)
                        FCLOSE(fp);
        }
        snprintf(buf, MSL, "%s%s", SYSTEM_DIR, HELP_FILE);
        if (file_size(buf) > sysdata.log_size)
        {
                FILE     *fp = fopen(buf, "w");

                if (fp)
                        FCLOSE(fp);
        }
        snprintf(buf, MSL, "%s%s", LOG_DIR, "swr.log");
        if (file_size(buf) > sysdata.log_size)
        {
                FILE     *fp = fopen(buf, "w");

                if (fp)
                        FCLOSE(fp);
        }
}
