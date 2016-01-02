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
 *                                 SWR HTML Header file                                  *
 ****************************************************************************************/
/**********************************************************************************
*Designship.c - Ship design skill for Engineers                                   *
*Coded By     - Ortluk  ortluk@hotmail.com                                        *
*Written for  - SWR 1.0  and derivitives                                          *
**********************************************************************************/
/* NOTE - If using SWFoTE you will have to change the ship classes and a couple of the ship 
   variables. I think I commented most of them.  Also if not using SWFoTE you can remove 
   the parts commented about SWFoTE */

#include <math.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#ifdef OLC_HOMES
#include "homes.h"
#endif
#include "space2.h"
#include "installations.h"


void write_ship_list args((void));
void fleet_make args((CHAR_DATA * ch, char *argument));
void smush_tilde args((char *str));
INSTALLATION_DATA *installation_from_room(int vnum);

int reserve_rooms_ship(int firstroom, int numrooms)
{
        AREA_DATA *tarea;
        ROOM_INDEX_DATA *room;
        int       i;

        for (tarea = first_area; tarea; tarea = tarea->next)
                if (!str_cmp(PSHIP_AREA, tarea->filename))
                        break;

        for (i = firstroom; i < firstroom + numrooms; i++)
        {
                room = make_room(i, tarea);
                if (!room)
                {
                        bug("reserve_rooms: make_room failed");
                        return -1;
                }
                xSET_BIT(room->room_flags, ROOM_SPACECRAFT);
        }
        fold_area(tarea, tarea->filename, TRUE, FALSE);
        return i;
}

int find_pvnum_block(int num_needed, char *areaname)
{
        bool      counting = FALSE;
        int       count = 0;
        AREA_DATA *tarea;
        int       lrange;
        int       trange;
        int       vnum;
        int       startvnum = -1;
        ROOM_INDEX_DATA *room;

        for (tarea = first_area; tarea; tarea = tarea->next)
                if (!str_cmp(areaname, tarea->filename))
                        break;
        lrange = tarea->low_r_vnum;
        trange = tarea->hi_r_vnum;
        for (vnum = lrange; vnum <= trange; vnum++)
        {
                if ((room = get_room_index(vnum)) == NULL)
                {
                        if (!counting)
                        {
                                counting = TRUE;
                                startvnum = vnum;
                        }
                        count++;
                        if (count == num_needed + 1)
                                break;
                }
                else if (counting)
                {
                        counting = FALSE;
                        count = 0;
                        startvnum = -1;
                }
        }
        return startvnum;
}

/* this function borrowed from SWRip codebase thanx guys - comment it out if you already 
have one*/


void transship(SHIP_DATA * ship, int destination)
{
        int       origShipyard;


        if (!ship)
                return;

        origShipyard = ship->shipyard;

        ship->shipyard = destination;
        ship->shipstate = SHIP_DOCKED;

        extract_ship(ship);
        ship_to_room(ship, ship->shipyard);

        ship->location = ship->shipyard;
        ship->lastdoc = ship->shipyard;

        ship->shipyard = origShipyard;

        if (ship->starsystem)
                ship_from_starsystem(ship, ship->starsystem);

        save_ship(ship);
}


CMDF do_designship(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      filename[MAX_STRING_LENGTH];
        int       percentage, numrooms, ship_class;
        bool      checktool, checkdura, checkcir, checksuper;
        ROOM_INDEX_DATA *room;
        OBJ_DATA *obj;
        SHIP_DATA *ship;
        PLANET_DATA *planet;
        int       vnum, durasteel, transparisteel, cost, fee;

        argument = one_argument(argument, arg1);
        mudstrlcpy(arg2, argument, MIL);


        switch (ch->substate)
        {
        default:

                if (!is_number(arg1) || arg2[0] == '\0')
                {
                        send_to_char
                                ("&RUsage: &Gdesignship &C<&cnumber of rooms&C> <&cname of ship&C>&w\r\n",
                                 ch);
                        return;
                }
                numrooms = atoi(arg1);
                if (numrooms > 100 || numrooms < 1)
                {
                        send_to_char
                                ("&RNumber of rooms MUST be between 1 and 100&C&w",
                                 ch);
                        return;
                }
                for (ship = first_ship; ship; ship = ship->next)
                {
                        if (!str_cmp(ship->name, argument))
                        {
                                send_to_char
                                        ("&CThat ship name is already in use. Choose another.\r\n",
                                         ch);
                                return;
                        }
                }

                checktool = FALSE;
                checkdura = FALSE;
                checkcir = FALSE;
                checksuper = FALSE;

                /*
                 * used import flag to avoid adding an extra shipyard flg. 
                 * it can be changed if you don't have my cargo snippet - Ortluk 
                 * Also if you're installing in swfote you can uncomment the first if statement 
                 * and comment the second one out to use the shipyard flag instead
                 */

                if (!xIS_SET(ch->in_room->room_flags, ROOM_IMPORT))
                {
                        send_to_char
                                ("You can't build that here!! Try a spaceport\r\n",
                                 ch);
                        return;
                }

/* uncomment these lines if swfote 
                if (numrooms > 100)
                   ship_class = SHIP_DESTROYER;
                else if(numrooms > 75)
                   ship_class = SHIP_DREADNAUGHT;
                else if(numrooms > 50)
                   ship_class = SHIP_CRUISER;
                else if(numrooms > 25)
                   ship_class = SHIP_CORVETTE;
                else if(numrooms > 15)
                   ship_class = SHIP_FRIGATE;
                else if(numrooms > 5)
                   ship_class = SHIP_FREIGHTER;
                else if (numrooms > 1) 
                   ship_class = SHIP_SHUTTLE;
                else 
                   ship_class = SHIP_FIGHTER; */

/* comment these for swfote */
                if (numrooms > 25)
                        ship_class = CAPITAL_SHIP;
                else if (numrooms > 5)
                        ship_class = MIDSIZE_SHIP;
                else
                        ship_class = FIGHTER_SHIP;

                /*
                 * these values come from  cargo v2 
                 */
                durasteel = ship_class * 150 + 100;
                transparisteel = ship_class * 20 + 10;

                if (xIS_SET(ch->in_room->room_flags, ROOM_INSTALLATION))
                {
                        INSTALLATION_DATA *installation =
                                installation_from_room(ch->in_room->vnum);
                        if (installation
                            && installation->type == SHIPYARD_INSTALLATION)
                                planet = installation->planet;
                }
                else
                        planet = ch->in_room->area->planet;

                if (!planet)
                {
                        send_to_char
                                ("&RAnd where do you think you're going to get the resources to build your ship?&C&w",
                                 ch);
                        return;
                }

                /*
                 * make sure the planet has the resources to build the ship 
                 */
                if (planet->resource[CARGO_DURASTEEL] < durasteel)
                {
                        send_to_char
                                ("&RYou'll Have to wait till they either import or produce more durasteel&C&w\r\n",
                                 ch);
                        return;
                }

                if (planet->resource[CARGO_TRANSPARISTEEL] < transparisteel)
                {
                        send_to_char
                                ("&RYou'll Have to wait till they either import or produce more transparisteel&C&w\r\n",
                                 ch);
                        return;
                }
                cost = 10;
                if (planet->cargoimport[CARGO_TRANSPARISTEEL] > 0)
                        cost += planet->cargoimport[CARGO_TRANSPARISTEEL] +
                                planet->cargoimport[CARGO_TRANSPARISTEEL] / 2;
                else if (planet->cargoexport[CARGO_TRANSPARISTEEL] > 0)
                        cost += planet->cargoexport[CARGO_TRANSPARISTEEL];
                else
                        cost += 10;

                if (planet->cargoimport[CARGO_DURASTEEL] > 0)
                        cost += planet->cargoimport[CARGO_DURASTEEL] +
                                planet->cargoimport[CARGO_DURASTEEL] / 2;
                else if (planet->cargoexport[CARGO_DURASTEEL] > 0)
                        cost += planet->cargoexport[CARGO_DURASTEEL];
                else
                        cost += 10;

                cost *= durasteel + transparisteel;
                fee = cost * ((ship_class * 5) / 100);
                cost += fee;
                if (ch->gold < cost)
                {
                        send_to_char
                                ("&RYou can't afford the materials to build that.\r\n",
                                 ch);
                        return;
                }

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_CIRCUIT)
                                checkcir = TRUE;
                        if (obj->item_type == ITEM_SUPERCONDUCTOR)
                                checksuper = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RI'd like to see you build a ship with no tools.\r\n",
                                 ch);
                        return;
                }
                if (!checkcir)
                {
                        send_to_char
                                ("&RYou could really use a circuit to for the control systems.\r\n",
                                 ch);
                        return;
                }
                if (!checksuper)
                {
                        send_to_char
                                ("&RSuch advanced circuitry requires a superconducter to work properly.\r\n",
                                 ch);
                        return;
                }

                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_shipdesign]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin the LONG process of building a ship.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and starts constructing a ship.\r\n",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 35, do_designship, 1);
                        ch->dest_buf = str_dup(arg1);
                        ch->dest_buf_2 = str_dup(arg2);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_shipdesign);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                {
                        bug("null ch->dest_buf2", 0);
                        return;
                }

                mudstrlcpy(arg1, (char *) ch->dest_buf, MSL);
                DISPOSE(ch->dest_buf);
                mudstrlcpy(arg2, (char *) ch->dest_buf_2, MSL);
                DISPOSE(ch->dest_buf_2);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);

                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }
        ch->substate = SUB_NONE;
        numrooms = atoi(arg1);

        if (numrooms > 25)
                ship_class = CAPITAL_SHIP;
        else if (numrooms > 5)
                ship_class = MIDSIZE_SHIP;
        else
                ship_class = FIGHTER_SHIP;

        /*
         * these values come from  cargo v2 
         */
        transparisteel = ship_class * 50 + 10;
        durasteel = ship_class * 100 + 100;
        planet = ch->in_room->area->planet;
        if (!planet)
        {
                send_to_char("&RYou must have been moved...I'd complain.&C&w",
                             ch);
                return;
        }

        cost = 10;
        if (planet->cargoimport[CARGO_TRANSPARISTEEL] > 0)
                cost += planet->cargoimport[CARGO_TRANSPARISTEEL] +
                        planet->cargoimport[CARGO_TRANSPARISTEEL] / 2;
        else if (planet->cargoexport[CARGO_TRANSPARISTEEL] > 0)
                cost += planet->cargoexport[CARGO_TRANSPARISTEEL];
        else
                cost += 10;


        if (planet->cargoimport[CARGO_DURASTEEL] > 0)
                cost += planet->cargoimport[CARGO_DURASTEEL] +
                        planet->cargoimport[CARGO_DURASTEEL] / 2;
        else if (planet->cargoexport[CARGO_DURASTEEL] > 0)
                cost += planet->cargoexport[CARGO_DURASTEEL];
        else
                cost += 10;

        cost *= durasteel + transparisteel;
        fee = cost * ((ship_class * 5) / 100);
        cost += fee;
        if (ch->gold < cost)
        {
                send_to_char
                        ("&RYou can't afford the materials....Stop that THIEF!!!\r\n",
                         ch);
                return;
        }
        ch->gold -= cost;
        planet->resource[CARGO_TRANSPARISTEEL] -= transparisteel;
        planet->resource[CARGO_DURASTEEL] -= durasteel;

        checktool = FALSE;
        checkdura = FALSE;
        checkcir = FALSE;
        checksuper = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_DURASTEEL && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                }
                if (obj->item_type == ITEM_CIRCUIT && checkcir == FALSE)
                {
                        checkcir = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                }
                if (obj->item_type == ITEM_SUPERCONDUCTOR
                    && checksuper == FALSE)
                {
                        checksuper = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                }

        }

/* ok so far so good...everything is cool...try to build the ship */

        vnum = find_pvnum_block(numrooms, PSHIP_AREA);
        if (vnum < 0)
        {
                bug("player ship area out of vnums", 0);
                send_to_char("Not enough vnums report to a coder.\r\n", ch);
                return;
        }
        if (reserve_rooms_ship(vnum, numrooms) < 0)
        {
                bug("do_designship: reserve_rooms failed", 0);
                send_to_char("Couldn't build your rooms report to coder.\r\n",
                             ch);
                return;
        }
        snprintf(filename, MSL, "%d.pship", vnum);

        CREATE(ship, SHIP_DATA, 1);
        LINK(ship, first_ship, last_ship, next, prev);
        ship->filename = STRALLOC(filename);
        ship->name = STRALLOC(arg2);
        ship->owner = STRALLOC(ch->name);
        ship->copilot = STRALLOC("");
        ship->pilot = STRALLOC("");
        ship->home = STRALLOC("");
        ship->type = PLAYER_SHIP;
        ship->prototype = get_protoship("generic");

