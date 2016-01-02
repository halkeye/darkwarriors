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
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "mud.h"
#include "msp.h"

char     *grab_word(char *argument, char *arg_first);
void      decorate_room(ROOM_INDEX_DATA * room);
ROOM_INDEX_DATA *generate_exit(ROOM_INDEX_DATA * in_room, EXIT_DATA ** pexit);
void      toggle_bexit_flag(EXIT_DATA * pexit, int flag);
void      remove_bexit_flag(EXIT_DATA * pexit, int flag);

const sh_int movement_loss[SECT_MAX] = {
        1, 2, 2, 3, 4, 6, 4, 1, 6, 10, 6, 5, 7, 4
};

char     *const dir_name[] = {
        "north", "east", "south", "west", "up", "down",
        "northeast", "northwest", "southeast", "southwest", "somewhere"
};

const int trap_door[] = {
        TRAP_N, TRAP_E, TRAP_S, TRAP_W, TRAP_U, TRAP_D,
        TRAP_NE, TRAP_NW, TRAP_SE, TRAP_SW
};


const sh_int rev_dir[] = {
        2, 3, 0, 1, 5, 4, 9, 8, 7, 6, 10
};


ROOM_INDEX_DATA *vroom_hash[64];


/*
 * Local functions.
 */
bool has_key args((CHAR_DATA * ch, int key));


char     *const sect_names[SECT_MAX][2] = {
        {"In a room", "inside"}, {"A City Street", "cities"},
        {"In a field", "fields"}, {"In a forest", "forests"},
        {"hill", "hills"}, {"On a mountain", "mountains"},
        {"In the water", "waters"}, {"In rough water", "waters"},
        {"Underwater", "underwaters"}, {"In the air", "air"},
        {"In a desert", "deserts"}, {"Somewhere", "unknown"},
        {"ocean floor", "ocean floor"}, {"underground", "underground"},
        {"On a Starship", "starship"}
};


const int sent_total[SECT_MAX] = {
        4, 24, 4, 4, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1
};

char     *const room_sents[SECT_MAX][25] = {
        {
         "The room walls are made of durasteel and duraplast.",
         "You can smell the fumes of ships and vehicles wafting in from outside.",
         "You notice signs of a recent battle from the bloodstains on the floor.",
         "The ceiling is set from a mold of duraplast.",
         "A few people can be heard near by.",
         "The edge of the room is a little cooler than the other side."},
        {
         "You notice the occasional stray looking for food.",
         "Tall buildings loom on either side of you stretching to the sky.",
         "Some street people are putting on an interesting display of talent trying to earn some credits.",
         "Two people nearby shout heated words of argument at one another.",
         "You think you can make out several shady figures talking down a dark alleyway.",
         "A slight breeze blows through the tall buildings.",
         "A small crowd of people have gathered at one side of the street.",
         "Clouds far above you obscure the tops of the highest skyscrapers.",
         "A speeder moves slowly through the street avoiding pedestrians.",
         "A cloudcar flys by overhead.",
         "The air is thick and hard to breath.",
         "The many smells of the city assault your senses.",
         "You hear a scream far of in the distance.",
         "The buildings around you seem endless in number.",
         "The city stretches seemingly endless in all directions.",
         "The street is wide and long.",
         "A swoop rider passes quickly by weaving in and out of pedestrians and other vehicles.",
         "The surface of the road is worn from many travellers.",
         "You feel it would be very easy to get lost in such an enormous city.",
         "You can see other streets above and bellow this one running in many directions.",
         "There are entrances to several buildings at this level.",
         "Along the edge of the street railings prevent pedestrians from falling to their death.",
         "In between the many towers you can see down into depths of the lower city.",
         "A grate in the street prevents rainwater from building up.",
         "You can see you reflection in several of the transparisteel windows as you pass by."
         "You hear a scream far of in the distance."},

        {
         "You notice sparce patches of brush and shrubs.",
         "There is a small cluster of trees far off in the distance.",
         "Around you are grassy fields as far as the eye can see.",
         "Throughout the plains a wide variety of weeds and wildflowers are scattered."},

        {
         "Tall, dark evergreens prevent you from seeing very far.",
         "Many huge oak trees that look several hundred years old are here.",
         "You notice a solitary lonely weeping willow.",
         "To your left is a patch of bright white birch trees, slender and tall."},

        {
         "The rolling hills are lightly speckled with violet wildflowers.",
         "Slowly rolling grassy ridges march off into the horizon.",
         "Sparse thickets of small trees dot the landscape ocasionally.",
         "Once in awhile you hear the soft noise of the wind moving over the hills."},

        {
         "The rocky mountain pass offers many hiding places.",
         "Grey-green rocks jut up towards the sky, where sparse cloud-cover caps the towering peaks.",
         "A large canyon stretches away, where at some time in the past a deep, fast river flowed."},

        {
         "The water is smooth as glass."},

        {
         "Rough waves splash about angrily."},

        {
         "A small school of fish swims by."},

        {
         "The land is far far below.",
         "A misty haze of clouds drifts by."},

        {
         "Around you is sand as far as the eye can see.",
         "You think you see an oasis far in the distance."},

        {
         "You notice nothing unusual."},

        {
         "There are many rocks and coral which litter the ocean floor."},

        {
         "You stand in a lengthy tunnel of rock."}
        ,
        {
         "The starship interior is done in basic durasteel."}

};

int wherehome(CHAR_DATA * ch)
{

        if (ch->pcdata && ch->pcdata->recall != 0)
                return ch->pcdata->recall;

        if (ch->plr_home)
                return ch->plr_home->vnum;

        if (get_trust(ch) >= LEVEL_IMMORTAL)
                return ROOM_START_IMMORTAL;
        return ROOM_VNUM_TEMPLE;
}

char     *grab_word(char *argument, char *arg_first)
{
        char      cEnd;
        sh_int    count;

        count = 0;

        while (isspace(*argument))
                argument++;

        cEnd = ' ';
        if (*argument == '\'' || *argument == '"')
                cEnd = *argument++;

        while (*argument != '\0' || ++count >= 255)
        {
                if (*argument == cEnd)
                {
                        argument++;
                        break;
                }
                *arg_first++ = *argument++;
        }
        *arg_first = '\0';

        while (isspace(*argument))
                argument++;

        return argument;
}

char     *smash_newline(const char *str)
{
        static char ret[MAX_STRING_LENGTH];
        char     *retptr;

        retptr = ret;
        for (; *str != '\0'; str++)
        {
                if (*str == '\n' || *str == '\r')
                {
                        *retptr = ' ';
                        retptr++;
                }
                else
                {
                        *retptr = *str;
                        retptr++;
                }
        }
        *retptr = '\0';
        return ret;
}

char     *wordwrap(char *txt, sh_int wrap)
{
        static char buf[MAX_STRING_LENGTH];
        char     *bufp;

        buf[0] = '\0';
        bufp = buf;
        if (txt != NULL)
        {
                char      line[MAX_STRING_LENGTH];
                char      temp[MAX_STRING_LENGTH];
                char     *ptr, *p;
                int       ln, x;

                ++bufp;
                line[0] = '\0';
                ptr = smash_newline(txt);
                while (*ptr)
                {
                        ptr = grab_word(ptr, temp);
                        ln = strlen(line);
                        x = strlen(temp);
                        if ((ln + x + 1) < wrap)
                        {
                                if (line[ln - 1] == '.')
                                        mudstrlcat(line, "  ", MSL);
                                else
                                        mudstrlcat(line, " ", MSL);
                                mudstrlcat(line, temp, MSL);
                                p = strchr(line, '\n');
                                if (!p)
                                        p = strchr(line, '\r');
                                if (p)
                                {
                                        mudstrlcat(buf, line, MSL);
                                        line[0] = '\0';
                                }
                        }
                        else
                        {
                                mudstrlcat(line, "\r\n", MSL);
                                mudstrlcat(buf, line, MSL);
                                mudstrlcpy(line, temp, MSL);
                        }
                }
                if (line[0] != '\0')
                        mudstrlcat(buf, line, MSL);
        }
        return bufp;
}


void decorate_room(ROOM_INDEX_DATA * room)
{
        char      buf[MAX_STRING_LENGTH];
        char      buf2[MAX_STRING_LENGTH];
        int       nRand;
        int       iRand, len;
        int       previous[8];
        int       sector = room->sector_type;

        if (room->name)
                STRFREE(room->name);
        if (room->description)
                STRFREE(room->description);

        room->name = STRALLOC(sect_names[sector][0]);
        buf[0] = '\0';
        nRand = number_range(1, UMIN(8, sent_total[sector]));

        for (iRand = 0; iRand < nRand; iRand++)
                previous[iRand] = -1;

        for (iRand = 0; iRand < nRand; iRand++)
        {
                while (previous[iRand] == -1)
                {
                        int       x, z;

                        x = number_range(0, sent_total[sector] - 1);

                        for (z = 0; z < iRand; z++)
                                if (previous[z] == x)
                                        break;

                        if (z < iRand)
                                continue;

                        previous[iRand] = x;

                        len = strlen(buf);
                        snprintf(buf2, MSL, "%s", room_sents[sector][x]);
                        if (len > 5 && buf[len - 1] == '.')
                        {
                                mudstrlcat(buf, "  ", MSL);
                                buf2[0] = UPPER(buf2[0]);
                        }
                        else if (len == 0)
                                buf2[0] = UPPER(buf2[0]);
                        mudstrlcat(buf, buf2, MSL);
                }
        }
        snprintf(buf2, MSL, "%s\n\r", wordwrap(buf, 78));
        room->description = STRALLOC(buf2);
}


CMDF do_autodescription(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      buf2[MAX_STRING_LENGTH];
        char      arg[MAX_STRING_LENGTH];
        int       nRand;
        int       iRand, len;
        int       previous[8];
        int       sector;
        ROOM_INDEX_DATA *room;

        one_argument(argument, arg);

        if (arg[0] == '\0')
                room = ch->in_room;
        else if ((room = get_room_index(atoi(arg))) == NULL)
        {
                send_to_char("That is not a valid room", ch);
                return;
        }

        sector = room->sector_type;

        if (room->description)
                STRFREE(room->description);


        buf[0] = '\0';
        nRand = number_range(1, UMIN(8, sent_total[sector]));

        for (iRand = 0; iRand < nRand; iRand++)
                previous[iRand] = -1;

        for (iRand = 0; iRand < nRand; iRand++)
        {
                while (previous[iRand] == -1)
                {
                        int       x, z;

                        x = number_range(0, sent_total[sector] - 1);

                        for (z = 0; z < iRand; z++)
                                if (previous[z] == x)
                                        break;

                        if (z < iRand)
                                continue;

                        previous[iRand] = x;

                        len = strlen(buf);
                        snprintf(buf2, MSL, "%s", room_sents[sector][x]);
                        if (len > 5 && buf[len - 1] == '.')
                        {
                                mudstrlcat(buf, "  ", MSL);
                                buf2[0] = UPPER(buf2[0]);
                        }
                        else if (len == 0)
                                buf2[0] = UPPER(buf2[0]);
                        mudstrlcat(buf, buf2, MSL);
                }
        }
        snprintf(buf2, MSL, "%s\n\r", wordwrap(buf, 78));
        room->description = STRALLOC(buf2);
}


