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
#include "mud.h"
#include "body.h"
#include "space2.h"
#include "installations.h"
#include <algorithm>
#include <list>

BODY_LIST bodies;
DOCK_DATA *first_dock;
DOCK_DATA *last_dock;

BODY_DATA *get_body(char *name, BODY_TYPES type)
{
        BODY_DATA *body = NULL;

        if (!name)
                return NULL;

        FOR_EACH_LIST(BODY_LIST, bodies, body)
        {
                if (!str_prefix(name, body->name()))
                {
                        if ((type != BODY_ALL && body->type() == type)
                            || type == BODY_ALL)
                                return body;
                }

        }

        return NULL;
}

BODY_DATA *get_body(char *name)
{
        return get_body(name, BODY_ALL);
}

ROOM_INDEX_DATA *BODY_DATA::get_rand_room(int bit, bool include)
{
        int count, areacount, roomcount, roomcountfind;
        AREA_DATA *area = NULL;
        ROOM_INDEX_DATA *room = NULL;

        count = areacount = roomcount = roomcountfind = 0;
        if (!this->_areas.empty())
        {
                // Can't we just do areas.size()?
                FOR_EACH_LIST(AREA_LIST, this->_areas, area) count++;
                areacount = number_range(1, count);
                count = 0;
                FOR_EACH_LIST(AREA_LIST, this->_areas, area)
                {
                        count++;
                        if ((areacount == count))
                                break;
                }

                if (!area || !area->filename)
                        return NULL;
                for (roomcountfind = area->low_r_vnum;
                     roomcountfind < area->hi_r_vnum; roomcountfind++)
                {
                        roomcount =
                                number_range(area->low_r_vnum,
                                             area->hi_r_vnum);
                        if (include)
                        {
                                if (((room =
                                      get_room_index(roomcount)) != NULL)
                                    && xIS_SET(room->room_flags, bit))
                                        return room;
                        }
                        else
                        {
                                if (((room =
                                      get_room_index(roomcount)) != NULL)
                                    && !xIS_SET(room->room_flags, bit))
                                        return room;
                        }
                }
        }
        return NULL;
}

int get_rflag(char *flag);
CMDF do_testbody(CHAR_DATA * ch, char *argument)
{
        char arg1[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH];
        int value, flag;
        BODY_DATA *body = NULL;
        ROOM_INDEX_DATA *room = NULL;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        value = atoi(argument);

        body = get_body(arg1);
        flag = get_rflag(arg2);

        if (!body)
        {
                send_to_char("Cannot find body", ch);
                return;
        }

        room = body->get_rand_room(ROOM_INDOORS, value);
        if (room)
                ch_printf(ch, "Room name: %s\n\rRoom VNUM: %d\n\r",
                          room->name, room->vnum);
        if (!room)
                send_to_char("The pointer returned NULL", ch);
}

BODY_DATA::BODY_DATA()
{
        this->_starsystem = NULL;
        this->_planet = NULL;
}

BODY_DATA::~BODY_DATA()
{
        PLANET_DATA *planet;
        AREA_DATA *tarea;
        DOCK_DATA *dock, *next_dock;

        this->starsystem(NULL);

        for (planet = first_planet; planet != NULL; planet = planet->next)
                if (planet->body == this)
                        planet->body = NULL;

        for (tarea = first_area; tarea; tarea = tarea->next)
                if (tarea->body == this)
                        tarea->body = NULL;

        this->_areas.clear();

        for (dock = first_dock; dock; dock = next_dock)
	    {
   			    next_dock = dock->next;
                if (dock->body == this)
                        free_dock(dock);
		}

        if (this->_filename)
                STRFREE(this->_filename);
        if (this->_name)
                STRFREE(this->_name);
}