/* you may want to adjust these to balance ships with your imm built ones 
   I use an array of maximum ship stats for the different classes of ships
   that's another project though */

        ship->maxenergy =
                (ship_class + 1) * (50 +
                                    (get_curr_int(ch) / 5)) *
                ch->pcdata->learned[gsn_shipdesign];
        ship->energy = ship->maxenergy;
        ship->maxhull =
                (ship_class + 1 +
                 (get_curr_int(ch) / 5)) * 10 *
                ch->pcdata->learned[gsn_shipdesign];
        ship->maxshield =
                (ship_class + 1) * 5 * ch->pcdata->learned[gsn_shipdesign];
        ship->realspeed =
                2 * ch->pcdata->learned[gsn_shipdesign] / (ship_class + 1);
        ship->hyperspeed =
                ch->pcdata->learned[gsn_shipdesign] + (ship_class + 1) * 60;
        ship->lasers =
                (ship_class +
                 1) * (ch->pcdata->learned[gsn_shipdesign] / 10) +
                (get_curr_int(ch) / 5);
        ship->manuever =
                ch->pcdata->learned[gsn_shipdesign] * (get_curr_int(ch) / 5);
        ship->comm =
                ch->pcdata->learned[gsn_shipdesign] * (get_curr_int(ch) / 5) * (ship_class + 1 );
        ship->sensor =
                ch->pcdata->learned[gsn_shipdesign] * (get_curr_int(ch) / 5) * (ship_class + 1 );
        if (ship_class == FIGHTER_SHIP)
                ship->maxcargo = ch->pcdata->learned[gsn_shipdesign] / 10;
        else
                ship->maxcargo =
                        ch->pcdata->learned[gsn_shipdesign] * (ship_class +
                                                               (get_curr_int
                                                                (ch) / 5));
        if (ship_class == FIGHTER_SHIP)
                ship->maxbattalions = 0;
        else if (ship_class == MIDSIZE_SHIP)
                ship->maxbattalions =
                        ch->pcdata->learned[gsn_shipdesign] / 10;
        else if (ship_class == CAPITAL_SHIP)
                ship->maxbattalions = ch->pcdata->learned[gsn_shipdesign];

        ship->hull = ship->maxhull;
        ship->in_room = NULL;
        ship->currjump = NULL;
        ship->target0 = NULL;
        ship->target1 = NULL;
        ship->target2 = NULL;
        ship->ship_class = ship_class;
        ship->selfdestruct = STRALLOC("Installed");
        ship->selfdpass = number_range(10001, 99999);
        ship->firstroom = vnum;
        ship->lastroom = vnum + numrooms - 1;
        ship->entrance = vnum;
        ship->lastbuilt = vnum;
        if (numrooms == 1)
        {
                ship->cockpit = vnum;
                ship->navseat = vnum;
                ship->gunseat = vnum;
                ship->pilotseat = vnum;
                ship->engineroom = vnum;
                ship->coseat = vnum;
        }

        gain_exp(ch, 10000, ENGINEERING_ABILITY);

/* I added this fee to balance the cost of building ships with that of selling them so as to 
   keep engis from making too much money too fast from just building and selling ships
*/
        ch_printf(ch,
                  "The planet's Government has assesed a Licence Fee of %d credits.\r\n",
                  fee);
        ch_printf(ch, "&WYou gain 10000 engineering experience.\r\n");
        learn_from_success(ch, gsn_shipdesign);
        transship(ship, ch->in_room->vnum);
        act(AT_PLAIN, "$n finishes building new ship, and climbs inside.", ch,
            NULL, argument, TO_ROOM);
        room = get_room_index(vnum);
        if (!room)
        {
                bug("designship..no such room", 0);
                return;
        }
        if (room->name)
                STRFREE(room->name);

        if (numrooms > 1)
                room->name = STRALLOC("Entrance Ramp");
        else
                room->name = STRALLOC("Cockpit");
        fold_area(room->area, room->area->filename, TRUE, FALSE);
        save_ship(ship);
        write_ship_list();
        char_from_room(ch);
        char_to_room(ch, get_room_index(vnum));

}

