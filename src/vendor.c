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
 *                $Id: vendor.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
/*

                          __   _    ______    ______    ______    _    _
                 ____    |  \ | |  |  __  |  | ____ |  |__   _|  | |  | |
                /  _/    | \ \| |  | |  | |  |  _  _|     | |    | |__| |
               / //      | |\   |  | |__| |  | | \ \      | |    |  __  |
 _            / //       |_| \__|  |______|  |_|  \_\     |_|    |_|  |_|
\ \__________/ /------------------------------------------------------------.
>O)XXXXXXXX<-<----=====================================================----->
/_/~~~~~~~~~~\ \------------------------------------------------------------'
              \ \\            _    _    ______    __   _    _____
               \ \\_         | |  | |  |__   _|  |  \ | |  |  __ \
                \___\        | |  | |     | |    | \ \| |  | |  \ |
                             | |/\| |   __| |_   | |\   |  | |__/ |
                             |__/\__|  |______|  |_| \__|  |_____/
*/


#include <sys/types.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "mud.h"

#define VENDOR_FEE  .05 /*fee vendor charges, taken out of all gode with teh GETGOLD command */

CHAR_DATA *find_keepernotext args((CHAR_DATA * ch));
bool      char_exists(char *player);

/*
This is the command used to buy a contract from a vendor to place a player
owned vendor
*/
CMDF do_buyvendor(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *keeper;
        OBJ_DATA *deed;
        char      buf1[MAX_STRING_LENGTH];

        argument = NULL;



        if (IS_NPC(ch))
                return;

        if ((keeper = find_keeper(ch)) == NULL)
        {
                send_to_char("There is no one to buy that from!\n\r", ch);
                return;
        }

        if ((ch->gold < COST_BUY_VENDOR))
        {
                snprintf(buf1, MSL, "%s says, You are to poor!\n\r",
                         keeper->name);
                send_to_char(buf1, ch);
                return;
        }

        if ((ch->top_level) < LEVEL_BUY_VENDOR)
        {
                snprintf(buf1, MSL, "you must be at least %d level.\n\r",
                         LEVEL_BUY_VENDOR);
                send_to_char(buf1, ch);
                return;
        }

        if ((get_obj_index(OBJ_VNUM_DEED)) == NULL)
        {
                bug("BUYVENDOR: Deed is missing!");
                return;
        }
        deed = create_object(get_obj_index(OBJ_VNUM_DEED), 0);
        obj_to_char(deed, ch);
        send_to_char_color
                ("&bVery well, you may have a contract for a vendor.\n\r",
                 ch);
        ch->gold = ch->gold - COST_BUY_VENDOR;

}


/*this is the command that places the vendor at the specified location*/
CMDF do_placevendor(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *vendor;
        MOB_INDEX_DATA *temp;
        OBJ_DATA *obj;
        int       vnum, check;
        char      buf[MAX_INPUT_LENGTH];

        argument = NULL;

        vnum = ch->in_room->vnum;
        check = 0;

        if (find_keepernotext(ch))
        {
                send_to_char("A vendor is already here!\n\r", ch);
                return;
        }

        if (IS_NPC(ch))
                return;



/* better way to do this? what if they have another object called deed?*/
/* added this to account for another with deed in name*/

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->pIndexData->vnum == 5)
                        check++;
        }

        if (check == 0)
        {
                send_to_char("You do not have a deed!\n\r", ch);
                return;
        }


        /*
         * checks to see if there in arena.. might want to pull this out if you dont have
         * arena
         */
        if (xIS_SET(ch->in_room->room_flags, ROOM_ARENA))
        {
                send_to_char("Not here\n\r", ch);
                log_string("do_placevendor: in the arena");
                return;
        }

        if (!xIS_SET(ch->in_room->room_flags, ROOM_PLAYERSHOP))
        {
                send_to_char("This is not an empty player shop.\n\r", ch);
                return;
        }