/*
 * Remove any unused virtual rooms				-Thoric
 */
void clear_vrooms()
{
        int       hash;
        ROOM_INDEX_DATA *room, *room_next, *prev;

        for (hash = 0; hash < 64; hash++)
        {
                while (vroom_hash[hash]
                       && !vroom_hash[hash]->first_person
                       && !vroom_hash[hash]->first_content)
                {
                        room = vroom_hash[hash];
                        vroom_hash[hash] = room->next;
                        clean_room(room);
                        DISPOSE(room);
                        --top_vroom;
                }
                prev = NULL;
                for (room = vroom_hash[hash]; room; room = room_next)
                {
                        room_next = room->next;
                        if (!room->first_person && !room->first_content)
                        {
                                if (prev)
                                        prev->next = room_next;
                                clean_room(room);
                                DISPOSE(room);
                                --top_vroom;
                        }
                        if (room)
                                prev = room;
                }
        }
}

/*
 * Function to get the equivelant exit of DIR 0-MAXDIR out of linked list.
 * Made to allow old-style diku-merc exit functions to work.	-Thoric
 */
EXIT_DATA *get_exit(ROOM_INDEX_DATA * room, sh_int dir)
{
        EXIT_DATA *xit;

        if (!room)
        {
                bug("Get_exit: NULL room", 0);
                return NULL;
        }

        for (xit = room->first_exit; xit; xit = xit->next)
                if (xit->vdir == dir)
                        return xit;
        return NULL;
}

/*
 * Function to get an exit, leading the the specified room
 */
EXIT_DATA *get_exit_to(ROOM_INDEX_DATA * room, sh_int dir, int vnum)
{
        EXIT_DATA *xit;

        if (!room)
        {
                bug("Get_exit: NULL room", 0);
                return NULL;
        }

        for (xit = room->first_exit; xit; xit = xit->next)
                if (xit->vdir == dir && xit->vnum == vnum)
                        return xit;
        return NULL;
}

/*
 * Function to get the nth exit of a room			-Thoric
 */
EXIT_DATA *get_exit_num(ROOM_INDEX_DATA * room, sh_int count)
{
        EXIT_DATA *xit;
        int       cnt;

        if (!room)
        {
                bug("Get_exit: NULL room", 0);
                return NULL;
        }

        for (cnt = 0, xit = room->first_exit; xit; xit = xit->next)
                if (++cnt == count)
                        return xit;
        return NULL;
}


/*
 * Modify movement due to encumbrance				-Thoric
 */
sh_int encumbrance(CHAR_DATA * ch, sh_int endurance)
{
        int       cur, max;

        max = can_carry_w(ch);
        cur = ch->carry_weight;
        if (cur >= max)
                return endurance * 7;
        else if (cur >= max * 0.95)
                return endurance * 6;
        else if (cur >= max * 0.90)
                return endurance * 5;
        else if (cur >= max * 0.85)
                return endurance * 4;
        else if (cur >= max * 0.80)
                return endurance * 3;
        else if (cur >= max * 0.75)
                return endurance * 2;
        else
                return endurance;
}


/*
 * Check to see if a character can fall down, checks for looping   -Thoric
 */
bool will_fall(CHAR_DATA * ch, int fall)
{
        if (!ch)
                return FALSE;

        if (xIS_SET(ch->in_room->room_flags, ROOM_NOFLOOR)
            && CAN_GO(ch, DIR_DOWN)
            && (!IS_AFFECTED(ch, AFF_FLYING)
                || (ch->mount && !IS_AFFECTED(ch->mount, AFF_FLYING))))
        {
                if (fall > 80)
                {
                        bug("Falling (in a loop?) more than 80 rooms: vnum %d", ch->in_room->vnum);
                        char_from_room(ch);
                        char_to_room(ch, get_room_index(wherehome(ch)));
                        fall = 0;
                        return TRUE;
                }
                set_char_color(AT_FALLING, ch);
                send_to_char("You're falling down...\n\r", ch);
                move_char(ch, get_exit(ch->in_room, DIR_DOWN), ++fall, FALSE);
                return TRUE;
        }
        return FALSE;
}


/*
 * create a 'virtual' room					-Thoric
 */
ROOM_INDEX_DATA *generate_exit(ROOM_INDEX_DATA * in_room, EXIT_DATA ** pexit)
{
        EXIT_DATA *xit, *bxit;
        EXIT_DATA *orig_exit = (EXIT_DATA *) * pexit;
        ROOM_INDEX_DATA *room, *backroom;
        int       brvnum;
        int       serial;
        int       distance = -1;
        int       vdir = orig_exit->vdir;
        sh_int    hash;
        bool      found = FALSE;

        if (in_room->vnum > MAX_VNUMS)  /* room is virtual */
        {
                serial = in_room->vnum;
                if ((serial & MAX_VNUMS) == orig_exit->vnum)
                {
                        brvnum = serial >> 16;
                }
                else
                {
                        brvnum = serial & MAX_VNUMS;
                        distance = orig_exit->distance - 1;
                }
                backroom = get_room_index(brvnum);
        }
        else
        {
                int       r1 = in_room->vnum;
                int       r2 = orig_exit->vnum;

                brvnum = r1;
                backroom = in_room;
                serial = (UMAX(r1, r2) << 16) | UMIN(r1, r2);
                distance = orig_exit->distance - 1;
        }
        hash = serial % 64;

        for (room = vroom_hash[hash]; room; room = room->next)
                if (!found)
                {
                        CREATE(room, ROOM_INDEX_DATA, 1);
                        room->area = in_room->area;
                        room->vnum = serial;
                        room->sector_type = in_room->sector_type;
                        room->room_flags = in_room->room_flags;
                        decorate_room(room);
                        room->next = vroom_hash[hash];
                        vroom_hash[hash] = room;
                        ++top_vroom;
                }
        if (!found || (xit = get_exit(room, vdir)) == NULL)
        {
                xit = make_exit(room, orig_exit->to_room, vdir);
                xit->keyword = STRALLOC("");
                xit->description = STRALLOC("");
                xit->key = -1;
                xit->distance = distance;
        }
        if (!found)
        {
                bxit = make_exit(room, backroom, rev_dir[vdir]);
                bxit->keyword = STRALLOC("");
                bxit->description = STRALLOC("");
                bxit->key = -1;
                {
                        EXIT_DATA *tmp;
                        int       fulldist = 0;

                        if ((tmp = get_exit(backroom, vdir)) != NULL)
                        {
                                fulldist = tmp->distance;
                        }

                        bxit->distance = fulldist - distance;
                }
        }
        /*
         * (EXIT_DATA *) pexit = xit; - FIXED - Gavin - This isn't actually right, should be deferfencing not typecasting 
         */
        *pexit = xit;
        return room;
}

