/* vim: ts=8 et ft=cpp sw=8
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
 *                                                                                       *
 * AFKMud Copyright 1997-2003 by Roger Libiez (Samson), Levi Beckerson (Whir),           *
 * Michael Ward (Tarl), Erik Wolfe (Dwip), Cameron Carroll (Cam), Cyberfox, Karangi,     *
 * Rathian, Raine, and Adjani. All Rights Reserved.                                      *
 *                                                                                       *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag, Blodkai, Haus, Narn,*
 * Scryn, Swordbearer, Tricops, Gorog, Rennard, Grishnakh, Fireblade, and Nivek.         *
 *                                                                                       *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                                   *
 *                                                                                       *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert,  *
 * and Sebastian Hammer.                                                                 *
 *****************************************************************************************
 *                                 Player commands Module                                *
 ****************************************************************************************/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "editor.h"
#include "bounty.h"
#include "account.h"
#include "races.h"

/*
 * Externs
 */
void birth_date args((CHAR_DATA * ch));

/*
 *  Locals
 */
char     *tiny_affect_loc_name(int location);
void write_clan_list args((void));
bool write_to_descriptor args((int desc, char *txt, int length));
char     *full_color args((char *str));

CMDF do_gold(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        set_char_color(AT_GOLD, ch);
        ch_printf(ch, "You have %s", num_punct_long(ch->gold));
        if (IS_NPC(ch))
        {
                send_to_char("\n\r", ch);
                return;
        }
        send_to_char(" credits on you.\n\r", ch);
        return;
}


/*
 * New score command by Haus
 */
CMDF do_score(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];

        /*
         * AFFECT_DATA    *paf; 
         */
        CLAN_DATA *clan;

        if (IS_NPC(ch))
        {
                do_oldscore(ch, argument);
                return;
        }
        clan = ch->pcdata->clan;
        send_to_pager
                ("&w<>&B---------------------------------------------------------------------&w<>\n\r",
                 ch);
        send_to_pager
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);
        pager_printf(ch,
                     "&B|| N&zame:&w   %-25.25s&BL&zog In:  &w%-24.24s&B  ||\n\r",
                     smash_color(ch->pcdata->title), ctime(&(ch->logon)));

        pager_printf(ch,
                     "&B|| R&zace:&w   %-3d year old %-12.12s&BT&zime:&w    %-24.24s&B  ||\n\r",
                     get_age(ch), capitalize(ch->race->name()),
                     ctime(&current_time));

        if (ch->pcdata->spouse && ch->pcdata->spouse[0] != '\0')
                pager_printf(ch,
                             "&B|| H&zitroll:&w%-2.2d                       &BS&zpouse:&w   %-23.23s  &B||\n\r",
                             GET_HITROLL(ch), ch->pcdata->spouse);
        else
                pager_printf(ch, "&B|| H&zitroll:&w%-60.2d&B||\n\r",
                             GET_HITROLL(ch),
                             ch->
                             save_time ? ctime(&(ch->save_time)) : "no\n");

        pager_printf(ch,
                     "&B|| D&zamroll:&w%-2.2d                       &BM&zarital &BS&ztatus:&w  %-18.18s&B||\n\r",
                     GET_DAMROLL(ch),
                     ch->pcdata->spouse[0] !=
                     '\0' ? (IS_SET(ch->pcdata->flags, PCFLAG_MARRIED) ?
                             "Married" : "Engaged") : "Single ");

        pager_printf(ch,
                     "&B|| A&zrmor:  &w%-5d                    &BH&zit &BP&zoints:&w  %-5d &zof &w%-13d&B||\n\r",
                     GET_AC(ch), ch->hit, ch->max_hit);

        pager_printf(ch,
                     "&B|| A&zlign:  &w%-5d                    &BE&zndurance:&w   %-5d &zof &w%-13d&B||\n\r",
                     ch->alignment, ch->endurance, ch->max_endurance);
#ifndef ACCOUNT
        pager_printf(ch, "&B|| W&zimpy:  &w%-60d&B||\n\r", ch->wimpy);
        pager_printf(ch,
                     "&B|| Q&zuest &BP&zoints:  &w%-5d             &BR&zole &BP&zlaying &BP&zoints:&w  %-5d        &B||\n\r",
                     ch->pcdata->quest_curr, ch->pcdata->rp);
#else
        pager_printf(ch,
                     "&B|| Q&zuest &BP&zoints:  &w%-5d             &BR&zole &BP&zlaying &BP&zoints:&w  %-5d        &B||\n\r",
                     ch->pcdata->quest_curr,
                     ch->pcdata->account ? ch->pcdata->account->
                     rppoints : ch->pcdata->rp);
        pager_printf(ch,
                     "&B|| W&zimpy:  &w%-5d                    &BC&zurrent &BR&zPP:&w          %-5d        &B||\n\r",
                     ch->wimpy,
                     ch->pcdata->account ? ch->pcdata->account->
                     rpcurrent : ch->pcdata->rp);
