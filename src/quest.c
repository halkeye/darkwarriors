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
 *                                 Quest Pruning Module                                  *
 ****************************************************************************************/
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*       ROM 2.4 is copyright 1993-1995 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@pacinfo.com)                              *
*           Gabrielle Taylor (gtaylor@pacinfo.com)                         *
*           Brian Moore (rom@rom.efn.org)                                  *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
***************************************************************************/

/***************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com   *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this  * 
*  code is allowed provided you add a credit line to the effect of:        *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest    *
*  of the standard diku/rom credits. If you use this or a modified version *
*  of this code, let me know via email: moongate@moongate.ams.com. Further *
*  updates will be posted to the rom mailing list. If you'd like to get    *
*  the latest version of quest.c, please send a request to the above add-  *
*  ress. Quest Code v2.00.                                                 *
***************************************************************************/

/***************************************************************************
*  Ported to SMAUG by Vir of Eternal Struggle (es.mudservices.com 4321)    *
*  Additional changes to make life easier also by Vir.  Quest Code         *
*  originally (C)opyright 1996 Ryan Addams of MOONGATE.  Thanx for the     *
*  code, Ryan!! For more SMAUG code, e-mail "leckey@rogers.wave.ca"        *
***************************************************************************/

/***************************************************************************
*                  Ported to Smaug 1.4 by LrdElder                         *
***************************************************************************/
/***************************************************************************
*                  Ported to SWR 1.01 by Gendi (no current MUD)            *
***************************************************************************/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* Object vnums for Quest Rewards */

#define QUEST_ITEM1 10326
#define QUEST_ITEM2 10325
#define QUEST_ITEM3 10324
#define QUEST_ITEM4 6608
#define QUEST_ITEM5 6607


#define QUEST_VALUE1 1500
#define QUEST_VALUE2 1000
#define QUEST_VALUE3 1000
#define QUEST_VALUE4 250
#define QUEST_VALUE5 200

/* Object vnums for object quest 'tokens'. In Moongate, the tokens are
   things like 'the Shield of Moongate', 'the Sceptre of Moongate'. These
   items are worthless and have the rot-death flag, as they are placed
   into the world when a player receives an object quest. */

#define QUEST_OBJQUEST1 10350
#define QUEST_OBJQUEST2 10351
#define QUEST_OBJQUEST3 10352
#define QUEST_OBJQUEST4 10353
#define QUEST_OBJQUEST5 10354

/* Local functions */

void      generate_quest
args((CHAR_DATA * ch, CHAR_DATA * questman, char *argument));
void quest_update args((void));
bool qchance args((int num));

bool qchance(int num)
{
        if (number_range(1, 100) <= num)
                return TRUE;
        else
                return FALSE;
}

/* The main quest function */