ch_ret move_char(CHAR_DATA * ch, EXIT_DATA * pexit, int fall, bool running)
{
        ROOM_INDEX_DATA *in_room;
        ROOM_INDEX_DATA *to_room;
        ROOM_INDEX_DATA *from_room;
        char      buf[MAX_STRING_LENGTH];
        char     *txt;
        char     *dtxt;
        ch_ret    retcode;
        sh_int    door, distance;
        bool      drunk = FALSE;
        bool      brief = FALSE;

        if (!IS_NPC(ch))
                if (IS_DRUNK(ch, 2) && (ch->position != POS_SHOVE)
                    && (ch->position != POS_DRAG))
                        drunk = TRUE;

        if (drunk && !fall)
        {
                door = number_door();
                pexit = get_exit(ch->in_room, door);
        }

#ifdef DEBUG
        if (pexit)
        {
                snprintf(buf, MSL, "move_char: %s to door %d", ch->name,
                         pexit->vdir);
                log_string(buf);
        }
#endif

        retcode = rSTOP;
        txt = NULL;

        if (IS_NPC(ch) && IS_SET(ch->act, ACT_MOUNTED))
                return retcode;

        in_room = ch->in_room;
        from_room = in_room;
        if (!pexit || (to_room = pexit->to_room) == NULL)
        {
                if (drunk)
                        send_to_char
                                ("You hit a wall in your drunken state.\n\r",
                                 ch);
                else
                        send_to_char("Alas, you cannot go that way.\n\r", ch);
                return rSTOP;
        }

        door = pexit->vdir;
        distance = pexit->distance;

        /*
         * Exit is only a "window", there is no way to travel in that direction
         * unless it's a door with a window in it       -Thoric
         */
        if (IS_SET(pexit->exit_info, EX_WINDOW)
            && !IS_SET(pexit->exit_info, EX_ISDOOR))
        {
                send_to_char("Alas, you cannot go that way.\n\r", ch);
                return rSTOP;
        }

        if (IS_SET(pexit->exit_info, EX_PORTAL) && IS_NPC(ch))
        {
                act(AT_PLAIN, "Mobs can't use portals.", ch, NULL, NULL,
                    TO_CHAR);
                return rSTOP;
        }

        if (IS_SET(pexit->exit_info, EX_NOMOB) && IS_NPC(ch))
        {
                act(AT_PLAIN, "Mobs can't enter there.", ch, NULL, NULL,
                    TO_CHAR);
                return rSTOP;
        }
        if (IS_NPC(ch))
                if (!check_pos(ch, 8) && !check_pos(ch, 9)
                    && !check_pos(ch, 10) && !check_pos(ch, 11))
                        return rSTOP;

        if (IS_SET(pexit->exit_info, EX_CLOSED)
            && (!IS_AFFECTED(ch, AFF_PASS_DOOR)
                || IS_SET(pexit->exit_info, EX_NOPASSDOOR)))
        {
                if (!IS_SET(pexit->exit_info, EX_SECRET)
                    && !IS_SET(pexit->exit_info, EX_DIG))
                {
                        if (drunk)
                        {
                                act(AT_PLAIN,
                                    "$n runs into the $d in $s drunken state.",
                                    ch, NULL, pexit->keyword, TO_ROOM);
                                act(AT_PLAIN,
                                    "You run into the $d in your drunken state.",
                                    ch, NULL, pexit->keyword, TO_CHAR);
                        }
                        else
                                act(AT_PLAIN, "The $d is closed.", ch, NULL,
                                    pexit->keyword, TO_CHAR);
                }
                else
                {
                        if (drunk)
                                send_to_char
                                        ("You hit a wall in your drunken state.\n\r",
                                         ch);
                        else
                                send_to_char
                                        ("Alas, you cannot go that way.\n\r",
                                         ch);
                }

                return rSTOP;
        }

        /*
         * Crazy virtual room idea, created upon demand.        -Thoric
         */
        if (distance > 1)
                if ((to_room = generate_exit(in_room, &pexit)) == NULL)
                        send_to_char("Alas, you cannot go that way.\n\r", ch);

        if (!fall
            && IS_AFFECTED(ch, AFF_CHARM)
            && ch->master && in_room == ch->master->in_room)
        {
                send_to_char("What?  And leave your beloved master?\n\r", ch);
                return rSTOP;
        }

        if (room_is_private(ch, to_room))
        {
                send_to_char("That room is private right now.\n\r", ch);
                return rSTOP;
        }

/*    if ( !IS_IMMORTAL(ch)
    &&  !IS_NPC(ch)
    &&  ch->in_room->area != to_room->area )
    {
	if ( ch->top_level < to_room->area->low_hard_range )
	{
	    set_char_color( AT_TELL, ch );
	    switch( to_room->area->low_hard_range - ch->top_level )
	    {
		case 1:
		  send_to_char( "A voice in your mind says, 'You are nearly ready to go that way...'", ch );
		  break;
		case 2:
		  send_to_char( "A voice in your mind says, 'Soon you shall be ready to travel down this path... soon.'", ch );
		  break;
		case 3:
		  send_to_char( "A voice in your mind says, 'You are not ready to go down that path... yet.'.\n\r", ch);
		  break;
		default:
		  send_to_char( "A voice in your mind says, 'You are not ready to go down that path.'.\n\r", ch);
	    }
	    return rSTOP;
	}
	else
	if ( ch->top_level > to_room->area->hi_hard_range )
	{
	    set_char_color( AT_TELL, ch );
	    send_to_char( "A voice in your mind says, 'There is nothing more for you down that path.'", ch );
	    return rSTOP;
	}          
    } I don't like these - Gavin */

        if (!fall && !IS_NPC(ch))
        {
                int       endurance;

                if (in_room->sector_type == SECT_AIR
                    || to_room->sector_type == SECT_AIR
                    || IS_SET(pexit->exit_info, EX_FLY))
                {
                        if (ch->mount && !IS_AFFECTED(ch->mount, AFF_FLYING))
                        {
                                send_to_char("Your mount can't fly.\n\r", ch);
                                return rSTOP;
                        }
                        if (!ch->mount && !IS_AFFECTED(ch, AFF_FLYING))
                        {
                                send_to_char
                                        ("You'd need to fly to go there.\n\r",
                                         ch);
                                return rSTOP;
                        }
                }

                if (in_room->sector_type == SECT_WATER_NOSWIM
                    || to_room->sector_type == SECT_WATER_NOSWIM)
                {
                        OBJ_DATA *obj;
                        bool      found;

                        found = FALSE;
                        if (ch->mount)
                        {
                                if (IS_AFFECTED(ch->mount, AFF_FLYING)
                                    || IS_AFFECTED(ch->mount, AFF_FLOATING))
                                        found = TRUE;
                        }
                        else if (IS_AFFECTED(ch, AFF_FLYING)
                                 || IS_AFFECTED(ch, AFF_FLOATING))
                                found = TRUE;

                        /*
                         * Look for a boat.
                         */
                        if (!found)
                                for (obj = ch->first_carrying; obj;
                                     obj = obj->next_content)
                                {
                                        if (obj->item_type == ITEM_BOAT)
                                        {
                                                found = TRUE;
                                                if (drunk)
                                                        txt = "paddles unevenly";
                                                else
                                                        txt = "paddles";
                                                break;
                                        }
                                }

                        if (!found)
                        {
                                send_to_char
                                        ("You'd need a boat to go there.\n\r",
                                         ch);
                                return rSTOP;
                        }
                }

                if (IS_SET(pexit->exit_info, EX_CLIMB))
                {
                        bool      found;

                        found = FALSE;
                        if (ch->mount && IS_AFFECTED(ch->mount, AFF_FLYING))
                                found = TRUE;
                        else if (IS_AFFECTED(ch, AFF_FLYING))
                                found = TRUE;

                        if (!found && !ch->mount)
                        {
                                if ((!IS_NPC(ch)
                                     && number_percent() >
                                     ch->pcdata->learned[gsn_climb]) || drunk
                                    || ch->mental_state < -90)
                                {
                                        send_to_char
                                                ("You start to climb... but lose your grip and fall!\n\r",
                                                 ch);
                                        learn_from_failure(ch, gsn_climb);
                                        if (pexit->vdir == DIR_DOWN)
                                        {
                                                retcode =
                                                        move_char(ch, pexit,
                                                                  1, running);
                                                return retcode;
                                        }
                                        if (number_percent() > 50
                                            && !IS_IMMORTAL(ch))
                                        {
                                                if (!IS_NPC(ch)
                                                    /*
                                                     * !IS_SET( ch->pcdata->cyber, CYBER_LEGS ) 
                                                     */
                                                        )
                                                {
                                                        if (!IS_SET
                                                            (ch->bodyparts,
                                                             BODY_L_LEG))
                                                                SET_BIT(ch->
                                                                        bodyparts,
                                                                        BODY_L_LEG);
                                                        else if (!IS_SET
                                                                 (ch->
                                                                  bodyparts,
                                                                  BODY_R_LEG))
                                                                SET_BIT(ch->
                                                                        bodyparts,
                                                                        BODY_R_LEG);
                                                }
                                        }

                                        set_char_color(AT_HURT, ch);
                                        send_to_char
                                                ("OUCH! You hit the ground!\n\r",
                                                 ch);
                                        WAIT_STATE(ch, 20);
                                        retcode =
                                                damage(ch, ch,
                                                       (pexit->vdir ==
                                                        DIR_UP ? 10 : 5),
                                                       TYPE_UNDEFINED);
                                        return retcode;
                                }
                                found = TRUE;
                                learn_from_success(ch, gsn_climb);
                                WAIT_STATE(ch, skill_table[gsn_climb]->beats);
                                txt = "climbs";
                        }

                        if (!found)
                        {
                                send_to_char("You can't climb.\n\r", ch);
                                return rSTOP;
                        }
                }

                if (ch->mount)
                {
                        switch (ch->mount->position)
                        {
                        case POS_DEAD:
                                send_to_char("Your mount is dead!\n\r", ch);
                                return rSTOP;
                                break;

                        case POS_MORTAL:
                        case POS_INCAP:
                                send_to_char
                                        ("Your mount is hurt far too badly to move.\n\r",
                                         ch);
                                return rSTOP;
                                break;

                        case POS_STUNNED:
                                send_to_char
                                        ("Your mount is too stunned to do that.\n\r",
                                         ch);
                                return rSTOP;
                                break;

                        case POS_SLEEPING:
                                send_to_char("Your mount is sleeping.\n\r",
                                             ch);
                                return rSTOP;
                                break;

                        case POS_RESTING:
                                send_to_char("Your mount is resting.\n\r",
                                             ch);
                                return rSTOP;
                                break;

                        case POS_SITTING:
                                send_to_char
                                        ("Your mount is sitting down.\n\r",
                                         ch);
                                return rSTOP;
                                break;

                        default:
                                break;
                        }

                        if (!IS_AFFECTED(ch->mount, AFF_FLYING)
                            && !IS_AFFECTED(ch->mount, AFF_FLOATING))
                                endurance =
                                        movement_loss[UMIN
                                                      (SECT_MAX - 1,
                                                       in_room->
                                                       sector_type)] * 3;
                        else
                                endurance = 3;
                        if (ch->mount->endurance < endurance)
                        {
                                send_to_char
                                        ("Your mount is too exhausted.\n\r",
                                         ch);
                                return rSTOP;
                        }
                }
                else
                {
                        if (!IS_AFFECTED(ch, AFF_FLYING)
                            && !IS_AFFECTED(ch, AFF_FLOATING))
                                endurance =
                                        encumbrance(ch,
                                                    movement_loss[UMIN
                                                                  (SECT_MAX -
                                                                   1,
                                                                   in_room->
                                                                   sector_type)]
                                                    * 3);
                        else
                                endurance = 3;
                        if (ch->endurance < endurance)
                        {
                                send_to_char("You are too exhausted.\n\r",
                                             ch);
                                return rSTOP;
                        }
                }

                WAIT_STATE(ch, 1);
                if (ch->mount)
                        ch->mount->endurance -= endurance;
                else
                        ch->endurance -= endurance;
        }

        /*
         * Check if player can fit in the room
         */
        if (to_room->tunnel > 0)
        {
                CHAR_DATA *ctmp;
                int       count = ch->mount ? 1 : 0;

                for (ctmp = to_room->first_person; ctmp;
                     ctmp = ctmp->next_in_room)
                        if (++count >= to_room->tunnel)
                        {
                                if (ch->mount && count == to_room->tunnel)
                                        send_to_char
                                                ("There is no room for both you and your mount in there.\n\r",
                                                 ch);
                                else
                                        send_to_char
                                                ("There is no room for you in there.\n\r",
                                                 ch);
                                return rSTOP;
                        }
        }

        /*
         * check for traps on exit - later 
         */

        if (!IS_AFFECTED(ch, AFF_SNEAK)
            && (IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS)))
        {
                if (fall)
                        txt = "falls";
                else if (!txt)
                {
                        if (ch->mount)
                        {
                                if (IS_AFFECTED(ch->mount, AFF_FLOATING))
                                        txt = "floats";
                                else if (IS_AFFECTED(ch->mount, AFF_FLYING))
                                        txt = "flys";
                                else
                                        txt = "rides";
                        }
                        else
                        {
                                if (IS_AFFECTED(ch, AFF_FLOATING))
                                {
                                        if (drunk)
                                                txt = "floats unsteadily";
                                        else
                                                txt = "floats";
                                }
                                else if (IS_AFFECTED(ch, AFF_FLYING))
                                {
                                        if (drunk)
                                                txt = "flys shakily";
                                        else
                                                txt = "flys";
                                }
                                else if (ch->position == POS_SHOVE)
                                        txt = "is shoved";
                                else if (ch->position == POS_DRAG)
                                        txt = "is dragged";
                                else
                                {
                                        if (drunk)
                                                txt = "stumbles drunkenly";
                                        else if (running)
                                                txt = "runs";
                                        else
                                                txt = "leaves";
                                }
                        }
                }
                if (ch->mount)
                {
                        snprintf(buf, MSL, "$n %s %s upon $N.", txt,
                                 dir_name[door]);
                        act(AT_ACTION, buf, ch, NULL, ch->mount, TO_NOTVICT);
                }
                else
                {
                        snprintf(buf, MSL, "$n %s $T.", txt);
                        act(AT_ACTION, buf, ch, NULL, dir_name[door],
                            TO_ROOM);
                }
        }

        rprog_leave_trigger(ch);
        if (char_died(ch))
                return global_retcode;


        char_from_room(ch);
        char_to_room(ch, to_room);
        retcode = rNONE;
        if (ch->mount)
        {
                rprog_leave_trigger(ch->mount);

                /*
                 * Mount bug fix test. -Orion
                 */
                if (char_died(ch->mount))
                        return global_retcode;

                if (ch->mount)
                {
                        char_from_room(ch->mount);
                        char_to_room(ch->mount, to_room);
                }
        }

        if (!IS_AFFECTED(ch, AFF_SNEAK)
            && (IS_NPC(ch) || !IS_SET(ch->act, PLR_WIZINVIS)))
        {
                if (fall)
                        txt = "falls";
                else if (ch->mount)
                {
                        if (IS_AFFECTED(ch->mount, AFF_FLOATING))
                                txt = "floats in";
                        else if (IS_AFFECTED(ch->mount, AFF_FLYING))
                                txt = "flys in";
                        else
                                txt = "rides in";
                }
                else
                {
                        if (IS_AFFECTED(ch, AFF_FLOATING))
                        {
                                if (drunk)
                                        txt = "floats in unsteadily";
                                else
                                        txt = "floats in";
                        }
                        else if (IS_AFFECTED(ch, AFF_FLYING))
                        {
                                if (drunk)
                                        txt = "flys in shakily";
                                else
                                        txt = "flys in";
                        }
                        else if (ch->position == POS_SHOVE)
                                txt = "is shoved in";
                        else if (ch->position == POS_DRAG)
                                txt = "is dragged in";
                        else
                        {
                                if (drunk)
                                        txt = "stumbles drunkenly in";
                                else if (running)
                                        txt = "runs in";
                                else
                                        txt = "arrives";
                        }
                }
                switch (door)
                {
                default:
                        dtxt = "somewhere";
                        break;
                case 0:
                        dtxt = "the south";
                        break;
                case 1:
                        dtxt = "the west";
                        break;
                case 2:
                        dtxt = "the north";
                        break;
                case 3:
                        dtxt = "the east";
                        break;
                case 4:
                        dtxt = "below";
                        break;
                case 5:
                        dtxt = "above";
                        break;
                case 6:
                        dtxt = "the south-west";
                        break;
                case 7:
                        dtxt = "the south-east";
                        break;
                case 8:
                        dtxt = "the north-west";
                        break;
                case 9:
                        dtxt = "the north-east";
                        break;
                }
                if (ch->mount)
                {
                        snprintf(buf, MSL, "$n %s from %s upon $N.", txt,
                                 dtxt);
                        act(AT_ACTION, buf, ch, NULL, ch->mount, TO_ROOM);
                }
                else
                {
                        snprintf(buf, MSL, "$n %s from %s.", txt, dtxt);
                        act(AT_ACTION, buf, ch, NULL, NULL, TO_ROOM);
                }
        }

        if (!running)
                do_look(ch, "auto");

        if (brief)
                SET_BIT(ch->act, PLR_BRIEF);



        /*
         * BIG ugly looping problem here when the character is mptransed back
         * to the starting room.  To avoid this, check how many chars are in 
         * the room at the start and stop processing followers after doing
         * the right number of them.  -- Narn
         */
        if (!fall)
        {
                CHAR_DATA *fch;
                CHAR_DATA *nextinroom;
                int       chars = 0, count = 0;

                for (fch = from_room->first_person; fch;
                     fch = fch->next_in_room)
                        chars++;

                for (fch = from_room->first_person; fch && (count < chars);
                     fch = nextinroom)
                {
                        nextinroom = fch->next_in_room;
                        count++;
                        if (fch != ch   /* loop room bug fix here by Thoric */
                            && fch->master == ch
                            && fch->position == POS_STANDING)
                        {
                                act(AT_ACTION, "You follow $N.", fch, NULL,
                                    ch, TO_CHAR);
                                move_char(fch, pexit, 0, running);
                        }
                }
        }

        if (ch->in_room->first_content)
        {
                retcode = check_room_for_traps(ch, TRAP_ENTER_ROOM);
        }

        if (char_died(ch))
                return retcode;

        mprog_entry_trigger(ch);
        if (char_died(ch))
                return retcode;

        rprog_enter_trigger(ch);
        if (char_died(ch))
                return retcode;

        mprog_greet_trigger(ch);
        if (char_died(ch))
                return retcode;

        oprog_greet_trigger(ch);
        if (char_died(ch))
                return retcode;

        if (!will_fall(ch, fall) && fall > 0)
        {
                if (!IS_AFFECTED(ch, AFF_FLOATING)
                    || (ch->mount && !IS_AFFECTED(ch->mount, AFF_FLOATING)))
                {
                        set_char_color(AT_HURT, ch);
                        send_to_char("OUCH! You hit the ground!\n\r", ch);
                        WAIT_STATE(ch, 20);
                        retcode = damage(ch, ch, 50 * fall, TYPE_UNDEFINED);
                }
                else
                {
                        set_char_color(AT_MAGIC, ch);
                        send_to_char
                                ("You lightly float down to the ground.\n\r",
                                 ch);
                }
        }
        return retcode;
}