#endif
        if (ch->top_level >= 10)
                pager_printf(ch,
                             "&B|| S&ztr: &w%2d  &BD&zex: &w%2d  &BC&zon: &w%2d  &BI&znt: &w%2d  &BW&zis: &w%2d  &BC&zha: &w%2d  &BF&zrc: &w%-9d&B||\n\r",
                             get_curr_str(ch), get_curr_dex(ch),
                             get_curr_con(ch), get_curr_int(ch),
                             get_curr_wis(ch), get_curr_cha(ch),
                             get_curr_frc(ch));
        else
                pager_printf(ch,
                             "&B|| S&ztr: &w%2d  &BD&zex: &w%2d  &BC&zon: &w%2d  &BI&znt: &w%2d  &BW&zis: &w%2d  &BC&zha: &w%2d  &BF&zrc: &w??       &B||\n\r",
                             get_curr_str(ch), get_curr_dex(ch),
                             get_curr_con(ch), get_curr_int(ch),
                             get_curr_wis(ch), get_curr_cha(ch));
        if (!IS_NPC(ch))
                birth_date(ch);
        send_to_pager
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);
        send_to_pager
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);
        {
                int       ability;

                for (ability = 0; ability < MAX_ABILITY; ability++)
                        if ((ability == FORCE_ABILITY
                             && ch->skill_level[FORCE_ABILITY] > 1)
                            || (get_trust(ch) >= 5
                                && ability != FORCE_ABILITY))
                                ch_printf(ch,
                                          "&B|| &w%-15s &BL&zevel: &w%-3d &BM&zax: &w%-3d &BE&zxp: &w%-10ld &BN&zext: &w%-10ld&B||\n\r",
                                          capitalize(ability_name[ability]),
                                          ch->skill_level[ability],
                                          max_level(ch, ability),
                                          ch->experience[ability],
                                          exp_level(ch->skill_level[ability] +
                                                    1));
                        else
                                ch_printf(ch,
                                          "&B|| &w%-15s &BL&zevel: &w%-3d &BM&zax: &w??? &BE&zxp: &w???        &BN&zext: &w???       &B||\n\r",
                                          capitalize(ability_name[ability]),
                                          ch->skill_level[ability],
                                          ch->experience[ability]);

        }
        send_to_pager
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);
        send_to_pager
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);

        pager_printf(ch,
                     "&B|| C&zredits: &w%15.15s &BP&zkills:&w %-5.5d   &BM&zkills:&w %-5.5d              &B||\n\r",
                     num_punct_long(ch->gold), ch->pcdata->pkills,
                     ch->pcdata->mkills);

        pager_printf(ch,
                     "&B|| W&zeight:            &w%5.5d &BM&zax: &w%7.7d&B    &BI&ztems:  &w%5.5d   &BM&zax: &w%5.5d&B &B||\n\r",
                     ch->carry_weight, can_carry_w(ch), ch->carry_number,
                     can_carry_n(ch));

        pager_printf(ch,
                     "&B|| P&zager: &B(&w%c&B) %5.5d   &BA&zutoExit&B(&w%c&B)  &BA&zutoLoot&B(&w%c&B)                         &B||\n\r",
                     IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) ? 'X' : ' ',
                     ch->pcdata->pagerlen, IS_SET(ch->act,
                                                  PLR_AUTOEXIT) ? 'X' : ' ',
                     IS_SET(ch->act, PLR_AUTOLOOT) ? 'X' : ' ');

        switch (ch->position)
        {
        case POS_DEAD:
                snprintf(buf, MSL,
                         "&B|| &BY&zou are &wslowly decomposing.                                         &B||\n\r");
                break;
        case POS_MORTAL:
                snprintf(buf, MSL,
                         "&B|| &BY&zou are &wmortally wounded.                                           &B||\n\r");
                break;
        case POS_INCAP:
                snprintf(buf, MSL,
                         "&B|| &BY&zou are &wincapacitated.                                              &B||\n\r");
                break;
        case POS_STUNNED:
                snprintf(buf, MSL,
                         "&B|| &BY&zou are &wstunned.                                                    &B||\n\r");
                break;
        case POS_SLEEPING:
                snprintf(buf, MSL,
                         "&B|| &BY&zou are &wsleeping.                                                   &B||\n\r");
                break;
        case POS_RESTING:
                snprintf(buf, MSL,
                         "&B|| &BY&zou are &wresting.                                                    &B||\n\r");
                break;
        case POS_STANDING:
                snprintf(buf, MSL,
                         "&B|| &BY&zou are &wstanding.                                                   &B||\n\r");
                break;
        case POS_FIGHTING:
                snprintf(buf, MSL,
                         "&B|| &BY&zou are &wfighting.                                                   &B||\n\r");
                break;
        case POS_MOUNTED:
                snprintf(buf, MSL,
                         "&B|| &BY&zou are &wmounted.                                                    &B||\n\r");
                break;
        case POS_SITTING:
                snprintf(buf, MSL,
                         "&B|| &BY&zou are &wsitting.                                                    &B||\n\r");
                break;
        }

        send_to_pager(buf, ch);

        if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
                send_to_pager
                        ("&B|| &BY&zou are &wdrunk.                                                      &B||\n\r",
                         ch);
        if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] == 0)
                send_to_pager
                        ("&B|| &BY&zou are in danger of &wdehydrating.                                   &B||\n\r",
                         ch);
        if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] == 0)
                send_to_pager
                        ("&B|| &BY&zou are starving to &wdeath.                                          &B||\n\r",
                         ch);
        if (!IS_NPC(ch) && ch->pcdata->illness > (0)
            && ch->pcdata->illness <= (4))
                send_to_pager
                        ("&B|| &BY&zou feel &wa little bit sick.                                           &B||\n\r",
                         ch);
        if (!IS_NPC(ch) && ch->pcdata->illness >= (5)
            && ch->pcdata->illness <= (7))
                send_to_pager
                        ("&B|| &BY&zou feel &wquite sick and queesey.                                      &B||\n\r",
                         ch);
        if (!IS_NPC(ch) && ch->pcdata->illness >= (8)
            && ch->pcdata->illness <= (10))
                send_to_pager
                        ("&B|| &BY&zou feel &wweak.                                                        &B||\n\r",
                         ch);
        if (!IS_NPC(ch) && ch->pcdata->illness >= (11)
            && ch->pcdata->illness <= (12))
                send_to_pager
                        ("&B|| &BY&zou feel &wvery ill.                                                    &B||\n\r",
                         ch);
        if (!IS_NPC(ch) && ch->pcdata->illness == (13))
                send_to_pager
                        ("&B|| &BY&zou feel &wvery depressed.                                              &B||\n\r",
                         ch);
        if (!IS_NPC(ch) && ch->pcdata->illness >= (14)
            && ch->pcdata->illness <= (15))
                send_to_pager
                        ("&B|| &BY&zou feel &wextremely sick.                                              &B||\n\r",
                         ch);

        /*
         * Wanted information 
         */
        if (ch->pcdata->first_wanted)
        {
                WANTED_DATA *wanted = NULL;

                mudstrlcpy(buf, "", MSL);
                for (wanted = ch->pcdata->first_wanted; wanted;
                     wanted = wanted->next)
                {
                        if (wanted->government == NULL ||
                            wanted->government->name == NULL ||
                            wanted->government->name[0] == '\0')
                                continue;

                        if (buf[0] != '\0')
                                mudstrlcpy(buf, ", ", MSL);
                        mudstrlcpy(buf, wanted->government->name, MSL);
                }
                pager_printf(ch, "&B|| &BW&zanted: &Y%-59.59s &B||\n\r", buf);
        }

        if (ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0')
                pager_printf(ch, "&B|| &BB&zestowments: %-54.54s &B||\n\r",
                             ch->pcdata->bestowments);

        if (ch->pcdata->clan)
        {
                send_to_pager
                        ("&B||---------------------------------------------------------------------||\n\r",
                         ch);
                send_to_pager
                        ("&B||---------------------------------------------------------------------||\n\r",
                         ch);
                pager_printf(ch,
                             "&B|| O&zRGANIZATION: &w%-30s &BP&zkills&w/&BD&zeaths:&w %3.3d/%3.3d &B||\n\r",
                             ch->pcdata->clan->name, ch->pcdata->clan->pkills,
                             ch->pcdata->clan->pdeaths);
                if (ch->pcdata->clanrank)
                        pager_printf(ch, "&B|| R&zANK:&w %-61s &B||\n\r",
                                     ch->pcdata->clan->rank[ch->pcdata->
                                                            clanrank]);
        }
        if (IS_IMMORTAL(ch))
        {
                send_to_pager
                        ("&B||---------------------------------------------------------------------||\n\r",
                         ch);
                send_to_pager
                        ("&B||---------------------------------------------------------------------||\n\r",
                         ch);
                pager_printf(ch,
                             "&B|| I&zMMORTAL DATA:  &BW&zizinvis &B[&w%s&B]  W&zizlevel &B(&w%-3d&B)                        ||\n\r",
                             IS_SET(ch->act, PLR_WIZINVIS) ? "X" : " ",
                             ch->pcdata->wizinvis);

                pager_printf(ch, "&B|| B&zamfin:&w  %58.58s &B||\n\r",
                             (ch->pcdata->bamfin[0] !=
                              '\0') ? smash_color(ch->pcdata->
                                                  bamfin) :
                             "%s appears in a swirling mist.", ch->name);
                pager_printf(ch, "&B|| B&zamfout:&w %58.58s &B||\n\r",
                             (ch->pcdata->bamfout[0] !=
                              '\0') ? smash_color(ch->pcdata->
                                                  bamfout) :
                             "%s leaves in a swirling mist.", ch->name);

                /*
                 * Area Loaded info - Scryn 8/11
                 */
                if (ch->pcdata->area)
                {
                        pager_printf(ch,
                                     "&B|| V&znums:   &BR&zoom &B(&w%-5.5d-%-5.5d&B) O&zbject &B(&w%-5.5d-%-5.5d&B) M&zob &B(&w%-5.5d-%-5.5d)  &B||\n\r",
                                     ch->pcdata->area->low_r_vnum,
                                     ch->pcdata->area->hi_r_vnum,
                                     ch->pcdata->area->low_o_vnum,
                                     ch->pcdata->area->hi_o_vnum,
                                     ch->pcdata->area->low_m_vnum,
                                     ch->pcdata->area->hi_m_vnum);
                        pager_printf(ch,
                                     "&B|| A&zrea Loaded &B[&w%-3s&B]                                                   &B||\n\r",
                                     (IS_SET
                                      (ch->pcdata->area->status,
                                       AREA_LOADED)) ? "yes" : "no");
                }
        }
        send_to_pager
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);
        if (ch->first_affect)
        {
                send_to_pager
                        ("&B||---------------------------------------------------------------------||\n\r",
                         ch);
                do_affected(ch, "score");
        }
        else
                send_to_pager
                        ("&w<>&B---------------------------------------------------------------------&w<>\n\r",
                         ch);
        send_to_pager("\n\r", ch);
        return;
}

CMDF do_immscore(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      arg[MAX_INPUT_LENGTH];
        AFFECT_DATA *paf;
        CLAN_DATA *clan;
        CHAR_DATA *victim;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Syntax: immscore <playername>\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("&RNo such person&W\n\r", ch);
                return;
        }
        if (IS_NPC(victim))
        {
                send_to_char("&RNot on NPC's&W\n\r", ch);
                return;
        }
        clan = victim->pcdata->clan;
        send_to_char
                ("&w<>&B---------------------------------------------------------------------&w<>\n\r",
                 ch);
        send_to_char
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);
        ch_printf(ch,
                  "&B|| N&zame:&w   %-25.25s&BL&zog In:  &w%-24.24s&B  ||\n\r",
                  smash_color(victim->pcdata->title),
                  ctime(&(victim->logon)));

        ch_printf(ch,
                  "&B|| R&zace:&w   %-3d year old %-12.12s&BT&zime:&w    %-24.24s&B  ||\n\r",
                  get_age(victim), capitalize(victim->race->name()),
                  ctime(&current_time));

        if (victim->pcdata->spouse && victim->pcdata->spouse[0] != '\0')
                ch_printf(ch,
                          "&B|| H&zitroll:&w%-2.2d                       &BS&zpouse:&w   %-23.23s  &B||\n\r",
                          GET_HITROLL(victim), victim->pcdata->spouse);
        else
                ch_printf(ch, "&B|| H&zitroll:&w%-60.2d&B||\n\r",
                          GET_HITROLL(victim),
                          victim->
                          save_time ? ctime(&(victim->save_time)) : "no\n");

        ch_printf(ch,
                  "&B|| D&zamroll:&w%-2.2d                       &BM&zarital &BS&ztatus:&w  %-18.18s&B||\n\r",
                  GET_DAMROLL(victim),
                  victim->pcdata->spouse[0] != '\0' ? "Married" : "Single ");

        ch_printf(ch,
                  "&B|| A&zrmor:  &w%-5d                    &BH&zit &BP&zoints:&w  %-5d &zof &w%-13d&B||\n\r",
                  GET_AC(victim), victim->hit, victim->max_hit);

        ch_printf(ch,
                  "&B|| A&zlign:  &w%-5d                    &BE&zndurance:&w   %-5d &zof &w%-13d&B||\n\r",
                  victim->alignment, victim->endurance,
                  victim->max_endurance);

        ch_printf(ch, "&B|| W&zimpy:  &w%-60d&B||\n\r", victim->wimpy);

#ifndef ACCOUNT
        pager_printf(ch, "&B|| W&zimpy:  &w%-60d&B||\n\r", victim->wimpy);
        pager_printf(ch,
                     "&B|| Q&zuest &BP&zoints:  &w%-5d             &BR&zole &BP&zlaying &BP&zoints:&w  %-5d        &B||\n\r",
                     victim->pcdata->quest_curr, victim->pcdata->rp);
