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
 *                   $Id: homes.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "mud.h"
#include "homes.h"
#include "editor.h"
#include "grid.h"
#include "installations.h"

#ifndef CMDF
#define CMDF void
#endif

struct HOME_ROOM_TYPES {
	char * type;
	char * name;
	int cost;
	char * description;

	EXT_BV flags;

	char * room_desc[5];
};

/* Private Hanger
 * Would need to interface with make_dock somehow
 * needs to mark max # of ships in a doc, so you don't have a small landing pad with 5000 or whatever
 *
 * ship->hanger->add(ship)
 * can_fit checks hanger->max_size is less than hanger->current_capacity
 * if (!ship->hanger->can_fit(ship)) {
 */
#define MAX_HOME_ROOM_TYPES 6
HOME_ROOM_TYPES home_types[MAX_HOME_ROOM_TYPES] = 
{
    {
        "connect", "", 100, "Reconnect rooms", meb(-1), 
        {
                "ERROR",
                "ERROR",
                "ERROR",
                "ERROR"
        }
    },
    { 
        "hallway", "A Hallway", 500, "Basic cheap room that connects other rooms", meb(-1),
        {/* Hallways */
         "The hallway is rather plain and empty.",
	     "The only thing that could really be considered a decoration is the walls, being a calming shade of blue.",
         "Empty, large, and hollow, this hallway looks like it was never used.",
         "The floor is undamaged, and the walls are bare, and the material they are made of is all that can be seen.",
         "A couple pictures are along the walls of the hallway, showing various scenic landscapes.",
/*         "The walls reflect the bright lights in the ceiling illuminating the hallway.",
         "With numerous pictures along this short passage of hallway taking up space in its otherwise plain design, the carpeting in here is particularly comfortable to the feet.",
         "A few pictures here, a painting there, the light above flickering on and off - the hallway could use some sprucing up.",
         "This hallway is no different than any other normal hallway.",
         "It is only a way to connect rooms together.",
         "With numerous pictures along this short passage of hallway taking up space in its otherwise plain design, the carpeting in here is particularly comfortable to the feet.",
         "The white walls reflect the bright lights in the ceiling illuminating the hallway.",
         "A nice and cozy sized bedroom, the walls are pained a light blue color.",
         "The floor is made of wood from the planet Kashyyyk, and a rug is placed under the bed, coming out a couple feet on each side of the bed.",
         "OOOOH, it makes me wonder.",
         "Here we are now, entertain us.",
         "Bring your friends, its fine, bring them to my tent.",
         "Cut my life into pieces, this is my last resort.",
         "Memories come through, like opening the wound.",
         "I don't want to be the one that battles all issues.",
         "I don't know why I instigate, and say what I don't mean.",
         "I'm breaking the habit tonight.",
         "I tightly lock the door, I try to catch ym breath again.",
         "Inside, I realise that I'm the one confuse.",
         "Sweet dreams are made of these."*/
			 },
    },
    { 
        "workshop", "A Workshop", 10000, "A room to do your engineering needs", multimeb(ROOM_FACTORY,ROOM_REFINERY,-1),
        {/* Work Shop */
         "The workbench against the wall has paint splotched on it",
         "There are some tools that looked to have been left behind sprawled out on the bench and floor.",
         "Though generic, there seems to be mass potential in the things that could be made in this small room.",
         "Throughout the plains a wide variety of weeds and wildflowers are scattered.",
		 "This workshop is cylindrical in shape, allowing for easy access to any of the counter space which completely lines the walls in here.",
/*		 "Many different types of tools hang from the ceilings, and there are several large pieces of equipment in the center of the room.",
		 "A large and rather spacious workshop, the walls are lined with various tools, both hand held and electric.",
		 "There is easily enough room for half a dozen people to be working at the same time."*/
		 },
    },
    {
        "kitchen", "A Kitchen", 10000, "COOK COOK COOK", meb(ROOM_CAFE),

        {/* Kitchen */
         "The kitchen is painted an off-white color, but the smells that it emits are far more important.",
         "This kitchen seems to be a great place to make delicious foods.",
         "The kitchen is full of culinary utensils ranging from pots and pans, to knives and spoons.",
         "Wonderful smells always seem to waft from this kitchen.",
		 "Highlighted by the large refrigerator/oven fixture on the far side of the room",
/*		 "This well-sized kitchen has plenty of counter space and sink room.", 
		 "Chopping board, blender, drink-mixers, and a half-bar for anything from preparing a feast to entertaining guests for drinks.",
		 "There are a few cuboards, but other than that, theres not much to look at in here.",
		 "This kitchen appears as though it was designed by someone with alot of credits."*/
		 },
    },
    { 
        "bedroom", "A Bedroom", 10000, "Sleeping quarters", meb(ROOM_HOTEL),
        {/* Bedroom */
         "It has an old bed in the middle of the room, and a couple dressers full of drawers.",
         "The bedroom here is pretty big in comparison to the rest of the house. ",
         "A dresser is placed against the far wall, and a mirror is hung on the other.",
         "This elegant bedroom's focus lies in the king-sized bed centered against the wall on the far side of the room.",
		 "A small nightstand rests next to the bed, with a simple clock/alarm chip sitting ontop of it.",
/*		 "A standard sized bedroom, the full size bed has a plain white bedspread that contrasts the dull gray paint on the walls." */
		 },
    },
    { 
        "office", "An Office", 10000, "A private office", meb(ROOM_OFFICE),

        {/* Office */
         "The office is just another room in the house.",
         "A desk is placed in the middle of the room, and a large window that overlooks the surrounding scenery is behind it.",
         "The desk is equipped with a comm-station and a computer for everyday occupational needs.",
		 "There is one small window in the room, and it seems to get rather hot.",
		 "A number of different-sized and shaped lights about the room, and a conference table capable of seating up to 8 people.",
/*		 "this simple office accomodates you with a medium-sized desk/computer terminal accentuated with a comfortable, adjustable chair",
		 "In the center of the table is a small, rather pricy holo-vid projector capable of projecting up to 4 images into any of the surrounding seats.",
		 "A rather uncomfortable chair can be seen behind the desk, making one almost not want to sit in here."*/
		 }
    }
    /* kitchen, bedroom, office */
};


struct HOME_PLOT_TYPES {
	char * type;
	char * name;
	int cost;
	int cols;
	int rows;
	int height;
};

#define MAX_HOME_PLOT_TYPES 5
HOME_PLOT_TYPES home_plot_types[MAX_HOME_PLOT_TYPES] = {
	{ "smallapt", "A Small Hallway", 500, 1,2,1 },
	{ "largeapt", "A Small Hallway", 500, 2,3,1 },
	{ "condo", "A Condo", 1000, 3,3,3 },
	{ "house", "A house", 10000, 5,3,3 },
	{ "mansion", "A Grand Sized Mansion", 100000, 10, 10, 4}
	/* kitchen, bedroom, office */
};


DECLARE_DO_FUN(do_addroom);

GRID * home_grid_fread args((FILE * fp));
void home_grid_fwrite args((GRID * grid, FILE * fp));
void generate_description(ROOM_INDEX_DATA *room, int type);

HOME_DATA *first_home;
HOME_DATA *last_home;
time_t    save_homes_time;

HOME_DATA::HOME_DATA() 
{
        this->name = NULL;
        this->filename = NULL;
        this->description = NULL;
        this->owner = NULL; /* No need to default this, we have it being checked during loading */
		this->grid = NULL;
		this->next = NULL;
		this->prev = NULL;
		this->price = 0;
}

ROOMMATE_DATA::ROOMMATE_DATA(void)
{
        this->type = 0;
        this->name = NULL;
}

ROOMMATE_DATA::~ROOMMATE_DATA() {
		if (this->name)
				STRFREE(this->name);
}