CMDF do_north(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        move_char(ch, get_exit(ch->in_room, DIR_NORTH), 0, FALSE);
        return;
}


CMDF do_east(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        move_char(ch, get_exit(ch->in_room, DIR_EAST), 0, FALSE);
        return;
}


CMDF do_south(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        move_char(ch, get_exit(ch->in_room, DIR_SOUTH), 0, FALSE);
        return;
}


CMDF do_west(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        move_char(ch, get_exit(ch->in_room, DIR_WEST), 0, FALSE);
        return;
}


CMDF do_up(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        move_char(ch, get_exit(ch->in_room, DIR_UP), 0, FALSE);
        return;
}


CMDF do_down(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        move_char(ch, get_exit(ch->in_room, DIR_DOWN), 0, FALSE);
        return;
}

CMDF do_northeast(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        move_char(ch, get_exit(ch->in_room, DIR_NORTHEAST), 0, FALSE);
        return;
}

CMDF do_northwest(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        move_char(ch, get_exit(ch->in_room, DIR_NORTHWEST), 0, FALSE);
        return;
}

CMDF do_southeast(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        move_char(ch, get_exit(ch->in_room, DIR_SOUTHEAST), 0, FALSE);
        return;
}

CMDF do_southwest(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        move_char(ch, get_exit(ch->in_room, DIR_SOUTHWEST), 0, FALSE);
        return;
}



EXIT_DATA *find_door(CHAR_DATA * ch, char *arg, bool quiet)
{
        EXIT_DATA *pexit;
        int       door;

        if (arg == NULL || !str_cmp(arg, ""))
                return NULL;

        pexit = NULL;
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
        {
                for (pexit = ch->in_room->first_exit; pexit;
                     pexit = pexit->next)
                {
                        if ((quiet || IS_SET(pexit->exit_info, EX_ISDOOR))
                            && pexit->keyword
                            && nifty_is_name(arg, pexit->keyword))
                                return pexit;
                }
                if (!quiet)
                        act(AT_PLAIN, "You see no $T here.", ch, NULL, arg,
                            TO_CHAR);
                return NULL;
        }

        if ((pexit = get_exit(ch->in_room, door)) == NULL)
        {
                if (!quiet)
                        act(AT_PLAIN, "You see no $T here.", ch, NULL, arg,
                            TO_CHAR);
                return NULL;
        }

        if (quiet)
                return pexit;

        if (IS_SET(pexit->exit_info, EX_SECRET))
        {
                act(AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR);
                return NULL;
        }

        if (!IS_SET(pexit->exit_info, EX_ISDOOR))
        {
                send_to_char("You can't do that.\n\r", ch);
                return NULL;
        }

        return pexit;
}


void toggle_bexit_flag(EXIT_DATA * pexit, int flag)
{
        EXIT_DATA *pexit_rev;

        TOGGLE_BIT(pexit->exit_info, flag);
        if ((pexit_rev = pexit->rexit) != NULL && pexit_rev != pexit)
                TOGGLE_BIT(pexit_rev->exit_info, flag);
}

void set_bexit_flag(EXIT_DATA * pexit, int flag)
{
        EXIT_DATA *pexit_rev;

        SET_BIT(pexit->exit_info, flag);
        if ((pexit_rev = pexit->rexit) != NULL && pexit_rev != pexit)
                SET_BIT(pexit_rev->exit_info, flag);
}

void remove_bexit_flag(EXIT_DATA * pexit, int flag)
{
        EXIT_DATA *pexit_rev;

        REMOVE_BIT(pexit->exit_info, flag);
        if ((pexit_rev = pexit->rexit) != NULL && pexit_rev != pexit)
                REMOVE_BIT(pexit_rev->exit_info, flag);
}

CMDF do_open(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        OBJ_DATA *obj;
        EXIT_DATA *pexit;
        int       door;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                do_openhatch(ch, "");
                return;
        }

        if ((pexit = find_door(ch, arg, TRUE)) != NULL)
        {
                /*
                 * 'open door' 
                 */
                EXIT_DATA *pexit_rev;

                if (!IS_SET(pexit->exit_info, EX_ISDOOR))
                {
                        send_to_char("You can't do that.\n\r", ch);
                        return;
                }
                if (!IS_SET(pexit->exit_info, EX_CLOSED))
                {
                        send_to_char("It's already open.\n\r", ch);
                        return;
                }
                if (IS_SET(pexit->exit_info, EX_LOCKED))
                {
                        send_to_char("It's locked.\n\r", ch);
                        return;
                }

                if (!IS_SET(pexit->exit_info, EX_SECRET)
                    || (pexit->keyword && nifty_is_name(arg, pexit->keyword)))
                {
                        act(AT_ACTION, "$n opens the $d.", ch, NULL,
                            pexit->keyword, TO_ROOM);
                        act(AT_ACTION, "You open the $d.", ch, NULL,
                            pexit->keyword, TO_CHAR);
                        if ((pexit_rev = pexit->rexit) != NULL
                            && pexit_rev->to_room == ch->in_room)
                        {
                                CHAR_DATA *rch;

                                for (rch = pexit->to_room->first_person; rch;
                                     rch = rch->next_in_room)
                                        act(AT_ACTION, "The $d opens.", rch,
                                            NULL, pexit_rev->keyword,
                                            TO_CHAR);
                                sound_to_room(pexit->to_room, "door");
                        }
                        remove_bexit_flag(pexit, EX_CLOSED);
                        if ((door = pexit->vdir) >= 0 && door < 10)
                                check_room_for_traps(ch, trap_door[door]);

                        sound_to_room(ch->in_room, "door");
                        return;
                }
        }

        if ((obj = get_obj_here(ch, arg)) != NULL)
        {
                /*
                 * 'open object' 
                 */
                if (obj->item_type != ITEM_CONTAINER)
                {
                        ch_printf(ch, "%s isn't a container.\n\r",
                                  capitalize(obj->short_descr));
                        return;
                }
                if (!IS_SET(obj->value[1], CONT_CLOSED))
                {
                        ch_printf(ch, "%s is already open.\n\r",
                                  capitalize(obj->short_descr));
                        return;
                }
                if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
                {
                        ch_printf(ch, "%s cannot be opened or closed.\n\r",
                                  capitalize(obj->short_descr));
                        return;
                }
                if (IS_SET(obj->value[1], CONT_LOCKED))
                {
                        ch_printf(ch, "%s is locked.\n\r",
                                  capitalize(obj->short_descr));
                        return;
                }

                REMOVE_BIT(obj->value[1], CONT_CLOSED);
                act(AT_ACTION, "You open $p.", ch, obj, NULL, TO_CHAR);
                act(AT_ACTION, "$n opens $p.", ch, obj, NULL, TO_ROOM);
                check_for_trap(ch, obj, TRAP_OPEN);
                return;
        }

        /*
         * Gavin - Fixed had a ; at the end of the iff statement 
         */
        if (!str_cmp(arg, "hatch"))
        {
                do_openhatch(ch, argument);
                return;
        }

        do_openhatch(ch, arg);
        return;
}