#else
        pager_printf(ch,
                     "&B|| Q&zuest &BP&zoints:  &w%-5d             &BR&zole &BP&zlaying &BP&zoints:&w  %-5d        &B||\n\r",
                     victim->pcdata->quest_curr,
                     victim->pcdata->account ? victim->pcdata->account->
                     rppoints : victim->pcdata->rp);
        pager_printf(ch,
                     "&B|| W&zimpy:  &w%-5d                    &BC&zurrent &BR&zPP:&w          %-5d        &B||\n\r",
                     victim->wimpy,
                     victim->pcdata->account ? victim->pcdata->account->
                     rpcurrent : victim->pcdata->rp);
#endif
        ch_printf(ch,
                  "&B|| S&ztr: &w%2d  &BD&zex: &w%2d  &BC&zon: &w%2d  &BI&znt: &w%2d  &BW&zis: &w%2d  &BC&zha: &w%2d  &BF&zrc: &w%-9d&B||\n\r",
                  get_curr_str(victim), get_curr_dex(victim),
                  get_curr_con(victim), get_curr_int(victim),
                  get_curr_wis(victim), get_curr_cha(victim),
                  get_curr_frc(victim));
        send_to_char
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);
        send_to_char
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);
        {
                int       ability;

                for (ability = 0; ability < MAX_ABILITY; ability++)
                        ch_printf(ch,
                                  "&B|| &w%-15s &BL&zevel: &w%-3d &BM&zax: &w%-3d &BE&zxp: &w%-10ld &BN&zext: &w%-10ld&B||\n\r",
                                  capitalize(ability_name[ability]),
                                  victim->skill_level[ability],
                                  max_level(victim, ability),
                                  victim->experience[ability],
                                  exp_level(victim->skill_level[ability] +
                                            1));
        }
        send_to_char
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);
        send_to_char
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);

        ch_printf(ch,
                  "&B|| C&zredits: &w%15.15s &BP&zkills:&w %-5.5d   &BM&zkills:&w %-5.5d              &B||\n\r",
                  num_punct_long(victim->gold), victim->pcdata->pkills,
                  victim->pcdata->mkills);

        ch_printf(ch,
                  "&B|| W&zeight:        &w%5.5d &BM&zax: &w%7.7d&B    &BI&ztems:  &w%5.5d   &BM&zax: &w%5.5d&B     &B||\n\r",
                  victim->carry_weight, can_carry_w(victim),
                  victim->carry_number, can_carry_n(victim));

        ch_printf(ch,
                  "&B|| P&zager: &B(&w%c&B) %5.5d   &BA&zutoExit&B(&w%c&B)  &BA&zutoLoot&B(&w%c&B)                         &B||\n\r",
                  IS_SET(victim->pcdata->flags, PCFLAG_PAGERON) ? 'X' : ' ',
                  victim->pcdata->pagerlen, IS_SET(victim->act,
                                                   PLR_AUTOEXIT) ? 'X' : ' ',
                  IS_SET(victim->act, PLR_AUTOLOOT) ? 'X' : ' ');

        switch (victim->position)
        {
        case POS_DEAD:
                snprintf(buf, MSL,
                         "&B|| &wYou are slowly decomposing.                                         &B||\n\r");
                break;
        case POS_MORTAL:
                snprintf(buf, MSL,
                         "&B|| &wYou are mortally wounded.                                           &B||\n\r");
                break;
        case POS_INCAP:
                snprintf(buf, MSL,
                         "&B|| &wYou are incapacitated.                                              &B||\n\r");
                break;
        case POS_STUNNED:
                snprintf(buf, MSL,
                         "&B|| &wYou are stunned.                                                    &B||\n\r");
                break;
        case POS_SLEEPING:
                snprintf(buf, MSL,
                         "&B|| &wYou are sleeping.                                                   &B||\n\r");
                break;
        case POS_RESTING:
                snprintf(buf, MSL,
                         "&B|| &wYou are resting.                                                    &B||\n\r");
                break;
        case POS_STANDING:
                snprintf(buf, MSL,
                         "&B|| &wYou are standing.                                                   &B||\n\r");
                break;
        case POS_FIGHTING:
                snprintf(buf, MSL,
                         "&B|| &wYou are fighting.                                                   &B||\n\r");
                break;
        case POS_MOUNTED:
                snprintf(buf, MSL,
                         "&B|| &wYou are mounted.                                                    &B||\n\r");
                break;
        case POS_SITTING:
                snprintf(buf, MSL,
                         "&B|| &wYou are sitting.                                                    &B||\n\r");
                break;
        }

        send_to_char(buf, ch);

        if (!IS_NPC(victim) && victim->pcdata->condition[COND_DRUNK] > 10)
                send_to_char
                        ("&B|| &wYou are &zdrunk.                                                      &B||\n\r",
                         ch);
        if (!IS_NPC(victim) && victim->pcdata->condition[COND_THIRST] == 0)
                send_to_char
                        ("&B|| &wYou are in danger of &zdehydrating.                                   &B||\n\r",
                         ch);
        if (!IS_NPC(victim) && victim->pcdata->condition[COND_FULL] == 0)
                send_to_char
                        ("&B|| &wYou are starving to &zdeath.                                          &B||\n\r",
                         ch);

        /*
         * Wanted information 
         */
        if (victim->pcdata->first_wanted)
        {
                WANTED_DATA *wanted = NULL;

                mudstrlcpy(buf, "", MSL);
                for (wanted = victim->pcdata->first_wanted; wanted;
                     wanted = wanted->next)
                {
                        if (wanted->government == NULL ||
                            wanted->government->name == NULL ||
                            wanted->government->name[0] == '\0')
                                continue;

                        if (buf[0] != '\0')
                                mudstrlcpy(buf, ", ", MSL);
                        mudstrlcpy(buf, wanted->government->name, MSL);
                }
                ch_printf(ch, "&B|| &BW&zanted: &Y%-59.59s &B||\n\r", buf);
        }

        if (victim->pcdata->bestowments
            && victim->pcdata->bestowments[0] != '\0')
                ch_printf(ch, "&B|| Bestowments: %-54.54s &B||\n\r",
                          victim->pcdata->bestowments);

        if (victim->pcdata->clan)
        {
                send_to_char
                        ("&B||---------------------------------------------------------------------||\n\r",
                         ch);
                send_to_char
                        ("&B||---------------------------------------------------------------------||\n\r",
                         ch);
                ch_printf(ch,
                          "&B|| O&zRGANIZATION: &w%-30s &BP&zkills&w/&BD&zeaths:&w %3.3d/%3.3d &B||\n\r",
                          victim->pcdata->clan->name,
                          victim->pcdata->clan->pkills,
                          victim->pcdata->clan->pdeaths);
                if (victim->pcdata->clanrank)
                        ch_printf(ch, "&B|| R&zANK:&w %-61s &B||\n\r",
                                  victim->pcdata->clan->rank[victim->pcdata->
                                                             clanrank]);
        }
        if (IS_IMMORTAL(victim))
        {
                send_to_char
                        ("&B||---------------------------------------------------------------------||\n\r",
                         ch);
                send_to_char
                        ("&B||---------------------------------------------------------------------||\n\r",
                         ch);
                ch_printf(ch,
                          "&B|| I&zMMORTAL DATA:  &BW&zizinvis &B[&w%s&B]  W&zizlevel &B(&w%-3d&B)                        ||\n\r",
                          IS_SET(victim->act, PLR_WIZINVIS) ? "X" : " ",
                          victim->pcdata->wizinvis);

                ch_printf(ch, "&B|| B&zamfin:&w  %58.58s &B||\n\r",
                          (victim->pcdata->bamfin[0] !=
                           '\0') ? smash_color(victim->pcdata->
                                               bamfin) :
                          "%s appears in a swirling mist.", victim->name);
                ch_printf(ch, "&B|| B&zamfout:&w %58.58s &B||\n\r",
                          (victim->pcdata->bamfout[0] !=
                           '\0') ? smash_color(victim->pcdata->
                                               bamfout) :
                          "%s leaves in a swirling mist.", victim->name);

                /*
                 * Area Loaded info - Scryn 8/11
                 */
                if (victim->pcdata->area)
                {
                        ch_printf(ch,
                                  "&B|| V&znums:   &BR&zoom &B(&w%-5.5d-%-5.5d&B) O&zbject &B(&w%-5.5d-%-5.5d&B) M&zob &B(&w%-5.5d-%-5.5d)  &B||\n\r",
                                  victim->pcdata->area->low_r_vnum,
                                  victim->pcdata->area->hi_r_vnum,
                                  victim->pcdata->area->low_o_vnum,
                                  victim->pcdata->area->hi_o_vnum,
                                  victim->pcdata->area->low_m_vnum,
                                  victim->pcdata->area->hi_m_vnum);
                        ch_printf(ch,
                                  "&B|| A&zrea Loaded &B[&w%-3s&B]                                                   &B||\n\r",
                                  (IS_SET
                                   (victim->pcdata->area->status,
                                    AREA_LOADED)) ? "yes" : "no");
                }
        }
        send_to_char
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);
        if (victim->first_affect)
        {
                int       count = 0;
                SKILLTYPE *skill;
                char     *affname;

                set_pager_color(AT_BLUE, ch);

                for (paf = ch->first_affect; paf; paf = paf->next)
                {
                        if ((skill = get_skilltype(paf->type)) != NULL)
                        {
                                if (((count++) % 2) == 0)
                                        send_to_pager("&B||", ch);
                                set_pager_color(AT_SCORE, ch);
                                pager_printf(ch, "  &B%c&z%-9.9s ",
                                             UPPER(skill->name[0]),
                                             (skill->name + 1));
                                if (ch->top_level >= 20)
                                {
                                        affname =
                                                tiny_affect_loc_name(paf->
                                                                     location);

                                        if (paf->modifier == 0)
                                                pager_printf(ch,
                                                             "&B(&w%5d&B)           ",
                                                             paf->duration);
                                        else if (paf->modifier > 999)
                                                pager_printf(ch,
                                                             "&B(&w%5d&B)&z[&w%9.9s&z]",
                                                             paf->duration,
                                                             affname);
                                        else
                                                pager_printf(ch,
                                                             "&B(&w%5d&B)&z[&w%+-3.3d %4.s&z]",
                                                             paf->duration,
                                                             paf->modifier,
                                                             affname);

                                }
                                else
                                        send_to_pager("                ", ch);
                                send_to_pager("   ", ch);
                                if ((count % 2) == 0)
                                        send_to_pager(" &B||\n\r", ch);
                        }
                }
                if (count != 0 && (count % 2) == 1)
                        send_to_pager
                                ("                                   &B||\n\r",
                                 ch);

        }

        send_to_char
                ("||---------------------------------------------------------------------||\n\r",
                 ch);
        send_to_char
                ("&w<>&B---------------------------------------------------------------------&w<>\n\r",
                 ch);
        send_to_char("\n\r", ch);
        return;
}