HOME_DATA::~HOME_DATA()
{
        ROOMMATE_DATA *roomie;
		ROOM_INDEX_DATA * room;

        if (this->filename)
                STRFREE(this->filename);
        if (this->name)
                STRFREE(this->name);
        if (this->description)
                STRFREE(this->description);
        if (this->owner)
                STRFREE(this->owner);
		FOR_EACH_LIST(ROOMMATE_LIST, this->roommates, roomie)
        {
			delete roomie;
        }
		FOR_EACH_LIST(ROOM_LIST, this->rooms, room)
			room->home = NULL;
		UNLINK(this, first_home, last_home, next, prev);
		this->roommates.clear();
		this->rooms.clear();
}

HOME_DATA *get_home(char *name)
{
        HOME_DATA *home;

        for (home = first_home; home; home = home->next)
                if (!str_cmp(name, home->name))
                        return home;

        for (home = first_home; home; home = home->next)
                if (nifty_is_name_prefix(name, home->name))
                        return home;

        return NULL;
}

void fread_roommate(ROOMMATE_DATA * roomie, FILE * fp)
{
        char      buf[MAX_STRING_LENGTH];
        const char *word;
        bool      fMatch;

        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);
                fMatch = FALSE;

                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;

                case 'N':
                        KEY("Name", roomie->name, fread_string(fp));
                        break;

                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!roomie->name)
                                        roomie->name = STRALLOC("Unowned");
                                return;
                        }
                        break;

                case 'O':
                        KEY("Typer", roomie->type, fread_number(fp));
                        break;
                }

                if (!fMatch)
                {
                        snprintf(buf, MSL, "Fread_home: no match: %s", word);
                        bug(buf, 0);
                        boot_log(buf, 0);
                }
        }
}

void fread_home(HOME_DATA * home, FILE * fp)
{
        char      buf[MAX_STRING_LENGTH];
        const char *word;
        bool      fMatch;

        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);
                fMatch = FALSE;

                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;

                case 'D':
                        KEY("Description", home->description,
                            fread_string(fp));
                        break;

                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!home->name)
                                        home->name = STRALLOC(home->filename);
                                if (!home->owner)
                                        home->owner = STRALLOC("Unowned");
                                if (!home->description)
                                        home->description = STRALLOC("");
                                return;
                        }
						/* Temporary */
                        if (!str_cmp(word, "Entrace"))
						{
							ROOM_INDEX_DATA * room;
							int vnum = fread_number(fp);
							room = get_room_index(vnum);
							if (room) 
								home->add(room);
							fMatch = TRUE;
						}
                        break;

                case 'F':
                        KEY("Filename", home->filename, fread_string(fp));
						/* Temporary */
                        if (!str_cmp(word, "Firstroom"))
						{
							ROOM_INDEX_DATA * room;
							int vnum = fread_number(fp);
							room = get_room_index(vnum);
							if (room) 
								home->add(room);
							fMatch = TRUE;
						}
                        break;

                case 'R':
                        if (!str_cmp(word, "Room"))
						{
							ROOM_INDEX_DATA * room;
							int vnum = fread_number(fp);
							room = get_room_index(vnum);
							if (room) 
								home->add(room);
							fMatch = TRUE;
						}
                        break;

                case 'L':
						/* Temporary */
                        if (!str_cmp(word, "Lastroom"))
						{
							ROOM_INDEX_DATA * room;
							int vnum = fread_number(fp);
							room = get_room_index(vnum);
							if (room)
								home->add(room);
							fMatch = TRUE;
						}
                        break;

                case 'N':
                        KEY("Name", home->name, fread_string(fp));
                        break;

                case 'O':
                        KEY("Owner", home->owner, fread_string(fp));
                        break;

                case 'P':
                        KEY("Price", home->price, fread_number(fp));
                        break;

                }

                if (!fMatch)
                {
                        snprintf(buf, MSL, "Fread_home: no match: %s", word);
                        bug(buf, 0);
                        boot_log(buf, 0);
                }
        }
}

/*
 * Load in all the home files.
 */
