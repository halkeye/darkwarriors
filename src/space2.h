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
 *                $Id: space2.h 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/

class     BODY_DATA;

/* dock structure */
struct DOCK_DATA
{
        CLAN_DATA *clan;
        int       vnum;
        bool      hidden;
        bool      temporary;    /* For installations and beacons */
        BODY_DATA *body;
        char     *name;
        /*
         * next dock in list 
         */
        DOCK_DATA *next;
        /*
         * previous dock in list 
         */
        DOCK_DATA *prev;
        DOCK_DATA *next_in_body;
        DOCK_DATA *prev_in_body;
        DOCK_DATA *next_in_installation;
        DOCK_DATA *prev_in_installation;
};

#define DOCK_DIR       "../dock/"
#define FILE_DOCK_LIST	"dock.lst"
#define SHIPIMMAGE_DIR       "../shipimages/"

void free_dock args((DOCK_DATA * dock));
void fread_planet args((PLANET_DATA * planet, FILE * fp));
bool load_planet_file args((char *planetfile));
void write_planet_list args((void));
void fread_dock args((DOCK_DATA * dock, FILE * fp));
bool load_dock_file args((char *dockfile));
void write_dock_list args((void));
void fread_body args((BODY_DATA * bpdy, FILE * fp));
bool load_body_file args((char *bodyfile));
void write_body_list args((void));
PLANET_DATA *get_planet args((char *name));
void load_planets args((void));
void save_planet args((PLANET_DATA * planet, bool copyover));
void load_docks args((void));
void fwrite_dock args((FILE * fp, DOCK_DATA * dock));
void save_body args((BODY_DATA * body));
void makedock args((INSTALLATION_DATA * installation));

int distance_ship_ship args((SHIP_DATA * target, SHIP_DATA * ship));
int distance_missile_ship args((MISSILE_DATA * missile, SHIP_DATA * ship));
char     *get_direction_body args((BODY_DATA * body, SHIP_DATA * ship));
char     *get_direction_ship args((SHIP_DATA * target, SHIP_DATA * ship));