CMDF do_close(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        OBJ_DATA *obj;
        EXIT_DATA *pexit;
        int       door;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                do_closehatch(ch, "");
                return;
        }

        if ((pexit = find_door(ch, arg, TRUE)) != NULL)
        {
                /*
                 * 'close door' 
                 */
                EXIT_DATA *pexit_rev;

                if (!IS_SET(pexit->exit_info, EX_ISDOOR))
                {
                        send_to_char("You can't do that.\n\r", ch);
                        return;
                }
                if (IS_SET(pexit->exit_info, EX_CLOSED))
                {
                        send_to_char("It's already closed.\n\r", ch);
                        return;
                }

                act(AT_ACTION, "$n closes the $d.", ch, NULL, pexit->keyword,
                    TO_ROOM);
                act(AT_ACTION, "You close the $d.", ch, NULL, pexit->keyword,
                    TO_CHAR);

                /*
                 * close the other side 
                 */
                if ((pexit_rev = pexit->rexit) != NULL
                    && pexit_rev->to_room == ch->in_room)
                {
                        CHAR_DATA *rch;

                        SET_BIT(pexit_rev->exit_info, EX_CLOSED);
                        for (rch = pexit->to_room->first_person; rch;
                             rch = rch->next_in_room)
                                act(AT_ACTION, "The $d closes.", rch, NULL,
                                    pexit_rev->keyword, TO_CHAR);
                }
                set_bexit_flag(pexit, EX_CLOSED);
                if ((door = pexit->vdir) >= 0 && door < 10)
                        check_room_for_traps(ch, trap_door[door]);
                return;
        }

        if ((obj = get_obj_here(ch, arg)) != NULL)
        {
                /*
                 * 'close object' 
                 */
                if (obj->item_type != ITEM_CONTAINER)
                {
                        ch_printf(ch, "%s isn't a container.\n\r",
                                  capitalize(obj->short_descr));
                        return;
                }
                if (IS_SET(obj->value[1], CONT_CLOSED))
                {
                        ch_printf(ch, "%s is already closed.\n\r",
                                  capitalize(obj->short_descr));
                        return;
                }
                if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
                {
                        ch_printf(ch, "%s cannot be opened or closed.\n\r",
                                  capitalize(obj->short_descr));
                        return;
                }

                SET_BIT(obj->value[1], CONT_CLOSED);
                act(AT_ACTION, "You close $p.", ch, obj, NULL, TO_CHAR);
                act(AT_ACTION, "$n closes $p.", ch, obj, NULL, TO_ROOM);
                check_for_trap(ch, obj, TRAP_CLOSE);
                return;
        }

        if (!str_cmp(arg, "hatch"))
        {
                do_closehatch(ch, argument);
                return;
        }

        do_closehatch(ch, arg);
        return;
}


bool has_key(CHAR_DATA * ch, int key)
{
        OBJ_DATA *obj;

        for (obj = ch->first_carrying; obj; obj = obj->next_content)
                if (obj->pIndexData->vnum == key || obj->value[0] == key)
                        return TRUE;

        return FALSE;
}


CMDF do_lock(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        OBJ_DATA *obj;
        EXIT_DATA *pexit;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Lock what?\n\r", ch);
                return;
        }

        if ((pexit = find_door(ch, arg, TRUE)) != NULL)
        {
                /*
                 * 'lock door' 
                 */

                if (!IS_SET(pexit->exit_info, EX_ISDOOR))
                {
                        send_to_char("You can't do that.\n\r", ch);
                        return;
                }
                if (!IS_SET(pexit->exit_info, EX_CLOSED))
                {
                        send_to_char("It's not closed.\n\r", ch);
                        return;
                }
                if (pexit->key < 0)
                {
                        send_to_char("It can't be locked.\n\r", ch);
                        return;
                }
                if (!has_key(ch, pexit->key))
                {
                        send_to_char("You lack the key.\n\r", ch);
                        return;
                }
                if (IS_SET(pexit->exit_info, EX_LOCKED))
                {
                        send_to_char("It's already locked.\n\r", ch);
                        return;
                }

                if (!IS_SET(pexit->exit_info, EX_SECRET)
                    || (pexit->keyword && nifty_is_name(arg, pexit->keyword)))
                {
                        send_to_char("*Click*\n\r", ch);
                        act(AT_ACTION, "$n locks the $d.", ch, NULL,
                            pexit->keyword, TO_ROOM);
                        set_bexit_flag(pexit, EX_LOCKED);
                        return;
                }
        }

        if ((obj = get_obj_here(ch, arg)) != NULL)
        {
                /*
                 * 'lock object' 
                 */
                if (obj->item_type != ITEM_CONTAINER)
                {
                        send_to_char("That's not a container.\n\r", ch);
                        return;
                }
                if (!IS_SET(obj->value[1], CONT_CLOSED))
                {
                        send_to_char("It's not closed.\n\r", ch);
                        return;
                }
                if (obj->value[2] < 0)
                {
                        send_to_char("It can't be locked.\n\r", ch);
                        return;
                }
                if (!has_key(ch, obj->value[2]))
                {
                        send_to_char("You lack the key.\n\r", ch);
                        return;
                }
                if (IS_SET(obj->value[1], CONT_LOCKED))
                {
                        send_to_char("It's already locked.\n\r", ch);
                        return;
                }

                SET_BIT(obj->value[1], CONT_LOCKED);
                send_to_char("*Click*\n\r", ch);
                act(AT_ACTION, "$n locks $p.", ch, obj, NULL, TO_ROOM);
                return;
        }

        ch_printf(ch, "You see no %s here.\n\r", arg);
        return;
}



CMDF do_unlock(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        OBJ_DATA *obj;
        EXIT_DATA *pexit;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Unlock what?\n\r", ch);
                return;
        }

        if ((pexit = find_door(ch, arg, TRUE)) != NULL)
        {
                /*
                 * 'unlock door' 
                 */

                if (!IS_SET(pexit->exit_info, EX_ISDOOR))
                {
                        send_to_char("You can't do that.\n\r", ch);
                        return;
                }
                if (!IS_SET(pexit->exit_info, EX_CLOSED))
                {
                        send_to_char("It's not closed.\n\r", ch);
                        return;
                }
                if (pexit->key < 0)
                {
                        send_to_char("It can't be unlocked.\n\r", ch);
                        return;
                }
                if (!has_key(ch, pexit->key))
                {
                        send_to_char("You lack the key.\n\r", ch);
                        return;
                }
                if (!IS_SET(pexit->exit_info, EX_LOCKED))
                {
                        send_to_char("It's already unlocked.\n\r", ch);
                        return;
                }

                if (!IS_SET(pexit->exit_info, EX_SECRET)
                    || (pexit->keyword && nifty_is_name(arg, pexit->keyword)))
                {
                        send_to_char("*Click*\n\r", ch);
                        act(AT_ACTION, "$n unlocks the $d.", ch, NULL,
                            pexit->keyword, TO_ROOM);
                        remove_bexit_flag(pexit, EX_LOCKED);
                        return;
                }
        }

        if ((obj = get_obj_here(ch, arg)) != NULL)
        {
                /*
                 * 'unlock object' 
                 */
                if (obj->item_type != ITEM_CONTAINER)
                {
                        send_to_char("That's not a container.\n\r", ch);
                        return;
                }
                if (!IS_SET(obj->value[1], CONT_CLOSED))
                {
                        send_to_char("It's not closed.\n\r", ch);
                        return;
                }
                if (obj->value[2] < 0)
                {
                        send_to_char("It can't be unlocked.\n\r", ch);
                        return;
                }
                if (!has_key(ch, obj->value[2]))
                {
                        send_to_char("You lack the key.\n\r", ch);
                        return;
                }
                if (!IS_SET(obj->value[1], CONT_LOCKED))
                {
                        send_to_char("It's already unlocked.\n\r", ch);
                        return;
                }

                REMOVE_BIT(obj->value[1], CONT_LOCKED);
                send_to_char("*Click*\n\r", ch);
                act(AT_ACTION, "$n unlocks $p.", ch, obj, NULL, TO_ROOM);
                return;
        }

        ch_printf(ch, "You see no %s here.\n\r", arg);
        return;
}

CMDF do_bashdoor(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *gch;
        EXIT_DATA *pexit;
        char      arg[MAX_INPUT_LENGTH];

        if (!IS_NPC(ch) && ch->pcdata->learned[gsn_bashdoor] <= 0)
        {
                send_to_char
                        ("You're not enough of a warrior to bash doors!\n\r",
                         ch);
                return;
        }

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Bash what?\n\r", ch);
                return;
        }

        if (ch->fighting)
        {
                send_to_char("You can't break off your fight.\n\r", ch);
                return;
        }

        if ((pexit = find_door(ch, arg, FALSE)) != NULL)
        {
                ROOM_INDEX_DATA *to_room;
                EXIT_DATA *pexit_rev;
                int       percent_chance;
                char     *keyword;

                if (!IS_SET(pexit->exit_info, EX_CLOSED))
                {
                        send_to_char("Calm down.  It is already open.\n\r",
                                     ch);
                        return;
                }

                WAIT_STATE(ch, skill_table[gsn_bashdoor]->beats);

                if (IS_SET(pexit->exit_info, EX_SECRET))
                        keyword = "wall";
                else
                        keyword = pexit->keyword;
                if (!IS_NPC(ch))
                        percent_chance =
                                ch->pcdata->learned[gsn_bashdoor] / 2;
                else
                        percent_chance = 90;

                if (!IS_SET(pexit->exit_info, EX_BASHPROOF)
                    && ch->endurance >= 15
                    && number_percent() <
                    (percent_chance + 4 * (get_curr_str(ch) - 19)))
                {
                        REMOVE_BIT(pexit->exit_info, EX_CLOSED);
                        if (IS_SET(pexit->exit_info, EX_LOCKED))
                                REMOVE_BIT(pexit->exit_info, EX_LOCKED);
                        SET_BIT(pexit->exit_info, EX_BASHED);

                        act(AT_SKILL, "Crash!  You bashed open the $d!", ch,
                            NULL, keyword, TO_CHAR);
                        act(AT_SKILL, "$n bashes open the $d!", ch, NULL,
                            keyword, TO_ROOM);
                        learn_from_success(ch, gsn_bashdoor);

                        if ((to_room = pexit->to_room) != NULL
                            && (pexit_rev = pexit->rexit) != NULL
                            && pexit_rev->to_room == ch->in_room)
                        {
                                CHAR_DATA *rch;

                                REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
                                if (IS_SET(pexit_rev->exit_info, EX_LOCKED))
                                        REMOVE_BIT(pexit_rev->exit_info,
                                                   EX_LOCKED);
                                SET_BIT(pexit_rev->exit_info, EX_BASHED);

                                for (rch = to_room->first_person; rch;
                                     rch = rch->next_in_room)
                                {
                                        act(AT_SKILL, "The $d crashes open!",
                                            rch, NULL, pexit_rev->keyword,
                                            TO_CHAR);
                                }
                        }
                        damage(ch, ch, (ch->max_hit / 20), gsn_bashdoor);

                }
                else
                {
                        act(AT_SKILL,
                            "WHAAAAM!!!  You bash against the $d, but it doesn't budge.",
                            ch, NULL, keyword, TO_CHAR);
                        act(AT_SKILL,
                            "WHAAAAM!!!  $n bashes against the $d, but it holds strong.",
                            ch, NULL, keyword, TO_ROOM);
                        damage(ch, ch, (ch->max_hit / 20) + 10, gsn_bashdoor);
                        learn_from_failure(ch, gsn_bashdoor);
                }
        }
        else
        {
                act(AT_SKILL,
                    "WHAAAAM!!!  You bash against the wall, but it doesn't budge.",
                    ch, NULL, NULL, TO_CHAR);
                act(AT_SKILL,
                    "WHAAAAM!!!  $n bashes against the wall, but it holds strong.",
                    ch, NULL, NULL, TO_ROOM);
                damage(ch, ch, (ch->max_hit / 20) + 10, gsn_bashdoor);
                learn_from_failure(ch, gsn_bashdoor);
        }

        return;
}