/*makes sure they cant put it in certain rooms for sure! dont want vendors floatin
around in hell do we?*/
        if (vnum == 300 || vnum <= 115)
        {
                send_to_char("Not here!\n\r", ch);
                log_string("do_placevendor: place vendor imm only");
                return;
        }


        if ((temp = get_mob_index(MOB_VNUM_VENDOR)) == NULL)
        {
                log_string("do_placevendor: no vendor vnum");
                return;
        }



        char_to_room(create_mobile(temp), ch->in_room);
        vendor = get_char_room(ch, temp->player_name);


        snprintf(buf, MSL, vendor->long_descr, ch->name);
        if (vendor->long_descr)
                STRFREE(vendor->long_descr);
        vendor->long_descr = STRALLOC(buf);


        snprintf(buf, MSL, "%s", ch->name);

        if (vendor->owner)
                STRFREE(vendor->owner);
        vendor->owner = STRALLOC(buf);
        vendor->home = ch->in_room;


        save_vendor(vendor);

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->pIndexData->vnum == 5)
                        continue;
        }
        obj = get_obj_carry(ch, "deed");
        separate_obj(obj);
        extract_obj(obj);


        act(AT_ACTION, "$n appears in a swirl of smoke.\n", vendor, NULL,
            NULL, TO_ROOM);
}



CMDF do_pricevendor(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *vendor;
        CHAR_DATA *ch1;
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        OBJ_DATA *obj;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char("usage:> pricevendor <item> <cost>\n\r", ch);
                return;
        }

        if (arg2[0] == '\0')
        {
                send_to_char("usage:> pricevendor <item> <cost>\n\r", ch);
                return;
        }


        if ((vendor = find_keeper(ch)) == NULL)
        {
                send_to_char("What Vendor?\n\r", ch);
                return;
        }

        if ((ch1 = get_char_room(ch, vendor->owner)) == NULL)
        {
                send_to_char("This isnt your vendor!\n\r", ch);
                return;
        }

        if (ch->fighting)
        {
                send_to_char("Not while you fightZ!\n\r", ch);
                return;
        }

        if ((obj = get_obj_carry(vendor, arg1)) != NULL)
        {
                obj->cost = atoi(arg2);
                send_to_char("The price has been changed\n\r", ch);
                save_vendor(vendor);
                return;
        }


        send_to_char("He doesnt have that item!\n\r", ch);
        save_vendor(vendor);
        return;

}


CMDF do_collectcredits(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *vendor;
        CHAR_DATA *ch1;
        long      gold;
        char      buf[MAX_INPUT_LENGTH];
        char      name[MAX_INPUT_LENGTH];

        argument = NULL;

        if ((vendor = find_keeper(ch)) == NULL)
        {
                send_to_char("What Vendor?\n\r", ch);
                return;
        }

        if (vendor->owner == NULL)
        {
                send_to_char("thats not a vendor!\n\r", ch);
                return;
        }

        snprintf(name, MSL, "%s", vendor->owner);

        if ((ch1 = get_char_room(ch, vendor->owner)) == NULL)
        {
                send_to_char("Trying to steal huh?\n\r", ch);
                return;
        }

        if (!(ch == ch1))
        {
                snprintf(buf, MSL, "collectgold: %s and ch1 = %s\n\r", name,
                         ch1->name);
                log_string(buf);

                send_to_char("This isnt your vendor!\n\r", ch);
                return;
        }

        if (ch->fighting)
        {
                send_to_char("Not while you fightZ!\n\r", ch);
                return;
        }

        gold = vendor->gold;
        gold -= (long int) (gold * VENDOR_FEE);
        vendor->gold = 0;
        ch->gold += gold;

        send_to_char_color
                ("&GYour vendor gladly hands over his earnings minus a small fee of course..\n\r",
                 ch);
        act(AT_ACTION, "$n hands over some money.\n\r", vendor, NULL, NULL,
            TO_ROOM);
}