CMDF do_addroom(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        ROOM_INDEX_DATA *room;
        SHIP_DATA *ship;
        bool      match, tset = FALSE;
        EXIT_DATA *pexit;


		if (IS_NPC(ch)) 
			return;

#ifdef OLC_HOMES
		if (ch->in_room && ch->in_room->home) {
			ch->in_room->home->add_room(ch, argument);
			return;
		}
#endif


        match = FALSE;
        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0' || arg[0] == '\0')
        {
                send_to_char
                        ("Usage: addroom <ship/installation> <direction> <type>\r\n",
                         ch);
                send_to_char
                        ("   HELP addroom for a list of room types and costs\r\n",
                         ch);
                return;
        }

/* To make them enter a decent room description. 80 characters a line * 5 lines == 400 */
        if (strlen(ch->in_room->description) < 6)
        {
                send_to_char
                        ("You must use decorateroom to make a description approximately 6 lines long.\r\n",
                         ch);
                return;
        }

        if (!str_cmp(arg, "installation"))
        {
                snprintf(buf, MSL, "%s %s", arg1, arg2);
                addroominstallation(ch, buf);
                return;
        }

        ship = ship_from_room(ch->in_room->vnum);

        if (!ship)
        {
                send_to_char("You can only add rooms to your ships.\r\n", ch);
                return;
        }
        if (!check_pilot(ch, ship) || !str_cmp(ship->owner, "Public"))
        {
                send_to_char
                        ("You don't have permission to build on this ship\r\n",
                         ch);
                return;
        }
        if (!argument)
        {
                send_to_char("You really should name your room\r\n", ch);
                return;
        }
        if (ship->lastbuilt == ship->lastroom
            || ship->lastbuilt < ship->firstroom)
        {
                send_to_char
                        ("You have no more rooms to build on this ship\r\n",
                         ch);
                if (ship->lastbuilt < ship->firstroom)
                {
                        ship->lastbuilt = ship->lastroom;
                        save_ship(ship);
                }
                return;
        }
        room = get_room_index(ship->lastbuilt + 1);

        if (!room)
        {
                bug("addroom: no such room", 0);
                return;
        }

        if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north") ||
            !str_cmp(arg1, "s") || !str_cmp(arg1, "south") ||
            !str_cmp(arg1, "e") || !str_cmp(arg1, "east") ||
            !str_cmp(arg1, "w") || !str_cmp(arg1, "west") ||
            !str_cmp(arg1, "sw") || !str_cmp(arg1, "southwest") ||
            !str_cmp(arg1, "nw") || !str_cmp(arg1, "northwest") ||
            !str_cmp(arg1, "se") || !str_cmp(arg1, "southeast") ||
            !str_cmp(arg1, "ne") || !str_cmp(arg1, "northeast") ||
            !str_cmp(arg1, "u") || !str_cmp(arg1, "up") ||
            !str_cmp(arg1, "d") || !str_cmp(arg1, "down"))
        {
                if (get_exit(ch->in_room, get_dir(arg1)))
                {
                        send_to_char
                                ("There is already an exit in that direction\r\n",
                                 ch);
                        return;
                }
        }
        else
        {
                send_to_char("Not a valid direction\r\n", ch);
                return;
        }

        if (!str_cmp(arg2, "cockpit"))
        {
                if (ship->ship_class > MIDSIZE_SHIP)
                {
                        send_to_char
                                ("Ships of this size don't have cockpits\r\n",
                                 ch);
                        return;
                }
                if (ch->gold < 3000)
                {
                        send_to_char
                                ("You haven't got the money for a cockpit...it's gonna be hard to fly this one\r\n",
                                 ch);
                        return;
                }

                ship->pilotseat = room->vnum;
                ship->coseat = room->vnum;
                ship->navseat = room->vnum;
                ship->gunseat = room->vnum;
                match = TRUE;
                ch->gold -= 3000;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("Cockpit");
        }
        if (!str_cmp(arg2, "pilot"))
        {
                if (ch->gold < 1000)
                {
                        send_to_char
                                ("You haven't got the money for that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 1000;
                ship->pilotseat = room->vnum;
                match = TRUE;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("Pilot's Station");

        }
        if (!str_cmp(arg2, "nav"))
        {
                if (ch->gold < 1000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 1000;
                ship->navseat = room->vnum;
                match = TRUE;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("Navigator's Station");
        }
        if (!str_cmp(arg2, "gun"))
        {
                if (ch->gold < 1000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 1000;
                match = TRUE;
                ship->gunseat = room->vnum;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("Gunner's Station");
        }
        if (!str_cmp(arg2, "copilot"))
        {
                if (ch->gold < 1000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 1000;
                match = TRUE;
                ship->coseat = room->vnum;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("Co-Pilot's Station");
        }
        if (!str_cmp(arg2, "engine"))
        {
                if (ch->gold < 1000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 1000;
                match = TRUE;
                ship->engineroom = room->vnum;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("The Engine Room");
        }
        if (!str_cmp(arg2, "turret"))
        {
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        send_to_char
                                ("starfighters are too small to have turrets\r\n",
                                 ch);
                        return;
                }
                if (ship->turret1 == 0)
                {
                        tset = TRUE;
                        ship->turret1 = room->vnum;
                }
                else if (ship->turret2 == 0)
                {
                        tset = TRUE;
                        ship->turret2 = room->vnum;
                }
                if (!tset)
                {
                        send_to_char("There is no more room for turrets\r\n",
                                     ch);
                        return;
                }
                if (ch->gold < 10000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 10000;
                match = TRUE;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("Turret");
        }
        if (!str_cmp(arg2, "hanger"))
        {
                if (ship->ship_class < MIDSIZE_SHIP)
                {
                        send_to_char("Starfighters don't have hangers!!\r\n",
                                     ch);
                        return;
                }
                if (ship->hanger != 0)
                {
                        send_to_char("sorry only one hanger per ship\r\n",
                                     ch);
                        return;
                }
                if (ch->gold < 5000)
                {
                        send_to_char
                                ("You haven't got the money for that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 5000;

                match = TRUE;
                ship->hanger = room->vnum;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("The Hanger");
        }
        if (!str_cmp(arg2, "workshop"))
        {
                if (ch->gold < 10000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 10000;
                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_FACTORY);
                xSET_BIT(room->room_flags, ROOM_REFINERY);
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("A Workshop");
        }
        if (!str_cmp(arg2, "bacta"))
        {
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        send_to_char
                                ("starfighters are too small to have turrets\r\n",
                                 ch);
                        return;
                }

                if (ch->gold < 25000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 25000;
                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_BACTA);
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("Bacta Tank");
        }
        if (!str_cmp(arg2, "meditate"))
        {
                if (ch->gold < 100000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 100000;

                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_SILENCE);
                xSET_BIT(room->room_flags, ROOM_SAFE);
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("A Quiet Meditation Chamber");
        }
        if (!str_cmp(arg2, "hotel"))
        {
                if (ch->gold < 5000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 5000;

                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_HOTEL);
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("The Passenger's Lounge");
        }
        if (!str_cmp(arg2, "home"))
        {
                if (ch->gold < 10000)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 10000;

                match = TRUE;
                xSET_BIT(room->room_flags, ROOM_EMPTY_HOME);
                xSET_BIT(room->room_flags, ROOM_HOTEL);
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("An Empty Apartment");

        }
        if (!str_cmp(arg2, "turbolift"))
        {
                if (ch->gold < 500)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 500;

                match = TRUE;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("A Turbolift");
        }
        if (!str_cmp(arg2, "corridor"))
        {
                if (ch->gold < 500)
                {
                        send_to_char
                                ("You haven't got the money for a that!\r\n",
                                 ch);
                        return;
                }
                ch->gold -= 500;

                match = TRUE;
                if (room->name)
                        STRFREE(room->name);
                room->name = STRALLOC("A Corridor");
        }

        if (!match)
        {
                send_to_char("&RNo such room type\r\n", ch);
                return;
        }
        if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north") ||
            !str_cmp(arg1, "s") || !str_cmp(arg1, "south") ||
            !str_cmp(arg1, "e") || !str_cmp(arg1, "east") ||
            !str_cmp(arg1, "w") || !str_cmp(arg1, "west") ||
            !str_cmp(arg1, "sw") || !str_cmp(arg1, "southwest") ||
            !str_cmp(arg1, "nw") || !str_cmp(arg1, "northwest") ||
            !str_cmp(arg1, "se") || !str_cmp(arg1, "southeast") ||
            !str_cmp(arg1, "ne") || !str_cmp(arg1, "northeast") ||
            !str_cmp(arg1, "u") || !str_cmp(arg1, "up") ||
            !str_cmp(arg1, "d") || !str_cmp(arg1, "down"))
        {
                if (get_exit(ch->in_room, get_dir(arg1)))
                {
                        send_to_char
                                ("There is already an exit in that direction\r\n",
                                 ch);
                        return;
                }
                else
                {
                        pexit = make_exit(ch->in_room, room, get_dir(arg1));
                        pexit->keyword = STRALLOC("");
                        pexit->description = STRALLOC("");
                        pexit->key = -1;
                        pexit = make_exit(room, ch->in_room,
                                          rev_dir[get_dir(arg1)]);
                        pexit->keyword = STRALLOC("");
                        pexit->description = STRALLOC("");
                        pexit->key = -1;
                }
        }

        else
        {
                send_to_char("Not a valid direction\r\n", ch);
                return;
        }
        ship->lastbuilt = room->vnum;
        save_ship(ship);
        fold_area(room->area, room->area->filename, TRUE, FALSE);
}

CMDF do_decorate(CHAR_DATA * ch, char *argument)
{
        SHIP_DATA *ship;
        ROOM_INDEX_DATA *room;
        AREA_DATA *tarea;
        int       tmplvl;
        char      arg1[MAX_INPUT_LENGTH];
        char      buf[MSL];
        INSTALLATION_DATA *installation;

#ifdef OLC_HOMES
		if (ch->in_room && ch->in_room->home) {
			ch->in_room->home->decorate_room(ch, argument);
			return;
		}
#endif

        argument = one_argument(argument, arg1);

        if (arg1[0] != '\0' && !str_cmp(arg1, "name"))
        {
                room = ch->in_room;
                if (!room)
                {
                        bug("do_decorate: Trying to decorate NULL room", 0);
                        return;
                }

                ship = ship_from_room(room->vnum);
                installation = installation_from_room(room->vnum);
                if (!ship && !installation && !IS_IMMORTAL(ch))
                {
                        send_to_char
                                ("&RYou can currently only do this on ships an in installations.\r\n&C&w",
                                 ch);
                        return;
                }

                if (ship && !installation)
                        if (!check_pilot(ch, ship))
                        {
                                send_to_char
                                        ("&RTry asking the owner first&C&w\r\n",
                                         ch);
                                return;
                        }
                if (ship && !installation)
                        if (ship->type != PLAYER_SHIP)
                        {
                                send_to_char
                                        ("Currently you can only decorate custom ships...\r\n",
                                         ch);
                                send_to_char
                                        ("Petition the imms for a change of policy if ya don't like it\r\n",
                                         ch);
                                return;
                        }
                if (!ship && installation)
                        if (IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan
                            || ch->pcdata->clan != installation->clan)
                        {
                                send_to_char
                                        ("You must be in this installations clan to do that.\r\n",
                                         ch);
                                return;
                        }

                if (!ship && installation)
                {
                        if (installation
                            &&
                            ((ch->pcdata && ch->pcdata->bestowments
                              && is_name("installations",
                                         ch->pcdata->bestowments))
                             || !str_cmp(ch->name, ch->pcdata->clan->leader)
                             || !str_cmp(ch->name, ch->pcdata->clan->number1)
                             || !str_cmp(ch->name,
                                         ch->pcdata->clan->number2)))
                                ;
                        else
                        {
                                send_to_char
                                        ("Your clan has not empowered you with that ability.\n\r",
                                         ch);
                                return;
                        }
                }

/*possible security hole...temporarily sets the char to level 110 so they can use redit 
  desc..might want to either figure out another way to do this or make sure your dangerous
  imm commands check to make sure it's not coming from redit */

                snprintf(buf, MSL, "name %s", argument);
                tarea = room->area;
                tmplvl = ch->top_level;
                ch->top_level = 152;
                do_redit(ch, buf);
                ch->top_level = tmplvl;
                fold_area(tarea, tarea->filename, FALSE, FALSE);
                return;

        }

        if (arg1[0] != '\0' && !str_cmp(arg1, "ambiance"))
        {
                MPROG_DATA *mprog, *mprg;
                int       mptype, len;
                char      buf[MSL];

                if (strlen(arg1) > 80)
                {
                        send_to_char
                                ("Your line cannot be more than 80 characters long",
                                 ch);
                        return;
                }

                mprog = ch->in_room->mudprogs;
                mptype = get_mpflag("rand");

                if (mptype == -1)
                {
                        bug("do_decorateroom : %s in %d\n\r\tUnknown program type.\n\r", ch->name, ch->in_room->vnum);
                        return;
                }

                if (mprog)
                        for (; mprog->next; mprog = mprog->next);
                CREATE(mprg, MPROG_DATA, 1);

                if (mprog)
                        mprog->next = mprg;
                else
                        ch->in_room->mudprogs = mprg;

                ch->in_room->progtypes |= (1 << mptype);
                mprg->type = 1 << mptype;
                mprg->arglist = STRALLOC("100");

                if (mprg->comlist)
                        STRFREE(mprg->comlist);

                snprintf(buf, MSL, "mpecho %s", argument);
                smush_tilde(buf);
                len = strlen(buf);
                if (buf && buf[len - 1] == '~')
                        buf[len - 1] = '\0';
                else
                        mudstrlcat(buf, "\n\r", MSL);
                mprg->comlist = STRALLOC(buf);
                return;
        }

        room = ch->in_room;
        room = ch->in_room;
        if (!room)
        {
                bug("do_decorate: Trying to decorate NULL room", 0);
                return;
        }

        ship = ship_from_room(room->vnum);
        installation = installation_from_room(room->vnum);
        if (!ship && !installation)
        {
                send_to_char
                        ("&RYou can currently only do this on ships an in installations.\r\n&C&w",
                         ch);
                return;
        }

        if (ship && !installation)
                if (!check_pilot(ch, ship))
                {
                        send_to_char("&RTry asking the owner first&C&w\r\n",
                                     ch);
                        return;
                }
        if (ship && !installation)
                if (ship->type != PLAYER_SHIP)
                {
                        send_to_char
                                ("Currently you can only decorate custom ships...\r\n",
                                 ch);
                        send_to_char
                                ("Petition the imms for a change of policy if ya don't like it\r\n",
                                 ch);
                        return;
                }
        if (!ship && installation)
                if (IS_NPC(ch) || !ch->pcdata || !ch->pcdata->clan
                    || ch->pcdata->clan != installation->clan)
                {
                        send_to_char
                                ("You must be in this installations clan to do that.\r\n",
                                 ch);
                        return;
                }

        if (!ship && installation)
        {
                if (installation && ((ch->pcdata && ch->pcdata->bestowments
                                      && is_name("installations",
                                                 ch->pcdata->bestowments))
                                     || !str_cmp(ch->name,
                                                 ch->pcdata->clan->leader)
                                     || !str_cmp(ch->name,
                                                 ch->pcdata->clan->number1)
                                     || !str_cmp(ch->name,
                                                 ch->pcdata->clan->number2)))
                        ;
                else
                {
                        send_to_char
                                ("Your clan has not empowered you with that ability.\n\r",
                                 ch);
                        return;
                }
        }
/*possible security hole...temporarily sets the char to level 110 so they can use redit 
  desc..might want to either figure out another way to do this or make sure your dangerous
  imm commands check to make sure it's not coming from redit */

        tarea = room->area;
        tmplvl = ch->top_level;
        ch->top_level = 152;
        do_redit(ch, "desc");
        ch->top_level = tmplvl;
        fold_area(tarea, tarea->filename, FALSE, FALSE);
        return;
}

/* this was added by request from the implementor of the mud i player tested it on
   gives engis an alternative to selling the ships to keep the vnums free and to 
   make some of the expense of building it back. 
   Note - You may also want to adjust do_sellship to severely cut the price on 
          player ships to give them a reson to recycle rather than sell.
*/

CMDF do_recycle(CHAR_DATA * ch, char *argument)
{
        long      price;
        SHIP_DATA *ship;

        if (argument[0] == '\0')
        {
                send_to_char("Usage: recycle <ship>\r\n", ch);
                return;
        }

        ship = ship_in_room(ch->in_room, argument);
        if (!ship)
        {
                act(AT_PLAIN, "I see no $T here.", ch, NULL, argument,
                    TO_CHAR);
                return;
        }

        if (str_cmp(ship->owner, ch->name) && !IS_IMMORTAL(ch))
        {
                send_to_char("&RThat isn't your ship!", ch);
                return;
        }

        if (ship->type != PLAYER_SHIP)
        {
                send_to_char("You can only recycle custom built ships\r\n",
                             ch);
                return;
        }

        price = get_ship_value(ship) / 2;

        ch->gold += price;
        ch_printf(ch,
                  "&GYou receive %ld credits from recycling your ship.\n\r",
                  price);
        send_to_char
                ("\r\nSeveral heavy droids chop up and carry off your ship.\r\n",
                 ch);

        act(AT_PLAIN,
            "$n walks over to a terminal and makes a credit transaction.", ch,
            NULL, argument, TO_ROOM);
        act(AT_PLAIN, "Several heavy droids chop up and carry off a ship.",
            ch, NULL, argument, TO_ROOM);
        transship(ship, 45);
        really_destroy_ship(ship);
}



CMDF do_fleet(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];

        argument = one_argument(argument, arg);

        if (!str_cmp(arg, "make"))
                fleet_make(ch, argument);
/*	else if ( !str_cmp(arg, "addship" ) )
		fleet_addship(ch, argument);
	else if ( !str_cmp(arg, "remship" ) )
		fleet_remship(ch, argument);
	else if ( !str_cmp(arg, "commandship" ) )
		fleet_commandship(ch, argument);
	else if ( !str_cmp(arg, "commandfleet" ) )
		fleet_commandfleet(ch, argument);
	else if ( !str_cmp(arg, "disbandfleet" ) )
		fleet_disbandfleet(ch, argument);	*/
}

void fleet_make(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      filename[MAX_STRING_LENGTH];
        int       percentage, ship_class;
        SHIP_DATA *ship;
        PLANET_DATA *planet;
        SPACE_DATA *starsystem;
        CLAN_DATA *clan;
        int       durasteel, transparisteel, cost;

        argument = one_argument(argument, arg1);
        mudstrlcpy(arg2, argument, MIL);



        switch (ch->substate)
        {
        default:


                if (arg1[0] == '\0' || arg2[0] == '\0')
                {
                        send_to_char
                                ("&RSyntax: &GFleet make <type> <name>\r\n",
                                 ch);
                        send_to_char
                                ("&RSyntax: &GTypes: Fighter, Mid, Cap&w\r\n",
                                 ch);
                        return;
                }

                clan = ch->pcdata->clan;
                if (!clan)
                {
                        send_to_char("You are not in a clan.\n\r", ch);
                        return;
                }



                for (ship = first_ship; ship; ship = ship->next)
                {
                        if (!str_cmp(ship->name, arg2))
                        {
                                send_to_char
                                        ("&CThat ship name is already in use. Choose another.\r\n",
                                         ch);
                                return;
                        }
                }


                /*
                 * used import flag to avoid adding an extra shipyard flg. 
                 * it can be changed if you don't have my cargo snippet - Ortluk 
                 * Also if you're installing in swfote you can uncomment the first if statement 
                 * and comment the second one out to use the shipyard flag instead
                 */

                if (!xIS_SET(ch->in_room->room_flags, ROOM_IMPORT))
                {
                        send_to_char
                                ("You can't do that here!! Try a spaceport\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg1, "cap"))
                        ship_class = CAPITAL_SHIP;
                else if (!str_cmp(arg1, "mid"))
                        ship_class = MIDSIZE_SHIP;
                else if (!str_cmp(arg1, "fighter"))
                        ship_class = FIGHTER_SHIP;
                else
                {
                        send_to_char("&GThats not a ship type.\n\r", ch);
                        return;
                }
                durasteel = ship_class * 15 + 10;
                transparisteel = ship_class * 2 + 1;
                if (xIS_SET(ch->in_room->room_flags, ROOM_INSTALLATION))
                {
                        INSTALLATION_DATA *installation =
                                installation_from_room(ch->in_room->vnum);
                        if (installation
                            && installation->type == SHIPYARD_INSTALLATION)
                                planet = installation->planet;
                }
                else
                        planet = ch->in_room->area->planet;
                if (!planet)
                {
                        send_to_char
                                ("&RAnd where do you think you're going to get the resources to build your ship?&C&w",
                                 ch);
                        return;
                }

                if (planet->governed_by && planet->governed_by != clan)
                {
                        send_to_char
                                ("You do not belong to this planets government, how can you constuct ships here?",
                                 ch);
                        return;
                }

                /*
                 * make sure the planet has the resources to build the ship 
                 */
                if (planet->resource[CARGO_DURASTEEL] < durasteel)
                {
                        send_to_char
                                ("&RYou'll Have to wait till they either import or produce more durasteel&C&w\r\n",
                                 ch);
                        return;
                }

                if (planet->resource[CARGO_TRANSPARISTEEL] < transparisteel)
                {
                        send_to_char
                                ("&RYou'll Have to wait till they either import or produce more transparisteel&C&w\r\n",
                                 ch);
                        return;
                }
                cost = 10;
                if (planet->cargoimport[CARGO_TRANSPARISTEEL] > 0)
                        cost += planet->cargoimport[CARGO_TRANSPARISTEEL] +
                                planet->cargoimport[CARGO_TRANSPARISTEEL] / 2;
                else if (planet->cargoexport[CARGO_TRANSPARISTEEL] > 0)
                        cost += planet->cargoexport[CARGO_TRANSPARISTEEL];
                else
                        cost += 10;

                if (planet->cargoimport[CARGO_DURASTEEL] > 0)
                        cost += planet->cargoimport[CARGO_DURASTEEL] +
                                planet->cargoimport[CARGO_DURASTEEL] / 2;
                else if (planet->cargoexport[CARGO_DURASTEEL] > 0)
                        cost += planet->cargoexport[CARGO_DURASTEEL];
                else
                        cost += 10;

                cost *= durasteel + transparisteel;
                if (clan->funds < cost)
                {
                        send_to_char
                                ("&RYour government can't afford the materials to build that.\r\n",
                                 ch);
                        return;
                }



                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_fleet_command1]);
                if (number_percent() < percentage)
                {
                        send_to_char("&GYou begin to launch a ship.\n\r", ch);
                        add_timer(ch, TIMER_DO_FUN, 45, fleet_make, 1);
                        ch->dest_buf = str_dup(arg1);
                        ch->dest_buf_2 = str_dup(arg2);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to launch the ship.\n\r",
                         ch);
                learn_from_failure(ch, gsn_fleet_command1);
                return;

        case 1:

                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                {
                        bug("null ch->dest_buf2", 0);
                        return;
                }

                mudstrlcpy(arg1, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                mudstrlcpy(arg2, (char *) ch->dest_buf_2, MIL);
                DISPOSE(ch->dest_buf_2);
                break;

        case SUB_TIMER_DO_ABORT:

                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;

        /*
         * these values come from  cargo v2 
         */

        if (!str_cmp(arg1, "cap"))
                ship_class = CAPITAL_SHIP;
        else if (!str_cmp(arg1, "mid"))
                ship_class = MIDSIZE_SHIP;
        else
                ship_class = FIGHTER_SHIP;

        transparisteel = ship_class * 5 + 1;
        durasteel = ship_class * 10 + 10;
        planet = ch->in_room->area->planet;
        starsystem = planet->starsystem;
        if (!planet)
        {
                send_to_char
                        ("&RYou must have been moved...I'd complain about the planet.&C&w",
                         ch);
                return;
        }

        if (!starsystem)
        {
                send_to_char
                        ("&RYou must have been moved...I'd complain about the starsystem.&C&w",
                         ch);
                return;
        }


        clan = ch->pcdata->clan;
        if (!clan)
        {
                send_to_char("You are not in a clan.\n\r", ch);
                return;
        }

        cost = 10;
        if (planet->cargoimport[CARGO_TRANSPARISTEEL] > 0)
                cost += planet->cargoimport[CARGO_TRANSPARISTEEL] +
                        planet->cargoimport[CARGO_TRANSPARISTEEL] / 2;
        else if (planet->cargoexport[CARGO_TRANSPARISTEEL] > 0)
                cost += planet->cargoexport[CARGO_TRANSPARISTEEL];
        else
                cost += 10;


        if (planet->cargoimport[CARGO_DURASTEEL] > 0)
                cost += planet->cargoimport[CARGO_DURASTEEL] +
                        planet->cargoimport[CARGO_DURASTEEL] / 2;
        else if (planet->cargoexport[CARGO_DURASTEEL] > 0)
                cost += planet->cargoexport[CARGO_DURASTEEL];
        else
                cost += 10;

        cost *= durasteel + transparisteel;
        if (clan->funds < cost)
        {
                send_to_char
                        ("&RYour clan can't afford the materials....Stop that THIEF!!!\r\n",
                         ch);
                return;
        }
        clan->funds -= cost;
        planet->resource[CARGO_TRANSPARISTEEL] -= transparisteel;
        planet->resource[CARGO_DURASTEEL] -= durasteel;

        snprintf(filename, MSL, "%s.mship", arg2);

        CREATE(ship, SHIP_DATA, 1);
        LINK(ship, first_ship, last_ship, next, prev);
        ship->filename = STRALLOC(smash_space(filename));
        ship->name = STRALLOC(arg2);
        ship->owner = clan->name;
        ship->clan = clan;
        ship->copilot = STRALLOC("");
        ship->pilot = STRALLOC("");
        ship->home = STRALLOC("");
        ship->type = CLAN_MOB_SHIP;
        ship->selfdestruct = STRALLOC("Installed");
        ship->selfdpass = number_range(10001, 99999);

/* you may want to adjust these to balance ships with your imm built ones 
   I use an array of maximum ship stats for the different classes of ships
   that's another project though */

        ship->maxenergy =
                (ship_class +
                 1) * 50 * ch->pcdata->learned[gsn_fleet_command1];
        ship->energy = ship->maxenergy;
        ship->maxhull =
                (ship_class +
                 1) * 10 * ch->pcdata->learned[gsn_fleet_command1];
        ship->maxshield =
                (ship_class +
                 1) * 5 * ch->pcdata->learned[gsn_fleet_command1];
        ship->realspeed =
                2 * ch->pcdata->learned[gsn_fleet_command1] / (ship_class +
                                                               1);
        ship->hyperspeed =
                ch->pcdata->learned[gsn_fleet_command1] + (ship_class +
                                                           1) * 20;
        ship->lasers =
                (ship_class +
                 1) * (ch->pcdata->learned[gsn_fleet_command1] / 20);
        ship->manuever =
                ch->pcdata->learned[gsn_fleet_command1] * 2 / (ship_class +
                                                               1);
        ship->comm =
                ch->pcdata->learned[gsn_fleet_command1] * 2 / (ship_class +
                                                               1);
        ship->sensor =
                ch->pcdata->learned[gsn_fleet_command1] * 2 / (ship_class +
                                                               1);


        ship->description = "";
        ship->hull = ship->maxhull;
        ship->in_room = NULL;
        ship->currjump = NULL;
        ship->target0 = NULL;
        ship->target1 = NULL;
        ship->target2 = NULL;
        ship->ship_class = ship_class;
        ship->home = starsystem->name;
        ship->vx = number_range(-5000, 5000);
        ship->vy = number_range(-5000, 5000);
        ship->vz = number_range(-5000, 5000);
        ship->hx = 1;
        ship->hy = 1;
        ship->hz = 1;
        ship->currspeed = 0;


        send_to_char
                ("You complete your work constructing part of your fleet\n\r",
                 ch);
        gain_exp(ch, 100000, PILOTING_ABILITY);
        ship_to_starsystem(ship, starsystem_from_name(ship->home));
        ch_printf(ch, "&WYou gain 100000 engineering experience.\r\n");
        learn_from_success(ch, gsn_fleet_command1);

        act(AT_PLAIN, "$n finishes launching the new ship.", ch,
            NULL, NULL, TO_ROOM);
        save_ship(ship);
        write_ship_list();
}

CMDF do_modifyship(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        SHIP_DATA *ship = NULL;
        bool      match, checktool;
        int       percentage;
        OBJ_DATA *obj;

        match = FALSE;

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        switch (ch->substate)
        {
        default:
                checktool = FALSE;
                if (arg[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0')
                {
                        send_to_char
                                ("Usage: modifyship <ship> <add> <remove>\r\n",
                                 ch);
                        send_to_char
                                ("Options: speed manuever hyperspeed cargo battalions\r\n",
                                 ch);
                        send_to_char
                                ("Options: lasers ions missiles rockets torpedos\r\n",
                                 ch);
                        send_to_char
                                ("Options: tractors chaff shields hull energy\r\n",
                                 ch);
                        send_to_char("Options: stealth cloak interdictor\r\n",
                                     ch);

                        return;
                }
                ship = ship_in_room(ch->in_room, arg);
                if (!ship)
                {
                        send_to_char("That ship is not here.\r\n", ch);
                        return;
                }
                if (!check_pilot(ch, ship) || !str_cmp(ship->owner, "public"))
                {
                        send_to_char
                                ("You don't have permission to modify this ship\r\n",
                                 ch);
                        return;
                }
                if (str_cmp(arg1, "speed") && str_cmp(arg1, "cargo")
                    && str_cmp(arg1, "battalions") && str_cmp(arg1, "lasers")
                    && str_cmp(arg1, "ions") && str_cmp(arg1, "missiles")
                    && str_cmp(arg1, "torpedos")
                    && str_cmp(arg1, "hyperspeed") && str_cmp(arg1, "hull")
                    && str_cmp(arg1, "shields") && str_cmp(arg1, "cloak")
                    && str_cmp(arg1, "stealth") && str_cmp(arg1, "energy")
                    && str_cmp(arg1, "tractor") && str_cmp(arg1, "rockets")
                    && str_cmp(arg1, "selfdestruct") && str_cmp(arg1, "chaff")
                    && str_cmp(arg1, "manuever")
                    && str_cmp(arg1, "interdictor"))
                {
                        send_to_char
                                ("That is not a valid ship field to modify. Try help modifyship\r\n",
                                 ch);
                        return;
                }
                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RI'd like to see you modify a ship with no tools.\r\n",
                                 ch);
                        return;
                }

                percentage =
                        IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                            learned
                                                            [gsn_modifyship]);


                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou begin to tinker with the ship.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and starts to work on a ship.\r\n",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 35, do_modifyship, 1);
                        ch->dest_buf = str_dup(arg);
                        ch->dest_buf_2 = str_dup(arg1);
                        ch->dest_buf_3 = str_dup(arg2);
                        return;
                }

                send_to_char("&RYou can't figure out where to begin.\n\r",
                             ch);
                learn_from_failure(ch, gsn_modifyship);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                {
                        bug("null ch->dest_buf2", 0);
                        return;
                }
                if (!ch->dest_buf_3)
                {
                        bug("null ch->dest_buf3", 0);
                        return;
                }
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                DISPOSE(ch->dest_buf);
                mudstrlcpy(arg1, (char *) ch->dest_buf_2, MIL);
                DISPOSE(ch->dest_buf_2);
                mudstrlcpy(arg2, (char *) ch->dest_buf_3, MIL);
                DISPOSE(ch->dest_buf_3);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                DISPOSE(ch->dest_buf_3);

                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work.\n\r",
                         ch);
                return;
        }
        ch->substate = SUB_NONE;

        checktool = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
        }

        percentage =
                IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->
                                                    learned[gsn_modifyship]);

        if (number_percent() > percentage * 2 || (!checktool))
        {
                send_to_char
                        ("&RYou stand back and gather your diagnostic tools.\n\r",
                         ch);
                send_to_char
                        ("&RWith your tools, you realize that you just wasted a lot of time.\n\r",
                         ch);
                learn_from_failure(ch, gsn_modifyship);
                return;
        }

        if ((ship = ship_in_room(ch->in_room, arg)) == NULL)
        {
                send_to_char
                        ("The ship you were working on seems to have moved.\r\n",
                         ch);
                return;
        }

        if (!str_cmp(arg1, "speed"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ships speed\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ships speed\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ships speed\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->realspeed >= 255)
                        {
                                send_to_char
                                        ("This ship has reached its maximum capacity for speed enhancements\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->realspeed >= 200)
                        {
                                send_to_char
                                        ("This ship has reached its maximum capacity for speed enhancements\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->realspeed >= 100)
                        {
                                send_to_char
                                        ("This ship has reached its maximum capacity for speed enhancements\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "cargo")) && (str_cmp(arg2, "battalions"))
                    && (str_cmp(arg2, "manuever"))
                    && (str_cmp(arg2, "hyperspeed")))
                {
                        send_to_char
                                ("You may not substitute that for speed\r\n",
                                 ch);
                        send_to_char
                                ("Options are: cargo, battalions, manuever, hyperspeed\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "cargo"))
                {
                        if (ship->maxcargo < 50)
                        {
                                send_to_char
                                        ("There is not enough free cargo space to fit more engines.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxcargo > 50)
                        {
                                ship->maxcargo -= 50;
                                send_to_char("Ships cargo reduced by 50\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "battalions"))
                {
                        if (ship->maxbattalions < 1)
                        {
                                send_to_char
                                        ("There are not enough free battalion holds to fit more engines.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxbattalions >= 1)
                        {
                                ship->maxbattalions -= 1;
                                send_to_char
                                        ("Ships battalion holds reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "manuever"))
                {
                        if (ship->manuever < 26)
                        {
                                send_to_char
                                        ("There are not enough manuevering thrusters to convert into engines.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->manuever > 26)
                        {
                                ship->manuever -= 25;
                                send_to_char
                                        ("Ships manuever reduced by 25\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "hyperspeed"))
                {
                        if (ship->hyperspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough hyperspace drives to convert into engines\r\n",
                                         ch);
                                return;
                        }
                        if (ship->hyperspeed > 26)
                        {
                                ship->hyperspeed -= 25;
                                send_to_char
                                        ("Ships hyperspeed reduced by 25\r\n",
                                         ch);
                        }
                }

                ship->realspeed += 25;
                send_to_char("Ships speed increased by 25.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "cargo"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->maxcargo >= 10)
                        {
                                send_to_char
                                        ("This ship has reached its maximum cargo capacity.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->maxcargo >= 800)
                        {
                                send_to_char
                                        ("This ship has reached its maximum cargo capacity.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->maxcargo >= 5000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum cargo capacity.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "speed")) && (str_cmp(arg2, "battalions"))
                    && (str_cmp(arg2, "manuever"))
                    && (str_cmp(arg2, "hyperspeed")))
                {
                        send_to_char
                                ("You may not substitute that for cargo\r\n",
                                 ch);
                        send_to_char
                                ("Options are: speed, battalions, manuever, hyperspeed\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "speed"))
                {
                        if (ship->realspeed < 25)
                        {
                                send_to_char
                                        ("There are not enough engines to free up any cargo space..\r\n",
                                         ch);
                                return;
                        }
                        if (ship->realspeed > 25)
                        {
                                ship->realspeed -= 25;
                                send_to_char("Ships speed reduced by 25\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "battalions"))
                {
                        if (ship->maxbattalions < 1)
                        {
                                send_to_char
                                        ("There are not enough free battalion holds to free up any cargo space.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxbattalions >= 1)
                        {
                                ship->maxbattalions -= 1;
                                send_to_char
                                        ("Ships battalion holds reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "manuever"))
                {
                        if (ship->manuever < 26)
                        {
                                send_to_char
                                        ("There are not enough manuevering thrusters to free up any cargo space.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->manuever > 26)
                        {
                                ship->manuever -= 25;
                                send_to_char
                                        ("Ships manuever reduced by 25\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "hyperspeed"))
                {
                        if (ship->hyperspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough hyperspace drives to free up any cargo space.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->hyperspeed > 26)
                        {
                                ship->hyperspeed -= 25;
                                send_to_char
                                        ("Ships hyperspeed reduced by 25\r\n",
                                         ch);
                        }
                }

                ship->maxcargo += 50;
                send_to_char("Ships cargo space increased by 50.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }


        if (!str_cmp(arg1, "battalions"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->maxbattalions >= 0)
                        {
                                send_to_char
                                        ("This ship has reached its maximum capacity for battalions.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->maxbattalions >= 10)
                        {
                                send_to_char
                                        ("This ship has reached its maximum capacity for battalions.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->maxbattalions >= 100)
                        {
                                send_to_char
                                        ("This ship has reached its maximum capacity for battalions.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "cargo")) && (str_cmp(arg2, "speed"))
                    && (str_cmp(arg2, "manuever"))
                    && (str_cmp(arg2, "hyperspeed")))
                {
                        send_to_char
                                ("You may not substitute that for battalion holds\r\n",
                                 ch);
                        send_to_char
                                ("Options are: cargo, speed, manuever, hyperspeed\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "cargo"))
                {
                        if (ship->maxcargo < 50)
                        {
                                send_to_char
                                        ("There is not enough free cargo space to fit more battalion holds..\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxcargo > 50)
                        {
                                ship->maxcargo -= 50;
                                send_to_char("Ships cargo reduced by 50\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "speed"))
                {
                        if (ship->realspeed < 25)
                        {
                                send_to_char
                                        ("There are not enough engines to add a battalion hold.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->realspeed > 25)
                        {
                                ship->realspeed -= 25;
                                send_to_char("Ships speed reduced by 25\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "manuever"))
                {
                        if (ship->manuever < 26)
                        {
                                send_to_char
                                        ("There are not enough manuevering thrusters to fit more battalion holds.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->manuever > 26)
                        {
                                ship->manuever -= 25;
                                send_to_char
                                        ("Ships manuever reduced by 25\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "hyperspeed"))
                {
                        if (ship->hyperspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough hyperspace drives to fit more battalion holds.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->hyperspeed > 26)
                        {
                                ship->hyperspeed -= 25;
                                send_to_char
                                        ("Ships hyperspeed reduced by 25\r\n",
                                         ch);
                        }
                }

                ship->maxbattalions += 1;
                send_to_char("Ships battalion holds increased by 1.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }




        if (!str_cmp(arg1, "manuever"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->manuever >= 255)
                        {
                                send_to_char
                                        ("This ship has reached its maximum manueverability.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->manuever >= 200)
                        {
                                send_to_char
                                        ("This ship has reached its maximum manueverability.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->manuever >= 100)
                        {
                                send_to_char
                                        ("This ship has reached its maximum manueverability.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "cargo")) && (str_cmp(arg2, "battalions"))
                    && (str_cmp(arg2, "speed"))
                    && (str_cmp(arg2, "hyperspeed")))
                {
                        send_to_char
                                ("You may not substitute that for manueverability\r\n",
                                 ch);
                        send_to_char
                                ("Options are: cargo, battalions, speed, hyperspeed\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "cargo"))
                {
                        if (ship->maxcargo < 50)
                        {
                                send_to_char
                                        ("There is not enough free cargo space to convert into manuevering thrusters.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxcargo > 50)
                        {
                                ship->maxcargo -= 50;
                                send_to_char("Ships cargo reduced by 50\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "battalions"))
                {
                        if (ship->maxbattalions < 1)
                        {
                                send_to_char
                                        ("There are not enough free battalion holds to convert into manuevering thrusters.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxbattalions >= 1)
                        {
                                ship->maxbattalions -= 1;
                                send_to_char
                                        ("Ships battalion holds reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "speed"))
                {
                        if (ship->realspeed < 25)
                        {
                                send_to_char
                                        ("There are not enough engines to convert into manuevering thrusters.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->realspeed > 25)
                        {
                                ship->realspeed -= 25;
                                send_to_char("Ships speed reduced by 25\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "hyperspeed"))
                {
                        if (ship->hyperspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough hyperspace drives to convert into manuevering thrusters.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->hyperspeed > 26)
                        {
                                ship->hyperspeed -= 25;
                                send_to_char
                                        ("Ships hyperspeed reduced by 25\r\n",
                                         ch);
                        }
                }

                ship->manuever += 25;
                send_to_char("Ships manueverability increased by 25.\r\n",
                             ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "hyperspeed"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->hyperspeed >= 255)
                        {
                                send_to_char
                                        ("This ship has reached its maximum hyperspeed.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->realspeed >= 200)
                        {
                                send_to_char
                                        ("This ship has reached its maximum hyperspeed.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->realspeed >= 100)
                        {
                                send_to_char
                                        ("This ship has reached its maximum hyperspeed.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "cargo")) && (str_cmp(arg2, "battalions"))
                    && (str_cmp(arg2, "manuever"))
                    && (str_cmp(arg2, "speed")))
                {
                        send_to_char
                                ("You may not substitute that for hyperspeed\r\n",
                                 ch);
                        send_to_char
                                ("Options are: cargo, battalions, manuever, speed\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "cargo"))
                {
                        if (ship->maxcargo < 50)
                        {
                                send_to_char
                                        ("There is not enough free cargo space to fit more engines.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxcargo > 50)
                        {
                                ship->maxcargo -= 50;
                                send_to_char("Ships cargo reduced by 50\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "battalions"))
                {
                        if (ship->maxbattalions < 1)
                        {
                                send_to_char
                                        ("There are not enough free battalion holds to fit more engines.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxbattalions >= 1)
                        {
                                ship->maxbattalions -= 1;
                                send_to_char
                                        ("Ships battalion holds reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "manuever"))
                {
                        if (ship->manuever < 26)
                        {
                                send_to_char
                                        ("There are not enough manuevering thrusters to convert into engines.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->manuever > 26)
                        {
                                ship->manuever -= 25;
                                send_to_char
                                        ("Ships manuever reduced by 25\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "speed"))
                {
                        if (ship->realspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough engines to convert into hyperspace drives.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->realspeed > 26)
                        {
                                ship->realspeed -= 25;
                                send_to_char("Ships speed reduced by 25\r\n",
                                             ch);
                        }
                }

                ship->hyperspeed += 25;
                send_to_char("Ships hyperspeed increased by 25.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "lasers"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->lasers >= 6)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of lasers.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->lasers >= 15)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of lasers.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->lasers >= 30)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of lasers.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "ions")) && (str_cmp(arg2, "rockets"))
                    && (str_cmp(arg2, "torpedos"))
                    && (str_cmp(arg2, "missiles")) && (str_cmp(arg2, "chaff"))
                    && (str_cmp(arg2, "tractor")))
                {
                        send_to_char
                                ("You may not substitute that for lasers\r\n",
                                 ch);
                        send_to_char
                                ("Options are: ions, rockets, torpedos, missiles, chaff, tractor\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "chaff"))
                {
                        if (ship->maxchaff < 1)
                        {
                                send_to_char
                                        ("There are not any chaff pods to replace with lasers.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxchaff > 1)
                        {
                                ship->maxchaff -= 1;
                                send_to_char
                                        ("Ships chaff pods reduced by 1\r\n",
                                         ch);
                        }
                }
                if (!str_cmp(arg2, "tractor"))
                {
                        if (ship->tractorbeam <= 0)
                        {
                                send_to_char
                                        ("There are not any tractor beams to replace with lasers.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->tractorbeam >= 1)
                        {
                                ship->tractorbeam -= 1;
                                send_to_char
                                        ("Ships tractor beams reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "ions"))
                {
                        if (ship->ions <= 0)
                        {
                                send_to_char
                                        ("There are not any ion cannons to replace with lasers.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->ions >= 1)
                        {
                                ship->ions -= 1;
                                send_to_char
                                        ("Ships ion cannons reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "rockets"))
                {
                        if (ship->maxrockets < 2)
                        {
                                send_to_char
                                        ("There are not enough rockets to replace with lasers.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxrockets >= 2)
                        {
                                ship->maxrockets -= 2;
                                send_to_char("Ships rockets reduced by 2\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "torpedos"))
                {
                        if (ship->maxtorpedos < 4)
                        {
                                send_to_char
                                        ("There are not enough torpedos to replace with lasers.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxtorpedos >= 4)
                        {
                                ship->maxtorpedos -= 4;
                                send_to_char
                                        ("Ships torpedos reduced by 4\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "missiles"))
                {
                        if (ship->maxmissiles < 6)
                        {
                                send_to_char
                                        ("There are not enough missiles to replace with lasers\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxmissiles >= 6)
                        {
                                ship->maxmissiles -= 6;
                                send_to_char("Ships missiles by 6\r\n", ch);
                        }
                }

                ship->lasers += 1;
                send_to_char("Ships lasers increased by 1.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "ions"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->ions >= 6)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of ion cannons.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->ions >= 15)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of ion cannons.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->ions >= 30)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of ion cannons.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "lasers")) && (str_cmp(arg2, "rockets"))
                    && (str_cmp(arg2, "torpedos"))
                    && (str_cmp(arg2, "missiles")) && (str_cmp(arg2, "chaff"))
                    && (str_cmp(arg2, "tractor")))
                {
                        send_to_char
                                ("You may not substitute that for ion cannons\r\n",
                                 ch);
                        send_to_char
                                ("Options are: lasers, rockets, torpedos, missiles, chaff, tractor\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "chaff"))
                {
                        if (ship->maxchaff < 1)
                        {
                                send_to_char
                                        ("There are not any chaff pods to replace with ion cannons.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxchaff >= 1)
                        {
                                ship->maxchaff -= 1;
                                send_to_char
                                        ("Ships chaff pods reduced by 1\r\n",
                                         ch);
                        }
                }
                if (!str_cmp(arg2, "tractor"))
                {
                        if (ship->tractorbeam <= 0)
                        {
                                send_to_char
                                        ("There are not any tractor beams to replace with ion cannons.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->tractorbeam >= 1)
                        {
                                ship->tractorbeam -= 1;
                                send_to_char
                                        ("Ships tractor beams reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "lasers"))
                {
                        if (ship->lasers < 1)
                        {
                                send_to_char
                                        ("There are not any lasers to replace with ion cannons.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->lasers >= 1)
                        {
                                ship->lasers -= 1;
                                send_to_char("Ships lasers reduced by 1\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "rockets"))
                {
                        if (ship->maxrockets < 2)
                        {
                                send_to_char
                                        ("There are not enough rockets to replace with ion cannons.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxrockets >= 2)
                        {
                                ship->maxrockets -= 2;
                                send_to_char("Ships rockets reduced by 2\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "torpedos"))
                {
                        if (ship->maxtorpedos < 4)
                        {
                                send_to_char
                                        ("There are not enough torpedos to replace with ion cannons.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxtorpedos >= 4)
                        {
                                ship->maxtorpedos -= 4;
                                send_to_char
                                        ("Ships torpedos reduced by 4\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "missiles"))
                {
                        if (ship->maxmissiles < 6)
                        {
                                send_to_char
                                        ("There are not enough missiles to replace with ion cannons\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxmissiles >= 6)
                        {
                                ship->maxmissiles -= 6;
                                send_to_char
                                        ("Ships missiles reduced by 6\r\n",
                                         ch);
                        }
                }

                ship->ions += 1;
                send_to_char("Ships ion cannons increased by 1.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                ch->gold -= 30000;
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "missiles"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->maxmissiles >= 10)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of missiles.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->maxmissiles >= 30)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of missiles.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->maxmissiles >= 90)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of missiles.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "ions")) && (str_cmp(arg2, "rockets"))
                    && (str_cmp(arg2, "torpedos"))
                    && (str_cmp(arg2, "lasers")) && (str_cmp(arg2, "chaff"))
                    && (str_cmp(arg2, "tractor")))
                {
                        send_to_char
                                ("You may not substitute that for missiles\r\n",
                                 ch);
                        send_to_char
                                ("Options are: ions, rockets, torpedos, lasers, chaff, tractor\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "chaff"))
                {
                        if (ship->maxchaff < 1)
                        {
                                send_to_char
                                        ("There are not any chaff pods to replace with missiles.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxchaff >= 1)
                        {
                                ship->maxchaff -= 1;
                                send_to_char
                                        ("Ships chaff pods reduced by 1\r\n",
                                         ch);
                        }
                }
                if (!str_cmp(arg2, "tractor"))
                {
                        if (ship->tractorbeam <= 0)
                        {
                                send_to_char
                                        ("There are not any tractor beams to replace with missiles.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->tractorbeam >= 1)
                        {
                                ship->tractorbeam -= 1;
                                send_to_char
                                        ("Ships tractor beams reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "ions"))
                {
                        if (ship->ions <= 0)
                        {
                                send_to_char
                                        ("There are not any ion cannons to replace with missiles.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->ions >= 1)
                        {
                                ship->ions -= 1;
                                send_to_char
                                        ("Ships ion cannons reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "rockets"))
                {
                        if (ship->maxrockets < 2)
                        {
                                send_to_char
                                        ("There are not enough rockets to replace with missiles.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxrockets >= 2)
                        {
                                ship->maxrockets -= 2;
                                send_to_char("Ships rockets reduced by 2\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "torpedos"))
                {
                        if (ship->maxtorpedos < 4)
                        {
                                send_to_char
                                        ("There are not enough torpedos to replace with missiles.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxtorpedos >= 4)
                        {
                                ship->maxtorpedos -= 4;
                                send_to_char
                                        ("Ships torpedos reduced by 4\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "lasers"))
                {
                        if (ship->lasers < 1)
                        {
                                send_to_char
                                        ("There are not any lasers to replace with missiles\r\n",
                                         ch);
                                return;
                        }
                        if (ship->lasers >= 1)
                        {
                                ship->lasers -= 1;
                                send_to_char("Ships lasers reduced by 1\r\n",
                                             ch);
                        }
                }

                ship->maxmissiles += 6;
                send_to_char("Ships missiles increased by 6.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "torpedos"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->maxtorpedos >= 5)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of torpedos.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->maxtorpedos >= 15)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of torpedos.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->maxtorpedos >= 45)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of torpedos.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "ions")) && (str_cmp(arg2, "rockets"))
                    && (str_cmp(arg2, "lasers"))
                    && (str_cmp(arg2, "missiles")) && (str_cmp(arg2, "chaff"))
                    && (str_cmp(arg2, "tractor")))
                {
                        send_to_char
                                ("You may not substitute that for torpedos\r\n",
                                 ch);
                        send_to_char
                                ("Options are: ions, rockets, lasers, missiles, chaff, tractor\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "chaff"))
                {
                        if (ship->maxchaff < 1)
                        {
                                send_to_char
                                        ("There are not any chaff pods to replace with torpedos.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxchaff >= 1)
                        {
                                ship->maxchaff -= 1;
                                send_to_char
                                        ("Ships chaff pods reduced by 1\r\n",
                                         ch);
                        }
                }
                if (!str_cmp(arg2, "tractor"))
                {
                        if (ship->tractorbeam <= 0)
                        {
                                send_to_char
                                        ("There are not any tractor beams to replace with torpedos.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->tractorbeam >= 1)
                        {
                                ship->tractorbeam -= 1;
                                send_to_char
                                        ("Ships tractor beams reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "ions"))
                {
                        if (ship->ions <= 0)
                        {
                                send_to_char
                                        ("There are not any ion cannons to replace with torpedos.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->ions >= 1)
                        {
                                ship->ions -= 1;
                                send_to_char
                                        ("Ships ion cannons reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "rockets"))
                {
                        if (ship->maxrockets < 2)
                        {
                                send_to_char
                                        ("There are not enough rockets to replace with torpedos.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxrockets >= 2)
                        {
                                ship->maxrockets -= 2;
                                send_to_char("Ships rockets reduced by 2\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "lasers"))
                {
                        if (ship->lasers < 1)
                        {
                                send_to_char
                                        ("There are not any lasers to replace with torpedos.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->lasers >= 1)
                        {
                                ship->lasers -= 1;
                                send_to_char("Ships lasers reduced by 1\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "missiles"))
                {
                        if (ship->maxmissiles < 6)
                        {
                                send_to_char
                                        ("There are not enough missiles to replace with torpedos\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxmissiles >= 6)
                        {
                                ship->maxmissiles -= 6;
                                send_to_char
                                        ("Ships missiles reduced by 6\r\n",
                                         ch);
                        }
                }

                ship->maxtorpedos += 4;
                send_to_char("Ships torpedos increased by 4.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "rockets"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->maxrockets >= 2)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of rockets.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->rockets >= 6)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of rockets.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->rockets >= 18)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of rockets.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "ions")) && (str_cmp(arg2, "lasers"))
                    && (str_cmp(arg2, "torpedos"))
                    && (str_cmp(arg2, "missiles")) && (str_cmp(arg2, "chaff"))
                    && (str_cmp(arg2, "tractor")))
                {
                        send_to_char
                                ("You may not substitute that for rockets\r\n",
                                 ch);
                        send_to_char
                                ("Options are: ions, lasers, torpedos, missiles, chaff, tractor\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "chaff"))
                {
                        if (ship->maxchaff < 1)
                        {
                                send_to_char
                                        ("There are not any chaff pods to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxchaff >= 1)
                        {
                                ship->maxchaff -= 1;
                                send_to_char
                                        ("Ships chaff pods reduced by 1\r\n",
                                         ch);
                        }
                }
                if (!str_cmp(arg2, "tractor"))
                {
                        if (ship->tractorbeam <= 0)
                        {
                                send_to_char
                                        ("There are not any tractor beams to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->tractorbeam >= 1)
                        {
                                ship->tractorbeam -= 1;
                                send_to_char
                                        ("Ships tractor beams reduced by 1\r\n",
                                         ch);
                        }
                }


                if (!str_cmp(arg2, "ions"))
                {
                        if (ship->ions <= 0)
                        {
                                send_to_char
                                        ("There are not any ion cannons to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->ions >= 1)
                        {
                                ship->ions -= 1;
                                send_to_char
                                        ("Ships ion cannons reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "lasers"))
                {
                        if (ship->lasers < 1)
                        {
                                send_to_char
                                        ("There are not enough lasers to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->lasers >= 1)
                        {
                                ship->lasers -= 1;
                                send_to_char("Ships lasers reduced by 1\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "torpedos"))
                {
                        if (ship->maxtorpedos < 4)
                        {
                                send_to_char
                                        ("There are not enough torpedos to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxtorpedos >= 4)
                        {
                                ship->maxtorpedos -= 4;
                                send_to_char
                                        ("Ships torpedos reduced by 4\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "missiles"))
                {
                        if (ship->maxmissiles < 6)
                        {
                                send_to_char
                                        ("There are not enough missiles to replace with rockets\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxmissiles >= 6)
                        {
                                ship->maxmissiles -= 6;
                                send_to_char("Ships missiles by 6\r\n", ch);
                        }
                }

                ship->maxrockets += 2;
                send_to_char("Ships rockets increased by 2.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "chaff"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->maxchaff >= 5)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of chaff pods.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->maxchaff >= 20)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of chaff pods.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->lasers >= 50)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of chaff pods.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "ions")) && (str_cmp(arg2, "lasers"))
                    && (str_cmp(arg2, "torpedos"))
                    && (str_cmp(arg2, "missiles"))
                    && (str_cmp(arg2, "rockets"))
                    && (str_cmp(arg2, "tractor")))
                {
                        send_to_char
                                ("You may not substitute that for chaff pods\r\n",
                                 ch);
                        send_to_char
                                ("Options are: ions, lasers, rockets, torpedos, missiles, tractor\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "rockets"))
                {
                        if (ship->maxrockets < 2)
                        {
                                send_to_char
                                        ("There are not enough rockets to replace with chaff pods..\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxrockets >= 2)
                        {
                                ship->maxrockets -= 2;
                                send_to_char("Ships rockets reduced by 2\r\n",
                                             ch);
                        }
                }
                if (!str_cmp(arg2, "tractor"))
                {
                        if (ship->tractorbeam <= 0)
                        {
                                send_to_char
                                        ("There are not any tractor beams to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->tractorbeam >= 1)
                        {
                                ship->tractorbeam -= 1;
                                send_to_char
                                        ("Ships tractor beams reduced by 1\r\n",
                                         ch);
                        }
                }


                if (!str_cmp(arg2, "ions"))
                {
                        if (ship->ions <= 0)
                        {
                                send_to_char
                                        ("There are not any ion cannons to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->ions >= 1)
                        {
                                ship->ions -= 1;
                                send_to_char
                                        ("Ships ion cannons reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "lasers"))
                {
                        if (ship->lasers < 1)
                        {
                                send_to_char
                                        ("There are not enough lasers to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->lasers >= 1)
                        {
                                ship->lasers -= 1;
                                send_to_char("Ships lasers reduced by 1\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "torpedos"))
                {
                        if (ship->maxtorpedos < 4)
                        {
                                send_to_char
                                        ("There are not enough torpedos to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxtorpedos >= 4)
                        {
                                ship->maxtorpedos -= 4;
                                send_to_char
                                        ("Ships torpedos reduced by 4\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "missiles"))
                {
                        if (ship->maxmissiles < 6)
                        {
                                send_to_char
                                        ("There are not enough missiles to replace with rockets\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxmissiles >= 6)
                        {
                                ship->maxmissiles -= 6;
                                send_to_char
                                        ("Ships missiles reduced by 6\r\n",
                                         ch);
                        }
                }

                ship->maxchaff += 1;
                send_to_char("Ships chaff increased by 1.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "tractor"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        send_to_char
                                ("Fighter ships cannot use tractor beams.\r\n",
                                 ch);
                        return;
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->tractorbeam >= 1)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number tractor beams.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->tractorbeam >= 5)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of tractor beams.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "ions")) && (str_cmp(arg2, "lasers"))
                    && (str_cmp(arg2, "torpedos"))
                    && (str_cmp(arg2, "missiles")) && (str_cmp(arg2, "chaff"))
                    && (str_cmp(arg2, "rockets")))
                {
                        send_to_char
                                ("You may not substitute that for tractor beams\r\n",
                                 ch);
                        send_to_char
                                ("Options are: ions, lasers, rockets, torpedos, missiles, chaff\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "chaff"))
                {
                        if (ship->maxchaff < 1)
                        {
                                send_to_char
                                        ("There are not any chaff pods to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxchaff >= 1)
                        {
                                ship->maxchaff -= 1;
                                send_to_char
                                        ("Ships chaff pods reduced by 1\r\n",
                                         ch);
                        }
                }
                if (!str_cmp(arg2, "rockets"))
                {
                        if (ship->maxrockets < 2)
                        {
                                send_to_char
                                        ("There are not enough rockets to replace with tractor beams..\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxrockets >= 2)
                        {
                                ship->maxrockets -= 2;
                                send_to_char
                                        ("Ships rockets beams reduced by 2\r\n",
                                         ch);
                        }
                }


                if (!str_cmp(arg2, "ions"))
                {
                        if (ship->ions <= 0)
                        {
                                send_to_char
                                        ("There are not any ion cannons to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->ions >= 1)
                        {
                                ship->ions -= 1;
                                send_to_char
                                        ("Ships ion cannons reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "lasers"))
                {
                        if (ship->lasers < 1)
                        {
                                send_to_char
                                        ("There are not enough lasers to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->lasers >= 1)
                        {
                                ship->lasers -= 1;
                                send_to_char("Ships lasers reduced by 1\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "torpedos"))
                {
                        if (ship->maxtorpedos < 4)
                        {
                                send_to_char
                                        ("There are not enough torpedos to replace with rockets.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxtorpedos >= 4)
                        {
                                ship->maxtorpedos -= 4;
                                send_to_char
                                        ("Ships torpedos reduced by 4\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "missiles"))
                {
                        if (ship->maxmissiles < 6)
                        {
                                send_to_char
                                        ("There are not enough missiles to replace with rockets\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxmissiles >= 6)
                        {
                                ship->maxmissiles -= 6;
                                send_to_char("Ships missiles by 6\r\n", ch);
                        }
                }

                ship->tractorbeam += 1;
                send_to_char("Ships tractpr beams increased by 1.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }


        if (!str_cmp(arg1, "hull"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->maxhull >= 1000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum armour.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->maxhull >= 3000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum armour.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->maxhull >= 10000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum armour.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "shields")) && (str_cmp(arg2, "energy")))
                {
                        send_to_char
                                ("You may not substitute that for hull armour\r\n",
                                 ch);
                        send_to_char("Options are: shields, energy\r\n", ch);
                        return;
                }

                if (!str_cmp(arg2, "shields"))
                {
                        if (ship->maxshield < 11)
                        {
                                send_to_char
                                        ("There are not enough shield generators to replace with hull armour.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield >= 11)
                        {
                                ship->maxshield -= 10;
                                send_to_char
                                        ("Ships shields reduced by 10\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "energy"))
                {
                        if (ship->maxenergy < 101)
                        {
                                send_to_char
                                        ("There are not enough fuel cells to replace with hull armour.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxenergy > 101)
                        {
                                ship->maxenergy -= 100;
                                send_to_char
                                        ("Ships fuel cells reduced by 100\r\n",
                                         ch);
                        }
                }

                ship->maxhull += 100;
                send_to_char("Ships hull armour increased by 100.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "shields"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->maxshield >= 300)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of shield generators.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->maxshield >= 900)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of shield generators.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->maxshield >= 3000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of shield generators.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "hull")) && (str_cmp(arg2, "energy")))
                {
                        send_to_char
                                ("You may not substitute that for shield generators\r\n",
                                 ch);
                        send_to_char("Options are: hull, energy\r\n", ch);
                        return;
                }

                if (!str_cmp(arg2, "hull"))
                {
                        if (ship->maxhull < 101)
                        {
                                send_to_char
                                        ("There is not enough hull armor to replace with shield generators.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxhull >= 101)
                        {
                                ship->maxhull -= 100;
                                send_to_char("Ships hull reduced by 100\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "energy"))
                {
                        if (ship->maxenergy < 101)
                        {
                                send_to_char
                                        ("There are not enough fuel cells to replace with shield generators.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxenergy > 101)
                        {
                                ship->maxenergy -= 100;
                                send_to_char
                                        ("Ships fuel cells reduced by 100\r\n",
                                         ch);
                        }
                }

                ship->maxshield += 10;
                send_to_char("Ships shield generators increased by 10.\r\n",
                             ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                ch->gold -= 30000;
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "energy"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 10000)
                        {
                                send_to_char
                                        ("You need 10000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 50000)
                        {
                                send_to_char
                                        ("You need 50000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->maxenergy >= 5000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number fuel cells.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->maxenergy >= 15000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of fuel cells.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->maxenergy >= 36000)
                        {
                                send_to_char
                                        ("This ship has reached its maximum number of fuel cells.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "hull")) && (str_cmp(arg2, "shields")))
                {
                        send_to_char
                                ("You may not substitute that for fuel cells\r\n",
                                 ch);
                        send_to_char("Options are: hull, shields\r\n", ch);
                        return;
                }

                if (!str_cmp(arg2, "hull"))
                {
                        if (ship->maxhull < 101)
                        {
                                send_to_char
                                        ("There is not enough hull armor to replace with fuel cells.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxhull >= 101)
                        {
                                ship->maxhull -= 100;
                                send_to_char("Ships hull reduced by 100\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "shields"))
                {
                        if (ship->maxshield < 11)
                        {
                                send_to_char
                                        ("There are not enough shield generators to replace with fuel cells.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield > 11)
                        {
                                ship->maxshield -= 10;
                                send_to_char
                                        ("Ships shields reduced by 10\r\n",
                                         ch);
                        }
                }

                ship->maxenergy += 100;
                send_to_char("Ships fuel cells increased by 100.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 10000;
                        send_to_char
                                ("You spend 10000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 20000;
                        send_to_char
                                ("You spend 20000 credits on this upgrade.\r\n",
                                 ch);
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 30000;
                        send_to_char
                                ("You spend 30000 credits on this upgrade.\r\n",
                                 ch);
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "stealth"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 500000)
                        {
                                send_to_char
                                        ("You need 500000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 1000000)
                        {
                                send_to_char
                                        ("You need 1000000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->stealth >= 1)
                        {
                                send_to_char
                                        ("This ship already has a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->stealth >= 1)
                        {
                                send_to_char
                                        ("This ship already has a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->stealth >= 1 && str_cmp(arg2, "remove"))
                        {
                                send_to_char
                                        ("This ship already has a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "hull")) && (str_cmp(arg2, "shields"))
                    && (str_cmp(arg2, "energy")) && (str_cmp(arg2, "cargo"))
                    && (str_cmp(arg2, "battalions"))
                    && (str_cmp(arg2, "speed")) && (str_cmp(arg2, "manuever"))
                    && (str_cmp(arg2, "hyperspeed"))
                    && (str_cmp(arg2, "remove")))

                {
                        send_to_char
                                ("You may not substitute that for a stealth drive\r\n",
                                 ch);
                        send_to_char
                                ("Options are: hull, shields, energy, cargo,\r\n",
                                 ch);
                        send_to_char
                                ("             battalions, speed, manuever, hyperspeed.\r\n",
                                 ch);
                        send_to_char
                                ("             &Rremove&w will remove stealth from the ship.\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "remove"))
                {
                        if (ship->stealth <= 0)
                        {
                                send_to_char
                                        ("There is no stealth drive to remove.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->stealth > 0)
                        {
                                ship->stealth -= 1;
                                send_to_char("Stealth drive removed.\r\n",
                                             ch);
                                learn_from_success(ch, gsn_modifyship);
                                return;
                        }
                }

                if (!str_cmp(arg2, "hull"))
                {
                        if (ship->maxhull < 101)
                        {
                                send_to_char
                                        ("There is not enough hull armor to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxhull >= 101)
                        {
                                ship->maxhull -= 100;
                                send_to_char("Ships hull reduced by 100\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "shields"))
                {
                        if (ship->maxshield < 11)
                        {
                                send_to_char
                                        ("There are not enough shield generators to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield > 11)
                        {
                                ship->maxshield -= 10;
                                send_to_char
                                        ("Ships shields reduced by 10\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "energy"))
                {
                        if (ship->maxenergy < 101)
                        {
                                send_to_char
                                        ("There are not enough fuel cells to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxenergy > 101)
                        {
                                ship->maxenergy -= 100;
                                send_to_char
                                        ("Ships energy reduced by 100\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "cargo"))
                {
                        if (ship->maxcargo < 50)
                        {
                                send_to_char
                                        ("There is not enough cargo space to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxcargo > 50)
                        {
                                ship->maxcargo -= 50;
                                send_to_char
                                        ("Ships cargo capacity reduced by 50\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "battalions"))
                {
                        if (ship->maxbattalions < 1)
                        {
                                send_to_char
                                        ("There are not battalion holds to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxbattalions >= 1)
                        {
                                ship->maxbattalions -= 1;
                                send_to_char
                                        ("Ships battalion holds reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "speed"))
                {
                        if (ship->realspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough engines to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->realspeed > 25)
                        {
                                ship->realspeed -= 25;
                                send_to_char("Ships speed reduced by 25\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "manuever"))
                {
                        if (ship->manuever < 26)
                        {
                                send_to_char
                                        ("There are not enough manuevering thrusters to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield > 25)
                        {
                                ship->manuever -= 25;
                                send_to_char
                                        ("Ships manueverability reduced by 25\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "hyperspeed"))
                {
                        if (ship->hyperspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough hyper drives to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->hyperspeed > 25)
                        {
                                ship->hyperspeed -= 25;
                                send_to_char
                                        ("Ships hyperspeed reduced by 25\r\n",
                                         ch);
                        }
                }

                ship->stealth += 1;
                send_to_char("Stealth drive installed.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 100000;
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 500000;
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 1000000;
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "cloak"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 500000)
                        {
                                send_to_char
                                        ("You need 500000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 1000000)
                        {
                                send_to_char
                                        ("You need 1000000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ship->cloak >= 1)
                        {
                                send_to_char
                                        ("This ship already has a cloaking device.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ship->cloak >= 1)
                        {
                                send_to_char
                                        ("This ship already has a cloaking device.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->cloak >= 1 && str_cmp(arg2, "remove"))
                        {
                                send_to_char
                                        ("This ship already has a cloaking device.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "hull")) && (str_cmp(arg2, "shields"))
                    && (str_cmp(arg2, "energy")) && (str_cmp(arg2, "cargo"))
                    && (str_cmp(arg2, "battalions"))
                    && (str_cmp(arg2, "speed")) && (str_cmp(arg2, "manuever"))
                    && (str_cmp(arg2, "hyperspeed"))
                    && (str_cmp(arg2, "remove")))

                {
                        send_to_char
                                ("You may not substitute that for a stealth drive\r\n",
                                 ch);
                        send_to_char
                                ("Options are: hull, shields, energy, cargo,\r\n",
                                 ch);
                        send_to_char
                                ("             battalions, speed, manuever, hyperspeed.\r\n",
                                 ch);
                        send_to_char
                                ("             &Rremove&w will remove cloak from the ship.\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "remove"))
                {
                        if (ship->cloak <= 0)
                        {
                                send_to_char
                                        ("There is no cloaking device to remove.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->cloak > 0)
                        {
                                ship->cloak -= 1;
                                send_to_char("Cloaking device removed.\r\n",
                                             ch);
                                learn_from_success(ch, gsn_modifyship);
                                return;
                        }
                }

                if (!str_cmp(arg2, "hull"))
                {
                        if (ship->maxhull < 101)
                        {
                                send_to_char
                                        ("There is not enough hull armor to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxhull >= 101)
                        {
                                ship->maxhull -= 100;
                                send_to_char("Ships hull reduced by 100\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "shields"))
                {
                        if (ship->maxshield < 11)
                        {
                                send_to_char
                                        ("There are not enough shield generators to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield > 11)
                        {
                                ship->maxshield -= 10;
                                send_to_char
                                        ("Ships shields reduced by 10\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "energy"))
                {
                        if (ship->maxenergy < 101)
                        {
                                send_to_char
                                        ("There are not enough fuel cells to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxenergy > 101)
                        {
                                ship->maxenergy -= 100;
                                send_to_char
                                        ("Ships energy reduced by 100\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "cargo"))
                {
                        if (ship->maxcargo < 50)
                        {
                                send_to_char
                                        ("There is not enough cargo space to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxcargo > 50)
                        {
                                ship->maxcargo -= 50;
                                send_to_char
                                        ("Ships cargo capacity reduced by 50\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "battalions"))
                {
                        if (ship->maxbattalions < 1)
                        {
                                send_to_char
                                        ("There are not battalion holds to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxbattalions >= 1)
                        {
                                ship->maxbattalions -= 1;
                                send_to_char
                                        ("Ships battalion holds reduced by 1\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "speed"))
                {
                        if (ship->realspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough engines to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->realspeed > 25)
                        {
                                ship->realspeed -= 25;
                                send_to_char("Ships speed reduced by 25\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "manuever"))
                {
                        if (ship->manuever < 26)
                        {
                                send_to_char
                                        ("There are not enough manuevering thrusters to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield > 25)
                        {
                                ship->manuever -= 25;
                                send_to_char
                                        ("Ships manueverability reduced by 25\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "hyperspeed"))
                {
                        if (ship->hyperspeed < 26)
                        {
                                send_to_char
                                        ("There are not enough hyper drives to replace with a stealth drive.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->hyperspeed > 25)
                        {
                                ship->hyperspeed -= 25;
                                send_to_char
                                        ("Ships hyperspeed reduced by 25\r\n",
                                         ch);
                        }
                }

                ship->cloak += 1;
                send_to_char("Cloaking device installed.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 100000;
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 500000;
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 1000000;
                }
                learn_from_success(ch, gsn_modifyship);
        }

        if (!str_cmp(arg1, "interdictor"))
        {

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        if (ch->gold < 100000)
                        {
                                send_to_char
                                        ("You need 100000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        if (ch->gold < 500000)
                        {
                                send_to_char
                                        ("You need 500000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ch->gold < 1000000)
                        {
                                send_to_char
                                        ("You need 1000000 credits to modify this ship.\r\n",
                                         ch);
                                return;
                        }
                }

                if (ship->ship_class == FIGHTER_SHIP)
                {
                        send_to_char
                                ("Starfighters cannot house gravity cones.\r\n",
                                 ch);
                        return;
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        send_to_char
                                ("Midships cannot house gravity cones.\r\n",
                                 ch);
                        return;
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        if (ship->interdictor >= 1 && str_cmp(arg2, "remove"))
                        {
                                send_to_char
                                        ("This ship already has a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                }

                if ((str_cmp(arg2, "hull")) && (str_cmp(arg2, "shields"))
                    && (str_cmp(arg2, "energy")) && (str_cmp(arg2, "cargo"))
                    && (str_cmp(arg2, "battalions"))
                    && (str_cmp(arg2, "speed")) && (str_cmp(arg2, "manuever"))
                    && (str_cmp(arg2, "hyperspeed"))
                    && (str_cmp(arg2, "remove")))
                {
                        send_to_char
                                ("You may not substitute that for a stealth drive\r\n",
                                 ch);
                        send_to_char
                                ("Options are: hull, shields, energy, cargo,\r\n",
                                 ch);
                        send_to_char
                                ("             battalions, speed, manuever, hyperspeed.\r\n",
                                 ch);
                        send_to_char
                                ("             &Rremove&w will remove the gravity cone from the ship.\r\n",
                                 ch);
                        return;
                }

                if (!str_cmp(arg2, "remove"))
                {
                        if (ship->interdictor <= 0)
                        {
                                send_to_char
                                        ("There is no gravity cone to remove.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->interdictor > 0)
                        {
                                ship->interdictor -= 1;
                                send_to_char("Gravity cone removed.\r\n", ch);
                                learn_from_success(ch, gsn_modifyship);
                                return;
                        }
                }

                if (!str_cmp(arg2, "hull"))
                {
                        if (ship->maxhull < 501)
                        {
                                send_to_char
                                        ("There is not enough hull armor to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxhull >= 501)
                        {
                                ship->maxhull -= 500;
                                send_to_char("Ships hull reduced by 500\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "shields"))
                {
                        if (ship->maxshield < 51)
                        {
                                send_to_char
                                        ("There are not enough shield generators to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield > 51)
                        {
                                ship->maxshield -= 50;
                                send_to_char
                                        ("Ships shields reduced by 50\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "energy"))
                {
                        if (ship->maxenergy < 501)
                        {
                                send_to_char
                                        ("There are not enough fuel cells to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxenergy > 501)
                        {
                                ship->maxenergy -= 500;
                                send_to_char
                                        ("Ships energy reduced by 500\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "cargo"))
                {
                        if (ship->maxcargo < 500)
                        {
                                send_to_char
                                        ("There is not enough cargo space to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxcargo > 500)
                        {
                                ship->maxcargo -= 500;
                                send_to_char
                                        ("Ships cargo capacity reduced by 500\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "battalions"))
                {
                        if (ship->maxbattalions < 10)
                        {
                                send_to_char
                                        ("There are not enough battalion holds to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxbattalions > 10)
                        {
                                ship->maxbattalions -= 10;
                                send_to_char
                                        ("Ships battalion holds reduced by 10\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "speed"))
                {
                        if (ship->realspeed < 56)
                        {
                                send_to_char
                                        ("There are not enough engines to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->realspeed > 55)
                        {
                                ship->realspeed -= 55;
                                send_to_char("Ships speed reduced by 55\r\n",
                                             ch);
                        }
                }

                if (!str_cmp(arg2, "manuever"))
                {
                        if (ship->manuever < 56)
                        {
                                send_to_char
                                        ("There are not enough manuevering thrusters to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->maxshield > 55)
                        {
                                ship->manuever -= 55;
                                send_to_char
                                        ("Ships manueverability reduced by 55\r\n",
                                         ch);
                        }
                }

                if (!str_cmp(arg2, "hyperspeed"))
                {
                        if (ship->hyperspeed < 56)
                        {
                                send_to_char
                                        ("There are not enough hyper drives to replace with a gravity cone.\r\n",
                                         ch);
                                return;
                        }
                        if (ship->hyperspeed > 55)
                        {
                                ship->hyperspeed -= 55;
                                send_to_char
                                        ("Ships hyperspeed reduced by 55\r\n",
                                         ch);
                        }
                }

                ship->interdictor += 1;
                send_to_char("Gravity cone installed.\r\n", ch);
                if (ship->ship_class == FIGHTER_SHIP)
                {
                        ch->gold -= 100000;
                }
                if (ship->ship_class == MIDSIZE_SHIP)
                {
                        ch->gold -= 500000;
                }
                if (ship->ship_class == CAPITAL_SHIP)
                {
                        ch->gold -= 1000000;
                }
                learn_from_success(ch, gsn_modifyship);
        }

        save_ship(ship);

}


CMDF do_modifyexit(CHAR_DATA * ch, char *argument)
{
        EXIT_DATA *xit;
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        int       percentage, type = 0;
        bool      checktool, checkdura;
        OBJ_DATA *obj;

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);
        switch (ch->substate)
        {
        default:

                if (arg[0] == '\0')
                {
                        send_to_char
                                ("&RUsage: modifyexit <direction> <type>\n\r&w",
                                 ch);
                        send_to_char("&RAvailable options for type are:\n\r",
                                     ch);
                        send_to_char("\t door, hidden\n\r", ch);
                        return;
                }

                if (!ship_from_room(ch->in_room->vnum)
                    && !installation_from_room(ch->in_room->vnum))
                {
                        send_to_char
                                ("&RThis isn't a good place to try that...\n\r&w",
                                 ch);
                        return;
                }
                if (!str_cmp(arg, "n") || !str_cmp(arg, "north") ||
                    !str_cmp(arg, "s") || !str_cmp(arg, "south") ||
                    !str_cmp(arg, "e") || !str_cmp(arg, "east") ||
                    !str_cmp(arg, "w") || !str_cmp(arg, "west") ||
                    !str_cmp(arg, "sw") || !str_cmp(arg, "southwest") ||
                    !str_cmp(arg, "nw") || !str_cmp(arg, "northwest") ||
                    !str_cmp(arg, "se") || !str_cmp(arg, "southeast") ||
                    !str_cmp(arg, "ne") || !str_cmp(arg, "northeast") ||
                    !str_cmp(arg, "u") || !str_cmp(arg, "up") ||
                    !str_cmp(arg, "d") || !str_cmp(arg, "down"))
                {
                        if (!get_exit(ch->in_room, get_dir(arg)))
                        {
                                send_to_char
                                        ("There is no an exit in that direction\r\n",
                                         ch);
                                return;
                        }
                }

                else
                {
                        send_to_char("Not a valid direction\r\n", ch);
                        return;
                }

                if (!str_cmp(arg2, "door"))
                        type = EX_ISDOOR;
                else if (!str_cmp(arg2, "hidden"))
                        type = EX_HIDDEN;
                else
                {
                        send_to_char
                                ("&RUsage: modifyexit <direction> <type>\n\r&w",
                                 ch);
                        send_to_char("&RAvailable options for type are:\n\r",
                                     ch);
                        send_to_char("\t door, hidden\n\r", ch);
                        return;
                }



                checktool = FALSE;
                checkdura = FALSE;

                for (obj = ch->last_carrying; obj; obj = obj->prev_content)
                {
                        if (obj->item_type == ITEM_TOOLKIT)
                                checktool = TRUE;
                        if (obj->item_type == ITEM_DURASTEEL)
                                checkdura = TRUE;
                }

                if (!checktool)
                {
                        send_to_char
                                ("&RYou need toolkit to make a new door and lock.\n\r",
                                 ch);
                        return;
                }

                if (!checkdura)
                {
                        send_to_char
                                ("&RYou need something to make it out of.\n\r",
                                 ch);
                        return;
                }


                percentage = IS_NPC(ch) ? ch->top_level
                        : (int) (ch->pcdata->learned[gsn_modifyexit]);
                if (number_percent() < percentage)
                {
                        send_to_char
                                ("&GYou take your equipment and begin to modify your exit.\n\r",
                                 ch);
                        act(AT_PLAIN,
                            "$n takes $s tools and begins to work on something.",
                            ch, NULL, argument, TO_ROOM);
                        add_timer(ch, TIMER_DO_FUN, 25, do_modifyexit, 1);
                        ch->dest_buf = str_dup(arg);
                        ch->dest_buf_2 = str_dup(arg2);
                        return;
                }
                send_to_char
                        ("&RYou can't figure out how to fit the parts together.\n\r",
                         ch);
                learn_from_failure(ch, gsn_modifyexit);
                return;

        case 1:
                if (!ch->dest_buf)
                        return;
                if (!ch->dest_buf_2)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                mudstrlcpy(arg2, (char *) ch->dest_buf_2, MIL);
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RSuddenly stop your actions, and drop your toolkit onto the floor.\n\r",
                         ch);
                return;
        }

        ch->substate = SUB_NONE;
        if (!str_cmp(arg2, "door"))
                type = EX_ISDOOR;
        else if (!str_cmp(arg2, "hidden"))
                type = EX_HIDDEN;
        checktool = FALSE;
        checkdura = FALSE;

        for (obj = ch->last_carrying; obj; obj = obj->prev_content)
        {
                if (obj->item_type == ITEM_TOOLKIT)
                        checktool = TRUE;
                if (obj->item_type == ITEM_DURASTEEL && checkdura == FALSE)
                {
                        checkdura = TRUE;
                        separate_obj(obj);
                        obj_from_char(obj);
                        extract_obj(obj);
                }
        }

        percentage = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_modifyexit]);
        if (number_percent() > percentage * 2 || (!checktool) || (!checkdura))
        {
                send_to_char
                        ("&RYou finish your  construction and stand back to take a look.\n\r",
                         ch);
                send_to_char
                        ("&RAs you look at it, it falls off the frame and almost hits you.\n\r",
                         ch);
                learn_from_failure(ch, gsn_modifyship);
                return;
        }

        if (!str_cmp(arg, "n") || !str_cmp(arg, "north") ||
            !str_cmp(arg, "s") || !str_cmp(arg, "south") ||
            !str_cmp(arg, "e") || !str_cmp(arg, "east") ||
            !str_cmp(arg, "w") || !str_cmp(arg, "west") ||
            !str_cmp(arg, "sw") || !str_cmp(arg, "southwest") ||
            !str_cmp(arg, "nw") || !str_cmp(arg, "northwest") ||
            !str_cmp(arg, "se") || !str_cmp(arg, "southeast") ||
            !str_cmp(arg, "ne") || !str_cmp(arg, "northeast") ||
            !str_cmp(arg, "u") || !str_cmp(arg, "up") ||
            !str_cmp(arg, "d") || !str_cmp(arg, "down"))
        {
                if ((xit = find_door(ch, arg, TRUE)) == NULL)
                {
                        send_to_char
                                ("There is already an exit in that direction, but this should not have happened. Talk to Greven.\r\n",
                                 ch);
                        return;
                }
                else
                        set_bexit_flag(xit, type);
        }

        else
        {
                send_to_char
                        ("Not a valid direction, but this should not have happened. Talk to Greven.\r\n",
                         ch);
                return;
        }

        send_to_char
                ("&GYou finish your work and examine the modifications you've performed.&w\n\r",
                 ch);
        act(AT_PLAIN, "$n finishes their work.", ch, NULL, argument, TO_ROOM);

        {
                long      xpgain;

                xpgain = UMIN(20000,
                              (exp_level
                               (ch->skill_level[ENGINEERING_ABILITY] + 1) -
                               exp_level(ch->
                                         skill_level[ENGINEERING_ABILITY])));
                gain_exp(ch, xpgain, ENGINEERING_ABILITY);
                ch_printf(ch, "You gain %d engineering experience.", xpgain);
        }

        learn_from_success(ch, gsn_modifyexit);

        if (ship_from_room(ch->in_room->vnum) != NULL)
                fold_area(ch->in_room->area, ch->in_room->area->filename,
                          FALSE, FALSE);

        if (installation_from_room(ch->in_room->vnum) != NULL)
                fold_area(ch->in_room->area, ch->in_room->area->filename,
                          FALSE, FALSE);
}

CMDF do_addmprogstring(CHAR_DATA * ch, char *argument)
{
        MPROG_DATA *mprog, *mprg;
        int       mptype, len;
        char      buf[MSL];

        argument = NULL;

        mprog = ch->in_room->mudprogs;
        mptype = get_mpflag("rand");
        if (mptype == -1)
        {
                send_to_char("Unknown program type.\n\r", ch);
                return;
        }
        if (mprog)
                for (; mprog->next; mprog = mprog->next);
        CREATE(mprg, MPROG_DATA, 1);
        if (mprog)
                mprog->next = mprg;
        else
                ch->in_room->mudprogs = mprg;
        ch->in_room->progtypes |= (1 << mptype);
        mprg->type = 1 << mptype;
        mprg->arglist = STRALLOC("100");
        if (!mprg->comlist)
                mprg->comlist = STRALLOC("");
        if (mprg->comlist)
                STRFREE(mprg->comlist);
        snprintf(buf, MSL, "%s", "mpecho This is a test done by the code");
        smush_tilde(buf);
        len = strlen(buf);
        if (buf && buf[len - 1] == '~')
                buf[len - 1] = '\0';
        else
                mudstrlcat(buf, "\n\r", MSL);
        mprg->comlist = STRALLOC(buf);
        send_to_char(buf, ch);
        return;
}

bool dismantle_values(SHIP_DATA * ship, PLANET_DATA * planet,int * durasteel, int * transparisteel, int * cost, int * fee)
{
        *durasteel = 0;
        *transparisteel = 0;
        *cost = 0;
        *fee = 0;

        if (!ship || !planet)
                return FALSE;

        /*
         * these values come from  cargo v2 
         */
        *durasteel = ship->ship_class * 150 + 100;
        *transparisteel = ship->ship_class * 20 + 10;

        /*
         * This stuff needs to be tweaked for balancing, i  really don't understand the system though. - Gavin 
         */
        *cost = 10;
        if (planet->cargoexport[CARGO_TRANSPARISTEEL] > 0)
                *cost += planet->cargoexport[CARGO_TRANSPARISTEEL] +
                        planet->cargoexport[CARGO_TRANSPARISTEEL] / 2;
        else if (planet->cargoimport[CARGO_TRANSPARISTEEL] > 0)
                *cost += planet->cargoimport[CARGO_TRANSPARISTEEL];
        else
                *cost += 10;

        if (planet->cargoexport[CARGO_DURASTEEL] > 0)
                *cost += planet->cargoexport[CARGO_DURASTEEL] +
                        planet->cargoexport[CARGO_DURASTEEL] / 2;
        else if (planet->cargoimport[CARGO_DURASTEEL] > 0)
                *cost += planet->cargoimport[CARGO_DURASTEEL];
        else
                *cost += 10;

        *cost *= *durasteel + *transparisteel;
        *fee = *cost * ((ship->ship_class * 5) / 100);

/*         *fee = get_ship_value(ship) / 2; */
        return TRUE;
}

CMDF do_dismantle_ship(CHAR_DATA * ch, char *argument)
{
        int       percentage;
        bool      checktool;
        int       durasteel = 0;
        int       transparisteel = 0;
        int       cost = 0;
        int       fee = 0;
        ROOM_INDEX_DATA *room;
        OBJ_DATA *obj;
        SHIP_DATA *ship = NULL;
        PLANET_DATA *planet;

        if (xIS_SET(ch->in_room->room_flags, ROOM_INSTALLATION))
        {
                INSTALLATION_DATA *installation =
                        installation_from_room(ch->in_room->vnum);
                if (installation
                    && installation->type == SHIPYARD_INSTALLATION)
                        planet = installation->planet;
        }
        else
                planet = ch->in_room->area->planet;

        if (!planet)
        {
                /*
                 * Installations might eventually be spacestations 
                 */
                send_to_char
                        ("Better to actually do this on a planet or installation.",
                         ch);
                return;
        }

        if (!xIS_SET(ch->in_room->room_flags, ROOM_IMPORT))
        {
                send_to_char
                        ("You can't build that here!! Try a spaceport\r\n",
                         ch);
                return;
        }

        switch (ch->substate)
        {
        default:
                {
                        if (argument[0] == '\0')
                        {
                                send_to_char
                                        ("&RUsage: &Gdismantle_ship &C<&cname of ship&C>&w\r\n",
                                         ch);
                                return;
                        }
                        ship = ship_in_room(ch->in_room, argument);

                        if (!ship)
                        {
                                send_to_char("Which ship?\n\r", ch);
                                return;
                        }

                        if (!check_pilot(ch, ship))
                        {
                                send_to_char
                                        ("You don't have permission to build on this ship\r\n",
                                         ch);
                                return;
                        }
						dismantle_values(ship, planet,&cost,&durasteel,&transparisteel,&fee);

                        checktool = FALSE;

                        if (ch->gold < cost+fee)
                        {
                                send_to_char
                                        ("&RYou can't afford to hire the workforce for that.\r\n",
                                         ch);
                                return;
                        }

                        for (obj = ch->last_carrying; obj;
                             obj = obj->prev_content)
                        {
                                if (obj->item_type == ITEM_TOOLKIT)
                                        checktool = TRUE;
                        }

                        if (!checktool)
                        {
                                send_to_char
                                        ("Unable to get any work done without a set of tools.\r\n",
                                         ch);
                                return;
                        }
                        percentage =
                                IS_NPC(ch) ? ch->top_level : (int) (ch->
                                                                    pcdata->
                                                                    learned
                                                                    [gsn_dismantle_ship]);
                        if (number_percent() < percentage)
                        {
                                send_to_char
                                        ("&GYou begin the LONG process of dismantling a ship.\n\r",
                                         ch);
                                act(AT_PLAIN,
                                    "$n takes $s tools and starts dismantling a ship.\r\n",
                                    ch, NULL, argument, TO_ROOM);
                                add_timer(ch, TIMER_DO_FUN, 35,
                                          do_dismantle_ship, 1);
                                ch->dest_buf = ship;
                                return;
                        }
                        send_to_char("&RYou can't figure how to start.\n\r",
                                     ch);
                        learn_from_failure(ch, gsn_dismantle_ship);
                        return;
                }

        case 1:
                ch->substate = SUB_NONE;
                if (!ch->dest_buf)
                        return;
                break;

        case SUB_TIMER_DO_ABORT:
                ch->dest_buf = NULL;

                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to finish your work, luckly you didn't damage the ship matirials.\n\r",
                         ch);
                return;
        }
        ship = NULL;

        for (SHIP_DATA * tmp = ch->in_room->first_ship; tmp;
             tmp = tmp->next_in_room)
        {
                if (tmp == (SHIP_DATA *) ch->dest_buf)
                {
                        ship = tmp;
                        break;
                }
        }
        ch->substate = SUB_NONE;
        ch->dest_buf = NULL;


        if (!ship)
        {
                send_to_char
                        ("&RShip must have left. Make sure the ship is stationary and landed next time.\n\r",
                         ch);
                return;
        }

        if (!check_pilot(ch, ship))
        {
                send_to_char
                        ("You don't have permission to build on this ship\r\n",
                         ch);
                return;
        }
        dismantle_values(ship, planet,&cost,&durasteel,&transparisteel,&fee);

        if (ch->gold < cost+fee)
        {
                send_to_char
                        ("&RYou can't afford the materials....Stop that THIEF!!!\r\n",
                         ch);
                return;
        }

        room = get_room_index(ship->firstroom);
        if (room)
        {
                AREA_DATA *area;

                area = room->area;
                /*
                 * Just in case we try to use this after we destroy the ship, that wouldn't be fun 
                 */
                room = NULL;
                fold_area(area, area->filename, TRUE, FALSE);
        }
		
		/* Remove the ships */
		really_destroy_ship(ship);
		ship = NULL;
		write_ship_list();

		/* Remove costs + fees and add stuff back to the planet */
		ch->gold -= cost;
		ch->gold -= fee;
		planet->resource[CARGO_TRANSPARISTEEL] += transparisteel;
		planet->resource[CARGO_DURASTEEL] += durasteel;


        /*
         * I added this fee to balance the cost of building ships with that of selling them so as to 
         * keep engis from making too much money too fast from just building and selling ships
         */
        ch_printf(ch,
                  "The planet's Government has assesed a decommision Licence Fee of %d credits.\r\n",
                  fee);
        ch_printf(ch,
                  "It costs you %d credits for materials, tools, and labour.\r\n",
                  fee);
        gain_exp(ch, 10000, ENGINEERING_ABILITY);
        ch_printf(ch, "&WYou gain 10000 engineering experience.\r\n");
        learn_from_success(ch, gsn_dismantle_ship);
        act(AT_PLAIN, "$n finishes dismantling the ship..", ch, NULL,
            argument, TO_ROOM);
        return;
}
