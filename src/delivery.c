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
 *                                     Delivery Code                                     *
 *****************************************************************************************
 *                $Id: delivery.c 1330 2005-12-05 03:23:24Z halkeye $                 *
 *****************************************************************************************/



#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"


void fwrite_delivery(CHAR_DATA * ch, OBJ_DATA * obj)
{
        FILE     *fp = NULL;
        char      strsave[MAX_INPUT_LENGTH];
        EXTRA_DESCR_DATA *ed;
        AFFECT_DATA *paf;
        sh_int    wear, wear_loc, x;

        if (!obj)
        {
                bug("Fwrite_delivery: NULL object.", 0);
                bug(ch->name, 0);
                return;
        }

        snprintf(strsave, MSL, "%s%s", MAIL_DIR, capitalize(ch->name));

        if ((fp = fopen(strsave, "w")) != NULL)
        {

                fprintf(fp, "#OBJECT\n");

                if (obj->count > 1)
                        fprintf(fp, "Count        %d\n", obj->count);
                if (QUICKMATCH(obj->name, obj->pIndexData->name) == 0)
                        fprintf(fp, "Name         %s~\n", obj->name);
                if (QUICKMATCH(obj->short_descr, obj->pIndexData->short_descr)
                    == 0)
                        fprintf(fp, "ShortDescr   %s~\n", obj->short_descr);
                if (QUICKMATCH(obj->description, obj->pIndexData->description)
                    == 0)
                        fprintf(fp, "Description  %s~\n", obj->description);
                if (QUICKMATCH(obj->action_desc, obj->pIndexData->action_desc)
                    == 0)
                        fprintf(fp, "ActionDesc   %s~\n", obj->action_desc);
                fprintf(fp, "Vnum         %d\n", obj->pIndexData->vnum);
                if (obj->extra_flags != obj->pIndexData->extra_flags)
                        fprintf(fp, "ExtraFlags   %d\n", obj->extra_flags);
                if (obj->wear_flags != obj->pIndexData->wear_flags)
                        fprintf(fp, "WearFlags    %d\n", obj->wear_flags);
                wear_loc = -1;
                for (wear = 0; wear < MAX_WEAR; wear++)
                        for (x = 0; x < MAX_LAYERS; x++)
                                if (obj == save_equipment[wear][x])
                                {
                                        wear_loc = wear;
                                        break;
                                }
                                else if (!save_equipment[wear][x])
                                        break;
                if (wear_loc != -1)
                        fprintf(fp, "WearLoc      %d\n", wear_loc);
                if (obj->item_type != obj->pIndexData->item_type)
                        fprintf(fp, "ItemType     %d\n", obj->item_type);
                if (obj->weight != obj->pIndexData->weight)
                        fprintf(fp, "Weight       %d\n", obj->weight);
                if (obj->level)
                        fprintf(fp, "Level        %d\n", obj->level);
                if (obj->timer)
                        fprintf(fp, "Timer        %d\n", obj->timer);
                if (obj->cost != obj->pIndexData->cost)
                        fprintf(fp, "Cost         %d\n", obj->cost);
                if (obj->value[0] || obj->value[1] || obj->value[2]
                    || obj->value[3] || obj->value[4] || obj->value[5])
                        fprintf(fp, "Values       %d %d %d %d %d %d\n",
                                obj->value[0], obj->value[1], obj->value[2],
                                obj->value[3], obj->value[4], obj->value[5]);

                switch (obj->item_type)
                {
                case ITEM_PILL:    /* was down there with staff and wand, wrongly - Scryn */
                case ITEM_POTION:
                case ITEM_SCROLL:
                        if (IS_VALID_SN(obj->value[1]))
                                fprintf(fp, "Spell 1      '%s'\n",
                                        skill_table[obj->value[1]]->name);

                        if (IS_VALID_SN(obj->value[2]))
                                fprintf(fp, "Spell 2      '%s'\n",
                                        skill_table[obj->value[2]]->name);

                        if (IS_VALID_SN(obj->value[3]))
                                fprintf(fp, "Spell 3      '%s'\n",
                                        skill_table[obj->value[3]]->name);

                        break;

                case ITEM_STAFF:
                case ITEM_WAND:
                        if (IS_VALID_SN(obj->value[3]))
                                fprintf(fp, "Spell 3      '%s'\n",
                                        skill_table[obj->value[3]]->name);

                        break;
                case ITEM_SALVE:
                        if (IS_VALID_SN(obj->value[4]))
                                fprintf(fp, "Spell 4      '%s'\n",
                                        skill_table[obj->value[4]]->name);

                        break;
                }

                for (paf = obj->first_affect; paf; paf = paf->next)
                {
                        /*
                         * Save extra object affects                -Thoric
                         */
                        if (paf->type < 0 || paf->type >= top_sn)
                        {
                                fprintf(fp, "Affect       %d %d %d %d %d\n",
                                        paf->type,
                                        paf->duration,
                                        ((paf->location == APPLY_WEAPONSPELL
                                          || paf->location == APPLY_WEARSPELL
                                          || paf->location ==
                                          APPLY_REMOVESPELL
                                          || paf->location == APPLY_STRIPSN)
                                         && IS_VALID_SN(paf->
                                                        modifier)) ?
                                        skill_table[paf->modifier]->
                                        slot : paf->modifier, paf->location,
                                        paf->bitvector);
                        }
                        else
                                fprintf(fp, "AffectData   '%s' %d %d %d %d\n",
                                        skill_table[paf->type]->name,
                                        paf->duration,
                                        ((paf->location == APPLY_WEAPONSPELL
                                          || paf->location == APPLY_WEARSPELL
                                          || paf->location ==
                                          APPLY_REMOVESPELL
                                          || paf->location == APPLY_STRIPSN)
                                         && IS_VALID_SN(paf->
                                                        modifier)) ?
                                        skill_table[paf->modifier]->
                                        slot : paf->modifier, paf->location,
                                        paf->bitvector);
                }

                for (ed = obj->first_extradesc; ed; ed = ed->next)
                        fprintf(fp, "ExtraDescr   %s~ %s~\n",
                                ed->keyword, ed->description);


                fprintf(fp, "End\n\n");

                if (obj->first_content)
                        fwrite_obj(ch, obj->last_content, fp, 0, OS_CARRY,
                                   FALSE);

        }
        fprintf(fp, "#END \n\r");
        FCLOSE(fp);
        return;
}