/*
 * Return ascii name of an affect location.
 */
char     *tiny_affect_loc_name(int location)
{
        switch (location)
        {
        case APPLY_NONE:
                return "NIL";
        case APPLY_STR:
                return "STR";
        case APPLY_DEX:
                return "DEX";
        case APPLY_INT:
                return "INT";
        case APPLY_WIS:
                return "WIS";
        case APPLY_CON:
                return "CON";
        case APPLY_CHA:
                return "CHA";
        case APPLY_LCK:
                return "LCK";
        case APPLY_SEX:
                return "SEX";
        case APPLY_LEVEL:
                return "LVL";
        case APPLY_AGE:
                return "AGE";
        case APPLY_MANA:
                return "ENDUR";
        case APPLY_HIT:
                return "HIT";
        case APPLY_MOVE:
                return "ENDUR";
        case APPLY_GOLD:
                return "GOLD";
        case APPLY_EXP:
                return "EXP";
        case APPLY_AC:
                return " AC";
        case APPLY_HITROLL:
                return " HITRL";
        case APPLY_DAMROLL:
                return "DAMRL";
        case APPLY_SAVING_POISON:
                return "SV POI";
        case APPLY_SAVING_ROD:
                return "SV ROD";
        case APPLY_SAVING_PARA:
                return "SV PARA";
        case APPLY_SAVING_BREATH:
                return "SV BRTH";
        case APPLY_SAVING_SPELL:
                return "SV SPLL";
        case APPLY_HEIGHT:
                return "HEIGHT";
        case APPLY_WEIGHT:
                return "WEIGHT";
        case APPLY_AFFECT:
                return "AFF BY";
        case APPLY_RESISTANT:
                return "RESIST";
        case APPLY_IMMUNE:
                return "IMMUNE";
        case APPLY_SUSCEPTIBLE:
                return "SUSCEPT";
        case APPLY_WEAPONSPELL:
                return " WEAPON";
        case APPLY_BACKSTAB:
                return "BACKSTB";
        case APPLY_PICK:
                return "PICK";
        case APPLY_TRACK:
                return "TRACK";
        case APPLY_STEAL:
                return "STEAL";
        case APPLY_SNEAK:
                return "SNEAK";
        case APPLY_HIDE:
                return "HIDE";
        case APPLY_PALM:
                return "PALM";
        case APPLY_DETRAP:
                return "DETRAP";
        case APPLY_DODGE:
                return "DODGE";
        case APPLY_PEEK:
                return "PEEK";
        case APPLY_SCAN:
                return "SCAN";
        case APPLY_GOUGE:
                return "GOUGE";
        case APPLY_SEARCH:
                return "SEARCH";
        case APPLY_MOUNT:
                return "MOUNT";
        case APPLY_DISARM:
                return "DISARM";
        case APPLY_KICK:
                return "KICK";
        case APPLY_PARRY:
                return "PARRY";
        case APPLY_BASH:
                return "BASH";
        case APPLY_STUN:
                return "STUN";
        case APPLY_PUNCH:
                return "PUNCH";
        case APPLY_CLIMB:
                return "CLIMB";
        case APPLY_GRIP:
                return "GRIP";
        case APPLY_SCRIBE:
                return "SCRIBE";
        case APPLY_BREW:
                return "BREW";
        case APPLY_WEARSPELL:
                return "WEAR";
        case APPLY_REMOVESPELL:
                return "REMOVE";
        case APPLY_EMOTION:
                return "EMOTION";
        case APPLY_MENTALSTATE:
                return "MENTAL";
        case APPLY_STRIPSN:
                return "DISPEL";
        case APPLY_REMOVE:
                return "REMOVE";
        case APPLY_DIG:
                return "DIG";
        case APPLY_FULL:
                return "HUNGER";
        case APPLY_THIRST:
                return "THIRST";
        case APPLY_DRUNK:
                return "DRUNK";
        case APPLY_BLOOD:
                return "BLOOD";
        }

        bug("Affect_location_name: unknown location %d.", location);
        return ("?");
}