/* Write vendor to file */
void fwrite_vendor(FILE * fp, CHAR_DATA * mob)
{
        if (!IS_NPC(mob) || !fp)
                return;
        fprintf(fp, "Vnum     %d\n", mob->pIndexData->vnum);
        if (mob->gold > 0)
                fprintf(fp, "Gold     %ld\n", mob->gold);
        if (mob->home)
                fprintf(fp, "Home     %d\n", mob->home->vnum);
        if (mob->owner != NULL)
                fprintf(fp, "Owner     %s~\n", mob->owner);
        if (QUICKMATCH(mob->short_descr, mob->pIndexData->short_descr) == 0)
                fprintf(fp, "Short	    %s~\n", mob->short_descr);
        fprintf(fp, "Position   %d\n", mob->position);
        fprintf(fp, "Flags   %d\n", mob->act);
        fprintf(fp, "END\n");

        return;
}


/* read vendor from file */
CHAR_DATA *fread_vendor(FILE * fp)
{
        CHAR_DATA *mob = NULL;

        const char *word;
        bool      fMatch;
        int       inroom = 0;
        ROOM_INDEX_DATA *pRoomIndex = NULL;
        CHAR_DATA *victim;
        CHAR_DATA *vnext;
        char      buf[MAX_INPUT_LENGTH];
        char      vnum1[MAX_INPUT_LENGTH];

        word = feof(fp) ? "END" : fread_word(fp);
        if (!strcmp(word, "Vnum"))
        {
                int       vnum;

                vnum = fread_number(fp);
                mob = create_mobile(get_mob_index(vnum));
                if (!mob)
                {
                        for (;;)
                        {
                                word = feof(fp) ? "END" : fread_word(fp);
                                if (!strcmp(word, "END"))
                                        break;
                        }
                        bug("Fread_mobile: No index data for vnum %d", vnum);
                        return NULL;
                }
        }
        else
        {
                for (;;)
                {
                        word = feof(fp) ? "END" : fread_word(fp);
                        if (!strcmp(word, "END"))
                                break;
                }
                extract_char(mob, TRUE);
                bug("Fread_vendor: Vnum not found", 0);
                return NULL;
        }
        for (;;)
        {
                word = feof(fp) ? "END" : fread_word(fp);
                fMatch = FALSE;
                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;
                case '#':
                        if (!strcmp(word, "#OBJECT"))
                        {
                                fread_obj(mob, fp, OS_CARRY);
                        }
                        break;
                case 'D':
                        KEY("Description", mob->description,
                            fread_string(fp));
                        break;
                case 'E':

                        if (!strcmp(word, "END"))
                        {
                                if (!mob->owner || mob->owner[0] == '\0')
                                {
                                        if (!mob->in_room)
                                                mob->in_room =
                                                        get_room_index
                                                        (ROOM_VNUM_LIMBO);
                                        extract_char(mob, TRUE);
                                        return NULL;
                                }
                                if (inroom == 0)
                                        inroom = ROOM_VNUM_VENSTOR;
                                pRoomIndex = get_room_index(inroom);
                                if (!pRoomIndex)
                                        pRoomIndex =
                                                get_room_index
                                                (ROOM_VNUM_VENSTOR);
                                mob->home = pRoomIndex;
                                mob->in_room = pRoomIndex;
                                /*
                                 * the following code is to make sure no more then one player owned vendor
                                 * * is in the room - meckteck 
                                 */
                                for (victim = mob->in_room->first_person;
                                     victim; victim = vnext)
                                {
                                        vnext = victim->next_in_room;
                                        if (victim->home != NULL)
                                        {
                                                extract_char(victim, TRUE);
                                                break;
                                        }

                                }

                                char_to_room(mob, pRoomIndex);
                                snprintf(vnum1, MSL, "%d",
                                         mob->pIndexData->vnum);
                                do_makeshop(mob, vnum1);
                                snprintf(buf, MSL, mob->long_descr,
                                         mob->owner);
                                if (mob->long_descr)
                                        STRFREE(mob->long_descr);
                                mob->long_descr = STRALLOC(buf);
                                mob->hit = 10000;
                                mob->max_hit = 10000;
                                return mob;
                        }
                        break;
                case 'F':
                        KEY("Flags", mob->act, fread_number(fp));
                        break;
                case 'G':
                        KEY("Gold", mob->gold, fread_number(fp));
                        break;
                case 'H':
                        KEY("Home", inroom, fread_number(fp));
                        break;
                case 'L':
                        break;
                case 'N':
                        break;
                case 'O':
                        if (!str_cmp("Owner", word))
                        {
                                char     *temp = fread_string(fp);

                                fMatch = TRUE;
                                if (char_exists(temp))
                                        mob->owner = temp;
                                else
                                {
                                        bug("Fread_vendor: Owner Not found %s", temp);
                                        if (!mob->in_room)
                                                mob->in_room =
                                                        get_room_index
                                                        (ROOM_VNUM_LIMBO);
                                        extract_char(mob, TRUE);
                                        STRFREE(temp);
                                        return NULL;
                                }
                                break;
                        }
                        break;
                case 'P':
                        KEY("Position", mob->position, fread_number(fp));
                        break;
                case 'S':
                        KEY("Short", mob->short_descr, fread_string(fp));
                        break;
                }
                if (!fMatch)
                {
                        bug("Fread_mobile: no match.", 0);
                        bug(word, 0);
                }
        }
        return NULL;
}