void load_homes()
{
        FILE     *fpList;
        const char *filename;
        char      homelist[256];
        char      buf[MAX_STRING_LENGTH];

        first_home = NULL;
        last_home = NULL;

        boot_log("Setting current save time");
        save_homes_time = current_time + (HOME_SAVE_TIME);

        snprintf(homelist, 256, "%s%s", HOMEDIR, HOME_LIST);
        FCLOSE(fpReserve);

        if ((fpList = fopen(homelist, "r")) == NULL)
        {
                mkdir(HOMEDIR, 0x755);
                perror(homelist);
                return;
        }

        for (;;)
        {

                filename = feof(fpList) ? "$" : fread_word(fpList);

                if (filename[0] == '$')
                        break;

                if (!load_home_file((char *) filename))
                {
                        snprintf(buf, MSL, "Cannot load home file: %s",
                                 filename);
                        boot_log(buf, 0);
                        bug(buf, 0);
                }

        }
        FCLOSE(fpList);
        boot_log(" Done homes ");
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

/*
 * Load a home file
 */

bool load_home_file(char *homefile)
{
        char      filename[256];
        HOME_DATA *home;
        FILE     *fp;
        bool      found;

        home = new HOME_DATA();

        found = FALSE;
        snprintf(filename, 256, "%s%s", HOMEDIR, homefile);

        if ((fp = fopen(filename, "r")) != NULL)
        {

                found = TRUE;
                for (;;)
                {
                        char      letter;
                        char     *word;

                        letter = fread_letter(fp);
                        if (letter == '*')
                        {
                                fread_to_eol(fp);
                                continue;
                        }

                        if (letter != '#')
                        {
                                bug("Load_home_file: # not found.", 0);
                                boot_log("Load_home_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "HOME"))
                        {
                                fread_home(home, fp);
                                LINK(home, first_home, last_home, next, prev);
                        }
                        else if (!str_cmp(word, "ROOMMATE"))
                        {
                                ROOMMATE_DATA *roommate = new ROOMMATE_DATA();

                                fread_roommate(roommate, fp);
								home->add(roommate);
                        }
                        else if (!str_cmp(word, "OBJECT"))
                        {
                                OBJ_DATA *tobj;
                                OBJ_DATA *tobj_next;
                                ROOM_INDEX_DATA *room;

                                fread_obj(supermob, fp, OS_CARRY);
                                for (tobj = supermob->first_carrying; tobj;
                                     tobj = tobj_next)
                                {
                                        room = get_room_index(tobj->
                                                              room_vnum);
                                        tobj_next = tobj->next_content;
                                        obj_from_char(tobj);
                                        obj_to_room(tobj, room);
                                }
                        }
                        else if (!str_cmp(word, "CORPSE"))
                        {
                                OBJ_DATA *tobj;
                                OBJ_DATA *tobj_next;
                                ROOM_INDEX_DATA *room;

                                fread_obj(supermob, fp, OS_CORPSE);
                                for (tobj = supermob->first_carrying; tobj;
                                     tobj = tobj_next)
                                {
                                        room = get_room_index(tobj->
                                                              room_vnum);
                                        tobj_next = tobj->next_content;
                                        obj_from_char(tobj);
                                        obj_to_room(tobj, room);
                                }
                        }
                        else if (!str_cmp(word, "GRID"))
						{
								if (home->grid) delete home->grid;
                                home->grid = home_grid_fread(fp);
						}
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                char      buf[MAX_STRING_LENGTH];

                                snprintf(buf, MSL,
                                         "Load_home_file: bad section: %s.",
                                         word);
                                bug(buf, 0);
                                boot_log(buf);
                                break;
                        }
                }
                FCLOSE(fp);
        }
        if (!(found))
                DISPOSE(home);
        return found;
}

void fwrite_roommates(FILE * fp, HOME_DATA * home)
{
        ROOMMATE_DATA *roomie = NULL;

        if (home == NULL)
                return;
        if (home->roommates.empty())
                return;

		FOR_EACH_LIST(ROOMMATE_LIST, home->roommates, roomie)
        {
                fprintf(fp, "#ROOMMATE\n");
                fprintf(fp, "Name         %s~\n", roomie->name);
                fprintf(fp, "End\n\n");
        }

        return;
}

void HOME_DATA::save(void)
{
        FILE     *fp;
        char      filename[256];
        char      buf[MAX_STRING_LENGTH];
        OBJ_DATA *contents;

        if (!this->filename || this->filename[0] == '\0')
        {
                snprintf(buf, MSL, "HOME_DATA::save: %s has no filename",
                         this->name);
                bug(buf, 0);
                return;
        }

        snprintf(filename, 256, "%s%s", HOMEDIR, this->filename);

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("HOME_DATA::save fopen", 0);
                perror(filename);
        }
        else
        {
				ROOM_INDEX_DATA * room;
                fprintf(fp, "#HOME\n");
                fprintf(fp, "Name         %s~\n", this->name);
                fprintf(fp, "Filename     %s~\n", this->filename);
                fprintf(fp, "Description  %s~\n", this->description);
                fprintf(fp, "Owner        %s~\n", this->owner);
                fprintf(fp, "Price        %ld\n", this->price);

				FOR_EACH_LIST(ROOM_LIST, this->rooms, room)
				{
					if (room)
						fprintf(fp, "Room    %d\n", room->vnum);
				}

                fprintf(fp, "End\n\n");

				if (this->grid) {
					home_grid_fwrite(this->grid,fp);
				}
                /*
                 * Lets do roomates now 
                 */
                fwrite_roommates(fp, this);
                /*
                 * Lets do Contents 
                 */
                {
						FOR_EACH_LIST(ROOM_LIST, this->rooms, room)
						{
							if (room)
							{
								contents = room->last_content;
								if (contents)
									fwrite_obj(NULL, contents, fp,0, OS_CARRY, TRUE);
							}
                        }
                }
                fprintf(fp, "#END\n");
        }
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        return;
}


CMDF do_sethome(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        HOME_DATA *home;
        int       tempnum, value;
        ROOM_INDEX_DATA *roomindex;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        mudstrlcpy(arg3, argument, MIL);

        value = is_number(arg3) ? atoi(arg3) : -1;

        if (atoi(arg3) < -1 && value == -1)
                value = atoi(arg3);


        if (arg1[0] == '\0' || arg2[0] == '\0' || arg1[0] == '\0')
        {
                send_to_char("Usage: sethome <home> <field> <values>\n\r",
                             ch);
                send_to_char("\n\rField being one of:\n\r", ch);
                send_to_char("filename name owner description \n\r", ch);
                send_to_char("firstroom lastroom entrance\n\r", ch);
                return;
        }

        if ((home = get_home(arg1)) == NULL)
        {
                send_to_char("No such home.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "owner"))
        {
                STRFREE(home->owner);
                home->owner = STRALLOC(argument);
        }
        else if (!str_cmp(arg2, "name"))
        {
                STRFREE(home->name);
                home->name = STRALLOC(argument);
        }
        else if (!str_cmp(arg2, "filename"))
        {
                STRFREE(home->filename);
                home->filename = STRALLOC(argument);
                write_home_list();
        }
        else if (!str_cmp(arg2, "desc"))
        {
                STRFREE(home->description);
                home->description = STRALLOC(argument);
        }
        else if (!str_cmp(arg2, "price"))
        {
                long int  price;

                price = atoi(arg3);
                if (price > 0)
                {
                        home->price = price;
                        ch_printf(ch, "%s is now %ld gold", home->name,
                                  home->price);
                }
                else
                {
                        send_to_char("Bad Price", ch);
                        return;
                }
                return;
        }
        else
        {
                do_sethome(ch, "");
                return;
        }
        send_to_char("Done.\n\r", ch);
		home->save();
        return;
}

CMDF do_homes(CHAR_DATA * ch, char *argument)
{
        HOME_DATA *home;
        ROOM_INDEX_DATA *room;
        int       count;

        argument = NULL;

        if (IS_NPC(ch))
                return;

        count = 0;
        send_to_char
                ("&YThe following homes are owned by you or by your organization:\n\r",
                 ch);
        send_to_char("\n\r&WHome                               Owner\n\r",
                     ch);
        for (home = first_home; home; home = home->next)
        {
                set_char_color(AT_BLUE, ch);
				room = home->rooms.front();

                if (home->rooms.empty())
                {
                        ch_printf(ch,
                                  "&R[&WERROR&R]&B%-35s      %ld to buy &B[&zNo Firstroom on house.&B]\n\r",
                                  home->name, get_home_value(home));
                }
                else if (!home->owner || home->owner[0] == '\0' || !str_cmp(home->owner, "Unowned"))
                {
                        ch_printf(ch,
                                  "&R[&WUnowned&R]&B%-35s      %ld to buy &B[&z%s&B]\n\r",
                                  home->name, get_home_value(home),
                                  room->area->planet ? room->area->planet->
                                  name : "somewhere");
                }
                else if (!str_cmp(home->owner, "Public"))
                {
                        ch_printf(ch,
                                  "&R[&WPublic&R]&B%-35s     (rentcost) &B[&z%s&B]\n\r",
                                  home->name,
                                  room->area->planet ? room->area->planet->
                                  name : "somewhere");
                }
                else if (!str_cmp(home->owner, ch->name))
                {
                        ch_printf(ch, "&R[&WOwner&R]&B%-35s     %s.\n\r",
                                  home->name,
                                  room->area->planet ? room->area->planet->
                                  name : "somewhere");
                }
                else if (home->check_member(ch))
                {
                        ch_printf(ch, "&R[&WOMember&R]&B%-35s     %s.\n\r",
                                  home->name,
                                  room->area->planet ? room->area->planet->
                                  name : "somewhere");
                }
                else if (str_cmp(home->owner, "Unowned"))
                {
                        if (room->area->planet)
                                ch_printf(ch,
                                          "&R[&WOwned&R]&B%-35s      %s.\n\r",
                                          home->name,
                                          room->area->planet->name);
                        else
                                ch_printf(ch,
                                          "&R[&WOwned&R]&B%-35s      %s.\n\r",
                                          home->name, "somewhere");
                }
                else
                        continue;
                count++;
        }

        if (!count)
        {
                send_to_char("There are no homes owned by you.\n\r", ch);
        }
}

void write_home_list()
{
        HOME_DATA *thome;
        FILE     *fpout;
        char      filename[256];

        snprintf(filename, 256, "%s%s", HOMEDIR, HOME_LIST);
        fpout = fopen(filename, "w");
        if (!fpout)
        {
                bug("FATAL: cannot open home.lst for writing!\n\r", 0);
                return;
        }
        for (thome = first_home; thome; thome = thome->next)
                fprintf(fpout, "%s\n", thome->filename);
        fprintf(fpout, "$\n");
        FCLOSE(fpout);
}

long int get_home_value(HOME_DATA * home)
{
        long int  price;

        if (!home->price)
        {
/*                if (home->home_class == APARTMENT_HOME)
                        price = 5000;
                else if (home->home_class == MIDSIZE_HOME)
                        price = 50000;
                else if (home->home_class == GIANT_HOME)
                        price = 500000;
                else*/

				price = 2000;

				price += (home->rooms.size() * 2);

                price *= 10;
        }
        else
                price = home->price;

        return price;
}

bool HOME_DATA::check_member(CHAR_DATA * ch)
{
        ROOMMATE_DATA *roomie = NULL;

		/* this needs removing and moving to a different funciton
		 * like if allowed_access 
        if (!str_cmp("Unowned", this->owner))
                return TRUE;
	     */
        if (IS_NPC(ch) && IS_SET(ch->act, ACT_PET) && ch->master
            && this->check_member(ch->master))
                return TRUE;
        if (!str_cmp(ch->name, this->owner))
                return TRUE;
		if (!this->roommates.empty()) {
			FOR_EACH_LIST(ROOMMATE_LIST, this->roommates, roomie)
				if (!str_cmp(ch->name, roomie->name))
                        return TRUE;
		}
        return FALSE;
}


void HOME_DATA::echo(int color, char *argument)
{
        ROOM_INDEX_DATA * room;
		FOR_EACH_LIST(ROOM_LIST, this->rooms, room)
		{
			if (room)
                echo_to_room(color, room, argument);
        }

}

void HOME_DATA::reset()
{
        ROOMMATE_DATA *roomie, *rm_next;

        if (str_cmp(this->owner, "Public"))
        {
                STRFREE(this->owner);
                this->owner = STRALLOC("Unowned");

				FOR_EACH_LIST(ROOMMATE_LIST, this->roommates, roomie)
                {
						this->roommates.erase(find(this->roommates.begin(), this->roommates.end(), roomie));
                        STRFREE(roomie->name);
                        DISPOSE(roomie);
                }
        }

		this->save();
}

CMDF do_resethome(CHAR_DATA * ch, char *argument)
{
        HOME_DATA *home;

        home = get_home(argument);
        if (home == NULL)
        {
                send_to_char("&RNo such home!", ch);
                return;
        }
		home->reset();
}

/***************************************************************************
**      MORTAL COMMANDS TODO WITH THE NEW HOMES CODE                      **
**      This involves things like buyhome and sell home                   **
****************************************************************************/

CMDF do_sellhome2(CHAR_DATA * ch, char *argument)
{
        long      price;
        HOME_DATA *home;
        ROOM_INDEX_DATA *room;

        if (IS_NPC(ch) || !ch->pcdata)
        {
                send_to_char("&ROnly players can do that!\n\r", ch);
                return;
        }

        home = ch->in_room->home;
        if (!home)
        {
                act(AT_PLAIN, "I see no home here.", ch, NULL, argument,
                    TO_CHAR);
                return;
        }
        room = ch->in_room;

        if (!home->check_member(ch)
            || !str_cmp(home->owner, "Public")
            || !str_cmp(home->owner, "Unowned"))
        {
                send_to_char("&RHey, thats not your home!\n\r", ch);
                return;
        }

        price = get_home_value(home);

        ch->gold += (price - price / 10);
        ch_printf(ch, "&GYou receive %ld credits from selling your home.\n\r",
                  price - price / 10);

        act(AT_PLAIN,
            "$n walks over to a terminal and makes a credit transaction.", ch,
            NULL, argument, TO_ROOM);
		home->reset();
}

CMDF do_buyhome2(CHAR_DATA * ch, char *argument)
{
        long      price;
        HOME_DATA *home;
        ROOM_INDEX_DATA *room;
        AREA_DATA *pArea;

        argument[0] = '\0';
        if (IS_NPC(ch) || !ch->pcdata)
        {
                send_to_char("&ROnly players can do that!\n\r", ch);
                return;
        }

        home = ch->in_room->home;
        if (home)
        {
                act(AT_PLAIN, "I see no home here, please goto the entrance.",
                    ch, NULL, argument, TO_CHAR);
                return;
        }
        room = ch->in_room;

        for (pArea = first_bsort; pArea; pArea = pArea->next_sort)
        {
                if (room->area == pArea)
                {
                        send_to_char("&RThis area isn't installed yet!\n\r&w",
                                     ch);
                        return;
                }
        }

        if (str_cmp(home->owner, "Unowned"))
        {
                send_to_char("&RThat home isn't for sale!\n\r", ch);
                return;
        }
        price = get_home_value(home);
        if (ch->gold < price)
        {
                ch_printf(ch,
                          "&RThis home costs %ld. You don't have enough credits!\n\r",
                          price);
                return;
        }

        ch->gold -= price;
        ch_printf(ch, "&G%s pays %ld credits to purchase this fine home.\n\r",
                  ch->name, price);

        act(AT_PLAIN,
            "$n walks over to a terminal and makes a credit transaction.", ch,
            NULL, argument, TO_ROOM);
        if (home->owner)
                STRFREE(home->owner);
        home->owner = STRALLOC(ch->name);

		home->save();
}

CMDF do_roommate(CHAR_DATA * ch, char *argument)
{
        HOME_DATA *home;
        char      arg1[MAX_INPUT_LENGTH];
        ROOM_INDEX_DATA *location;
        AREA_DATA *pArea;

        argument = one_argument(argument, arg1);
        if (!ch->desc)
        {
                send_to_char("You have no descriptor.\n\r", ch);
                return;
        }
        switch (ch->substate)
        {
        default:
                break;
        case SUB_ROOM_DESC:
                location = (ROOM_INDEX_DATA *) ch->dest_buf;
                if (!location)
                {
                        bug("redit: sub_room_desc: NULL ch->dest_buf", 0);
                        location = ch->in_room;
                }
                STRFREE(location->description);
                location->description = copy_buffer(ch);
                stop_editing(ch);
                ch->substate = ch->tempnum;
                for (pArea = first_bsort; pArea; pArea = pArea->next_sort)
                {
                        if (location->area == pArea)
                        {
                                send_to_char
                                        ("&RThis area isn't installed yet!\n\r&w",
                                         ch);
                                return;
                        }
                }
                fold_area(location->area, location->area->filename, FALSE,
                          FALSE);
                return;
        }

        location = ch->in_room;
        if (arg1[0] == '\0' || !str_cmp(arg1, "?"))
        {
                send_to_char("Syntax: roommate <field> value\n\r", ch);
                send_to_char("\n\r", ch);
                send_to_char("Field being one of:\n\r", ch);
                send_to_char("  add remove list\n\r", ch);
                return;
        }

        if (arg1[0] == '\0')
        {
                send_to_char
                        ("Syntax:\n\r\tRoommate [add/remove] [roommate name]\n\r",
                         ch);
                return;
        }
        if ((home = ch->in_room->home) == NULL)
        {
                send_to_char
                        ("&G&WYou must be in the entrance of the home to do that!\n\r",
                         ch);
                return;
        }

        /*
         * Only Owners can add roommates? 
         */
        if (!home->check_member(ch)
            || !str_cmp(home->owner, "Public")
            || !str_cmp(home->owner, "Unowned"))
        {
                send_to_char("&RThis isn't your house!", ch);
                return;
        }

        if (!str_cmp("add", arg1))
        {
                ROOMMATE_DATA *roommate;

                if (argument[0] == '\0')
                {
                        send_to_char("&RAdd whom?\n\r", ch);
                        return;
                }
				FOR_EACH_LIST(ROOMMATE_LIST, home->roommates, roommate)
                {
                        if (roommate == NULL || roommate->name == NULL)
                                continue;
                        if (!str_cmp(argument, roommate->name))
                                break;
                }

                if (roommate == NULL)
                {
                        /*
                         * Now add a roommate 
                         */
                        roommate = new ROOMMATE_DATA();
                        roommate->name = STRALLOC(capitalize(argument));
						home->add(roommate);
                }
                send_to_char("Done.", ch);
        }
        else if (!str_cmp("remove", arg1) || !str_cmp("del", arg1))
        {
                bool      found = FALSE;
                ROOMMATE_DATA *roomie = NULL;

                if (argument[0] == '\0')
                {
                        send_to_char("&BR&zemove which roommate?", ch);
                        return;
                }

				FOR_EACH_LIST(ROOMMATE_LIST, home->roommates, roomie)
                {
                        if (!str_cmp(argument, roomie->name))
                        {
								home->remove(roomie);
                                STRFREE(roomie->name);
                                DISPOSE(roomie);
                                found = TRUE;
                                break;
                        }
                }
                if (found == FALSE)
                {
                        send_to_char
                                ("&BT&zhat person isn't listed as a roommate.",
                                 ch);
                        return;
                }
        }
        else if (!str_cmp(arg1, "list") || !str_cmp(arg1, "info"))
        {
                int       count = 0;
                ROOMMATE_DATA *roomie = NULL;

                send_to_char("&BR&zoommates:\n\r&B----------\n\r", ch);
				FOR_EACH_LIST(ROOMMATE_LIST, home->roommates, roomie)
                {
                        if (roomie == NULL || roomie->name == NULL)
                                continue;
                        count++;
                        ch_printf(ch, "\t&B%d&z} &B%c&z%s\n\r", count,
                                  UPPER(roomie->name[0]), roomie->name + 1);
                }
                if (count == 0)
                {
                        send_to_char("\t&zNone.\n\r", ch);
                }
                return;
        }
        else
        {
                send_to_char("What?", ch);
                return;
        }
		home->save();
        return;
}

CMDF do_freehomes(CHAR_DATA * ch, char *argument)
{
	HOME_DATA *home;
	ROOM_INDEX_DATA *room;
	int       count = 0;
	int       category;

	argument[0] = '\0';

	if (IS_NPC(ch))
		return;

	send_to_char("&Y\n\rThe following homes are for sale:\n\r", ch);

	for (home = first_home; home; home = home->next)
	{
		if (home->rooms.empty())
			continue;
		set_char_color(AT_BLUE, ch);

		if (!str_cmp(home->owner, "Unowned"))
		{
			if (room->area->planet)
			{
				ch_printf(ch,
						"%-34s %ld to buy \t%s.\n\r",
						home->name,
						get_home_value(home),
						room->area->planet->name);
			}
			else
			{
				ch_printf(ch,
						"%-34s %ld to buy \t%s.\n\r",
						home->name,
						get_home_value(home),
						"somewhere");
			}
			count++;
		}
		else
			continue;
	}
	if (!count)
		send_to_char
			("There are no homes currently for sale.\n\r",
			 ch);
}

void save_homes_check()
{
        HOME_DATA *home = NULL;

        if (save_homes_time > current_time)
                return;

        for (home = first_home; home; home = home->next)
			home->save();

        /*
         * 60 seconds * 20 minutes 
         */
        save_homes_time = current_time + (HOME_SAVE_TIME);
}

#if 0
/* FIXME - Later */
CMDF do_rap(CHAR_DATA * ch, char *argument)
{
        EXIT_DATA *pexit;
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_INPUT_LENGTH];

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Rap on what?\n\r", ch);
                return;
        }
        if (ch->fighting)
        {
                send_to_char
                        ("You have better things to do with your hands right now.\n\r",
                         ch);
                return;
        }
        if ((pexit = get_exit(ch->in_room, get_dir(arg))) != NULL)
        {
                ROOM_INDEX_DATA *to_room;
                EXIT_DATA *pexit_rev;
                char     *keyword;

                if ((to_room = pexit->to_room) != NULL
                    && xIS_SET(to_room->room_flags, ROOM_EMPTY_HOME))
                {
                        send_to_char
                                ("No Need to use the intercom, nobody lives there!\n\r",
                                 ch);
                        return;
                }
                else if ((to_room = pexit->to_room) != NULL
                         && !xIS_SET(to_room->room_flags, ROOM_PLR_HOME)
                         && !xIS_SET(to_room->room_flags, ROOM_HOUSE))
                {
                        send_to_char("Nobody Owns That Home!\n\r", ch);
                        return;
                }
                keyword = capitalize(dir_name[pexit->vdir]);
                act(AT_ACTION, "You use the intercom to the $d.", ch, NULL,
                    keyword, TO_CHAR);
                act(AT_ACTION, "$n uses the intercom to the $d.", ch, NULL,
                    keyword, TO_ROOM);
                if ((to_room = pexit->to_room) != NULL
                    && (pexit_rev = pexit->rexit) != NULL
                    && pexit_rev->to_room == ch->in_room)
                {
                        snprintf(buf, MIL,
                                 "%s uses the intercom from outside!\n\r",
                                 ch->name);
                        echo_to_room(AT_ACTION, to_room, buf);
                        ch->buzzedfrom = ch->in_room->vnum;
                        ch->buzzed = pexit->to_room->vnum;
                }
        }
        else
        {
                send_to_char("There is no home there!\n\r", ch);
        }
        return;
}
#endif

#if 0
needs buzzed field into mud.h CMDF do_invite(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        ROOM_INDEX_DATA *room;
        HOME_DATA *home;
        CHAR_DATA *rch;

        one_argument(argument, arg);

        if (IS_NPC(ch))
                return;

        if (arg[0] == '\0')
        {
                send_to_char("invite whom?\n\r", ch);
                return;
        }
        if (ch->fighting)
        {
                send_to_char
                        ("You have better things to do with your hands right now.\n\r",
                         ch);
                return;
        }

        if ((home = home_from_entrance(ch->in_room->vnum)) == NULL)
        {
                send_to_chaR("This is not a house", ch);
                return;
        }

        if (home && !check_member(ch, home))
        {
                send_to_char("You do not own this home", ch);
                return;
        }

        rch = get_char_world(ch, arg);
        if (!rch)
        {
                send_to_char("There is no-one there!\n\r", ch);
                return;
        }
        if (!rch->buzzed || !rch->buzzedfrom)
                return;
        if (rch->buzzed != ch->in_room->vnum
            || rch->buzzedfrom != rch->in_room->vnum)
        {
                ch_printf(ch, "%s doesn't want in!\n\r", he_she[rch->sex]);
                return;
        }

        room = ch->in_room;
        ch_printf(rch, "%s invites you in!\n\r", ch->name);
        ch_printf(ch, "You invite %s in!\n\r", rch->name);

        char_from_room(rch);
        char_to_room(rch, ch->in_room);

        act(AT_ACTION, "$N gets invited into $n's room!", ch, NULL, rch,
            TO_NOTVICT);

        return;
}
#endif

GRID * home_grid_fread( FILE * fp)
{
	int width = fread_number(fp);
	int length = fread_number(fp);
	int height = fread_number(fp);
	int base = fread_number(fp);
	const char *word;
	bool      fMatch;

	GRID * grid = new GRID(base,width,length,height);
	for (;;)
	{
		word = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		switch (UPPER(word[0]))
		{
			case '*':
				fMatch = TRUE;
				fread_to_eol(fp);
				break;

			case 'E':
				if (!str_cmp(word, "End"))
				{
					return grid;
				}
				break;

			case 'C':
				if (!str_cmp(word, "Coord"))
				{
					char line[100];
					int column, row, height, roomvnum;
					fgets(line, 100, fp);
					sscanf(line, "%d %d %d - %d", &column, &row, &height, &roomvnum);
					grid->set(column,row,height,get_room_index(roomvnum));
					fMatch = TRUE;
					break;
				}
				break;
		}

		if (!fMatch)
			bug("Fread_installations: no match: %s", word);
	}
	delete grid;
	grid = NULL;
	return NULL;
}


void home_grid_fwrite(GRID * grid, FILE * fp)
{
	fprintf(fp, "#GRID\n%d\n%d\n%d\n%d\n", grid->width(),grid->length(),grid->height(),grid->base());
	for (int height = 0; height < grid->height(); height++) {
		for (int row = 0; row < grid->width(); row++) {
			for (int column = 0; column < grid->length(); column++) {
				ROOM_INDEX_DATA * room = (ROOM_INDEX_DATA *) grid->get(column,row,height);
				if (!room) continue;
				fprintf(fp, "Coord %d %d %d - %d\n", column,row,height,room->vnum);
			}
		}
	}
	fprintf(fp, "End\n\n");
	return;
}

CMDF do_transferhouse(CHAR_DATA * ch, char * argument)
{
	CHAR_DATA *victim;
	HOME_DATA *home;
	if (argument[0] == '\0' || (victim = get_char_room(ch, argument)) == NULL)
	{
		send_to_char("They aren't here.\n\r", ch);
		return;
	}

	if (victim == ch)
	{
		send_to_char("...\n\r", ch);
		return;
	}

	if (ch->in_room->home) {
		send_to_char("Which house?\n\r", ch);
		return;
	}

	home = ch->in_room->home;
	if (!str_cmp(ch->name, home->owner))
	{
		send_to_char("This is not your house.\n\r", ch);
		return;
	}

	STRFREE(home->owner);
	home->owner = STRALLOC(victim->name);
	send_to_char("Ok.\n\r", ch);
}	


CMDF do_realitor(CHAR_DATA * ch, char * argument) 
{
	OBJ_DATA *obj;
	HOME_DATA * home;
	char      buf[MAX_STRING_LENGTH];
	char      arg[MAX_INPUT_LENGTH];
	int       percent, xp, amount, percentage;
	int 	  plot_type;

	if (IS_NPC(ch))
		return;

	mudstrlcpy(arg, argument, MIL);

	if (ch->mount)
	{
		send_to_char("You can't do that while mounted.\n\r",
				ch);
		return;
	}
	if (ms_find_obj(ch))
		return;

	if (ch->position == POS_FIGHTING)
	{
		send_to_char("You can't do that while fighting.\n\r",
				ch);
		return;
	}

	if (ch->position <= POS_SLEEPING)
	{
		send_to_char("In your dreams or what?\n\r", ch);
		return;
	}

	if (!xIS_SET(ch->in_room->room_flags,ROOM_EMPTYPLOT) || ch->in_room->home) {
		send_to_char("This isn't an empty land plot.\n\r",ch);
		return;
	}

	switch (ch->substate)
	{
		default:

			if (argument[0] == '\0') {
				send_to_char("The realitor computer provides you with the following zones you can zone this land as:\n\r", ch);
				for (plot_type = 0; plot_type < MAX_HOME_PLOT_TYPES; plot_type++) {
					ch_printf(ch, "&W\t%-15s\t%-6d\t%d x %d x %d\n\r", 
							home_plot_types[plot_type].type,
							home_plot_types[plot_type].cost,
							home_plot_types[plot_type].cols,
							home_plot_types[plot_type].rows,
							home_plot_types[plot_type].height
							);
				}
				return;
			}

			for (plot_type = 0; plot_type < MAX_HOME_PLOT_TYPES; plot_type++) {
				if (!str_cmp(argument, home_plot_types[plot_type].type))
					break;
			}
			if (plot_type == MAX_HOME_PLOT_TYPES) 
			{
				send_to_char("&RNo such zone type.\n\rZone Classifications are:\n\r", ch);
				for (plot_type = 0; plot_type < MAX_HOME_PLOT_TYPES; plot_type++) {
					ch_printf(ch, "&W\t%-15s\t%-6d\t%d x %d x %d\n\r", 
							home_plot_types[plot_type].type,
							home_plot_types[plot_type].cost,
							home_plot_types[plot_type].cols,
							home_plot_types[plot_type].rows,
							home_plot_types[plot_type].height
							);
				}
				return;
			}

			if (ch->gold < home_plot_types[plot_type].cost )
			{
				send_to_char("You haven't got the money for a that!\r\n",ch);
				return;
			}

			percentage = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_realitor]);
			if (number_percent() < percentage)
			{
				send_to_char
					("&GYou sit down and start to make transactions.\n\r",
					 ch);
				act(AT_PLAIN,
						"$n sits down and starts to make transactions.",
						ch, NULL, argument, TO_ROOM);
				add_timer(ch, TIMER_DO_FUN, 11, do_realitor, 1);
				ch->dest_buf = str_dup(arg);
				return;
			}
			send_to_char("&RYou can't figure out the governmental estate system.\n\r",
					ch);
			learn_from_failure(ch, gsn_realitor);
			return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
				argument = arg;
                DISPOSE(ch->dest_buf);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to make your transactions.\n\r",
                         ch);
                return;

	}

	ch->substate = SUB_NONE;
	
	for (plot_type = 0; plot_type < MAX_HOME_PLOT_TYPES; plot_type++) {
		if (!str_cmp(argument, home_plot_types[plot_type].type))
			break;
	}

	if (plot_type == MAX_HOME_PLOT_TYPES) 
	{
		send_to_char("&RNo such zone type.\n\rZone Classifications are:\n\r", ch);
		for (plot_type = 0; plot_type < MAX_HOME_PLOT_TYPES; plot_type++) {
			ch_printf(ch, "&W\t%-15s\t%-6d\t%d x %d x %d\n\r", 
					home_plot_types[plot_type].type,
					home_plot_types[plot_type].cost,
					home_plot_types[plot_type].cols,
					home_plot_types[plot_type].rows,
					home_plot_types[plot_type].height
					);
		}
		return;
	}
	
	if (ch->gold < home_plot_types[plot_type].cost )
	{
		send_to_char("You haven't got the money for a that!\r\n",ch);
		return;
	}
	ch->gold -= home_plot_types[plot_type].cost;
	/* Should be a bit of a cost modifier based on chance - FIXME */

	percent = number_percent() - ch->skill_level[OCCUPATION_ABILITY];

	if (percent > ch->pcdata->learned[gsn_realitor])
	{
		send_to_char("You try to buy the land and rezone, but instead, you end up putting the money into a governmental pension plan!\n\r",ch);
		act(AT_ACTION,"There is a beep on the computer as $n trys to make transactions, but makes a mistake!\n\r",ch, NULL, ch, TO_ROOM);
		learn_from_failure(ch, gsn_realitor);
		return;
	}



	home = new HOME_DATA();
	if (home->owner)
		STRFREE(home->owner);
	if (home->name)
		STRFREE(home->name);
	if (home->filename)
		STRFREE(home->filename);
	home->name = STRALLOC(home_plot_types[plot_type].name);

	/*
	 * Lets make this an occupational skill
	 * and then addroom or buildhouse will go on top of this, as an engineering skill
	 * its good to have teamwork.
	 *
	 * that way you can do "claimplot 3 3 3" and the plot is claimed.
	 *
	 * Plots can be used to build homes or installations
	 */
	home->grid = new GRID(0,home_plot_types[plot_type].cols,home_plot_types[plot_type].rows,home_plot_types[plot_type].height);
	if (home_plot_types[plot_type].height > 1) 
		home->grid->set(0,0,1,ch->in_room);
	else
		home->grid->set(0,0,0,ch->in_room);
	snprintf(buf, MSL, "%d.home", ch->in_room->vnum);
	home->filename = STRALLOC(buf);
	home->owner = STRALLOC(ch->name);
	home->description = STRALLOC("");
	home->add(ch->in_room);
	LINK(home, first_home, last_home, next, prev);
	home->save();
	write_home_list();

	xREMOVE_BIT(ch->in_room->room_flags, ROOM_EMPTYPLOT);
	ch->in_room->sector_type = SECT_INSIDE;
	xSET_BIT(ch->in_room->room_flags, ROOM_INDOORS);
	if (!ch->in_room->name)
		STRFREE(ch->in_room->name);
	ch->in_room->name = STRALLOC("Entrance Way");
	fold_area(ch->in_room->area, ch->in_room->area->filename, TRUE, FALSE);
	/* house created */

	send_to_char("The land is now yours, and has been rezoned!\n\r", ch);
	learn_from_success(ch, gsn_realitor);
	xp = UMIN(amount * 10,
			(exp_level(ch->skill_level[OCCUPATION_ABILITY] + 1) -
			 exp_level(ch->skill_level[OCCUPATION_ABILITY])));
	gain_exp(ch, xp, OCCUPATION_ABILITY);
	ch_printf(ch, "&WYou gain %ld occupation experience points!\n\r", xp);
	return;
}