CMDF do_stand(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        switch (ch->position)
        {
        case POS_SLEEPING:
                if (IS_AFFECTED(ch, AFF_SLEEP))
                {
                        send_to_char("You can't seem to wake up!\n\r", ch);
                        return;
                }

                send_to_char("You wake and climb quickly to your feet.\n\r",
                             ch);
                act(AT_ACTION, "$n arises from $s slumber.", ch, NULL, NULL,
                    TO_ROOM);
                ch->position = POS_STANDING;
                break;

        case POS_RESTING:
                send_to_char("You gather yourself and stand up.\n\r", ch);
                act(AT_ACTION, "$n rises from $s rest.", ch, NULL, NULL,
                    TO_ROOM);
                ch->position = POS_STANDING;
                break;

        case POS_SITTING:
                send_to_char("You move quickly to your feet.\n\r", ch);
                act(AT_ACTION, "$n rises up.", ch, NULL, NULL, TO_ROOM);
                ch->position = POS_STANDING;
                break;

        case POS_STANDING:
                send_to_char("You are already standing.\n\r", ch);
                break;

        case POS_FIGHTING:
                send_to_char("You are already fighting!\n\r", ch);
                break;
        default:
                return;
        }

        return;
}


CMDF do_sit(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj = NULL;


        if (ch->position == POS_FIGHTING)
        {
                send_to_char("Maybe you should finish this fight first?\n\r",
                             ch);
                return;
        }

        /*
         * okay, now that we know we can sit, find an object to sit on 
         */
        if (argument[0] != '\0')
        {
                obj = get_obj_list(ch, argument, ch->in_room->first_content);
                if (obj == NULL)
                {
                        send_to_char("You don't see that here.\n\r", ch);
                        return;
                }
        }

        if (obj != NULL)
        {
                if (obj->item_type != ITEM_FURNITURE)
                {
                        send_to_char("You can't sit on that.\n\r", ch);
                        return;
                }

                if (obj != NULL && ch->on != obj
                    && count_users(obj) >= obj->value[0])
                {
                        act(AT_ACTION, "There's no more room on $p.", ch, obj,
                            NULL, TO_CHAR);
                        return;
                }

                ch->on = obj;
        }
        switch (ch->position)
        {
        case POS_SLEEPING:
                if (IS_AFFECTED(ch, AFF_SLEEP))
                {
                        send_to_char("You can't wake up!\n\r", ch);
                        return;
                }

                if (obj == NULL)
                {
                        send_to_char("You wake and sit up.\n\r", ch);
                        act(AT_ACTION, "$n wakes and sits up.", ch, NULL,
                            NULL, TO_ROOM);
                }
                else if (obj->value[2] == SIT_AT)
                {
                        act(AT_ACTION, "You wake and sit at $p.", ch, obj,
                            NULL, TO_CHAR);
                        act(AT_ACTION, "$n wakes and sits at $p.", ch, obj,
                            NULL, TO_ROOM);

                }
                else if (obj->value[2] == SIT_ON)
                {
                        act(AT_ACTION, "You wake and sit on $p.", ch, obj,
                            NULL, TO_CHAR);
                        act(AT_ACTION, "$n wakes and sits at $p.", ch, obj,
                            NULL, TO_ROOM);
                }
                else
                {
                        act(AT_ACTION, "You wake and sit in $p.", ch, obj,
                            NULL, TO_CHAR);
                        act(AT_ACTION, "$n wakes and sits in $p.", ch, obj,
                            NULL, TO_ROOM);
                }

                ch->position = POS_SITTING;
                break;
        case POS_RESTING:
                if (obj == NULL)
                        send_to_char("You stop resting.\n\r", ch);
                else if (obj->value[2] == SIT_AT)
                {
                        act(AT_ACTION, "You sit at $p.", ch, obj, NULL,
                            TO_CHAR);
                        act(AT_ACTION, "$n sits at $p.", ch, obj, NULL,
                            TO_ROOM);
                }

                else if (obj->value[2] == SIT_ON)
                {
                        act(AT_ACTION, "You sit on $p.", ch, obj, NULL,
                            TO_CHAR);
                        act(AT_ACTION, "$n sits on $p.", ch, obj, NULL,
                            TO_ROOM);
                }
                ch->position = POS_SITTING;
                break;
        case POS_SITTING:
                send_to_char("You are already sitting down.\n\r", ch);
                break;

        case POS_STANDING:
                if (obj == NULL)
                {
                        send_to_char("You sit down.\n\r", ch);
                        act(AT_ACTION, "$n sits down on the ground.", ch,
                            NULL, NULL, TO_ROOM);
                }
                else if (obj->value[2] == SIT_AT)
                {
                        act(AT_ACTION, "You sit down at $p.", ch, obj, NULL,
                            TO_CHAR);
                        act(AT_ACTION, "$n sits down at $p.", ch, obj, NULL,
                            TO_ROOM);
                }
                else if (obj->value[2] == SIT_ON)
                {
                        act(AT_ACTION, "You sit on $p.", ch, obj, NULL,
                            TO_CHAR);
                        act(AT_ACTION, "$n sits on $p.", ch, obj, NULL,
                            TO_ROOM);
                }
                else
                {
                        act(AT_ACTION, "You sit down in $p.", ch, obj, NULL,
                            TO_CHAR);
                        act(AT_ACTION, "$n sits down in $p.", ch, obj, NULL,
                            TO_ROOM);
                }
                ch->position = POS_SITTING;
                break;
        default:
                {
                }
        }
        return;
}

CMDF do_rest(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj = NULL;

        /*
         * okay, now that we know we can rest, find an object to rest on 
         */
        if (argument[0] != '\0')
        {
                obj = get_obj_list(ch, argument, ch->in_room->first_content);
                if (obj == NULL)
                {
                        send_to_char("You don't see that here.\n\r", ch);
                        return;
                }
        }
        else
                obj = ch->on;

        if (obj != NULL)
        {
                if (obj->item_type != ITEM_FURNITURE)
                {
                        send_to_char("You can't rest on that.\n\r", ch);
                        return;
                }

                if (obj != NULL && ch->on != obj
                    && count_users(obj) >= obj->value[0])
                {
                        act(AT_ACTION, "There's no more room on $p.", ch, obj,
                            NULL, TO_CHAR);
                        return;
                }

                ch->on = obj;
        }

        switch (ch->position)
        {
        case POS_SLEEPING:
                if (IS_AFFECTED(ch, AFF_SLEEP))
                {
                        send_to_char("You can't wake up!\n\r", ch);
                        return;
                }

                if (obj == NULL)
                {
                        send_to_char("You wake up and start resting.\n\r",
                                     ch);
                        act(AT_ACTION, "$n wakes up and starts resting.", ch,
                            NULL, NULL, TO_ROOM);
                }
                else if (obj->value[2] == REST_AT)
                {
                        act(AT_ACTION, "You wake up and rest at $p.", ch, obj,
                            NULL, TO_CHAR);
                        act(AT_ACTION, "$n wakes up and rests at $p.", ch,
                            obj, NULL, TO_ROOM);
                }
                else if (obj->value[2] == REST_ON)
                {
                        act(AT_ACTION, "You wake up and rest on $p.", ch, obj,
                            NULL, TO_CHAR);
                        act(AT_ACTION, "$n wakes up and rests on $p.", ch,
                            obj, NULL, TO_ROOM);
                }
                else
                {
                        act(AT_ACTION, "You wake up and rest in $p.", ch, obj,
                            NULL, TO_CHAR);
                        act(AT_ACTION, "$n wakes up and rests in $p.", ch,
                            obj, NULL, TO_ROOM);
                }
                ch->position = POS_RESTING;
                break;

        case POS_RESTING:
                send_to_char("You are already resting.\n\r", ch);
                break;


        case POS_STANDING:
                if (obj == NULL)
                {
                        send_to_char("You rest.\n\r", ch);
                        act(AT_ACTION, "$n sits down and rests.", ch, NULL,
                            NULL, TO_ROOM);
                }
                else if (obj->value[2] == REST_AT)
                {
                        act(AT_ACTION, "You sit down at $p and rest.", ch,
                            obj, NULL, TO_CHAR);
                        act(AT_ACTION, "$n sits down at $p and rests.", ch,
                            obj, NULL, TO_ROOM);
                }
                else if (obj->value[2] == REST_ON)
                {
                        act(AT_ACTION, "You sit on $p and rest.", ch, obj,
                            NULL, TO_CHAR);
                        act(AT_ACTION, "$n sits on $p and rests.", ch, obj,
                            NULL, TO_ROOM);
                }
                else
                {
                        act(AT_ACTION, "You rest in $p.", ch, obj, NULL,
                            TO_CHAR);
                        act(AT_ACTION, "$n rests in $p.", ch, obj, NULL,
                            TO_ROOM);
                }
                ch->position = POS_RESTING;
                break;

        case POS_SITTING:
                if (obj == NULL)
                {
                        send_to_char("You rest.\n\r", ch);
                        act(AT_ACTION, "$n rests.", ch, NULL, NULL, TO_ROOM);
                }
                else if (obj->value[2] == REST_AT)
                {
                        act(AT_ACTION, "You rest at $p.", ch, obj, NULL,
                            TO_CHAR);
                        act(AT_ACTION, "$n rests at $p.", ch, obj, NULL,
                            TO_ROOM);
                }
                else if (obj->value[2] == REST_ON)
                {
                        act(AT_ACTION, "You rest on $p.", ch, obj, NULL,
                            TO_CHAR);
                        act(AT_ACTION, "$n rests on $p.", ch, obj, NULL,
                            TO_ROOM);
                }
                else
                {
                        act(AT_ACTION, "You rest in $p.", ch, obj, NULL,
                            TO_CHAR);
                        act(AT_ACTION, "$n rests in $p.", ch, obj, NULL,
                            TO_ROOM);
                }
                ch->position = POS_RESTING;
                break;

        case POS_MOUNTED:
                send_to_char("You'd better dismount first.\n\r", ch);
                return;
        default:
                {
                }
        }

        rprog_rest_trigger(ch);
        return;
}

