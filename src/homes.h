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
 *                                SWR Hotboot module                                     *
 *****************************************************************************************
 *                   $Id: homes.h 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/

#include <list>
#include <algorithm>

struct ROOMMATE_DATA
{
        int       type; /* Dunno what this if for anymore */
        char     *name; /* Hashed */
	public: 
		ROOMMATE_DATA(void);
		~ROOMMATE_DATA(void);
};

typedef std::list < ROOM_INDEX_DATA * > ROOM_LIST;
typedef std::list < ROOMMATE_DATA * > ROOMMATE_LIST;

class GRID; 
struct HOME_DATA
{
        HOME_DATA *next;
        HOME_DATA *prev;
        char     *filename;
        char     *name;
        char     *description;
        char     *owner;    /* Can be clan or a player, or 'public' */
        long int  price;
        GRID      * grid;

        ROOMMATE_LIST roommates;
        ROOM_LIST rooms;
		/* Status: Public, private
		 * (Default) Private - Only allowed list of people allowed in
		 *           Public  - Anyone allowed in, good for shops and stuff
		 */

	public:
		/* Constructor */
		HOME_DATA();
		/* Destructor */
		~HOME_DATA();
		/** Save data to file (specified by ->filename */
        void save(void);
		/** tie in for addroom skill */
		void add_room(CHAR_DATA * ch, char * argument);
		/** tie in for decorate skill */
		void decorate_room(CHAR_DATA * ch, char * argument);
		/**  is ch a member of the house */
		bool check_member(CHAR_DATA * ch);
      /* can we enter that room? check privacy and stuff */
		bool can_enter(CHAR_DATA * ch);
		/** Echo a message to the entire house */
		void echo(int color, char *argument);
		/** Reset your home */
		void reset();

		/** remove a roommate */
		inline void remove(ROOMMATE_DATA * roomie) {
			this->roommates.erase(find(this->roommates.begin(), this->roommates.end(), roomie));
		}
		/** Add a roommate */
		inline void add(ROOMMATE_DATA * roomie) {
			this->roommates.push_back(roomie);
		}
		/** remove a room*/
		inline void remove(ROOM_INDEX_DATA * room) {
			this->rooms.erase(find(this->rooms.begin(), this->rooms.end(), room));
			room->home = NULL;
		}
		/** Add a roommate */
		inline void add(ROOM_INDEX_DATA * room) {
			this->rooms.push_back(room);
			room->home = this;
		}
};

#define HOMEDIR		"../homes/"
#define HOME_LIST       "homes.lst"

#define	HOME_SAVE_TIME  60*20   /* 20 Minutes */
/* homes.c */

HOME_DATA *get_home args((char *name));
void save_home2 args((HOME_DATA * home));
void write_home_list args((void));
void fwrite_roommates args((FILE * fp, HOME_DATA * home));
void load_homes args((void));
void fread_roommate args((ROOMMATE_DATA * roomie, FILE * fp));
void fread_home args((HOME_DATA * home, FILE * fp));

void save_homes_check args((void));
long get_home_value args((HOME_DATA * home));

bool load_home_file args((char *homefile));

extern HOME_DATA *first_home;
extern HOME_DATA *last_home;
extern char *const home_flags[];
extern time_t save_homes_time;

DECLARE_DO_FUN(do_homes);
DECLARE_DO_FUN(do_makehome);
DECLARE_DO_FUN(do_showhome);
DECLARE_DO_FUN(do_sethome);
DECLARE_DO_FUN(do_resethome);
DECLARE_DO_FUN(do_buyhome2);
DECLARE_DO_FUN(do_sellhome2);
DECLARE_DO_FUN(do_freehomes);