CMDF do_aquest(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *questman;
        OBJ_DATA *obj = NULL, *obj_next;
        OBJ_INDEX_DATA *obj1, *obj2, *obj3, *obj4, *obj5;
        OBJ_INDEX_DATA *questinfoobj;
        MOB_INDEX_DATA *questinfo;
        char      buf[MAX_STRING_LENGTH];
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (!str_cmp(arg1, "info"))
        {
                if (IS_SET(ch->act, PLR_QUESTOR))
                {
                        if (ch->questmob == -1
                            && ch->questgiver->short_descr != NULL)
                        {
                                snprintf(buf, MSL,
                                         "Your quest is ALMOST complete!\n\rGet back to %s before your time runs out!\n\r",
                                         ch->questgiver->short_descr);
                                send_to_char(buf, ch);
                        }
                        else if (ch->questobj > 0)
                        {
                                questinfoobj = get_obj_index(ch->questobj);
                                if (questinfoobj != NULL)
                                {
                                        snprintf(buf, MSL,
                                                 "You are on a quest to recover the fabled %s!\n\r",
                                                 questinfoobj->name);
                                        send_to_char(buf, ch);
                                }
                                else
                                        send_to_char
                                                ("You aren't currently on a quest.\n\r",
                                                 ch);
                                return;
                        }
                        else if (ch->questmob > 0)
                        {
                                questinfo = get_mob_index(ch->questmob);
                                if (questinfo != NULL)
                                {
                                        snprintf(buf, MSL,
                                                 "You are on a quest to slay the dreaded %s!\n\r",
                                                 questinfo->short_descr);
                                        send_to_char(buf, ch);
                                }
                                else
                                        send_to_char
                                                ("You aren't currently on a quest.\n\r",
                                                 ch);
                                return;
                        }
                }
                else
                        send_to_char("You aren't currently on a quest.\n\r",
                                     ch);
                return;
        }
        if (!str_cmp(arg1, "points"))
        {
                snprintf(buf, MSL, "You have %d quest points.\n\r",
                         ch->pcdata->quest_curr);
                send_to_char(buf, ch);
                return;
        }
        else if (!str_cmp(arg1, "time"))
        {
                if (!IS_SET(ch->act, PLR_QUESTOR))
                {
                        send_to_char("You aren't currently on a quest.\n\r",
                                     ch);
                        if (ch->nextquest > 1)
                        {
                                snprintf(buf, MSL,
                                         "There are %d minutes remaining until you can go on another quest.\n\r",
                                         ch->nextquest);
                                send_to_char(buf, ch);
                        }
                        else if (ch->nextquest == 1)
                        {
                                snprintf(buf, MSL,
                                         "There is less than a minute remaining until you can go on another quest.\n\r");
                                send_to_char(buf, ch);
                        }
                }
                else if (ch->countdown > 0)
                {
                        snprintf(buf, MSL,
                                 "Time left for current quest: %d\n\r",
                                 ch->countdown);
                        send_to_char(buf, ch);
                }
                return;
        }

/* Checks for a character in the room with spec_questmaster set. This special
   procedure must be defined in special.c. You could instead use an 
   ACT_QUESTMASTER flag instead of a special procedure. */

        for (questman = ch->in_room->first_person; questman != NULL;
             questman = questman->next_in_room)
        {
                if (!IS_NPC(questman))
                        continue;
                if (questman->spec_fun == spec_lookup("spec_questmaster"))
                        break;
        }

        if (questman == NULL
            || questman->spec_fun != spec_lookup("spec_questmaster"))
        {
                send_to_char("You can't do that here.\n\r", ch);
                return;
        }

        if (questman->position == POS_FIGHTING)
        {
                send_to_char("Wait until the fighting stops.\n\r", ch);
                return;
        }

        ch->questgiver = questman;

/* And, of course, you will need to change the following lines for YOUR
   quest item information. Quest items on Moongate are unbalanced, very
   very nice items, and no one has one yet, because it takes awhile to
   build up quest points :> Make the item worth their while. */

        obj1 = get_obj_index(QUEST_ITEM1);
        obj2 = get_obj_index(QUEST_ITEM2);
        obj3 = get_obj_index(QUEST_ITEM3);
        obj4 = get_obj_index(QUEST_ITEM4);
        obj5 = get_obj_index(QUEST_ITEM5);

        if (obj1 == NULL || obj2 == NULL || obj3 == NULL || obj4 == NULL
            || obj5 == NULL)
        {
                bug("Error loading quest objects. Char: ", ch->name);
                return;
        }

        if (!str_cmp(arg1, "list"))
        {
                act(AT_PLAIN, "$n asks $N for a list of quest items.", ch,
                    NULL, questman, TO_ROOM);
                act(AT_PLAIN, "You ask $N for a list of quest items.", ch,
                    NULL, questman, TO_CHAR);
                snprintf(buf, MSL, "Current Quest Items available for Purchase:\n\r\n\r\
[1] %dqp.......... %s\n\r\
[2] %dqp.......... %s\n\r\
[3] %dqp.......... %s\n\r\
[4] %dqp.......... %s\n\r\
[5] %dqp.......... %s\n\r\
[6] 200qp......... 100,000 credits\n\r\
[7] 200qp......... 30 hit points\n\r", QUEST_VALUE1, obj1->short_descr,
                         QUEST_VALUE2, obj2->short_descr, QUEST_VALUE3, obj3->short_descr, QUEST_VALUE4, obj4->short_descr, QUEST_VALUE5, obj5->short_descr);

                send_to_char(buf, ch);
                return;
        }

        if (!str_cmp(arg1, "fail"))
        {
                if (IS_SET(ch->act, PLR_QUESTOR))
                {
                        if (ch->countdown > 0)
                        {

                                ch->nextquest = 20;
                                snprintf(buf, MSL,
                                         "You give up on your quest. You may quest again in %d minutes.\n\r",
                                         ch->nextquest);
                                send_to_char(buf, ch);
                                REMOVE_BIT(ch->act, PLR_QUESTOR);
                                ch->questgiver = NULL;
                                ch->countdown = 0;
                                ch->questmob = 0;
                                return;
                        }
                }

                send_to_char("You are not on a quest!", ch);
                return;
        }

        else if (!str_cmp(arg1, "buy"))
        {
                if (arg2[0] == '\0')
                {
                        send_to_char
                                ("To buy an item, type 'QUEST BUY <item>'.\n\r",
                                 ch);
                        return;
                }
                if (is_name(arg2, "1"))
                {
                        if (ch->pcdata->quest_curr >= QUEST_VALUE1)
                        {
                                ch->pcdata->quest_curr -= QUEST_VALUE1;
                                obj = create_object(get_obj_index
                                                    (QUEST_ITEM1),
                                                    ch->top_level);
                        }
                        else
                        {
                                snprintf(buf, MSL,
                                         "Sorry, %s, but you don't have enough quest points for that.",
                                         ch->name);
                                do_say(questman, buf);
                                return;
                        }
                }
                else if (is_name(arg2, "2"))
                {
                        if (ch->pcdata->quest_curr >= QUEST_VALUE2)
                        {
                                ch->pcdata->quest_curr -= QUEST_VALUE2;
                                obj = create_object(get_obj_index
                                                    (QUEST_ITEM2),
                                                    ch->top_level);
                        }
                        else
                        {
                                snprintf(buf, MSL,
                                         "Sorry, %s, but you don't have enough quest points for that.",
                                         ch->name);
                                do_say(questman, buf);
                                return;
                        }
                }
                else if (is_name(arg2, "3"))
                {
                        if (ch->pcdata->quest_curr >= QUEST_VALUE3)
                        {
                                ch->pcdata->quest_curr -= QUEST_VALUE3;
                                obj = create_object(get_obj_index
                                                    (QUEST_ITEM3),
                                                    ch->top_level);
                        }
                        else
                        {
                                snprintf(buf, MSL,
                                         "Sorry, %s, but you don't have enough quest points for that.",
                                         ch->name);
                                do_say(questman, buf);
                                return;
                        }
                }
                else if (is_name(arg2, "4"))
                {
                        if (ch->pcdata->quest_curr >= QUEST_VALUE4)
                        {
                                ch->pcdata->quest_curr -= QUEST_VALUE4;
                                obj = create_object(get_obj_index
                                                    (QUEST_ITEM4),
                                                    ch->top_level);
                        }
                        else
                        {
                                snprintf(buf, MSL,
                                         "Sorry, %s, but you don't have enough quest points for that.",
                                         ch->name);
                                do_say(questman, buf);
                                return;
                        }
                }
                else if (is_name(arg2, "5"))
                {
                        if (ch->pcdata->quest_curr >= QUEST_VALUE5)
                        {
                                ch->pcdata->quest_curr -= QUEST_VALUE5;
                                obj = create_object(get_obj_index
                                                    (QUEST_ITEM5),
                                                    ch->top_level);
                        }
                        else
                        {
                                snprintf(buf, MSL,
                                         "Sorry, %s, but you don't have enough quest points for that.",
                                         ch->name);
                                do_say(questman, buf);
                                return;
                        }
                }
                else if (is_name(arg2, "7"))
                {
                        if (ch->questhp > 270)
                        {
                                snprintf(buf, MSL,
                                         "Sorry, %s, but you are too healthy.",
                                         ch->name);
                                do_say(questman, buf);
                                return;
                        }

                        if (ch->pcdata->quest_curr >= 200)
                        {
                                ch->pcdata->quest_curr -= 200;
                                ch->max_hit += 30;
                                ch->hit += 30;
                                ch->questhp += 30;
                                act(AT_MAGIC, "$N makes you more healthy!",
                                    ch, NULL, questman, TO_CHAR);
                                return;
                        }
                        else
                        {
                                snprintf(buf, MSL,
                                         "Sorry, %s, but you don't have enough quest points for that.",
                                         ch->name);
                                do_say(questman, buf);
                                return;
                        }
                }

                else if (is_name(arg2, "6"))
                {
                        if (ch->pcdata->quest_curr >= 200)
                        {
                                ch->pcdata->quest_curr -= 200;
                                ch->gold += 100000;
                                act(AT_MAGIC,
                                    "$N gives 100,000 credits to $n.", ch,
                                    NULL, questman, TO_ROOM);
                                act(AT_MAGIC, "$N hands you a cred stick.",
                                    ch, NULL, questman, TO_CHAR);
                                return;
                        }
                        else
                        {
                                snprintf(buf, MSL,
                                         "Sorry, %s, but you don't have enough quest points for that.",
                                         ch->name);
                                do_say(questman, buf);
                                return;
                        }
                }
                else
                {
                        snprintf(buf, MSL, "I don't have that item, %s.",
                                 ch->name);
                        do_say(questman, buf);
                }
                if (obj != NULL)
                {
                        act(AT_PLAIN, "$N gives something to $n.", ch, obj,
                            questman, TO_ROOM);
                        act(AT_PLAIN, "$N gives you your reward.", ch, obj,
                            questman, TO_CHAR);
                        obj_to_char(obj, ch);
                }
                return;
        }
        else if (!str_cmp(arg1, "request"))
        {
                act(AT_PLAIN, "$n asks $N for a quest.", ch, NULL, questman,
                    TO_ROOM);
                act(AT_PLAIN, "You ask $N for a quest.", ch, NULL, questman,
                    TO_CHAR);
                if (IS_SET(ch->act, PLR_QUESTOR))
                {
                        snprintf(buf, MSL,
                                 "But you're already on a quest!\n\rBetter hurry up and finish it!");
                        do_say(questman, buf);
                        return;
                }
                if (ch->nextquest > 0 && !IS_IMMORTAL(ch))
                {
                        snprintf(buf, MSL,
                                 "You're very brave, %s, but let someone else have a chance.",
                                 ch->name);
                        do_say(questman, buf);
                        snprintf(buf, MSL,
                                 "Please come back in about %d minutes.",
                                 ch->nextquest);
                        do_say(questman, buf);
                        return;
                }

                if (str_cmp(arg2, "object") && str_cmp(arg2, "mob"))
                {
                        do_say(questman,
                               "Would you prefer an object, or a mob for your quest?");
                        return;
                }

                snprintf(buf, MSL, "Thank you, %s!", ch->name);
                do_say(questman, buf);

                generate_quest(ch, questman, arg2);

                if (ch->questmob > 0 || ch->questobj > 0)
                {
                        ch->countdown = number_range(12, 28);
                        SET_BIT(ch->act, PLR_QUESTOR);
                        snprintf(buf, MSL,
                                 "You have %d minutes to complete this quest.",
                                 ch->countdown);
                        do_say(questman, buf);
                        snprintf(buf, MSL, "May the gods go with you!");
                        do_say(questman, buf);
                }
                return;
        }
        else if (!str_cmp(arg1, "complete"))
        {
                act(AT_PLAIN, "$n informs $N $e has completed $s quest.", ch,
                    NULL, questman, TO_ROOM);
                act(AT_PLAIN, "You inform $N you have completed $s quest.",
                    ch, NULL, questman, TO_CHAR);
                if (ch->questgiver != questman)
                {
                        snprintf(buf, MSL,
                                 "I never sent you on a quest! Perhaps you're thinking of someone else.");
                        do_say(questman, buf);
                        return;
                }

                if (IS_SET(ch->act, PLR_QUESTOR))
                {
                        if (ch->questmob == -1 && ch->countdown > 0)
                        {
                                int       reward, pointreward, dipexpreward;

                                reward = number_range(1000, 15000);
                                pointreward = number_range(15, 125);

                                snprintf(buf, MSL,
                                         "Congratulations on completing your quest!");
                                do_say(questman, buf);
                                snprintf(buf, MSL,
                                         "As a reward, I am giving you %d quest points, and %d credits.",
                                         pointreward, reward);
                                do_say(questman, buf);

                                if (qchance(15))
                                {
                                        dipexpreward =
                                                (ch->
                                                 skill_level
                                                 [DIPLOMACY_ABILITY] * 1000);
                                        snprintf(buf, MSL,
                                                 "You gain %d diplomatic experience!\n\r",
                                                 dipexpreward);
                                        send_to_char(buf, ch);
                                        gain_exp(ch, dipexpreward,
                                                 DIPLOMACY_ABILITY);
                                }


                                REMOVE_BIT(ch->act, PLR_QUESTOR);
                                ch->questgiver = NULL;
                                ch->countdown = 0;
                                ch->questmob = 0;
                                ch->questobj = 0;
                                ch->nextquest = 20;
                                ch->gold += reward;
                                ch->pcdata->quest_curr += pointreward;

                                return;
                        }
                        else if (ch->questobj > 0 && ch->countdown > 0)
                        {
                                bool      obj_found = FALSE;

                                for (obj = ch->first_carrying; obj != NULL;
                                     obj = obj_next)
                                {
                                        obj_next = obj->next_content;

                                        if (obj != NULL
                                            && obj->pIndexData->vnum ==
                                            ch->questobj)
                                        {
                                                obj_found = TRUE;
                                                break;
                                        }
                                }
                                if (obj_found == TRUE)
                                {
                                        int       reward, pointreward,
                                                dipexpreward;

                                        reward = number_range(1000, 15000);
                                        pointreward = number_range(15, 125);

                                        act(AT_PLAIN, "You hand $p to $N.",
                                            ch, obj, questman, TO_CHAR);
                                        act(AT_PLAIN, "$n hands $p to $N.",
                                            ch, obj, questman, TO_ROOM);

                                        snprintf(buf, MSL,
                                                 "Congratulations on completing your quest!");
                                        do_say(questman, buf);
                                        snprintf(buf, MSL,
                                                 "As a reward, I am giving you %d quest points, and %d credits.",
                                                 pointreward, reward);
                                        do_say(questman, buf);

                                        if (qchance(15))
                                        {
                                                dipexpreward =
                                                        (ch->
                                                         skill_level
                                                         [DIPLOMACY_ABILITY] *
                                                         1000);
                                                snprintf(buf, MSL,
                                                         "You gain %d diplomatic experience!\n\r",
                                                         dipexpreward);
                                                send_to_char(buf, ch);
                                                gain_exp(ch, dipexpreward,
                                                         DIPLOMACY_ABILITY);
                                        }


                                        REMOVE_BIT(ch->act, PLR_QUESTOR);
                                        ch->questgiver = NULL;
                                        ch->countdown = 0;
                                        ch->questmob = 0;
                                        ch->questobj = 0;
                                        ch->nextquest = 20;
                                        ch->gold += reward;
                                        ch->pcdata->quest_curr += pointreward;
                                        extract_obj(obj);
                                        return;
                                }
                                else
                                {
                                        snprintf(buf, MSL,
                                                 "You haven't completed the quest yet, but there is still time!");
                                        do_say(questman, buf);
                                        return;
                                }
                                return;
                        }
                        else if ((ch->questmob > 0 || ch->questobj > 0)
                                 && ch->countdown > 0)
                        {
                                snprintf(buf, MSL,
                                         "You haven't completed the quest yet, but there is still time!");
                                do_say(questman, buf);
                                return;
                        }
                }
                if (ch->nextquest > 0)
                        snprintf(buf, MSL,
                                 "But you didn't complete your quest in time!");
                else
                        snprintf(buf, MSL,
                                 "You have to REQUEST a quest first, %s.",
                                 ch->name);
                do_say(questman, buf);
                return;
        }

        send_to_char
                ("QUEST commands: POINTS INFO TIME REQUEST COMPLETE LIST BUY FAIL.\n\r",
                 ch);
        send_to_char("For more information, type 'HELP QUEST'.\n\r", ch);
        return;
}