CMDF do_sleep(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj = NULL;


        switch (ch->position)
        {
        case POS_SLEEPING:
                send_to_char("You are already sleeping.\n\r", ch);
                break;

        case POS_RESTING:
        case POS_SITTING:
        case POS_STANDING:
                if (argument[0] == '\0' && ch->on == NULL)
                {
                        send_to_char("You go to sleep.\n\r", ch);
                        act(AT_ACTION, "$n goes to sleep.", ch, NULL, NULL,
                            TO_ROOM);
                        ch->position = POS_SLEEPING;
                }
                else    /* find an object and sleep on it */
                {
                        if (argument[0] == '\0')
                                obj = ch->on;
                        else
                                obj = get_obj_list(ch, argument,
                                                   ch->in_room->
                                                   first_content);

                        if (obj == NULL)
                        {
                                send_to_char("You don't see that here.\n\r",
                                             ch);
                                return;
                        }
                        if (obj->item_type != ITEM_FURNITURE)
                        {
                                send_to_char("You can't sleep on that!\n\r",
                                             ch);
                                return;
                        }

                        if (ch->on != obj
                            && count_users(obj) >= obj->value[0])
                        {
                                act(AT_ACTION,
                                    "There is no room on $p for you.", ch,
                                    obj, NULL, TO_CHAR);
                                return;
                        }

                        ch->on = obj;
                        if (obj->value[2] == SLEEP_AT)
                        {
                                act(AT_ACTION, "You go to sleep at $p.", ch,
                                    obj, NULL, TO_CHAR);
                                act(AT_ACTION, "$n goes to sleep at $p.", ch,
                                    obj, NULL, TO_ROOM);
                        }
                        else if (obj->value[2] == SLEEP_ON)
                        {
                                act(AT_ACTION, "You go to sleep on $p.", ch,
                                    obj, NULL, TO_CHAR);
                                act(AT_ACTION, "$n goes to sleep on $p.", ch,
                                    obj, NULL, TO_ROOM);
                        }
                        else
                        {
                                act(AT_ACTION, "You go to sleep in $p.", ch,
                                    obj, NULL, TO_CHAR);
                                act(AT_ACTION, "$n goes to sleep in $p.", ch,
                                    obj, NULL, TO_ROOM);
                        }
                        ch->position = POS_SLEEPING;
                }
                break;

        case POS_FIGHTING:
                send_to_char("You are busy fighting!\n\r", ch);
                break;
        default:
                {
                }
        }

        rprog_sleep_trigger(ch);
        return;
}