void save_vendor(CHAR_DATA * ch)
{
        char      strsave[MAX_INPUT_LENGTH];
        FILE     *fp;

        if (!ch)
        {
                bug("Save_char_obj: null ch!", 0);
                return;
        }

        de_equip_char(ch);


        snprintf(strsave, MSL, "%s%s", VENDOR_DIR, capitalize(ch->owner));




        if ((fp = fopen(TEMP_FILE, "w")) == NULL)
        {
                perror(strsave);
                bug("Save_vendor: fopen", 0);
        }
        else
        {
                bool      ferr;

                fchmod(fileno(fp),
                       S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH |
                       S_IWOTH);
                fprintf(fp, "#VENDOR\n");
                fwrite_vendor(fp, ch);

                if (ch->first_carrying)
                        fwrite_obj(ch, ch->last_carrying, fp, 0, OS_CARRY,
                                   FALSE);

                fprintf(fp, "#END\n");
                ferr = ferror(fp);
                FCLOSE(fp);

                if (ferr)
                {
                        perror(strsave);
                        bug("Error writing temp file for %s -- not copying",
                            strsave);
                }

                else
                        rename(TEMP_FILE, strsave);
        }

        re_equip_char(ch);
        return;
}

CMDF do_nameshop(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *vendor;
        CHAR_DATA *wch;

        if (IS_NPC(ch))
                return;

        if (!xIS_SET(ch->in_room->room_flags, ROOM_PLAYERSHOP))
        {
                send_to_char("This is not a player shop.\n\r", ch);
                return;
        }

        if ((vendor = find_keeper(ch)) == NULL)
        {
                send_to_char("There is no vendor currently in this shop\n\r",
                             ch);
                return;
        }

        if (vendor->owner == NULL)
        {
                send_to_char("There is no vendor currently in this shop\n\r",
                             ch);
                return;
        }

        if ((wch = get_char_room(ch, vendor->owner)) == NULL)
        {
                send_to_char("You are not the owner of this shop.\n\r", ch);
                return;
        }

        if (ch != wch)
        {
                send_to_char("You are not the owner of this shop.\n\r", ch);
                return;
        }

        if (!argument || argument[0] == '\0')
        {
                send_to_char("Syntax: nameshop <name>\n\r", ch);
                return;
        }

        if (!ch->in_room)
        {
                send_to_char
                        ("You appear not to be in a room. Please contact an immortal.\n\r",
                         ch);
                return;
        }

        if (ch->in_room->name)
                STRFREE(ch->in_room->name);
        ch->in_room->name = STRALLOC(argument);

        if (ch->in_room->area && ch->in_room->area->filename
            && ch->in_room->area->filename[0] == '\0')
                fold_area(ch->in_room->area, ch->in_room->area->filename,
                          FALSE, FALSE);

        send_to_char("Shop name changed.\n\r", ch);
}