void generate_quest(CHAR_DATA * ch, CHAR_DATA * questman, char *argument)
{
        CHAR_DATA *victim = NULL;
        MOB_INDEX_DATA *vsearch;
        ROOM_INDEX_DATA *room;
        OBJ_DATA *questitem;
        char      buf[MAX_STRING_LENGTH];
        long      mcounter;
        int       level_diff, mob_vnum;

        /*
         * Randomly selects a mob from the world mob list. If you don't
         * want a mob to be selected, make sure it is immune to summon.
         * Or, you could add a new mob flag called ACT_NOQUEST. The mob
         * is selected for both mob and obj quests, even tho in the obj
         * quest the mob is not used. This is done to assure the level
         * of difficulty for the area isn't too great for the player. 
         */

        for (mcounter = 0; mcounter < 99999; mcounter++)
        {
                mob_vnum = number_range(50, 43000);

                if ((vsearch = get_mob_index(mob_vnum)) != NULL)
                {
                        level_diff = vsearch->level - ch->top_level;

                        /*
                         * Level differences to search for. Moongate has 350
                         * levels, so you will want to tweak these greater or
                         * less than statements for yourself. - Vassago 
                         */

                        if (((level_diff < 25 && level_diff > -25)
                             || (ch->top_level > 30 && ch->top_level < 40
                                 && vsearch->level > 30
                                 && vsearch->level < 50)
                             || (ch->top_level > 40 && vsearch->level > 40))
                            && (IS_EVIL(ch) ? IS_EVIL(vsearch) :
                                IS_GOOD(vsearch)) && vsearch->pShop == NULL
                            && vsearch->rShop == NULL
                            && !IS_SET(vsearch->act, ACT_TRAIN)
                            && !IS_SET(vsearch->act, ACT_PRACTICE)
                            && !IS_SET(vsearch->act, ACT_PROTOTYPE)
                            &&
                            !((victim =
                               get_char_world(ch,
                                              vsearch->player_name)) == NULL)
                            && IS_NPC(victim)
                            && victim->in_room
                            && !xIS_SET(victim->in_room->room_flags,
                                        ROOM_SAFE) && victim->in_room->area
                            && !IS_SET(victim->in_room->area->flags,
                                       AFLAG_NOQUEST)
                            && !IS_SET(vsearch->act, ACT_TRAIN)
                            && !IS_SET(vsearch->act, ACT_PRACTICE)
                            && !IS_SET(vsearch->act, ACT_IMMORTAL))
                                break;
                        else
                                vsearch = NULL;
                }
        }

        if (vsearch == NULL)
        {
                snprintf(buf, MSL,
                         "I'm sorry, but I don't have any quests for you at this time.");
                do_say(questman, buf);
                snprintf(buf, MSL, "Try again later.");
                do_say(questman, buf);
                ch->nextquest = 0;
                ch->questmob = 0;
                ch->questobj = 0;
                return;
        }

        if ((room = find_location(ch, victim->name)) == NULL)
        {
                snprintf(buf, MSL,
                         "I'm sorry, but I don't have any quests for you at this time.");
                do_say(questman, buf);
                snprintf(buf, MSL, "Try again later.");
                do_say(questman, buf);
                ch->questmob = 0;
                ch->questobj = 0;
                ch->nextquest = 1;
                return;
        }

        if (IS_SET(room->area->flags,AFLAG_NOQUEST))
        {
                snprintf(buf, MSL,
                         "I'm sorry, my intel is unable to locate the object of your quest.");
                do_say(questman, buf);
                snprintf(buf, MSL, "Try again later and i'll have another quest for you.");
                do_say(questman, buf);
                ch->questmob = 0;
                ch->questobj = 0;
                ch->nextquest = 1;
                return;
        }
        /*
         * 40% chance it will send the player on a 'recover item' quest. 
         */

        if (!str_cmp(argument, "object"))
        {
                int       objvnum = 0;

                switch (number_range(0, 4))
                {
                case 0:
                        objvnum = QUEST_OBJQUEST1;
                        break;

                case 1:
                        objvnum = QUEST_OBJQUEST2;
                        break;

                case 2:
                        objvnum = QUEST_OBJQUEST3;
                        break;

                case 3:
                        objvnum = QUEST_OBJQUEST4;
                        break;

                case 4:
                        objvnum = QUEST_OBJQUEST5;
                        break;
                }

                questitem =
                        create_object(get_obj_index(objvnum), ch->top_level);
                obj_to_room(questitem, room);
                ch->questobj = questitem->pIndexData->vnum;

                snprintf(buf, MSL,
                         "Someone has stolen %s from me. Please retreive it.",
                         questitem->short_descr);
                do_say(questman, buf);
                do_say(questman,
                       "I have used the force to find the location of the item.");

                /*
                 * I changed my area names so that they have just the name of the area
                 * and none of the level stuff. You may want to comment these next two
                 * lines. - Vassago 
                 */

                snprintf(buf, MSL, "Look in the general area of %s for %s!",
                         room->area->name, room->name);
                do_say(questman, buf);
                return;
        }

        /*
         * Quest to kill a mob 
         */

        else
        {
                switch (number_range(0, 1))
                {
                case 0:
                        snprintf(buf, MSL,
                                 "There is someone I need killed. %s has personally insulted me, and I want retribution!",
                                 victim->short_descr);
                        do_say(questman, buf);
                        snprintf(buf, MSL, "They must be destroyed!");
                        do_say(questman, buf);
                        break;

                case 1:
                        snprintf(buf, MSL,
                                 "The galaxy's most heinous criminal, %s, is loose!",
                                 victim->short_descr);
                        do_say(questman, buf);
                        snprintf(buf, MSL,
                                 "Since the escape, %s has murdered %d civillians!",
                                 victim->short_descr, number_range(2, 20));
                        do_say(questman, buf);
                        do_say(questman,
                               "The penalty for this crime is death, and you are to deliver the sentence!");
                        break;
                }

                if (room->name != NULL)
                {
                        snprintf(buf, MSL,
                                 "Seek %s out somewhere in the vicinity of %s!",
                                 victim->short_descr, room->name);
                        do_say(questman, buf);

                        /*
                         * I changed my area names so that they have just the name of the area
                         * and none of the level stuff. You may want to comment these next two
                         * lines. - Vassago 
                         */

                        snprintf(buf, MSL,
                                 "That location is in the general area of %s.",
                                 room->area->name);
                        do_say(questman, buf);
                }
                ch->questmob = victim->pIndexData->vnum;
        }
        return;
}