void HOME_DATA::decorate_room(CHAR_DATA * ch, char * argument)
{
	char      arg[MAX_INPUT_LENGTH];
	
	if (!ch->in_room->home || !ch->in_room->home->check_member(ch)) {
		send_to_char("I don't think you have permission.\n\r", ch);
		return;
	}

	switch (ch->substate)
	{
		default:
			break;
		case SUB_ROOM_DESC:
			ROOM_INDEX_DATA * location;
			location = (ROOM_INDEX_DATA *) ch->dest_buf;
			if (!location)
			{
				bug("redit: sub_room_desc: NULL ch->dest_buf", 0);
				location = ch->in_room;
			}
			STRFREE(location->description);
			location->description = copy_buffer(ch);
			stop_editing(ch);
			ch->substate = ch->tempnum;
			return;
	}

	arg[0] = '\0';
	argument = one_argument(argument, arg);

	if (arg[0] == '\0' || ch->in_room->home == NULL) {
		send_to_char("Usage: decorateroom <name/desc>\n\r",ch);
		return;
	}

	if (!str_cmp(arg, "name")) {
		if (argument[0] == '\0')
		{
				send_to_char
						("Set the room name.  A very brief single line room description.\n\r",
						 ch);
				send_to_char("Usage: decorateroom name <Room summary>\n\r",
							 ch);
				return;
		}
		if (ch->in_room->name)
			STRFREE(ch->in_room->name);
		ch->in_room->name = STRALLOC(argument);
		return;
	}
	else if (!str_cmp(arg, "desc")) {
		ch->tempnum = SUB_NONE;
		ch->substate = SUB_ROOM_DESC;
		ch->dest_buf = ch->in_room;
		start_editing(ch, ch->in_room->description);
		return;
	}
	else {
		send_to_char("What?\n\r", ch);
		return;
	}

}
/**
 */
