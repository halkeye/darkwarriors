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
 *                $Id: makeobjs.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


/*
 * Make a fire.
 */
void make_fire(ROOM_INDEX_DATA * in_room, sh_int timer)
{
        OBJ_DATA *fire;

        fire = create_object(get_obj_index(OBJ_VNUM_FIRE), 0);
        fire->timer = number_fuzzy(timer);
        obj_to_room(fire, in_room);
        return;
}

/*
 * Make a trap.
 */
OBJ_DATA *make_trap(int v0, int v1, int v2, int v3)
{
        OBJ_DATA *trap;

        trap = create_object(get_obj_index(OBJ_VNUM_TRAP), 0);
        trap->timer = 0;
        trap->value[0] = v0;
        trap->value[1] = v1;
        trap->value[2] = v2;
        trap->value[3] = v3;
        return trap;
}


/*
 * Turn an object into scraps.		-Thoric
 */
void make_scraps(OBJ_DATA * obj)
{
        OBJ_DATA *scraps, *tmpobj;
        CHAR_DATA *ch = NULL;

        separate_obj(obj);
        scraps = create_object(get_obj_index(OBJ_VNUM_SCRAPS), 0);
        scraps->timer = number_range(5, 15);

        /*
         * don't make scraps of scraps of scraps of ... 
         */
        if (obj->pIndexData->vnum == OBJ_VNUM_SCRAPS)
        {
                STRFREE(scraps->short_descr);
                scraps->short_descr = STRALLOC("some debris");
                STRFREE(scraps->description);
                scraps->description =
                        STRALLOC("Bits of debris lie on the ground here.");
        }
        else
        {
                stralloc_printf(&scraps->short_descr, scraps->short_descr,
                                obj->short_descr);
                stralloc_printf(&scraps->description, scraps->description,
                                obj->short_descr);
        }

        if (obj->carried_by)
        {
                act(AT_OBJECT, "$p falls to the ground in scraps!",
                    obj->carried_by, obj, NULL, TO_CHAR);
                if (obj == get_eq_char(obj->carried_by, WEAR_WIELD)
                    && (tmpobj =
                        get_eq_char(obj->carried_by,
                                    WEAR_DUAL_WIELD)) != NULL)
                        tmpobj->wear_loc = WEAR_WIELD;

                obj_to_room(scraps, obj->carried_by->in_room);
        }
        else if (obj->in_room)
        {
                if ((ch = obj->in_room->first_person) != NULL)
                {
                        act(AT_OBJECT,
                            "$p is reduced to little more than scraps.", ch,
                            obj, NULL, TO_ROOM);
                        act(AT_OBJECT,
                            "$p is reduced to little more than scraps.", ch,
                            obj, NULL, TO_CHAR);
                }
                obj_to_room(scraps, obj->in_room);
        }
        if ((obj->item_type == ITEM_CONTAINER
             || obj->item_type == ITEM_HOLSTER
             || obj->item_type == ITEM_CORPSE_PC) && obj->first_content)
        {
                if (ch && ch->in_room)
                {
                        act(AT_OBJECT,
                            "The contents of $p fall to the ground.", ch, obj,
                            NULL, TO_ROOM);
                        act(AT_OBJECT,
                            "The contents of $p fall to the ground.", ch, obj,
                            NULL, TO_CHAR);
                }
                if (obj->carried_by)
                        empty_obj(obj, NULL, obj->carried_by->in_room);
                else if (obj->in_room)
                        empty_obj(obj, NULL, obj->in_room);
                else if (obj->in_obj)
                        empty_obj(obj, obj->in_obj, NULL);
        }
        extract_obj(obj);
}


/*
 * Make a corpse out of a character.
 */
void make_corpse(CHAR_DATA * ch, CHAR_DATA * killer)
{
        OBJ_DATA *corpse;
        OBJ_DATA *obj;
        OBJ_DATA *obj_next;
        char     *name;

        if (IS_NPC(ch))
        {
                name = ch->short_descr;
                if (IS_SET(ch->act, ACT_DROID))
                        corpse = create_object(get_obj_index
                                               (OBJ_VNUM_DROID_CORPSE), 0);
                else
                        corpse = create_object(get_obj_index
                                               (OBJ_VNUM_CORPSE_NPC), 0);
                corpse->timer = 6;
                if (ch->gold > 0)
                {
                        if (ch->in_room)
                                ch->in_room->area->gold_looted += ch->gold;
                        obj_to_obj(create_money(ch->gold), corpse);
                        ch->gold = 0;
                }

                /*
                 * Using corpse cost to cheat, since corpses not sellable 
                 */
                corpse->cost = (-(int) ch->pIndexData->vnum);
                corpse->value[2] = corpse->timer;
        }
        else
        {
                name = ch->name;
                corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
                corpse->timer = 40;
                corpse->value[2] = (int) (corpse->timer / 8);
                corpse->value[3] = 0;
                if (ch->gold > 0)
                {
                        if (ch->in_room)
                                ch->in_room->area->gold_looted += ch->gold;
                        obj_to_obj(create_money(ch->gold), corpse);
                        ch->gold = 0;
                }
        }
        if (killer != NULL)
        {
                STRFREE(corpse->armed_by);
                corpse->armed_by = STRALLOC(killer->name);
        }
        /*
         * Added corpse name - make locate easier , other skills 
         */
        stralloc_printf(&corpse->name, "corpse %s", name);
        stralloc_printf(&corpse->short_descr, corpse->short_descr, name);
        stralloc_printf(&corpse->description, corpse->description, name);

        for (obj = ch->first_carrying; obj; obj = obj_next)
        {
                obj_next = obj->next_content;
                obj_from_char(obj);
                if (IS_OBJ_STAT(obj, ITEM_INVENTORY)
                    || IS_OBJ_STAT(obj, ITEM_DEATHROT))
                        extract_obj(obj);
                else
                        obj_to_obj(obj, corpse);
        }
        obj_to_room(corpse, ch->in_room);
        return;
}



void make_blood(CHAR_DATA * ch)
{
        OBJ_DATA *obj;

        obj = create_object(get_obj_index(OBJ_VNUM_BLOOD), 0);
        obj->timer = number_range(2, 4);
        obj->value[1] = number_range(3, UMIN(5, ch->top_level));
        obj_to_room(obj, ch->in_room);
}


void make_bloodstain(CHAR_DATA * ch)
{
        OBJ_DATA *obj;

        obj = create_object(get_obj_index(OBJ_VNUM_BLOODSTAIN), 0);
        obj->timer = number_range(1, 2);
        obj_to_room(obj, ch->in_room);
}


/*
 * make some coinage
 */
OBJ_DATA *create_money(int amount)
{
        OBJ_DATA *obj;

        if (amount <= 0)
        {
                bug("Create_money: zero or negative money %d.", amount);
                amount = 1;
        }

        if (amount == 1)
        {
                obj = create_object(get_obj_index(OBJ_VNUM_MONEY_ONE), 0);
        }
        else
        {
                obj = create_object(get_obj_index(OBJ_VNUM_MONEY_SOME), 0);
                stralloc_printf(&obj->short_descr, obj->short_descr, amount);
                obj->value[0] = amount;
        }

        return obj;
}