CMDF do_oldscore(CHAR_DATA * ch, char *argument)
{
        AFFECT_DATA *paf;
        SKILLTYPE *skill;

        argument = NULL;    /* Squelch Warning */
        if (IS_AFFECTED(ch, AFF_POSSESS))
        {
                send_to_char
                        ("You can't do that in your current state of mind!\n\r",
                         ch);
                return;
        }

        set_char_color(AT_SCORE, ch);
        ch_printf(ch,
                  "You are %s%s, level %d, %d years old (%d hours).\n\r",
                  ch->name,
                  IS_NPC(ch) ? "" : ch->pcdata->title,
                  ch->top_level, get_age(ch), (get_age(ch) - 17));

        if (get_trust(ch) != ch->top_level)
                ch_printf(ch, "You are trusted at level %d.\n\r",
                          get_trust(ch));

        if (IS_SET(ch->act, ACT_MOBINVIS))
                ch_printf(ch, "You are mobinvis at level %d.\n\r",
                          ch->mobinvis);


        ch_printf(ch,
                  "You have %d/%d hit, %d/%d endurance.\n\r",
                  ch->hit, ch->max_hit, ch->endurance, ch->max_endurance);

        ch_printf(ch,
                  "You are carrying %d/%d items with weight %d/%d kg.\n\r",
                  ch->carry_number, can_carry_n(ch),
                  ch->carry_weight, can_carry_w(ch));

        ch_printf(ch,
                  "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Cha: %d  Lck: ??  Frc: ??\n\r",
                  get_curr_str(ch),
                  get_curr_int(ch),
                  get_curr_wis(ch),
                  get_curr_dex(ch), get_curr_con(ch), get_curr_cha(ch));

        ch_printf(ch, "You have have %ld credits.\n\r", ch->gold);

        if (!IS_NPC(ch))
                ch_printf(ch,
                          "You have achieved %d glory during your life, and currently have %d.\n\r",
                          ch->pcdata->quest_accum, ch->pcdata->quest_curr);

        ch_printf(ch,
                  "Autoexit: %s   Autoloot: %s  Autocred: %s\n\r",
                  (!IS_NPC(ch)
                   && IS_SET(ch->act, PLR_AUTOEXIT)) ? "yes" : "no",
                  (!IS_NPC(ch)
                   && IS_SET(ch->act, PLR_AUTOLOOT)) ? "yes" : "no",
                  (!IS_NPC(ch)
                   && IS_SET(ch->act, PLR_AUTOGOLD)) ? "yes" : "no");

        ch_printf(ch, "Wimpy set to %d hit points.\n\r", ch->wimpy);

        if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
                send_to_char("You are drunk.\n\r", ch);
        if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] == 0)
                send_to_char("You are thirsty.\n\r", ch);
        if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] == 0)
                send_to_char("You are hungry.\n\r", ch);

        switch (ch->mental_state / 10)
        {
        default:
                send_to_char("You're completely messed up!\n\r", ch);
                break;
        case -10:
                send_to_char("You're barely conscious.\n\r", ch);
                break;
        case -9:
                send_to_char("You can barely keep your eyes open.\n\r", ch);
                break;
        case -8:
                send_to_char("You're extremely drowsy.\n\r", ch);
                break;
        case -7:
                send_to_char("You feel very unmotivated.\n\r", ch);
                break;
        case -6:
                send_to_char("You feel sedated.\n\r", ch);
                break;
        case -5:
                send_to_char("You feel sleepy.\n\r", ch);
                break;
        case -4:
                send_to_char("You feel tired.\n\r", ch);
                break;
        case -3:
                send_to_char("You could use a rest.\n\r", ch);
                break;
        case -2:
                send_to_char("You feel a little under the weather.\n\r", ch);
                break;
        case -1:
                send_to_char("You feel fine.\n\r", ch);
                break;
        case 0:
                send_to_char("You feel great.\n\r", ch);
                break;
        case 1:
                send_to_char("You feel energetic.\n\r", ch);
                break;
        case 2:
                send_to_char("Your mind is racing.\n\r", ch);
                break;
        case 3:
                send_to_char("You can't think straight.\n\r", ch);
                break;
        case 4:
                send_to_char("Your mind is going 100 miles an hour.\n\r", ch);
                break;
        case 5:
                send_to_char("You're high as a kite.\n\r", ch);
                break;
        case 6:
                send_to_char("Your mind and body are slipping appart.\n\r",
                             ch);
                break;
        case 7:
                send_to_char("Reality is slipping away.\n\r", ch);
                break;
        case 8:
                send_to_char
                        ("You have no idea what is real, and what is not.\n\r",
                         ch);
                break;
        case 9:
                send_to_char("You feel immortal.\n\r", ch);
                break;
        case 10:
                send_to_char("You are a Supreme Entity.\n\r", ch);
                break;
        }

        switch (ch->position)
        {
        case POS_DEAD:
                send_to_char("You are DEAD!!\n\r", ch);
                break;
        case POS_MORTAL:
                send_to_char("You are mortally wounded.\n\r", ch);
                break;
        case POS_INCAP:
                send_to_char("You are incapacitated.\n\r", ch);
                break;
        case POS_STUNNED:
                send_to_char("You are stunned.\n\r", ch);
                break;
        case POS_SLEEPING:
                send_to_char("You are sleeping.\n\r", ch);
                break;
        case POS_RESTING:
                send_to_char("You are resting.\n\r", ch);
                break;
        case POS_STANDING:
                send_to_char("You are standing.\n\r", ch);
                break;
        case POS_FIGHTING:
                send_to_char("You are fighting.\n\r", ch);
                break;
        case POS_MOUNTED:
                send_to_char("Mounted.\n\r", ch);
                break;
        case POS_SHOVE:
                send_to_char("Being shoved.\n\r", ch);
                break;
        case POS_DRAG:
                send_to_char("Being dragged.\n\r", ch);
                break;
        }

        if (ch->top_level >= 25)
                ch_printf(ch, "AC: %d.  ", GET_AC(ch));

        send_to_char("You are ", ch);
        if (GET_AC(ch) >= 101)
                send_to_char("WORSE than naked!\n\r", ch);
        else if (GET_AC(ch) >= 80)
                send_to_char("naked.\n\r", ch);
        else if (GET_AC(ch) >= 60)
                send_to_char("wearing clothes.\n\r", ch);
        else if (GET_AC(ch) >= 40)
                send_to_char("slightly armored.\n\r", ch);
        else if (GET_AC(ch) >= 20)
                send_to_char("somewhat armored.\n\r", ch);
        else if (GET_AC(ch) >= 0)
                send_to_char("armored.\n\r", ch);
        else if (GET_AC(ch) >= -20)
                send_to_char("well armored.\n\r", ch);
        else if (GET_AC(ch) >= -40)
                send_to_char("strongly armored.\n\r", ch);
        else if (GET_AC(ch) >= -60)
                send_to_char("heavily armored.\n\r", ch);
        else if (GET_AC(ch) >= -80)
                send_to_char("superbly armored.\n\r", ch);
        else if (GET_AC(ch) >= -100)
                send_to_char("divinely armored.\n\r", ch);
        else
                send_to_char("invincible!\n\r", ch);

        if (ch->top_level >= 15)
                ch_printf(ch, "Hitroll: %d  Damroll: %d.\n\r",
                          GET_HITROLL(ch), GET_DAMROLL(ch));

        if (ch->top_level >= 10)
                ch_printf(ch, "Alignment: %d.  ", ch->alignment);

        send_to_char("You are ", ch);
        if (ch->alignment > 900)
                send_to_char("angelic.\n\r", ch);
        else if (ch->alignment > 700)
                send_to_char("saintly.\n\r", ch);
        else if (ch->alignment > 350)
                send_to_char("good.\n\r", ch);
        else if (ch->alignment > 100)
                send_to_char("kind.\n\r", ch);
        else if (ch->alignment > -100)
                send_to_char("neutral.\n\r", ch);
        else if (ch->alignment > -350)
                send_to_char("mean.\n\r", ch);
        else if (ch->alignment > -700)
                send_to_char("evil.\n\r", ch);
        else if (ch->alignment > -900)
                send_to_char("demonic.\n\r", ch);
        else
                send_to_char("satanic.\n\r", ch);

        if (ch->first_affect)
        {
                send_to_char("You are affected by:\n\r", ch);
                for (paf = ch->first_affect; paf; paf = paf->next)
                        if ((skill = get_skilltype(paf->type)) != NULL)
                        {
                                ch_printf(ch, "Spell: '%s'", skill->name);

                                if (ch->top_level >= 20)
                                        ch_printf(ch,
                                                  " modifies %s by %d for %d rounds",
                                                  affect_loc_name(paf->
                                                                  location),
                                                  paf->modifier,
                                                  paf->duration);

                                send_to_char(".\n\r", ch);
                        }
        }

        if (!IS_NPC(ch) && IS_IMMORTAL(ch))
        {
                ch_printf(ch, "WizInvis level: %d   WizInvis is %s\n\r",
                          ch->pcdata->wizinvis,
                          IS_SET(ch->act, PLR_WIZINVIS) ? "ON" : "OFF");
                if (ch->pcdata->r_range_lo && ch->pcdata->r_range_hi)
                        ch_printf(ch, "Room Range: %d - %d\n\r",
                                  ch->pcdata->r_range_lo,
                                  ch->pcdata->r_range_hi);
                if (ch->pcdata->o_range_lo && ch->pcdata->o_range_hi)
                        ch_printf(ch, "Obj Range : %d - %d\n\r",
                                  ch->pcdata->o_range_lo,
                                  ch->pcdata->o_range_hi);
                if (ch->pcdata->m_range_lo && ch->pcdata->m_range_hi)
                        ch_printf(ch, "Mob Range : %d - %d\n\r",
                                  ch->pcdata->m_range_lo,
                                  ch->pcdata->m_range_hi);
        }

        return;
}

/*								-Thoric
 * Display your current exp, level, and surrounding level exp requirements
 */
CMDF do_level(CHAR_DATA * ch, char *argument)
{
        argument = NULL;    /* Squelch Warning */
        send_to_char
                ("&w<>&B---------------------------------------------------------------------&w<>\n\r",
                 ch);
        send_to_char
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);
        {
                int       ability;

                for (ability = 0; ability < MAX_ABILITY; ability++)
                        if ((ability == FORCE_ABILITY
                             && ch->skill_level[FORCE_ABILITY] > 1)
                            || (get_trust(ch) > 5
                                && ability != FORCE_ABILITY))
                                ch_printf(ch,
                                          "&B|| &w%-15s &BL&zevel: &w%-3d &BM&zax: &w%-3d &BE&zxp: &w%-10ld &BN&zext: &w%-10ld&B||\n\r",
                                          capitalize(ability_name[ability]),
                                          ch->skill_level[ability],
                                          max_level(ch, ability),
                                          ch->experience[ability],
                                          exp_level(ch->skill_level[ability] +
                                                    1));
                        else
                                ch_printf(ch,
                                          "&B|| &w%-15s &BL&zevel: &w%-3d &BM&zax: &w??? &BE&zxp: &w???        &BN&zext: &w???       &B||\n\r",
                                          capitalize(ability_name[ability]),
                                          ch->skill_level[ability],
                                          ch->experience[ability]);
        }
        send_to_char
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);
        send_to_char
                ("&w<>&B---------------------------------------------------------------------&w<>\n\r",
                 ch);
}

CMDF do_affected(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        AFFECT_DATA *paf;
        SKILLTYPE *skill;

        if (IS_NPC(ch))
                return;

        argument = one_argument(argument, arg);
        if (!str_cmp(arg, "by"))
        {
                set_char_color(AT_BLUE, ch);
                send_to_char("\n\rImbued with:\n\r", ch);
                set_char_color(AT_SCORE, ch);
                ch_printf(ch, "%s\n\r", affect_bit_name(ch->affected_by));
                if (ch->top_level >= 20)
                {
                        send_to_char("\n\r", ch);
                        if (ch->resistant > 0)
                        {
                                set_char_color(AT_BLUE, ch);
                                send_to_char("Resistances:  ", ch);
                                set_char_color(AT_SCORE, ch);
                                ch_printf(ch, "%s\n\r",
                                          flag_string(ch->resistant,
                                                      ris_flags));
                        }
                        if (ch->immune > 0)
                        {
                                set_char_color(AT_BLUE, ch);
                                send_to_char("Immunities:   ", ch);
                                set_char_color(AT_SCORE, ch);
                                ch_printf(ch, "%s\n\r",
                                          flag_string(ch->immune, ris_flags));
                        }
                        if (ch->susceptible > 0)
                        {
                                set_char_color(AT_BLUE, ch);
                                send_to_char("Suscepts:     ", ch);
                                set_char_color(AT_SCORE, ch);
                                ch_printf(ch, "%s\n\r",
                                          flag_string(ch->susceptible,
                                                      ris_flags));
                        }
                }
                return;
        }

        if (str_cmp(arg, "score"))
        {
                send_to_pager
                        ("&w<>&B---------------------------------------------------------------------&w<>\n\r",
                         ch);
                send_to_pager
                        ("&B||---------------------------------------------------------------------||\n\r",
                         ch);
        }

        if (!ch->first_affect)
        {
                set_pager_color(AT_SCORE, ch);
                send_to_pager
                        ("&B|| N&zo cantrip or skill affects you.                                    &B||\n\r",
                         ch);
        }
        else
        {
                int       count = 0;
                char     *affname;

                set_pager_color(AT_BLUE, ch);
                send_to_pager
                        ("&B|| &BA&zffected&z:                                                           &B||\n\r",
                         ch);

                for (paf = ch->first_affect; paf; paf = paf->next)
                {
                        if ((skill = get_skilltype(paf->type)) != NULL)
                        {
                                if (((count++) % 2) == 0)
                                        send_to_pager("&B||", ch);
                                set_pager_color(AT_SCORE, ch);
                                pager_printf(ch, "  &B%c&z%-9.9s ",
                                             UPPER(skill->name[0]),
                                             (skill->name + 1));
                                if (ch->top_level >= 20)
                                {
                                        affname =
                                                tiny_affect_loc_name(paf->
                                                                     location);

                                        if (paf->modifier == 0)
                                                pager_printf(ch,
                                                             "&B(&w%5d&B)            ",
                                                             paf->duration);
                                        else if (paf->modifier > 9999)
                                                pager_printf(ch,
                                                             "&B(&w%5d&B)&z[&w%10.10s&z]",
                                                             paf->duration,
                                                             affname);
                                        else
                                                pager_printf(ch,
                                                             "&B(&w%5d&B)&z[&w%+-4.4d %4.4s&z]",
                                                             paf->duration,
                                                             paf->modifier,
                                                             affname);

                                }
                                else
                                        send_to_pager("                 ", ch);
                                send_to_pager("  ", ch);
                                if ((count % 2) == 0)
                                        send_to_pager(" &B||\n\r", ch);
                        }
                }
                if (count != 0 && (count % 2) == 1)
                        send_to_pager
                                ("                                   &B||\n\r",
                                 ch);

        }
        send_to_pager
                ("&B||---------------------------------------------------------------------||\n\r",
                 ch);
        send_to_pager
                ("&w<>&B---------------------------------------------------------------------&w<>\n\r",
                 ch);
        return;
}