int home_grid_dir(char * direction, int * col, int * row, int * height) 
{
	/* 
	 * get_dir returns:
	 DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN,
	 DIR_NORTHEAST, DIR_NORTHWEST, DIR_SOUTHEAST, DIR_SOUTHWEST,
	 DIR_SOMEWHERE
	 */
	int dir = get_dir(direction);
	switch (dir) {
		case DIR_SOUTH:
			*row += 1;
			break;
		case DIR_NORTH:
			*row -= 1;
			break;
		case DIR_WEST:
			*col -= 1;
			break;
		case DIR_EAST:
			*col += 1;
			break;
		case DIR_UP:
			*height -= 1;
			break;
		case DIR_DOWN:
			*height += 1;
			break;
		case DIR_NORTHWEST:
			*row -= 1;
			*col -= 1;
			break;
		case DIR_NORTHEAST:
			*row -= 1;
			*col += 1;
			break;
		case DIR_SOUTHEAST:
			*row += 1;
			*col += 1;
			break;
		case DIR_SOUTHWEST:
			*row += 1;
			*col -= 1;
			break;
		default:
			return -1;
	}
	return dir;
}
void HOME_DATA::add_room(CHAR_DATA * ch, char * argument)
{
	char      arg[MAX_INPUT_LENGTH];
	char      arg2[MAX_INPUT_LENGTH];
	HOME_DATA*home;
	int        dir;
	int 	  col,row,height,pos;
	int       percent, xp, amount, percentage;
	int 	  room_type;
	int	      vnum;
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA * newroom;

	home = ch->in_room->home;
	/* Is this too lax? - Gavin */
	if (!home->check_member(ch)) {
		send_to_char("I don't think you have permission.\n\r", ch);
		return;
	}

	if (!home->grid) {
		/* FIXME - spell check */
		send_to_char("This building isn't modifiable sorry.\n\r", ch);
		return;
	}

	col = row = height = 0;
	pos = home->grid->find(ch->in_room,&col,&row,&height);
	if (pos == -1) {
		send_to_char("This building is bugged. Please contact a coder.\n\r",ch);
		return;
	}
	//home->grid->translate(pos,&col,&row,&height);

	switch (ch->substate)
	{
		default:
			arg[0] = '\0';
			if (argument[0] != '\0')
				argument = one_argument(argument, arg);

			if (argument[0] == '\0' || arg[0] == '\0' || ch->in_room->home == NULL) {
				send_to_char("Usage: addroom <direction> <type>\n\r",ch);
				send_to_char("Types are:\n\r", ch);
				send_to_char("Type\tCost\tDescrition\n\r",ch);
				/*
				 * this should be in a const array(?)
				 * maybe olc'd so we can add autodescs
				 */
				for (room_type = 0; room_type < MAX_HOME_ROOM_TYPES; room_type++) {
					ch_printf(ch, "&W\t%-15s\t%-6d\t%s\n\r", 
							home_types[room_type].type,
							home_types[room_type].cost,
							home_types[room_type].description
							);
				}
				return;
			}

			if ((dir = home_grid_dir(arg, &col,&row,&height)) == -1) {
				send_to_char("Which direction is that?\n\r", ch);
				return;
			}

			if (!home->grid->valid(col,row,height)) {
				send_to_char("Can't add on in that direction.\n\r",ch);
				return;
			}

			if (get_exit(ch->in_room, dir))
			{
				send_to_char("There is already an exit in that direction\r\n",ch);
				return;
			}

			for (room_type = 0; room_type < MAX_HOME_ROOM_TYPES; room_type++) {
				if (!str_cmp(argument, home_types[room_type].type))
					break;
			}

			if (room_type == MAX_HOME_ROOM_TYPES) 
			{
				send_to_char("&RNo such room type.\n\rOptions are:\n\r", ch);
				/*
				 * this should be in a const array(?)
				 * maybe olc'd so we can add autodescs
				 */
				for (room_type = 0; room_type < MAX_HOME_ROOM_TYPES; room_type++) {
					ch_printf(ch, "&W\t%-15s\t%-6d\t%s\n\r", 
							home_types[room_type].type,
							home_types[room_type].cost,
							home_types[room_type].description
							);
				}
				return;
			}
			if (ch->gold < home_types[room_type].cost )
			{
				send_to_char("You haven't got the money for a that!\r\n",ch);
				return;
			}
			percentage = IS_NPC(ch) ? ch->top_level : (int) (ch->pcdata->learned[gsn_roomconstruction]);
			if (number_percent() < percentage)
			{
				send_to_char("&GYou start the construction of a new room.\n\r",ch);
				act(AT_PLAIN,"$n starts building.", ch, NULL, argument, TO_ROOM);
				add_timer(ch, TIMER_DO_FUN, 11, do_addroom, 1);
				ch->dest_buf = str_dup(arg);
				ch->dest_buf_2 = str_dup(argument);
				return;
			}
			send_to_char("&RYou can't figure out what the next step is.\n\r", ch);
			learn_from_failure(ch, gsn_roomconstruction);
			return;


        case 1:
                if (!ch->dest_buf)
                        return;
                mudstrlcpy(arg, (char *) ch->dest_buf, MIL);
                mudstrlcpy(arg2, (char *) ch->dest_buf_2, MIL);
                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                break;

        case SUB_TIMER_DO_ABORT:
                DISPOSE(ch->dest_buf);
                DISPOSE(ch->dest_buf_2);
                ch->substate = SUB_NONE;
                send_to_char
                        ("&RYou are interupted and fail to make your transactions.\n\r",
                         ch);
                return;

	}

	ch->substate = SUB_NONE;

	if ((dir = home_grid_dir(arg, &col,&row,&height)) == -1) {
		send_to_char("Which direction is that?\n\r", ch);
		return;
	}
	if (!home->grid->valid(col,row,height)) {
		send_to_char("Can't add on in that direction.\n\r",ch);
		return;
	}

	if (get_exit(ch->in_room, dir))
	{
		send_to_char("There is already an exit in that direction\r\n",ch);
		return;
	}

	for (room_type = 0; room_type < MAX_HOME_ROOM_TYPES; room_type++) {
		if (!str_cmp(arg2, home_types[room_type].type))
			break;
	}

	if (room_type == MAX_HOME_ROOM_TYPES) 
	{
		send_to_char("&RNo such room type.\n\r How'd you get this far?\n\r",ch);
		bug("%s got to second room check using args (%s %s)", ch->name, arg, arg2);
		return;
	}
	if (ch->gold < home_types[room_type].cost )
	{
		send_to_char("You haven't got the money for a that!\r\n",ch);
		return;
	}

	/* Find the room if it exists */
	newroom = (ROOM_INDEX_DATA *) home->grid->get(col,row,height);
	if (!newroom) {
		vnum = find_pvnum_block(1,ch->in_room->area->filename);
		if (vnum == -1)
		{
			send_to_char("This location is full right now, try again later.\n\r", ch);
			return;
		}

		/* Only special case */
		if (!str_cmp(home_types[room_type].type, "connect")) {
			send_to_char("Can't do that, need to make a room there first.\n\r", ch);
			return;
		}
	}
	else {
		if (str_cmp(home_types[room_type].type, "connect")) {
			send_to_char("A room already exists in that direction. How about trying to connnect it instead.\n\r", ch);
			return;
		}

		if (ch->gold < home_types[room_type].cost )
		{
			send_to_char("You haven't got the money for a that!\r\n",ch);
			return;
		}
	}
	ch->gold -= home_types[room_type].cost;

	percent = number_percent() - ch->skill_level[ENGINEERING_ABILITY];

	if (!newroom) {
		if (percent > ch->pcdata->learned[gsn_roomconstruction])
		{
			send_to_char("The walls come crumbling down as you make a drastic mistake. You luckly make an escape from that room.!\n\r",ch);
			act(AT_ACTION,"$n comes running back from the new room just before it collapses!\n\r",ch, NULL, ch, TO_ROOM);
			learn_from_failure(ch, gsn_roomconstruction);
			return;
		}
		/* fixme - find empty vnums in this area */
		newroom = make_room(vnum, ch->in_room->area);
		newroom->area = ch->in_room->area;
		newroom->sector_type = SECT_INSIDE;
		xSET_BIT(newroom->room_flags, ROOM_INDOORS);
		xSET_BITS(newroom->room_flags, home_types[room_type].flags);
		home->grid->set(col,row,height,newroom);
		home->add(newroom);

		if (!newroom->name)
			STRFREE(newroom->name);
		newroom->name = STRALLOC(home_types[room_type].name);
		generate_description(newroom, room_type);

	}
	else {
		if (percent > ch->pcdata->learned[gsn_roomconstruction])
		{
			send_to_char("You are luckly you didn't damage the other room with your mistake..!\n\r",ch);
			act(AT_ACTION,"$n makes a mistake the the new doorway is damaged.!\n\r",ch, NULL, ch, TO_ROOM);
			learn_from_failure(ch, gsn_roomconstruction);
			return;
		}
		/* assume the room exists, delete the exists dealing with this room (FIXME) */
		/* somehow skip the type flag too if the room exists */
		/* maybe connect type? */
		//send_to_char("Warning:: Room already exists in that direction, and you didn't use the connect room type.\n\r",ch);
		if ((pexit = get_exit(newroom, rev_dir[dir])) != NULL)
			extract_exit(newroom, pexit);
	}

	/* figure this out too */
	pexit = make_exit(ch->in_room, newroom, dir);
	pexit->keyword = STRALLOC("");
	pexit->description = STRALLOC("");
	pexit->key = -1;
	pexit = make_exit(newroom, ch->in_room,rev_dir[dir]);
	pexit->keyword = STRALLOC("");
	pexit->description = STRALLOC("");
	pexit->key = -1;

	fold_area(ch->in_room->area, ch->in_room->area->filename, TRUE, FALSE);
	home->save();
	send_to_char("Ok.\n\r",ch);
	char_from_room(ch);
    char_to_room(ch, newroom);
	learn_from_success(ch, gsn_roomconstruction);
	xp = UMIN(amount * 10,
			(exp_level(ch->skill_level[ENGINEERING_ABILITY] + 1) -
			 exp_level(ch->skill_level[ENGINEERING_ABILITY])));
	gain_exp(ch, xp, ENGINEERING_ABILITY);
	ch_printf(ch, "&WYou gain %ld engineering experience points!\n\r", xp);
	return;
}

