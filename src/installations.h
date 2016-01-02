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
 *                                SWR Installation module                                *
 ****************************************************************************************/

//typedef struct dock_data DOCK_DATA;
//typedef struct installation_data INSTALLATION_DATA;

extern INSTALLATION_DATA *first_installation;
extern INSTALLATION_DATA *last_installation;

/* installation structure */
struct INSTALLATION_DATA
{
        INSTALLATION_DATA *next;
        INSTALLATION_DATA *prev;
        INSTALLATION_DATA *next_on_planet;
        INSTALLATION_DATA *prev_on_planet;
        DOCK_DATA *first_dock;
        DOCK_DATA *last_dock;
        CLAN_DATA *clan;
        char     *filename;
        int       first_room;
        int       flags;
        int       last_built;
        int       last_room;
        PLANET_DATA *planet;
        int       type;
        int       mainroom;
        sh_int    timer;
        int       weapon_time;
};

/* Type and flag declarations */

typedef enum
{
        BATTERY_INSTALLATION,
        ION_INSTALLATION,
        TURBOLASER_INSTALLATION,
        RESEARCH_INSTALLATION,
        CLANHQ_INSTALLATION,
        TRAINING_INSTALLATION,
        GOVERNMENT_INSTALLATION,
        SHIPYARD_INSTALLATION,
        MINING_INSTALLATION,
        INS_MAX
} installation_types;

/* Why were these bit vectors? these are mutually exclusive */
#define INS_UNDERGROUND         0
#define INS_ABOVEGROUND         1
#define INS_UNDERWATER          2

/* This should probably be a flag */
#define INS_SECURE             	BV00

#define MAX_INSTALLATION       	INS_MAX+1

#define INSTALLATIONS_DIR       "../installations/"
#define INSTALLATION_LIST	"installations.lst"
#define INSTALLATION_AREA       "pinstalls.are"

#define MOB_VNUM_INSTALL_GUARD           33001
#define MOB_VNUM_INSTALL_ENTERANCE_GUARD 33002
#define MOB_VNUM_INSTALL_DOCTOR          33003
#define MOB_VNUM_INSTALL_CUSTOMS         33004

#define OBJ_VNUM_INSTALL_BACTA_SPRAY     33001
#define OBJ_VNUM_INSTALL_MEDPAC          33002

int find_pvnum_block args((int num_needed, char *areaname));
void load_installations args((void));
void      echo_to_installation
args((sh_int AT_COLOR, char *argument, INSTALLATION_DATA * installation));
bool mob_reset args((CHAR_DATA * ch, char *type, bool check));
INSTALLATION_DATA *installation_from_room args((int vnum));
void addroominstallation args((CHAR_DATA * ch, char *argument));
void fireplanet_update args((void));
int planetary_installations args((PLANET_DATA * planet, int type));


struct installation_type
{
        const char *installation_name;  /* Installation name    */
        sh_int    shields;  /* shield */
        sh_int    ions; /* ions */
        sh_int    turbo;    /* turbo lasers */
        sh_int    rooms;    /* rooms */
        const char *main_name;  /* Name of the installations main room */
};

extern const struct installation_type installation_table[];