CMDF do_inventory(CHAR_DATA * ch, char *argument)
{
        argument = NULL;    /* Squelch Warning */
        set_char_color(AT_RED, ch);
        send_to_char("You are carrying:\n\r", ch);
        show_list_to_char(ch->first_carrying, ch, TRUE, TRUE);
        return;
}


CMDF do_equipment(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj;
        int       iWear, dam;
        bool      found;
        char      buf[MAX_STRING_LENGTH];

        argument = NULL;    /* Squelch Warning */
        set_char_color(AT_RED, ch);
        send_to_char("&BYou are using:\n\r", ch);
        found = FALSE;
        set_char_color(AT_OBJECT, ch);
        for (iWear = 0; iWear < MAX_WEAR; iWear++)
        {
                for (obj = ch->first_carrying; obj; obj = obj->next_content)
                        if (obj->wear_loc == iWear)
                        {
                                send_to_char("&z", ch);
                                send_to_char(where_name[iWear], ch);
                                if (can_see_obj(ch, obj))
                                {
                                        send_to_char("&w", ch);
                                        send_to_char(format_obj_to_char
                                                     (obj, ch, TRUE), ch);
                                        mudstrlcpy(buf, "", MSL);
                                        switch (obj->item_type)
                                        {
                                        default:
                                                break;
                                        case ITEM_HOLSTER:
                                                if (obj->first_content ==
                                                    NULL)
                                                        strcat(buf,
                                                               " (empty) ");
                                                else if (obj->first_content !=
                                                         NULL)
                                                {
                                                        strcat(buf, " (");
                                                        strcat(buf,
                                                               obj->
                                                               first_content->
                                                               short_descr);
                                                        strcat(buf, ") ");
                                                }
                                                send_to_char(buf, ch);
                                                break;
                                        case ITEM_ARMOR:
                                                if (obj->value[1] == 0)
                                                        obj->value[1] =
                                                                obj->value[0];
                                                if (obj->value[1] == 0)
                                                        obj->value[1] = 1;
                                                dam = (sh_int) ((obj->
                                                                 value[0] *
                                                                 10) /
                                                                obj->
                                                                value[1]);
                                                if (dam >= 10)
                                                        mudstrlcat(buf,
                                                                   " &B(&zsuperb&B) ",
                                                                   MSL);
                                                else if (dam >= 7)
                                                        mudstrlcat(buf,
                                                                   " &B(&zgood&B) ",
                                                                   MSL);
                                                else if (dam >= 5)
                                                        mudstrlcat(buf,
                                                                   " &B(&zworn&B) ",
                                                                   MSL);
                                                else if (dam >= 3)
                                                        mudstrlcat(buf,
                                                                   " &B(&zpoor&B) ",
                                                                   MSL);
                                                else if (dam >= 1)
                                                        mudstrlcat(buf,
                                                                   " &B(&zawful&B) ",
                                                                   MSL);
                                                else if (dam == 0)
                                                        mudstrlcat(buf,
                                                                   " &B(&zbroken&B) ",
                                                                   MSL);
                                                send_to_char(buf, ch);
                                                break;

                                        case ITEM_WEAPON:
                                                dam = INIT_WEAPON_CONDITION -
                                                        obj->value[0];
                                                if (dam < 2)
                                                        mudstrlcat(buf,
                                                                   " &B(&zsuperb&B) ",
                                                                   MSL);
                                                else if (dam < 4)
                                                        mudstrlcat(buf,
                                                                   " &B(&zgood&B) ",
                                                                   MSL);
                                                else if (dam < 7)
                                                        mudstrlcat(buf,
                                                                   " &B(&zworn&B) ",
                                                                   MSL);
                                                else if (dam < 10)
                                                        mudstrlcat(buf,
                                                                   " &B(&zpoor&B) ",
                                                                   MSL);
                                                else if (dam < 12)
                                                        mudstrlcat(buf,
                                                                   " &B(&zawful&B) ",
                                                                   MSL);
                                                else if (dam == 12)
                                                        mudstrlcat(buf,
                                                                   " &B(&zbroken&B) ",
                                                                   MSL);
                                                send_to_char(buf, ch);
                                                if (obj->value[3] ==
                                                    WEAPON_BLASTER)
                                                {
                                                        if (obj->
                                                            blaster_setting ==
                                                            BLASTER_FULL)
                                                                ch_printf(ch,
                                                                          "&BFULL");
                                                        else if (obj->
                                                                 blaster_setting
                                                                 ==
                                                                 BLASTER_HIGH)
                                                                ch_printf(ch,
                                                                          "&BHIGH");
                                                        else if (obj->
                                                                 blaster_setting
                                                                 ==
                                                                 BLASTER_NORMAL)
                                                                ch_printf(ch,
                                                                          "&BNORMAL");
                                                        else if (obj->
                                                                 blaster_setting
                                                                 ==
                                                                 BLASTER_HALF)
                                                                ch_printf(ch,
                                                                          "&BHALF");
                                                        else if (obj->
                                                                 blaster_setting
                                                                 ==
                                                                 BLASTER_LOW)
                                                                ch_printf(ch,
                                                                          "&BLOW");
                                                        else if (obj->
                                                                 blaster_setting
                                                                 ==
                                                                 BLASTER_STUN)
                                                                ch_printf(ch,
                                                                          "&BSTUN");
                                                        ch_printf(ch, " %d",
                                                                  obj->
                                                                  value[4]);
                                                }
                                                else if ((obj->value[3] ==
                                                          WEAPON_LIGHTSABER
                                                          || obj->value[3] ==
                                                          WEAPON_VIBRO_BLADE
                                                          || obj->value[3] ==
                                                          WEAPON_FORCE_PIKE
                                                          || obj->value[3] ==
                                                          WEAPON_BOWCASTER))
                                                {
                                                        ch_printf(ch, "&z%d",
                                                                  obj->
                                                                  value[4]);
                                                }
                                                break;
                                        }
                                        send_to_char("\n\r", ch);
                                }
                                else
                                        send_to_char("&zsomething.\n\r", ch);
                                found = TRUE;
                        }
        }
        if (!found)
                send_to_char("&BN&zothing.\n\r", ch);
        if (!IS_NPC(ch))
        {
                send_to_char("&BI&zmplants currently installed&w:\n\r", ch);
                {
                        int       count;

                        for (count = 0; count < MAX_IMPLANT_TYPES; count++)
                        {
                                if (ch->pcdata->implants[count] >= 0
                                    && ch->pcdata->implants[count] <= 3)
                                {
                                        found = TRUE;
                                        ch_printf(ch,
                                                  "&BN&zame: &w%-35s Status: %s\n\r",
                                                  implant_names[count],
                                                  implant_affect_names[ch->
                                                                       pcdata->
                                                                       implants
                                                                       [count]]);
                                }
                        }
                        if (found == FALSE)
                                send_to_char("&BN&zone\n\r", ch);
                }
        }
        return;
}



void set_title(CHAR_DATA * ch, char *title)
{
        if (IS_NPC(ch))
        {
                bug("Set_title: NPC.", 0);
                return;
        }
        if (ch->pcdata->title)
                STRFREE(ch->pcdata->title);
        ch->pcdata->title = STRALLOC(title);
        return;
}



CMDF do_title(CHAR_DATA * ch, char *argument)
{
        if (IS_NPC(ch))
                return;

        if (IS_SET(ch->pcdata->flags, PCFLAG_NOTITLE))
        {
                send_to_char("You try but the Force resists you.\n\r", ch);
                return;
        }


        if (argument[0] == '\0')
        {
                send_to_char("Change your title to what?\n\r", ch);
                return;
        }

        if ((get_trust(ch) <= LEVEL_IMMORTAL)
            && (!nifty_is_name(ch->name, smash_color(argument))))
        {
                send_to_char
                        ("You must include your name somewhere in your title!",
                         ch);
                return;
        }

        smash_tilde(argument);
        set_title(ch, argument);
        send_to_char("Ok.\n\r", ch);
}