CMDF do_homestat (CHAR_DATA * ch, char * argument)
{
	ROOMMATE_DATA * roomie;
	if (!ch->in_room->home) {
		return;
	}
	HOME_DATA * home = ch->in_room->home;

	ch_printf(ch, "%s\n\rFilename: %s\n\r",
			home->name, home->filename);
	ch_printf(ch, "Description: %s\n\rOwner: %s\n\r", 
			home->description,
			home->owner);
	ch_printf(ch, "Roommates: %d\n\rRooms: %d\n\r", 
			home->roommates.size(),
			home->rooms.size()
			);
	send_to_char("Roomies:\n\r", ch);
	FOR_EACH_LIST(ROOMMATE_LIST, home->roommates, roomie)
		ch_printf(ch, "\t\t%s\n\r", roomie->name);
}

void generate_description(ROOM_INDEX_DATA *room, int type)
{
        char      buf[MAX_STRING_LENGTH];
        char      buf2[MAX_STRING_LENGTH];
        int       nRand;
        int       iRand, len;
        int       previous[8];
        int       sector;
        int       num_descs = 0;


        if (!room)
        {
                bug("Null room passes: %s %s", __FUNCTION__, __LINE__);
                return;
        }

        if (type >= MAX_HOME_ROOM_TYPES) {
            bug("invalid type: %d", type);
            return;
        }
        if (room->description)
                STRFREE(room->description);

        num_descs = NUMITEMS(home_types[type].room_desc);

        buf[0] = '\0';
        nRand = number_range(1, UMIN(8, num_descs));//Hard Coded, don't want 25, want how many are there

        for (iRand = 0; iRand < nRand; iRand++)
                previous[iRand] = -1;

        for (iRand = 0; iRand < nRand; iRand++)
        {
                while (previous[iRand] == -1)
                {
                        int       x, z;

                        x = number_range(0, num_descs);//Hard Coded

                        for (z = 0; z < iRand; z++)
                                if (previous[z] == x)
                                        break;

                        if (z < iRand)
                                continue;

                        previous[iRand] = x;

                        len = strlen(buf);
                        snprintf(buf2, MSL, "%s", home_types[type].room_desc[x]);
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
        snprintf(buf2, MSL, "%s\n\r", wordwrap(buf, 75));
        room->description = STRALLOC(buf2);
}

/* can we enter that room? check privacy and stuff */
bool HOME_DATA::can_enter(CHAR_DATA * ch)
{
        /* If they are already inside, let them wander */
        if (ch->in_room->home == this)
                return TRUE;
        if (this->check_member(ch))
                return TRUE;
        return FALSE;
}
