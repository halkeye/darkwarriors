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
 *                $Id: olc_bounty.h 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#ifndef __OLC_BOUNTY_DATA_H__
#define __OLC_BOUNTY_DATA_H__

#include <list>
#include "mud.h"

enum
{
        BOUNTY_ALIVE, BOUNTY_DEAD, MAX_BOUNTY_TYPES
};

char     *const olc_bounty_types[] = {
        "Alive", "Dead", "MAX_TYPE"
};

#define OLC_BOUNTY_FILE SYSTEM_DIR "olcbounty.dat"

class     OLC_BOUNTY_DATA
{
      private:
        int       _owner;
        int       _vnum;
        int       _corpse;
        sh_int    _type;
        int       _amount;
        int       _exp;

      public:
                  inline sh_int type()
        {
                return this->_type;
        }
        inline bool type(int i)
        {
                if (MAX_BOUNTY_TYPES <= i)
                {
                        return FALSE;
                }

                this->_type = i;
                return TRUE;
        }

        inline int owner()
        {
                return this->_owner;
        }

        inline bool owner(int vnum)
        {
                if (vnum < 0 || vnum > MAX_VNUMS)
                {
                        return FALSE;
                }
                this->_owner = vnum;
                return TRUE;
        }

        inline int vnum()
        {
                return this->_vnum;
        }

        inline bool vnum(int vnum)
        {
                if (vnum < 0 || vnum > MAX_VNUMS)
                {
                        return FALSE;
                }
                this->_vnum = vnum;
                return TRUE;
        }
        inline int corpse()
        {
                return this->_corpse;
        }

        inline bool corpse(int vnum)
        {
                if (vnum < 0 || vnum > MAX_VNUMS)
                {
                        return FALSE;
                }
                this->_corpse = vnum;
                return TRUE;
        }
        inline int amount()
        {
                return this->_amount;
        }

        inline void amount(int credits)
        {
                this->_amount = credits;
        }

        inline int experience()
        {
                return this->_exp;
        }

        inline void experience(int experience)
        {
                this->_exp = experience;
        }

      public:
        OLC_BOUNTY_DATA();
        OLC_BOUNTY_DATA(int vnum);

        ~OLC_BOUNTY_DATA();
        void      save();
        void      load(FILE * fp);
        static void load_olc_bounties(void);
};


typedef std::list < OLC_BOUNTY_DATA * >OLC_BOUNTY_LIST;
extern OLC_BOUNTY_LIST olc_bounties;
OLC_BOUNTY_DATA *has_olc_bounty(CHAR_DATA * victim);
void mset_bounty(CHAR_DATA * ch, CHAR_DATA * mob, char *argument);
void print_olc_bounties_mob(CHAR_DATA * ch, CHAR_DATA * mob);
void load_olc_bounties(void);
bool check_given_bounty(CHAR_DATA * ch, CHAR_DATA * hunter, OBJ_DATA * obj);
int print_olc_bounties(CHAR_DATA * ch);
bool check_olc_bounties(ROOM_INDEX_DATA * room);

#endif
