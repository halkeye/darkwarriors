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

typedef struct channel_data CHANNEL_DATA;
extern CHANNEL_DATA *first_channel;
extern CHANNEL_DATA *last_channel;

#define CHANNEL_FILE SYSTEM_DIR "channel.dat"
struct channel_data
{
        CHANNEL_DATA *next;
        CHANNEL_DATA *prev;
        LOG_DATA *log;
        char     *name;
        char     *actmessage;   /* The title to sens, "OOC", "[INFO]", etc, accepts colors */
        char     *emotemessage;
        char     *socialmessage;
        sh_int    logtype;
        sh_int    type; /* IC, OOC, IMM? */
        sh_int    color;    /* Here, this is color of TEXT to send, best to reset title at the end with &D */
        sh_int    range;    /* Umm, planetary? system? I dunno */
        sh_int    level;    /* Minimum level to see this channel? */
        sh_int    logpos;   /* Current position in the log that this channel is at. Not setable. */
        sh_int    cost; /* Does it cost to use this channel? */
        bool      history;  /* Whether or not we are saving a log on thig channel */
        bool      enabled;  /* Whether we want people to use this channel at the moment */
};

typedef enum
{
        CHANNEL_ROOM, CHANNEL_AREA, CHANNEL_PLANET, CHANNEL_SYSTEM,
        CHANNEL_OOC_GLOBAL, CHANNEL_CLAN
} channel_ranges;

typedef enum
{
        CHANNEL_IC, CHANNEL_IC_COM, CHANNEL_OOC
} channel_types;

CHANNEL_DATA *get_channel args((char *name));
bool check_channel args((CHAR_DATA * ch, char *command, char *argument));
void      add_channel_log
args((CHAR_DATA * from, char *message, CHANNEL_DATA * channel));
int hasname args((const char *list, const char *name));
void addname args((char **list, const char *name));
void removename args((char **list, const char *name));
const char *getarg args((const char *argument, char *arg, int length));
void load_channels args((void));
extern char *const pc_displays[MAX_COLORS];