CMDF do_deliver(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj;
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        char      strsave[MAX_INPUT_LENGTH];
        char      mobbuf[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        CHAR_DATA *mob;
        FILE     *fp = NULL;
        int       cost;

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);

        if (IS_NPC(ch))
                return;

        for (mob = ch->in_room->first_person; mob; mob = mob->next_in_room)
                if (IS_NPC(mob) && IS_SET(mob->act, ACT_MAIL))
                        break;

        if (!mob)
        {
                send_to_char("You need to visit the post office.\n\r", ch);
                return;
        }



        if (arg[0] == '\0')
        {
                send_to_char("Deliver what to who?\n\r", ch);
                send_to_char("Syntax: deliver <object> <player>\n\r", ch);
                return;
        }
        if (!str_cmp(arg, "pickup"))
        {
                snprintf(strsave, MSL, "%s%s", MAIL_DIR,
                         capitalize(ch->name));

                if ((fp = fopen(strsave, "r")) != NULL)
                {
                        for (;;)
                        {
                                char      letter;
                                char     *word;

                                letter = fread_letter(fp);
                                if (letter == '#')
                                {
                                        word = fread_word(fp);

                                        if (!strcmp(word, "END"))
                                                break;

                                        if (!strcmp(word, "OBJECT"))
                                                fread_obj(ch, fp, OS_CARRY);
                                }
                        }
                        FCLOSE(fp);
                        if (!remove(strsave))
                        {
                                send_to_char("You retrive your delivery.\n\r",
                                             ch);
                                REMOVE_BIT(ch->pcdata->flags, PCFLAG_GOTMAIL);
                        }
                        else if (errno != ENOENT)
                                bug("Delivery unable to delete mailbox.\n\r",
                                    0);
                        return;
                }
                else
                {
                        send_to_char("You currently have no deliveries.\n\r",
                                     ch);
                        return;
                }
        }
        if (ms_find_obj(ch))
                return;

        if (!(obj = get_obj_carry(ch, arg)))
        {
                send_to_char("You do not have that item.\n\r", ch);
                return;
        }
        separate_obj(obj);
        if (IS_OBJ_STAT(obj, ITEM_NODROP) || IS_OBJ_STAT(obj, ITEM_PROTOTYPE))
        {
                snprintf(mobbuf, MSL,
                         "I'm sorry we dont handle objects of that nature.");
                do_say(mob, mobbuf);
                return;
        }

        if ((victim = get_char_world(ch, arg2)) == NULL || IS_NPC(victim))
        {
                snprintf(mobbuf, MSL, "%s",
                         "They're not here right now... let me check their mailbox.");
                do_say(mob, mobbuf);
                snprintf(strsave, MSL, "%s%c/%s", PLAYER_DIR,
                         tolower(arg2[0]), capitalize(arg2));
                if ((fp = fopen(strsave, "r")) != NULL);
                else
                {
                        snprintf(mobbuf, MSL,
                                 "I dont have a mailbox for %s... Are you sure thats the correct name?",
                                 capitalize(arg2));
                        do_say(mob, mobbuf);
                        return;
                }
                snprintf(strsave, MSL, "%s%s", MAIL_DIR, capitalize(arg2));
                if ((fp = fopen(strsave, "r")) != NULL)
                {
                        FCLOSE(fp);
                        snprintf(mobbuf, MSL, "%s",
                                 "Oh... I'm sorry, But that mailbox is already full. Anything else i can do for you?");
                        do_say(mob, mobbuf);
                        return;
                }
                else
                {
                        cost = obj->level * 300;

                        if (ch->gold - cost < 0)
                        {
                                send_to_char
                                        ("You cant afford to send that to them.\n\r",
                                         ch);
                                return;
                        }
                        ch->gold -= cost;
                        if ((fp = fopen(strsave, "w")) != NULL)
                        {
                                EXTRA_DESCR_DATA *ed;
                                AFFECT_DATA *paf;
                                sh_int    wear, wear_loc, x;

                                /*
                                 * Yes its redundant... But it works 
                                 */
                                fprintf(fp, "#OBJECT\n");

                                if (obj->count > 1)
                                        fprintf(fp, "Count        %d\n",
                                                obj->count);
                                if (QUICKMATCH
                                    (obj->name, obj->pIndexData->name) == 0)
                                        fprintf(fp, "Name         %s~\n",
                                                obj->name);
                                if (QUICKMATCH
                                    (obj->short_descr,
                                     obj->pIndexData->short_descr) == 0)
                                        fprintf(fp, "ShortDescr   %s~\n",
                                                obj->short_descr);
                                if (QUICKMATCH
                                    (obj->description,
                                     obj->pIndexData->description) == 0)
                                        fprintf(fp, "Description  %s~\n",
                                                obj->description);
                                if (QUICKMATCH
                                    (obj->action_desc,
                                     obj->pIndexData->action_desc) == 0)
                                        fprintf(fp, "ActionDesc   %s~\n",
                                                obj->action_desc);
                                fprintf(fp, "Vnum         %d\n",
                                        obj->pIndexData->vnum);
                                if (obj->extra_flags !=
                                    obj->pIndexData->extra_flags)
                                        fprintf(fp, "ExtraFlags   %d\n",
                                                obj->extra_flags);
                                if (obj->wear_flags !=
                                    obj->pIndexData->wear_flags)
                                        fprintf(fp, "WearFlags    %d\n",
                                                obj->wear_flags);
                                wear_loc = -1;
                                for (wear = 0; wear < MAX_WEAR; wear++)
                                        for (x = 0; x < MAX_LAYERS; x++)
                                                if (obj ==
                                                    save_equipment[wear][x])
                                                {
                                                        wear_loc = wear;
                                                        break;
                                                }
                                                else if (!save_equipment[wear]
                                                         [x])
                                                        break;
                                if (wear_loc != -1)
                                        fprintf(fp, "WearLoc      %d\n",
                                                wear_loc);
                                if (obj->item_type !=
                                    obj->pIndexData->item_type)
                                        fprintf(fp, "ItemType     %d\n",
                                                obj->item_type);
                                if (obj->weight != obj->pIndexData->weight)
                                        fprintf(fp, "Weight       %d\n",
                                                obj->weight);
                                if (obj->level)
                                        fprintf(fp, "Level        %d\n",
                                                obj->level);
                                if (obj->timer)
                                        fprintf(fp, "Timer        %d\n",
                                                obj->timer);
                                if (obj->cost != obj->pIndexData->cost)
                                        fprintf(fp, "Cost         %d\n",
                                                obj->cost);
                                if (obj->value[0] || obj->value[1]
                                    || obj->value[2] || obj->value[3]
                                    || obj->value[4] || obj->value[5])
                                        fprintf(fp,
                                                "Values       %d %d %d %d %d %d\n",
                                                obj->value[0], obj->value[1],
                                                obj->value[2], obj->value[3],
                                                obj->value[4], obj->value[5]);

                                switch (obj->item_type)
                                {
                                case ITEM_PILL:    /* was down there with staff and wand, wrongly - Scryn */
                                case ITEM_POTION:
                                case ITEM_SCROLL:
                                        if (IS_VALID_SN(obj->value[1]))
                                                fprintf(fp,
                                                        "Spell 1      '%s'\n",
                                                        skill_table[obj->
                                                                    value
                                                                    [1]]->
                                                        name);

                                        if (IS_VALID_SN(obj->value[2]))
                                                fprintf(fp,
                                                        "Spell 2      '%s'\n",
                                                        skill_table[obj->
                                                                    value
                                                                    [2]]->
                                                        name);

                                        if (IS_VALID_SN(obj->value[3]))
                                                fprintf(fp,
                                                        "Spell 3      '%s'\n",
                                                        skill_table[obj->
                                                                    value
                                                                    [3]]->
                                                        name);

                                        break;

                                case ITEM_STAFF:
                                case ITEM_WAND:
                                        if (IS_VALID_SN(obj->value[3]))
                                                fprintf(fp,
                                                        "Spell 3      '%s'\n",
                                                        skill_table[obj->
                                                                    value
                                                                    [3]]->
                                                        name);

                                        break;
                                case ITEM_SALVE:
                                        if (IS_VALID_SN(obj->value[4]))
                                                fprintf(fp,
                                                        "Spell 4      '%s'\n",
                                                        skill_table[obj->
                                                                    value
                                                                    [4]]->
                                                        name);

                                        break;
                                }

                                for (paf = obj->first_affect; paf;
                                     paf = paf->next)
                                {
                                        /*
                                         * Save extra object affects                -Thoric
                                         */
                                        if (paf->type < 0
                                            || paf->type >= top_sn)
                                        {
                                                fprintf(fp,
                                                        "Affect       %d %d %d %d %d\n",
                                                        paf->type,
                                                        paf->duration,
                                                        ((paf->location ==
                                                          APPLY_WEAPONSPELL
                                                          || paf->location ==
                                                          APPLY_WEARSPELL
                                                          || paf->location ==
                                                          APPLY_REMOVESPELL
                                                          || paf->location ==
                                                          APPLY_STRIPSN)
                                                         && IS_VALID_SN(paf->
                                                                        modifier))
                                                        ? skill_table[paf->
                                                                      modifier]->
                                                        slot : paf->modifier,
                                                        paf->location,
                                                        paf->bitvector);
                                        }
                                        else
                                                fprintf(fp,
                                                        "AffectData   '%s' %d %d %d %d\n",
                                                        skill_table[paf->
                                                                    type]->
                                                        name, paf->duration,
                                                        ((paf->location ==
                                                          APPLY_WEAPONSPELL
                                                          || paf->location ==
                                                          APPLY_WEARSPELL
                                                          || paf->location ==
                                                          APPLY_REMOVESPELL
                                                          || paf->location ==
                                                          APPLY_STRIPSN)
                                                         && IS_VALID_SN(paf->
                                                                        modifier))
                                                        ? skill_table[paf->
                                                                      modifier]->
                                                        slot : paf->modifier,
                                                        paf->location,
                                                        paf->bitvector);
                                }

                                for (ed = obj->first_extradesc; ed;
                                     ed = ed->next)
                                        fprintf(fp, "ExtraDescr   %s~ %s~\n",
                                                ed->keyword, ed->description);

                                fprintf(fp, "End\n\n");

                                if (obj->first_content)
                                        fwrite_obj(ch, obj->last_content, fp,
                                                   0, OS_CARRY, FALSE);

                        }
                        fprintf(fp, "#END \n\r");
                        FCLOSE(fp);
                }
                snprintf(mobbuf, MSL,
                         "Ok, I've put it in their mailbox for them. Thanks and may I assist you further %s?.",
                         ch->name);
                do_say(mob, mobbuf);
                extract_obj(obj);
                return;

        }
        if (!str_cmp(arg3, "express"))
        {
                cost = obj->level * 400;

                if (victim == ch)
                {
                        snprintf(mobbuf, MSL,
                                 "I'm not gonna waste my time delivering to you FROM you %s!",
                                 ch->name);
                        do_say(mob, mobbuf);
                        return;
                }

                if (ch->gold - cost < 0)
                {
                        send_to_char
                                ("You cant afford to send that to them.\n\r",
                                 ch);
                        return;
                }
                ch->gold -= cost;
                separate_obj(obj);
                obj_from_char(obj);
                obj_to_char(obj, victim);
                act(AT_MAGIC,
                    "You give the postmaster $p to express deliver to $N", ch,
                    obj, victim, TO_CHAR);
                snprintf(mobbuf, MSL, "Thanks %s! It's on its way to %s.",
                         ch->name, victim->name);
                do_say(mob, mobbuf);
                act(AT_MAGIC,
                    "A courier materliazes and hands you $p from $n.", ch,
                    obj, victim, TO_VICT);
                save_char_obj(victim);
                save_char_obj(ch);
                return;
        }
        cost = obj->level * 200;

        if (ch->gold - cost < 0)
        {
                send_to_char("You cant afford to send that to them.\n\r", ch);
                return;
        }
        snprintf(strsave, MSL, "%s%s", MAIL_DIR, capitalize(victim->name));
        if ((fp = fopen(strsave, "r")) != NULL)
        {
                send_to_char("Their mailbox is already full.\n\r", ch);
                FCLOSE(fp);
                return;
        }

        ch->gold -= cost;
        act(AT_MAGIC, "You give the postmaster $p to deliver to $N", ch, obj,
            victim, TO_CHAR);
        fwrite_delivery(victim, obj);
        extract_obj(obj);
        snprintf(mobbuf, MSL,
                 "Thanks %s! I've put your package in %s's box and sent a memo to them to pickup.",
                 ch->name, victim->name);
        do_say(mob, mobbuf);
        act(AT_MAGIC,
            "$n has sent you $p in the mail, and it is awaiting your pickup at the nearest post office.",
            ch, obj, victim, TO_VICT);
        SET_BIT(victim->pcdata->flags, PCFLAG_GOTMAIL);
        return;
}