char     *BODY_DATA::get_direction(SHIP_DATA * ship)
{
        static char buf[11];

        buf[0] = ' ';
        if (this->xpos() > ship->vx)
                buf[1] = 'E';
        else if (this->xpos() < ship->vx)
                buf[1] = 'W';
        else
                buf[1] = ' ';
        buf[2] = ' ';
        buf[3] = ' ';
        if (this->ypos() > ship->vy)
                buf[4] = 'N';
        else if (this->ypos() < ship->vy)
                buf[4] = 'S';
        else
                buf[4] = ' ';
        buf[5] = ' ';
        buf[6] = ' ';
        if (this->zpos() > ship->vz)
                buf[7] = 'U';
        else if (this->zpos() < ship->vz)
                buf[7] = 'D';
        else
                buf[7] = ' ';
        buf[8] = ' ';
        buf[9] = '\0';
        return buf;
}

int BODY_DATA::distance(SHIP_DATA * ship)
{
        return (int) sqrt(pow(((int) (ship->vx - this->xpos())), 2) +
                          pow(((int) (ship->vy - this->ypos())),
                              2) + pow(((int) (ship->vz - this->zpos())), 2));
}

int BODY_DATA::distance(BODY_DATA * pbody)
{
        return (int) sqrt(pow(((int) (pbody->xpos() - this->xpos())), 2) +
                          pow(((int) (pbody->ypos() - this->ypos())),
                              2) + pow(((int) (pbody->zpos() - this->zpos())),
                                       2));
}

int BODY_DATA::hyperdistance(SHIP_DATA * ship)
{
        return (int) sqrt(pow(((int) (ship->jx - this->xpos())), 2) +
                          pow(((int) (ship->jy - this->ypos())),
                              2) + pow(((int) (ship->jz - this->zpos())), 2));
}


void BODY_DATA::remove_area(AREA_DATA * area)
{
        this->_areas.
                erase(find(this->_areas.begin(), this->_areas.end(), area));
        area->body = NULL;
}

/* These should be in respected classes */
void BODY_DATA::add_area(AREA_DATA * area)
{
        if (area->body)
        {
                area->body->remove_area(area);
        }
        area->body = this;
        this->_areas.push_back(area);
}

void BODY_DATA::add_dock(DOCK_DATA * dock)
{
        this->_docks.push_back(dock);
}

void BODY_DATA::remove_dock(DOCK_DATA * dock)
{
        this->_docks.
                erase(find(this->_docks.begin(), this->_docks.end(), dock));
}

void BODY_DATA::starsystem(SPACE_DATA * starsystem)
{
        if (this->_starsystem != NULL)
        {
                this->_starsystem->bodies.
                        erase(find
                              (this->_starsystem->bodies.begin(),
                               this->_starsystem->bodies.end(), this));
                this->_starsystem = NULL;
        }
        if (starsystem != NULL)
        {
                starsystem->bodies.push_back(this);
                this->_starsystem = starsystem;
        }
}