CMDF do_homepage(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];

        if (IS_NPC(ch))
                return;

        if (argument[0] == '\0')
        {
                if (!ch->pcdata->homepage)
                        ch->pcdata->homepage = STRALLOC("");
                ch_printf(ch, "Your homepage is: %s\n\r",
                          show_tilde(ch->pcdata->homepage));
                return;
        }

        if (!str_cmp(argument, "clear"))
        {
                if (ch->pcdata->homepage)
                        STRFREE(ch->pcdata->homepage);
                ch->pcdata->homepage = STRALLOC("");
                send_to_char("Homepage cleared.\n\r", ch);
                return;
        }

        if (strstr(argument, "://"))
                mudstrlcpy(buf, argument, MSL);
        else
                snprintf(buf, MSL, "http://%s", argument);
        if (strlen(buf) > 70)
                buf[70] = '\0';

        hide_tilde(buf);
        if (ch->pcdata->homepage)
                STRFREE(ch->pcdata->homepage);
        ch->pcdata->homepage = STRALLOC(buf);
        send_to_char("Homepage set.\n\r", ch);
}



/*
 * Set your personal description				-Thoric
 */
CMDF do_description(CHAR_DATA * ch, char *argument)
{
        argument = NULL;    /* Squelch Warning */
        if (IS_NPC(ch))
        {
                send_to_char("Monsters are too dumb to do that!\n\r", ch);
                return;
        }

        if (!ch->desc)
        {
                bug("do_description: no descriptor", 0);
                return;
        }

        switch (ch->substate)
        {
        default:
                bug("do_description: illegal substate", 0);
                return;

        case SUB_RESTRICTED:
                send_to_char
                        ("You cannot use this command from within another command.\n\r",
                         ch);
                return;

        case SUB_NONE:
                ch->substate = SUB_PERSONAL_DESC;
                ch->dest_buf = ch;
                start_editing(ch, ch->description);
                return;

        case SUB_PERSONAL_DESC:
                STRFREE(ch->description);
                ch->description = copy_buffer(ch);
                stop_editing(ch);
                return;
        }
}

/* Ripped off do_description for whois bio's -- Scryn*/
CMDF do_bio(CHAR_DATA * ch, char *argument)
{
        argument = NULL;    /* Squelch Warning */
        if (IS_NPC(ch))
        {
                send_to_char("Mobs can't set bio's!\n\r", ch);
                return;
        }

        if (!ch->desc)
        {
                bug("do_bio: no descriptor", 0);
                return;
        }

        switch (ch->substate)
        {
        default:
                bug("do_bio: illegal substate", 0);
                return;

        case SUB_RESTRICTED:
                send_to_char
                        ("You cannot use this command from within another command.\n\r",
                         ch);
                return;

        case SUB_NONE:
                ch->substate = SUB_PERSONAL_BIO;
                ch->dest_buf = ch;
                start_editing(ch, ch->pcdata->bio);
                return;

        case SUB_PERSONAL_BIO:
                STRFREE(ch->pcdata->bio);
                ch->pcdata->bio = copy_buffer(ch);
                stop_editing(ch);
                return;
        }
}



CMDF do_report(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_INPUT_LENGTH];

        argument = NULL;    /* Squelch Warning */
        if (IS_AFFECTED(ch, AFF_POSSESS))
        {
                send_to_char
                        ("You can't do that in your current state of mind!\n\r",
                         ch);
                return;
        }


        ch_printf(ch,
                  "You report: %d/%d hp %d/%d mv.\n\r",
                  ch->hit, ch->max_hit, ch->endurance, ch->max_endurance);


        snprintf(buf, MSL, "$n reports: %d/%d hp %d/%d.",
                 ch->hit, ch->max_hit, ch->endurance, ch->max_endurance);

        act(AT_REPORT, buf, ch, NULL, NULL, TO_ROOM);

        return;
}

CMDF do_prompt(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MIL];

        if (IS_NPC(ch))
        {
                send_to_char("NPC's can't change their prompt..\n\r", ch);
                return;
        }
        smash_tilde(argument);
        one_argument(argument, arg);
        if (!*arg)
        {
                if (ch->desc && ch->desc->descriptor)
                {
                        sprintf(buf, "%s\n\r",
                                !str_cmp(ch->pcdata->prompt,
                                         "") ? "(default prompt)" : ch->
                                pcdata->prompt);
                        set_char_color(AT_WHITE, ch);
                        send_to_char("Your current prompt string:\n\r", ch);
                        ch_printf(ch, "%s\n\r",
                                  full_color(ch->pcdata->prompt));
                }
                send_to_char("Set prompt to what? (try help prompt)\n\r", ch);
                return;
        }
        if (ch->pcdata->prompt)
                STRFREE(ch->pcdata->prompt);

        if (strlen(argument) > 128)
                argument[128] = '\0';

        /*
         * Can add a list of pre-set prompts here if wanted.. perhaps
         * 'prompt 1' brings up a different, pre-set prompt 
         */
        if (!str_cmp(arg, "default"))
                ch->pcdata->prompt = STRALLOC("");
        else if (!str_cmp(arg, "prompt1"))
                ch->pcdata->prompt =
                        STRALLOC
                        ("&w--==[&BH&zealth %e&B]&w==--%_&w--==[&BE&zndurance &w%v&z/%V &BC&zredits: %g&w]==--");
        else if (!str_cmp(arg, "prompt2"))
                ch->pcdata->prompt =
                        STRALLOC
                        ("&B|----------------------------------------------------------------------------|%_| &BHealth:&w%h/%H    &BEndurance:&w%v/%V          &BCredits: &w%G |%_&B|----------------------------------------------------------------------------|&R&W");
        else if (!str_cmp(arg, "prompt3"))
                ch->pcdata->prompt =
                        STRALLOC("&BHealth:&C%h&B/%H &BMoney:&C%g %e");
        else if (!str_cmp(arg, "prompt4"))
                ch->pcdata->prompt = STRALLOC("&BHealth:&C%h&B/%H");
        else if (!str_cmp(arg, "prompt5"))
                ch->pcdata->prompt = STRALLOC("prompt %E &BMoney:&Y%g %e");
        else if (!str_cmp(arg, "prompt6"))
                ch->pcdata->prompt =
                        STRALLOC
                        ("&pEndurance: &P%m&p/%M Align: &P%a%_%E&OCredits: &Y%g %e");
        else if (!str_cmp(arg, "prompt7"))
                ch->pcdata->prompt =
                        STRALLOC
                        ("&pEndurance: &P%m&p/%M Align: &P%a%_%E&OCredits: &Y%g %e");
        else

                ch->pcdata->prompt = STRALLOC(argument);
        send_to_char("Ok.\n\r", ch);
        return;
}

char     *smash_color(char *str)
{
        static char ret[MAX_STRING_LENGTH];
        char     *retptr;

        retptr = ret;
        for (; *str != '\0'; str++)
        {
                if (*str == '&')
                        str++;
                else
                {
                        *retptr = *str;
                        retptr++;
                }
        }
        *retptr = '\0';
        return ret;
}

char     *smash_space(const char *str)
{
        static char ret[MAX_STRING_LENGTH];
        char     *retptr;

        retptr = ret;
        for (; *str != '\0'; str++)
        {
                if (*str == ' ')
                        continue;
                else
                {
                        *retptr = *str;
                        retptr++;
                }
        }
        *retptr = '\0';
        return ret;
}

char     *trim(const char *str)
{
        static char ret[MAX_STRING_LENGTH];
        char     *retptr;
        const char *beginptr;

        retptr = ret;
        beginptr = str;
        while (*str == ' ' && *str != '\0')
        {
                str++;
        }
        if (*str == '\0')
                return "";

        for (; *str != '\0'; str++)
        {
                *retptr = *str;
                retptr++;
        }
        *retptr = '\0';
        while (retptr > beginptr && *(retptr - 1) == ' ')
        {
                if (*(retptr - 1) != ' ')
                        break;
                retptr--;
                *retptr = '\0';
        }
        return ret;
}


void set_name(CHAR_DATA * ch, char *name)
{
        if (IS_NPC(ch))
        {
                bug("Set_name: NPC.", 0);
                return;
        }
        smash_tilde(name);
/*	name = smash_color(name); */

        name = trim(name);
        name[0] = UPPER(name[0]);
        if (ch->pcdata->full_name)
                STRFREE(ch->pcdata->full_name);
        ch->pcdata->full_name = STRALLOC(name);
        return;
}


CMDF do_fullname(CHAR_DATA * ch, char *argument)
{

        if (IS_NPC(ch))
                return;

        if (argument[0] == '\0')
        {
                send_to_char("&RSyntax: &Gfullname &C<&wname&C>&w\n\r", ch);
                return;
        }

        argument = smash_color(argument);

        if ((get_trust(ch) <= LEVEL_IMMORTAL)
            && (!nifty_is_name(ch->name, argument)))
        {
                send_to_char
                        ("You must include your name somewhere in your title!",
                         ch);
                return;
        }
        set_name(ch, argument);
        send_to_char("Ok.\n\r", ch);
        return;
}

