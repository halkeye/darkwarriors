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
 *                                SWR OLC Shuttle module                                 *
 ****************************************************************************************/

#ifndef _OLC_SHUTTLE_
#define _OLC_SHUTTLE_

#define SHUTTLE_DIR	"../shuttle/"
#define SHUTTLE_LIST	"shuttle.lst"


typedef struct shuttle_data SHUTTLE_DATA;
typedef struct stop_data STOP_DATA;

struct stop_data
{
        STOP_DATA *prev;    /* Previous Stop */
        STOP_DATA *next;    /* Next Stop */
        char     *stop_name;    /* Name of the Stop, ie 'Coruscant' or 'Monument Plaza' */
        int       room;
};

enum _shuttle_state
{
        SHUTTLE_STATE_LANDING,
        SHUTTLE_STATE_LANDED,
        SHUTTLE_STATE_TAKINGOFF,
        SHUTTLE_STATE_INSPACE,
        SHUTTLE_STATE_HYPERSPACE_LAUNCH,
        SHUTTLE_STATE_HYPERSPACE_END
};


typedef enum
{
        SHUTTLE_TURBOCAR,   /* Pretty much the same as SHUTTLE_SPACE IMO */
        SHUTTLE_SPACE,  /* Has some message about taking off and landing */
        SHUTTLE_HYPERSPACE
} SHUTTLE_CLASS;

struct shuttle_data
{
        /*
         * Linked List Stuff 
         */
        SHUTTLE_DATA *prev;
        SHUTTLE_DATA *next;

        /*
         * For wherever we are 
         */
        SHUTTLE_DATA *next_in_room;
        SHUTTLE_DATA *prev_in_room;

        /*
         * Where are We 
         */
        ROOM_INDEX_DATA *in_room;

        /*
         * HOTBOOT info, save vnum of current, then loop through on load to find it 
         */
        STOP_DATA *current;
        int       current_number;
        /*
         * Current State 
         */
        int       state;

        /*
         * Stops 
         */
        STOP_DATA *first_stop;
        STOP_DATA *last_stop;

        /*
         * Shuttle Class 
         */
        SHUTTLE_CLASS type;

        /*
         * Shuttle Filename 
         */
        char     *filename;
        /*
         * Shuttle Name 
         */
        char     *name;
        /*
         * Delay Between Stops 
         */
        int       delay;
        /*
         * Actual time for delay.. 
         */
        int       current_delay;
        /*
         * For echoing any messages 
         */
        int       start_room;
        int       end_room;
        int       entrance;
        /*
         * Descriptions 
         */
        char     *takeoff_desc;
        char     *land_desc;
        char     *approach_desc;
};

extern SHUTTLE_DATA *first_shuttle;
extern SHUTTLE_DATA *last_shuttle;

void      update_shuttle(void);
SHUTTLE_DATA *get_shuttle(char *argument);
void      write_shuttle_list(void);
bool      save_shuttle(SHUTTLE_DATA * shuttle);
SHUTTLE_DATA *make_shuttle(char *filename, char *name);
bool      extract_shuttle(SHUTTLE_DATA * shuttle);
bool      insert_shuttle(SHUTTLE_DATA * shuttle, ROOM_INDEX_DATA * room);
void      load_shuttles(void);
bool      load_shuttle_file(char *shuttlefile);
void      fread_shuttle(SHUTTLE_DATA * shuttle, FILE * fp);
void      fread_stop(STOP_DATA * stop, FILE * fp);
void      destroy_shuttle(SHUTTLE_DATA * shuttle);
void      show_shuttles_to_char(CHAR_DATA * ch, SHUTTLE_DATA * shuttle);
SHUTTLE_DATA *shuttle_in_room(ROOM_INDEX_DATA * room, char *name);
SHUTTLE_DATA *shuttle_from_entrance(int vnum);

#endif