BODY_DATA *BODY_DATA::load(FILE * fp)
{
        const char *word;
        bool fMatch;

        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);
                fMatch = FALSE;

                switch (UPPER(word[0]))
                {
                case '*':
                        bug("Matching *");
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;

                case 'A':
                        if (!str_cmp(word, "Area"))
                        {
                                char aName[MAX_STRING_LENGTH];
                                AREA_DATA *pArea = first_area;

                                snprintf(aName, MSL, "%s",
                                         fread_string_noalloc(fp));
                                for (pArea = first_area; pArea;
                                     pArea = pArea->next)
                                {
                                        if (pArea->filename
                                            && !str_cmp(pArea->filename,
                                                        aName))
                                                this->add_area(pArea);
                                }
                                fMatch = TRUE;
                        }
                        break;
                case 'C':
                        KEY("Centerx", this->_centerx, fread_number(fp));
                        KEY("Centery", this->_centery, fread_number(fp));
                        KEY("Centerz", this->_centerz, fread_number(fp));
                        break;

                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!this->_name)
                                        this->_name = STRALLOC("");
                                while (this->_xmove > -10
                                       && this->_xmove < 10)
                                        this->_xmove = number_range(-50, 50);
                                while (this->_ymove > -10
                                       && this->_ymove < 10)
                                        this->_ymove = number_range(-50, 50);
                                while (this->_zmove > -10
                                       && this->_zmove < 10)
                                        this->_zmove = number_range(-50, 50);
                                return this;
                        }
                        break;

                case 'F':
                        KEY("Filename", this->_filename, fread_string(fp));
                        break;

                case 'G':
                        KEY("Gravity", this->_gravity, fread_number(fp));
                        break;

                case 'N':
                        KEY("Name", this->_name, fread_string(fp));
                        break;

                case 'O':
                        KEY("Orbitcount", this->_orbitcount,
                            fread_number(fp));
                        break;

                case 'P':
                        KEY("Planet", this->_planet,
                            get_planet(fread_string_noalloc(fp)));
                        break;

                case 'S':
                        if (!str_cmp(word, "Starsystem"))
                        {
                                SPACE_DATA *starsystem =
                                        starsystem_from_name
                                        (fread_string_noalloc(fp));
                                if (starsystem)
                                        this->starsystem(starsystem);
                                fMatch = TRUE;
                                break;
                        }

                case 'T':
                        KEY("Type", this->_type, fread_number(fp));
                        break;

                case 'X':
                        KEY("Xpos", this->_xpos, fread_number(fp));
                        KEY("Xmove", this->_xmove, fread_number(fp));
                        break;

                case 'Y':
                        KEY("Ypos", this->_ypos, fread_number(fp));
                        KEY("Ymove", this->_ymove, fread_number(fp));
                        break;

                case 'Z':
                        KEY("Zpos", this->_zpos, fread_number(fp));
                        KEY("Zmove", this->_zmove, fread_number(fp));
                        break;
                }

                if (!fMatch)
                {
                        bug("Fread_body: no match: %s", word);
                }
        }
        return this;
}

