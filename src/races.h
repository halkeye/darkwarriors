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
 *                $Id: races.h 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#ifndef __RACE_DATA_H__
#define __RACE_DATA_H__

#include <list>

#define FILE_RACE_LIST       "race.lst"
#define RACES_DIR "../races/"

enum
{
        ATTR_STRENGTH, ATTR_INTELLIGENCE, ATTR_WISDOM,
        ATTR_DEXTERITY, ATTR_CONSTITUTION, ATTR_CHARISMA,
        ATTR_FORCE, ATTR_LUCK,
        MAX_ATTR
};

class     RACE_DATA
{
      private:
        // I think this will be best to do a reference
        EXT_BV _body_parts;
        LANGUAGE_DATA *_language;
        int       _affected;
        sh_int    _con_plus;
        sh_int    _cha_plus;
        sh_int    _dex_plus;
        sh_int    _frc_plus;
        sh_int    _hit;
        sh_int    _int_plus;
        sh_int    _lang_bonus;
        sh_int    _lck_plus;
        sh_int    _endurance;
        char     *_name;
        sh_int    _str_plus;
        sh_int    _wis_plus;
        sh_int    _class_modifier[MAX_ABILITY];
        sh_int    _attr_mod[MAX_ATTR];

        // Should this be planet ?
        int       _home;
        // Age at which a player begins to die 
        int       _death_age;
        // No more hard coding 
        int       _hunger_mod;
        int       _thirst_mod;
        int       _rpneeded;
        int       _start_age;
        int       _class_restriction;

      public:
                  inline LANGUAGE_DATA * language()
        {
                return this->_language;
        }
        inline void language(LANGUAGE_DATA * p)
        {
                this->_language = p;
        }
        // For Now
        inline    EXT_BV & body_parts()
        {
                return this->_body_parts;
        }
        // Set
        inline void body_parts(int bit, bool set)
        {
                if (set)
                {
                        xSET_BIT(this->_body_parts, bit);
                }
                else
                {
                        xREMOVE_BIT(this->_body_parts, bit);
                }
        }
        // Toggle
        inline void body_parts(int bit)
        {
                xTOGGLE_BIT(this->_body_parts, bit);
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
        inline int class_restriction()
        {
                return this->_class_restriction;
        }
        inline int class_restriction(int p)
        {
                return (this->_class_restriction = p);
        }
        inline int start_age()
        {
                return this->_start_age;
        }
        inline int start_age(int p)
        {
                return (this->_start_age = p);
        }
        inline int thirst_mod()
        {
                return this->_thirst_mod;
        }
        inline int thirst_mod(int p)
        {
                return (this->_thirst_mod = p);
        }
        inline int hunger_mod()
        {
                return this->_hunger_mod;
        }
        inline int hunger_mod(int p)
        {
                return (this->_hunger_mod = p);
        }
        inline int death_age()
        {
                return this->_death_age;
        }
        inline int death_age(int p)
        {
                return (this->_death_age = p);
        }
        inline int endurance()
        {
                return this->_endurance;
        }
        inline int endurance(int p)
        {
                return (this->_endurance = p);
        }
        inline int affected()
        {
                return this->_affected;
        }
        inline int affected(int p)
        {
                return (this->_affected = p);
        }
        inline int lang_bonus()
        {
                return this->_lang_bonus;
        }
        inline int lang_bonus(int p)
        {
                return (this->_lang_bonus = p);
        }
        inline int hit()
        {
                return this->_hit;
        }
        inline int hit(int p)
        {
                return (this->_hit = p);
        }
        inline int home()
        {
                return this->_home;
        }
        inline int home(int p)
        {
                return (this->_home = p);
        }
        inline int rpneeded()
        {
                return this->_rpneeded;
        }
        inline int rpneeded(int p)
        {
                return (this->_rpneeded = p);
        }
        inline sh_int class_modifier(int ability)
        {
                return this->_class_modifier[ability];
        }
        inline sh_int class_modifier(int ability, int p)
        {
                return (this->_class_modifier[ability] = p);
        }
        inline sh_int attr_modifier(int attr)
        {
                return this->_attr_mod[attr];
        }
        inline sh_int attr_modifier(int attr, int p)
        {
                return (this->_attr_mod[attr] = p);
        }


      public:
        RACE_DATA();
        ~RACE_DATA();
        void      save();
        void      load(FILE * fp);
        static bool load_race_file(char *racefile);
        static void fwrite_race_list(void);
        static void load_races(void);
};

typedef std::list < RACE_DATA * >RACE_LIST;
extern RACE_LIST races;

#endif