#ifdef ACCOUNT
CMDF do_rpreward(CHAR_DATA * ch, char *argument)
{
        char      arg[MIL];

        if (IS_NPC(ch))
                return;

        if (!argument || argument[0] == '\0')
        {
                send_to_char
                        ("&RAvailable rewards &z(&RDeducted from current RPP&z)&W:\n\r",
                         ch);
                send_to_char("  &G 3 RPP)&W train <stat> \n\r", ch);
                send_to_char("  &G 5 RPP)&W changerace <race> \n\r", ch);
                send_to_char("  &G 8 RPP)&W fastengineer \n\r", ch);
                send_to_char("  &G10 RPP)&W makeclan <clan name> \n\r", ch);
                send_to_char
                        ("&RAutomatic rewards &z(&RNo cost deducted&z)&W:\n\r",
                         ch);
                send_to_char("  &G 2 RPP)&W Able to join a clan.\n\r", ch);
                send_to_char
                        ("  &G20 RPP)&W Able to create full force class characters.\n\r",
                         ch);

                send_to_char
                        ("\n\r\n\r&cIf you want other options added to this command, use \"idea\"\n\r&D",
                         ch);
                return;
        }

        argument = one_argument(argument, arg);

        if (!str_cmp(arg, "fastengineer"))
        {
                if (ch->pcdata->account->rpcurrent < 8)
                {
                        send_to_char
                                ("&RYou need 8 RPP to buy this reward.&D\n\r",
                                 ch);
                        return;
                }

                if (IS_SET(ch->pcdata->flags, PCFLAG_FASTENGINEER))
                {
                        send_to_char("&RYou already have this ability!\n\r&D",
                                     ch);
                        return;
                }

                SET_BIT(ch->pcdata->flags, PCFLAG_FASTENGINEER);
                send_to_char
                        ("&GYou have been rewarded for your RP with the Fast Engineering ability.\n\r&D",
                         ch);
                ch->pcdata->account->rpcurrent -= 8;
                save_account(ch->pcdata->account);
                return;
        }

        if (!str_cmp(arg, "train"))
        {
                if (ch->pcdata->account->rpcurrent < 3)
                {
                        send_to_char
                                ("&RYou need 3 RPP to buy this reward.&D\n\r",
                                 ch);
                        return;
                }

                do_train(ch, argument);
                return;
        }

        if (!str_cmp(arg, "makeclan"))
        {
                CLAN_DATA *clan;
                char      filename[256];

                if (ch->pcdata->account->rpcurrent < 10)
                {
                        send_to_char
                                ("&RYou need 10 RPP to buy this reward.\n\r&D",
                                 ch);
                        return;
                }

                if (get_clan(argument))
                {
                        send_to_char
                                ("&RThere is already a clan by that name!\n\r&D",
                                 ch);
                        return;
                }

                if (ch->pcdata->clan)
                {
                        send_to_char
                                ("&RYou must resign from your current clan if you want a new one!&D\n\r",
                                 ch);
                        return;
                }

                snprintf(filename, 256, "%s%s", CLAN_DIR,
                         strlower(smash_space(argument)));

                CREATE(clan, CLAN_DATA, 1);
                LINK(clan, first_clan, last_clan, next, prev);
                clan->next_subclan = NULL;
                clan->prev_subclan = NULL;
                clan->last_subclan = NULL;
                clan->first_subclan = NULL;
                clan->mainclan = NULL;
                clan->name = STRALLOC(argument);
                clan->description = STRALLOC("");
                clan->motto = STRALLOC("");
                clan->leader = STRALLOC(ch->name);
                clan->number1 = STRALLOC("");
                clan->number2 = STRALLOC("");
                clan->tmpstr = STRALLOC("");
                clan->enliston = 0;
                clan->alignment = 0;
                clan->roster = STRALLOC(ch->name);
                clan->clan_type = CLAN_GUILD;
                clan->filename = str_dup(filename);
                for (int i = 0; i <= 13; i++)
                {
                        clan->rank[i] = STRALLOC("None Set");
                        clan->salary[i] = 0;
                }
                save_clan(clan);
                write_clan_list();
                send_to_char
                        ("&GYour new clan has been created. Use clanstat and clanset to modify.&D\n\r",
                         ch);
                ch->pcdata->clan = clan;
                ch->pcdata->account->rpcurrent -= 10;
                save_account(ch->pcdata->account);
                return;
        }

        if (!str_cmp(arg, "changerace"))
        {
                RACE_DATA *race;

                if (ch->pcdata->account->rpcurrent < 5)
                {
                        send_to_char
                                ("&RYou need 5 RPP to buy this reward.\n\r&D",
                                 ch);
                        return;
                }

                if (!(race = get_race(argument)))
                {
                        send_to_char("&RThat is not a valid race!\n\r&D", ch);
                        return;
                }

                if (race == ch->race)
                {
                        send_to_char
                                ("&RYou are already a member of that race!\n\r&D",
                                 ch);
                        return;
                }

                for (int sn = 0; sn < top_sn; sn++)
                {
                        if (skill_table[sn]->races
                            && skill_table[sn]->races[0] != '\0')
                                if (is_name
                                    (ch->race->name(),
                                     skill_table[sn]->races))
                                        ch->pcdata->learned[sn] = 0;

                        for (int p = 0; p < MAX_BITS; p++)
                                if (xIS_SET(ch->xflags, p)
                                    && xIS_SET(skill_table[sn]->body_parts,
                                               p))
                                        ch->pcdata->learned[sn] = 0;
                }

                ch->affected_by -= ch->race->affected();

                ch->race = race;

                for (int ability = 0; ability < MAX_ABILITY; ability++)
                {
                        if (ch->skill_level[ability] > max_level(ch, ability))
                        {
                                ch->experience[ability] = 0;
                                ch->skill_level[ability] = 1;
                                if (ability == COMBAT_ABILITY)
                                        ch->max_hit = 500;
                                if (ability == FORCE_ABILITY
                                    || ability == HUNTING_ABILITY)
                                {
                                        if (ch->perm_frc > 0)
                                                ch->max_endurance = 700;
                                        else
                                                ch->max_endurance = 500;

                                        if (ability == FORCE_ABILITY)
                                                ch->max_endurance +=
                                                        ch->
                                                        skill_level
                                                        [HUNTING_ABILITY] *
                                                        ch->perm_dex;
                                        if (ability == HUNTING_ABILITY)
                                                ch->max_endurance +=
                                                        ch->
                                                        skill_level
                                                        [HUNTING_ABILITY] *
                                                        20;
                                }

                                for (int level = ch->skill_level[ability];
                                     level < max_level(ch, ability); level++)
                                {
                                        ch->experience[ability] =
                                                exp_level(level + 1);
                                        gain_exp_new(ch, 0, ability, FALSE);
                                }
                        }
                }

                if (!xIS_EMPTY(ch->race->body_parts())
                    && xIS_EMPTY(ch->xflags))
                        ch->xflags = ch->race->body_parts();
                if (!xIS_EMPTY(ch->race->body_parts()))
                {
                        for (int sn = 0; sn < top_sn; sn++)
                        {
                                if (!xIS_EMPTY(skill_table[sn]->body_parts))
                                        for (int p = 0; p < MAX_BITS; p++)
                                                if (xIS_SET(ch->xflags, p)
                                                    &&
                                                    xIS_SET(skill_table[sn]->
                                                            body_parts, p))
                                                        ch->pcdata->
                                                                learned[sn] =
                                                                100;
                        }
                }

                if (ch->hit > ch->max_hit)
                        ch->hit = ch->max_hit;
                if (ch->endurance > ch->max_endurance)
                        ch->endurance = ch->max_endurance;

                ch->affected_by += ch->race->affected();

                send_to_char
                        ("&GYour race has been changed. Enjoy your new RP possibilities!&D\n\r",
                         ch);
                ch->pcdata->account->rpcurrent -= 5;
                save_account(ch->pcdata->account);
                return;
        }

        do_rpreward(ch, "");
}
#endif

char     *convert_newline(char *str)
{
        static char ret[MAX_STRING_LENGTH];
        char     *retptr;

        retptr = ret;
        for (; *str != '\0'; str++)
        {
                if (*str == '\\')
                {
                        if (*(str - 1) == '\\')
                                continue;
                        else if (*(str + 1) == 'n')
                        {
                                *retptr = '\n';
                                retptr++;
                                str++;
                                continue;
                        }
                }
                *retptr = *str;
                retptr++;
        }
        *retptr = '\0';
        return ret;
}

CMDF do_testnewling(CHAR_DATA * ch, char *argument)
{
        send_to_char(convert_newline(argument), ch);
}

CMDF do_fprompt(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MIL];

        if (IS_NPC(ch))
        {
                send_to_char("NPC's can't change their prompt..\n\r", ch);
                return;
        }

        smash_tilde(argument);
        one_argument(argument, arg);
        if (!*arg)
        {
                if (ch->desc && ch->desc->descriptor)
                {
                        set_char_color(AT_WHITE, ch);
                        send_to_char("Your current prompt string:\n\r", ch);
                        if (ch->pcdata->fprompt
                            && ch->pcdata->fprompt[0] != '\0')
                        {
                                ch_printf(ch, "%s\n\r",
                                          full_color(ch->pcdata->fprompt));
                        }
                        else
                        {
                                send_to_char("No prompt\n\r", ch);
                        }
                        return;

                }
                send_to_char("Set prompt to what? (try help prompt)\n\r", ch);
                return;
        }

        if (ch->pcdata->fprompt)
                STRFREE(ch->pcdata->fprompt);

        ch->pcdata->fprompt = STRALLOC(argument);
        send_to_char("Ok.\n\r", ch);
        return;
}