bool load_body_file(char *bodyfile)
{
        char filename[256];
        BODY_DATA *body;
        FILE     *fp;
        bool found;

        //CREATE(body, BODY_DATA, 1);
         /* If it never gets to BODY below, this goes dead, no pointer to it.*/

        found = FALSE;
        snprintf(filename, 256, "%s%s", BODY_DIR, bodyfile);

        if ((fp = fopen(filename, "r")) != NULL)
        {

                found = TRUE;
                for (;;)
                {
                        char letter;
                        char     *word;

                        letter = fread_letter(fp);
                        if (letter == '*')
                        {
                                fread_to_eol(fp);
                                continue;
                        }

                        if (letter != '#')
                        {
                                bug("Load_body_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "BODY"))
                        {
   							    body = new BODY_DATA; /* lets try this here instead */
                                body->load(fp);
                                bodies.push_front(body);
                        }
                        else if (!str_cmp(word, "DOCK"))
                        {
                                DOCK_DATA *dock;

                                CREATE(dock, DOCK_DATA, 1);
                                fread_dock(dock, fp);
                                dock->body = body;
                                LINK(dock, first_dock, last_dock, next, prev);
                                dock->body->add_dock(dock);
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                bug("Load_body_file: bad section: %s.", word);
                                break;
                        }
                }
                FCLOSE(fp);
        }

        if (!found)
                DELETE(body);

        return found;
}

void BODY_DATA::save()
{
        FILE     *fp;
        char filename[256];
        char buf[MAX_STRING_LENGTH];

        if (!this)
        {
                bug("save_body: null body pointer!", 0);
                return;
        }

        if (!this->_filename || this->_filename[0] == '\0')
        {
                snprintf(buf, MSL, "save_body: %s has no filename",
                         this->_name);
                bug(buf, 0);
                return;
        }
        snprintf(filename, 256, "%s%s", BODY_DIR, this->_filename);

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_body: fopen", 0);
                perror(filename);
        }
        else
        {
                AREA_DATA *pArea = NULL;
                DOCK_DATA *dock = NULL;

                fprintf(fp, "#BODY\n");
                fprintf(fp, "Name         %s~\n", this->_name);
                fprintf(fp, "Filename     %s~\n", this->_filename);
                fprintf(fp, "Type         %d\n", this->_type);
                fprintf(fp, "Xpos         %d\n", this->_xpos);
                fprintf(fp, "Ypos         %d\n", this->_ypos);
                fprintf(fp, "Zpos         %d\n", this->_zpos);
                fprintf(fp, "Xmove        %d\n", this->_xmove);
                fprintf(fp, "Ymove        %d\n", this->_ymove);
                fprintf(fp, "Zmove        %d\n", this->_zmove);
                fprintf(fp, "Centerx      %d\n", this->_centerx);
                fprintf(fp, "Centery      %d\n", this->_centery);
                fprintf(fp, "Centerz      %d\n", this->_centerz);
                fprintf(fp, "Gravity      %d\n", this->_gravity);
                fprintf(fp, "Orbitcount   %d\n", this->_orbitcount);
                if (this->_starsystem && this->_starsystem->name)
                        fprintf(fp, "Starsystem   %s~\n",
                                this->_starsystem->name);
                if (this->_planet && this->_planet->name)
                        fprintf(fp, "Planet   %s~\n", this->_planet->name);
                FOR_EACH_LIST(AREA_LIST, this->_areas, pArea)
                        if (pArea->filename)
                        fprintf(fp, "Area         %s~\n", pArea->filename);

                fprintf(fp, "End\n\n");

                FOR_EACH_LIST(DOCK_LIST, this->_docks, dock)
                {
                        if (dock->temporary)
                                continue;
                        fwrite_dock(fp, dock);
                }

                fprintf(fp, "#END\n");
        }
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

void write_body_list()
{
        BODY_DATA *tbody = NULL;
        FILE     *fpout;
        char filename[256];

        snprintf(filename, 256, "%s%s", BODY_DIR, FILE_BODY_LIST);
        fpout = fopen(filename, "w");
        if (!fpout)
        {
                bug("FATAL: cannot open body.lst for writing!\n\r", 0);
                return;
        }
        FOR_EACH_LIST(BODY_LIST, bodies, tbody)
                fprintf(fpout, "%s\n", tbody->filename());
        fprintf(fpout, "$\n");
        FCLOSE(fpout);
}

void load_bodies()
{
        FILE     *fpList;
        const char *filename;
        char bodylist[256];
        PLANET_DATA *planet;

        snprintf(bodylist, 256, "%s%s", BODY_DIR, FILE_BODY_LIST);
        FCLOSE(fpReserve);
        if ((fpList = fopen(bodylist, "r")) == NULL)
        {
                perror(bodylist);
                exit(1);
        }

        for (;;)
        {
                filename = feof(fpList) ? "$" : fread_word(fpList);
                if (filename[0] == '$')
                        break;

                if (!load_body_file((char *) filename))
                        bug("Cannot load body file: %s", filename);
        }
        FCLOSE(fpList);
        boot_log(" Done bodies ");
        fpReserve = fopen(NULL_FILE, "r");
        for (planet = first_planet; planet; planet = planet->next)
                planet->body = get_body(planet->bodyname);
        return;
}

char     *BODY_DATA::type_name()
{
        switch (this->type())
        {
        case STAR_BODY:
                return "Star";
        case PLANET_BODY:
                return "Planet";
        case MOON_BODY:
                return "Moon";
        case COMET_BODY:
                return "Comet";
        case ASTEROID_BODY:
                return "Asteroid";
        case BLACKHOLE_BODY:
                return "Blackhole";
        case NEBULA_BODY:
                return "Nebula";
        default:
                return "Unknown";
        }
}
