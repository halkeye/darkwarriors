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
#ifndef __BODY_DATA__
#define __BODY_DATA__

#include <list>
#include <memory>

#define BODY_DIR       "../body/"
#define FILE_BODY_LIST	"body.lst"
typedef std::list < DOCK_DATA * >DOCK_LIST;
extern DOCK_DATA *first_dock;
extern DOCK_DATA *last_dock;


/* body structure */
class BODY_DATA
{
      private:
        char     *_filename;
        int _gravity;
        char     *_name;
        int _type;
        int _xpos;
        int _ypos;
        int _zpos;
        int _orbitcount;
        int _xmove;
        int _ymove;
        int _zmove;
        int _centerx;
        int _centery;
        int _centerz;

        PLANET_DATA *_planet;
        SPACE_DATA *_starsystem;
        AREA_LIST _areas;
        DOCK_LIST _docks;

        /*
         * Get Functions 
         */
      public:
        BODY_DATA();
        ~BODY_DATA();
        inline SPACE_DATA *starsystem()
        {
                return this->_starsystem;
        }
        void starsystem(SPACE_DATA * s);

        inline PLANET_DATA *planet()
        {
                return this->_planet;
        }
        inline void planet(PLANET_DATA * p)
        {
                this->_planet = p;
        } inline DOCK_LIST & docks(void)
        {
                return this->_docks;
        }
        inline AREA_LIST & areas(void)
        {
                return this->_areas;
        }
        inline char *name()
        {
                return this->_name;
        }
        inline void name(char *newname)
        {
                if (this->_name)
                        STRFREE(this->_name);
                this->_name = STRALLOC(newname);
        }
        inline char *filename()
        {
                return this->_filename;
        }
        inline void filename(char *newfilename)
        {
                if (this->_filename)
                        STRFREE(this->_filename);
                this->_filename = STRALLOC(newfilename);
        }
        inline int gravity()
        {
                return this->_gravity;
        }
        inline void gravity(int a)
        {
                this->_gravity = a;
        } inline int type()
        {
                return this->_type;
        }
        inline void type(int a)
        {
                this->_type = a;
        } inline int xpos()
        {
                return this->_xpos;
        }
        inline void xpos(int a)
        {
                this->_xpos = a;
        } inline int ypos()
        {
                return this->_ypos;
        }
        inline void ypos(int a)
        {
                this->_ypos = a;
        } inline int zpos()
        {
                return this->_zpos;
        }
        inline void zpos(int a)
        {
                this->_zpos = a;
        } inline int xmove()
        {
                return this->_xmove;
        }
        inline void xmove(int a)
        {
                this->_xmove = a;
        } inline int ymove()
        {
                return this->_ymove;
        }
        inline void ymove(int a)
        {
                this->_ymove = a;
        } inline int zmove()
        {
                return this->_zmove;
        }
        inline void zmove(int a)
        {
                this->_zmove = a;
        } inline int centerx()
        {
                return this->_centerx;
        }
        inline void centerx(int a)
        {
                this->_centerx = a;
        } inline int centery()
        {
                return this->_centery;
        }
        inline void centery(int a)
        {
                this->_centery = a;
        } inline int centerz()
        {
                return this->_centerz;
        }
        inline void centerz(int a)
        {
                this->_centerz = a;
        } inline int orbitcount()
        {
                return this->_orbitcount;
        }
        inline void orbitcount(int a)
        {
                this->_orbitcount = a;
        }
        /*
         * Other Functions 
         */
      public:void save();

        BODY_DATA *load(FILE *);
        void remove_area(AREA_DATA * pArea);
        void add_area(AREA_DATA * pArea);
        void add_dock(DOCK_DATA *);
        void remove_dock(DOCK_DATA *);
        char     *get_direction(SHIP_DATA * ship);
        char     *type_name();
        int distance(SHIP_DATA * ship);
        int distance(BODY_DATA * pbody);
        int hyperdistance(SHIP_DATA * ship);
        ROOM_INDEX_DATA *get_rand_room(int bit, bool include);
};
typedef enum
{ STAR_BODY, PLANET_BODY, MOON_BODY, COMET_BODY, ASTEROID_BODY,
        BLACKHOLE_BODY, NEBULA_BODY, BODY_ALL
} BODY_TYPES;

BODY_DATA *get_body args((char *name));
DOCK_DATA *get_dock args((char *name));
DOCK_DATA *get_dock_isname args((SHIP_DATA * ship, char *name));
void load_bodies args((void));


#endif /*  */