CMDF do_wake(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                do_stand(ch, argument);
                return;
        }

        if (!IS_AWAKE(ch))
        {
                send_to_char("You are asleep yourself!\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_AWAKE(victim))
        {
                act(AT_PLAIN, "$N is already awake.", ch, NULL, victim,
                    TO_CHAR);
                return;
        }

        if (IS_AFFECTED(victim, AFF_SLEEP) || victim->position < POS_SLEEPING)
        {
                act(AT_PLAIN, "You can't seem to wake $M!", ch, NULL, victim,
                    TO_CHAR);
                return;
        }

        act(AT_ACTION, "You wake $M.", ch, NULL, victim, TO_CHAR);
        victim->position = POS_STANDING;
        act(AT_ACTION, "$n wakes you.", ch, NULL, victim, TO_VICT);
        return;
}


/*
 * "Climb" in a certain direction.				-Thoric
 */
CMDF do_climb(CHAR_DATA * ch, char *argument)
{
        EXIT_DATA *pexit;
        bool      found;

        found = FALSE;
        if (argument[0] == '\0')
        {
                for (pexit = ch->in_room->first_exit; pexit;
                     pexit = pexit->next)
                        if (IS_SET(pexit->exit_info, EX_xCLIMB))
                        {
                                move_char(ch, pexit, 0, FALSE);
                                return;
                        }
                send_to_char("You cannot climb here.\n\r", ch);
                return;
        }

        if ((pexit = find_door(ch, argument, TRUE)) != NULL
            && IS_SET(pexit->exit_info, EX_xCLIMB))
        {
                move_char(ch, pexit, 0, FALSE);
                return;
        }
        send_to_char("You cannot climb there.\n\r", ch);
        return;
}

/*
 * "enter" something (moves through an exit)			-Thoric
 */
CMDF do_enter(CHAR_DATA * ch, char *argument)
{
        EXIT_DATA *pexit;
        bool      found;

        found = FALSE;
        if (argument[0] == '\0')
        {
                for (pexit = ch->in_room->first_exit; pexit;
                     pexit = pexit->next)
                        if (IS_SET(pexit->exit_info, EX_xENTER))
                        {
                                move_char(ch, pexit, 0, FALSE);
                                return;
                        }
                send_to_char("You cannot find an entrance here.\n\r", ch);
                return;
        }

        if ((pexit = find_door(ch, argument, TRUE)) != NULL
            && IS_SET(pexit->exit_info, EX_xENTER))
        {
                move_char(ch, pexit, 0, FALSE);
                return;
        }
        do_board(ch, argument);
        return;
}

/*
 * Leave through an exit.					-Thoric
 */
CMDF do_leave(CHAR_DATA * ch, char *argument)
{
        EXIT_DATA *pexit;
        bool      found;

        found = FALSE;
        if (argument[0] == '\0')
        {
                for (pexit = ch->in_room->first_exit; pexit;
                     pexit = pexit->next)
                        if (IS_SET(pexit->exit_info, EX_xLEAVE))
                        {
                                move_char(ch, pexit, 0, FALSE);
                                return;
                        }
                do_leaveship(ch, "");
                return;
        }

        if ((pexit = find_door(ch, argument, TRUE)) != NULL
            && IS_SET(pexit->exit_info, EX_xLEAVE))
        {
                move_char(ch, pexit, 0, FALSE);
                return;
        }
        do_leaveship(ch, "");
        return;
}

/* Run command taken from DOTD codebase - Samson 2-25-99 */
/* Added argument to let players specify how far to run.
 * Fixed an infinite loop bug where somehow a closed door would cause problems.
 * Added Overland support to the command. Samson 4-4-01
 */
/* Supressed display of rooms/terrain until you stop to prevent buffer overflows - Samson 4-16-01 */
CMDF do_run(CHAR_DATA * ch, char *argument)
{
        ROOM_INDEX_DATA *from_room;
        EXIT_DATA *pexit;
        int       diff = 0;

        if (argument[0] == '\0')
        {
                send_to_char("Run where?\n\r", ch);
                return;
        }

        if (ch->position != POS_STANDING && ch->position != POS_MOUNTED)
        {
                send_to_char
                        ("You are not in the correct position for that.\n\r",
                         ch);
                return;
        }

        from_room = ch->in_room;

        while ((pexit = find_door(ch, argument, TRUE)) != NULL)
        {
                diff = ch->endurance;
                if (ch->endurance < 1)
                {
                        send_to_char
                                ("You are too exhausted to run anymore.\n\r",
                                 ch);
                        ch->endurance = 0;
                        break;
                }
                if (move_char(ch, pexit, 0, TRUE) == rSTOP)
                        break;
                diff -= ch->endurance;
                /*
                 * Double movement item for running 
                 */
                ch->endurance -= diff;
        }

        if (ch->in_room == from_room)
        {
                send_to_char("You try to run but don't get anywhere.\n\r",
                             ch);
                act(AT_ACTION, "$n tries to run but doesn't get anywhere.",
                    ch, NULL, NULL, TO_ROOM);
                return;
        }

        send_to_char("You slow down after your run.\n\r", ch);
        act(AT_ACTION, "$n slows down after $s run.", ch, NULL, NULL,
            TO_ROOM);

        do_look(ch, "auto");
        return;
}

CMDF do_struggle_binding(CHAR_DATA * ch)
{
        OBJ_DATA *obj = NULL;
        int       chance = 0;

        if (!ch->held)
        {
                bug("%s struggling on a binding without held TRUE!",
                    ch->name);
                return;
        }
        if ((obj = get_eq_char(ch, WEAR_BINDING)) == NULL)
        {
                bug("%s is do_struggle_binding with no binding!", ch->name);
                return;
        }
        if (ch->endurance < 100)
        {
                send_to_char("You're too tired to struggle more.", ch);
                return;
        }
        else
        {
                chance = (get_curr_str(ch) +
                          (get_curr_dex(ch) / 2)) /
                        number_range(get_curr_str(ch) / 5,
                                     get_curr_str(ch) / 2);
                if (obj->value[0] == 100)
                        chance = 0;
                chance = URANGE(1, chance - obj->value[1], 100);
                obj->value[2] = URANGE(1, obj->value[2] - chance, 99);
                ch->endurance =
                        URANGE(0, ch->endurance - number_range(150, 500),
                               ch->max_endurance);
                if (ch->endurance < 0)
                        ch->endurance = 0;  /* Added in to fix strange bug */
                if (obj->value[2] == 0)
                {
                        act(AT_ACTION,
                            "You struggle, and break free from $p!", ch, obj,
                            NULL, TO_CHAR);
                        act(AT_ACTION,
                            "$n struggles, and breaks free from $p!", ch, obj,
                            NULL, TO_ROOM);
                        unequip_char(ch, obj);
                        make_scraps(obj);
                        WAIT_STATE(ch, 3);
                        return;
                }
                else
                {
                        act(AT_ACTION, "You struggle, but $p holds strong.",
                            ch, obj, NULL, TO_CHAR);
                        act(AT_ACTION,
                            "$n struggles, to no avail, against $p.", ch, obj,
                            NULL, TO_ROOM);
                        return;
                }
        }
}
CMDF do_hold_person(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *victim = NULL;

        if (argument[0] == '\0')
        {
                send_to_char("Whom do you wish to grab ahold of?\n\r", ch);
                return;
        }
        if (ch->holding)
        {
                send_to_char("But you are already holding someone!\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, argument)) == NULL)
        {
                sprintf(buf, "You can't find %s.\n\r", argument);
                send_to_char(buf, ch);
                return;
        }
		if (ch == victim)
		{
			send_to_char("You love yourself that much, huh?\n\r", ch);
			return;
		}
        if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
        {
                send_to_char("You can't do that.", ch);
                return;
        }
        if (victim->holding != NULL)
        {
                act(AT_ACTION, "But $E is holding someone. Try MURDERING $M.",
                    ch, NULL, victim, TO_CHAR);
                return;
        }

        if (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
        {
                send_to_char
                        ("The shopkeeper struggles too much for you to get a good hold!\n\r",
                         ch);
                return;
        }

        act(AT_ACTION, "You grab ahold of $N!", ch, NULL, victim, TO_CHAR);
        act(AT_ACTION, "$n grabs ahold of $N!", ch, NULL, victim, TO_NOTVICT);
        act(AT_ACTION, "$n grabs ahold of you!", ch, NULL, victim, TO_VICT);
        ch->holding = victim;
        victim->heldby = ch;
        victim->held = TRUE;
        victim->position = POS_STANDING;
        victim->master = ch;
        victim->leader = ch;
        WAIT_STATE(ch, number_range(2, 7));
        return;
}

CMDF do_struggle(CHAR_DATA * ch)
{
        CHAR_DATA *holder = NULL;
        OBJ_DATA *obj = NULL;
        int       chance = 0, diff_str = 0;

        if (!ch->held)
        {
                send_to_char("You have nothing to struggle against.\n\r", ch);
                return;
        }
        if ((obj = get_eq_char(ch, WEAR_BINDING)) != NULL)
        {
                do_struggle_binding(ch);
                return;
        }
        if (!ch->heldby)
        {
                ch->held = FALSE;
                ch->heldby = NULL;
                ch->master = NULL;
                ch->leader = NULL;
                do_struggle(ch);
                return;
        }
        else
                holder = ch->heldby;

        if (ch->endurance < 100)
        {
                send_to_char("You are too tired to struggle more.\n\r", ch);
                return;
        }
        if ((diff_str = (get_curr_str(ch) - get_curr_str(holder))) > 3)
        {
                act(AT_ACTION,
                    "With little effort, you break free of $N's grip on you.",
                    ch, NULL, holder, TO_CHAR);
                act(AT_ACTION,
                    "With little effort, $n breaks free of your grip!", ch,
                    NULL, holder, TO_VICT);
                act(AT_ACTION,
                    "With little effort, $n breaks free of $N's grip.", ch,
                    NULL, holder, TO_NOTVICT);
                ch->leader = NULL;
                ch->heldby = NULL;
                ch->held = FALSE;
                holder->holding = NULL;
                ch->master = NULL;
                WAIT_STATE(holder, number_range(2, 7));
                return;
        }
        else
        {
                chance = URANGE(0,
                                ((get_curr_str(ch) + get_curr_dex(ch)) +
                                 number_range(10, 40)), 100);
                if (number_percent() > chance)
                {
                        act(AT_ACTION,
                            "You struggle against $N's grip on you, to no avail.",
                            ch, NULL, holder, TO_CHAR);
                        act(AT_ACTION,
                            "$n struggles against your grip on $m, but fails to escape.",
                            ch, NULL, holder, TO_VICT);
                        act(AT_ACTION,
                            "$n struggles against $N's grip on $m, but $N holds strong.",
                            ch, NULL, holder, TO_NOTVICT);
                        ch->endurance =
                                URANGE(0,
                                       ch->endurance - number_range(120, 700),
                                       ch->max_endurance);
                        WAIT_STATE(ch, number_range(5, 12));
                        return;
                }
                else
                {
                        act(AT_ACTION,
                            "You struggle against $N's grip, breaking free!",
                            ch, NULL, holder, TO_CHAR);
                        act(AT_ACTION,
                            "$n struggles against $N's grip, breaking free!",
                            ch, NULL, holder, TO_NOTVICT);
                        act(AT_ACTION,
                            "$n struggles against your grip, breaking free!",
                            ch, NULL, holder, TO_VICT);
                        ch->heldby = NULL;
                        ch->held = FALSE;
                        holder->holding = NULL;
                        ch->master = NULL;
                        ch->leader = NULL;
                        WAIT_STATE(holder, number_range(2, 7));
                        return;
                }
        }
}
CMDF do_unbind(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim = NULL;
        OBJ_DATA *obj = NULL;
        char      arg[MAX_INPUT_LENGTH];
        int       keycode = 0;
        bool      keybind = FALSE;


        if (argument[0] == '\0')
        {
                send_to_char("Usage: unbind <target> <binding code>\n\r", ch);
                return;
        }
        argument = one_argument(argument, arg);
        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("You don't see anyone like that here.\n\r", ch);
                return;
        }
        if ((obj = get_eq_char(victim, WEAR_BINDING)) == NULL)
        {
                act(AT_ACTION, "$n, $E doesn't seem to be bound.", ch, NULL,
                    victim, TO_CHAR);
                return;
        }
        if (obj->value[4] != 0)
        {
                keybind = TRUE;
                keycode = atoi(argument);
        }
        if (obj->value[3] != keycode)
        {
                act(AT_ACTION, "Thats not the right code to unlock $N's $p.",
                    ch, obj, victim, TO_CHAR);
                act(AT_ACTION,
                    "$n presses a few buttons on $p, but nothing happens.",
                    ch, obj, victim, TO_VICT);
                act(AT_ACTION,
                    "$n presses a few buttons on $N's $p, but nothing happens.",
                    ch, obj, victim, TO_NOTVICT);
                return;
        }
        else
        {
                unequip_char(ch, obj);
                separate_obj(obj);
                obj_from_char(obj);
                obj_to_char(obj, ch);
                if (victim->heldby == NULL)
                        victim->held = FALSE;
                if (keybind)
                {
                        act(AT_ACTION,
                            "You enter the code to unlock $p on $N.", ch, obj,
                            victim, TO_CHAR);
                        act(AT_ACTION,
                            "$n presses a few buttons on $p, and it unlocks!",
                            ch, obj, victim, TO_VICT);
                        act(AT_ACTION,
                            "$n presses a few buttons on $p, and it unlocks.",
                            ch, obj, victim, TO_NOTVICT);
                        return;
                }
                else
                {
                        act(AT_ACTION, "You unbind $p from $N.", ch, obj,
                            victim, TO_CHAR);
                        act(AT_ACTION, "$n unbinds $p from $N.", ch, obj,
                            victim, TO_NOTVICT);
                        act(AT_ACTION, "$n unbinds you, removing $p!", ch,
                            obj, victim, TO_VICT);
                        return;
                }
        }
}



CMDF do_subdue(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *victim = NULL;
        int       strain_amount = 0;

        if (argument[0] == '\0')
        {
                send_to_char("Who do you want to subdue?\n\r", ch);
                return;
        }
        if ((victim = get_char_room(ch, argument)) == NULL)
        {
                sprintf(buf, "You don't see any %s nearby.\n\r", argument);
                send_to_char(buf, ch);
                return;
        }
        if (!victim->held)
        {
                act(AT_ACTION, "But $E isn't held or bound.", ch, NULL,
                    victim, TO_CHAR);
                return;
        }
        if (victim->endurance < 80)
        {
                act(AT_ACTION, "$N looks pretty subdued already.", ch, NULL,
                    victim, TO_CHAR);
                return;
        }
        if ((strain_amount =
             (get_curr_str(ch) + get_curr_dex(ch) + (ch->endurance / 10))) ==
            0)
        {
                bug("%s returning 0 value for strain_amount.", ch->name);
                return;
        }
        else
        {
                victim->endurance =
                        URANGE(0,
                               victim->endurance - (strain_amount +
                                                    number_range(strain_amount
                                                                 / 5,
                                                                 strain_amount
                                                                 / 2)),
                               victim->max_endurance);
                act(AT_ACTION, "You rough $M up, and $E looks weaker.", ch,
                    NULL, victim, TO_CHAR);
                act(AT_ACTION, "$n roughs you up, and you feel drained!", ch,
                    NULL, victim, TO_VICT);
                act(AT_ACTION, "$n roughs $N up, and $E looks weaker.", ch,
                    NULL, victim, TO_NOTVICT);
                WAIT_STATE(ch, number_range(4, 8));
                return;
        }
}
CMDF do_bind(CHAR_DATA * ch, char *argument)
{
        OBJ_DATA *obj = NULL;
        CHAR_DATA *victim = NULL;
        char      arg[MAX_INPUT_LENGTH];
        int       keycode = 0;
        bool      keylock = FALSE;

        if (argument[0] == '\0')
        {
                send_to_char("Usage: bind <victim>\n\r", ch);
                send_to_char
                        ("Note: You must be holding something to bind them with.\n\r",
                         ch);
                return;
        }
        argument = one_argument(argument, arg);
        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("You don't see anyone like that nearby.\n\r",
                             ch);
                return;
        }
        if (!victim->held && IS_AWAKE(victim) && victim->endurance > 100)
        {
                act(AT_ACTION,
                    "Well, $N seems a little too lively to be bound.", ch,
                    NULL, victim, TO_CHAR);
                return;
        }
        if (victim->held == TRUE && victim->endurance > 100)
        {
                act(AT_ACTION, "Try subduing $M first.", ch, NULL, victim,
                    TO_CHAR);
                return;
        }
        if ((obj = get_eq_char(ch, WEAR_HOLD)) == NULL
            || obj->item_type != ITEM_BINDING)
        {
                act(AT_ACTION,
                    "You need to be holding something to bind $M with.", ch,
                    NULL, victim, TO_CHAR);
                return;
        }

        else
        {
                if (obj->value[4] > 0)
                {
                        keylock = TRUE;
                        if (argument[0] == '\0')
                                keycode = 0;
                        else
                                keycode = atoi(argument);
                }
                if (keylock)
                        obj->value[3] = keycode;
                unequip_char(ch, obj);
                SET_BIT(obj->wear_loc, WEAR_HOLD);
                SET_BIT(obj->wear_loc, ITEM_WEAR_BINDING);
                SET_BIT(obj->extra_flags, ITEM_NOREMOVE);
                separate_obj(obj);
                obj_from_char(obj);
                obj = obj_to_char(obj, victim);
                equip_char(victim, obj, WEAR_BINDING);
                victim->held = TRUE;
                if (victim->heldby)
                {
                        victim->heldby->holding = NULL;
                        victim->heldby = NULL;
                }
                act(AT_ACTION, "You bind $M up with $p.", ch, obj, victim,
                    TO_CHAR);
                act(AT_ACTION, "$n binds you up with $p.", ch, obj, victim,
                    TO_VICT);
                act(AT_ACTION, "$n binds $N up with $p.", ch, obj, victim,
                    TO_NOTVICT);
                WAIT_STATE(ch, number_range(2, 6));
                return;
        }
}
CMDF do_release(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim = NULL;
        OBJ_DATA *obj = NULL;

        argument = NULL;

        if (ch->holding == NULL)
        {
                send_to_char
                        ("Release what? You're not holding or carrying anyone.\n\r",
                         ch);
                return;
        }

        victim = ch->holding;
        if (ch->in_room != victim->in_room)
        {
                send_to_char("They arn't here", ch);
                return;
        }
        else
        {
                act(AT_ACTION, "You release $N.", ch, NULL, victim, TO_CHAR);
                act(AT_ACTION, "$n releases $N.", ch, NULL, victim,
                    TO_NOTVICT);
                act(AT_ACTION, "$n releases you.", ch, NULL, victim, TO_VICT);
                if ((obj = get_eq_char(victim, WEAR_BINDING)) == NULL)
                        victim->held = FALSE;
                ch->holding = NULL;
                victim->master = NULL;
                victim->heldby = NULL;
                victim->leader = NULL;
                WAIT_STATE(ch, 2);
                WAIT_STATE(victim, 4);
        }
        return;
}