/* Called from update_handler() by pulse_area */

void quest_update(void)
{
        CHAR_DATA *ch, *ch_next;

        for (ch = first_char; ch != NULL; ch = ch_next)
        {
                ch_next = ch->next;

                if (IS_NPC(ch))
                        continue;

                if (ch->nextquest > 0)
                {
                        ch->nextquest--;

                        if (ch->nextquest == 0)
                        {
                                send_to_char("You may now quest again.\n\r",
                                             ch);
                                return;
                        }
                }
                else if (IS_SET(ch->act, PLR_QUESTOR))
                {
                        if (--ch->countdown <= 0)
                        {
                                char      buf[MAX_STRING_LENGTH];

                                ch->nextquest = 20;
                                snprintf(buf, MSL,
                                         "You have run out of time for your quest!\n\rYou may quest again in %d minutes.\n\r",
                                         ch->nextquest);
                                send_to_char(buf, ch);
                                REMOVE_BIT(ch->act, PLR_QUESTOR);
                                ch->questgiver = NULL;
                                ch->countdown = 0;
                                ch->questmob = 0;
                        }
                        if (ch->countdown > 0 && ch->countdown < 6)
                        {
                                send_to_char
                                        ("Better hurry, you're almost out of time for your quest!\n\r",
                                         ch);
                                return;
                        }
                }
        }
        return;
}
