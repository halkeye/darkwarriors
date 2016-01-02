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
 *                                SWR Online Creation module                             *
 ****************************************************************************************/
#ifndef _OLC_H_
#define _OLC_H_

#include "mud.h"

typedef struct class_data CLASS_DATA;
typedef struct protoship_data PROTOSHIP_DATA;
typedef struct language_data LANGUAGE_DATA;
typedef struct list_data LIST_DATA;
typedef struct illness_data ILLNESS_DATA;

extern CLASS_DATA *first_class;
extern CLASS_DATA *last_class;
extern LANGUAGE_DATA *first_language;
extern LANGUAGE_DATA *last_language;
extern PROTOSHIP_DATA *first_protoship;
extern PROTOSHIP_DATA *last_protoship;
extern PROTOSHIP_DATA *first_list;
extern PROTOSHIP_DATA *last_list;
extern ILLNESS_DATA *first_illness;
extern ILLNESS_DATA *last_illness;

#define CLASSES_DIR "../classes/"
#define PROTOSHIP_DIR "../protoships/"
#define LIST_DIR "../list/"
#define ILLNESS_DIR "../illness/"
#define PROTOSHIP_LIST       "protoship.lst"
#define CLASS_LIST       "class.lst"
#define LIST_LIST       "list.lst"
#define LANGUAGE_LIST       "language.lst"
#define ILLNESS_LIST		"illness.lst"

/* olc.c function prototypes */
PROTOSHIP_DATA *get_protoship args((char *name));

/* class structure */
struct class_data
{
        char     *class_name;
        sh_int    value;
        CLASS_DATA *first_foil;
        CLASS_DATA *last_foil;
        CLASS_DATA *next;   /* next class in list           */
        CLASS_DATA *prev;   /* previous class in list       */
};


/* class structure */
struct language_data
{
        char     *name;
        sh_int    min_intelligence;
        LANGUAGE_DATA *next;
        LANGUAGE_DATA *prev;
};

/* protosjip structure */
struct protoship_data
{
        char     *name;
        char     *description;
        char     *shipclass;
        sh_int    mingroundspeed;
        sh_int    maxgroundspeed;
        sh_int    minrooms;
        sh_int    maxrooms;
        sh_int    mincomm;
        sh_int    maxcomm;
        sh_int    minsensor;
        sh_int    maxsensor;
        sh_int    minastro_array;
        sh_int    maxastro_array;
        sh_int    minhyperspeed;
        sh_int    maxhyperspeed;
        sh_int    minspeed;
        sh_int    maxspeed;
        sh_int    minmissiles;
        sh_int    maxmissiles;
        sh_int    mintorpedos;
        sh_int    maxtorpedos;
        sh_int    minrockets;
        sh_int    maxrockets;
        sh_int    minlasers;
        sh_int    maxlasers;
        sh_int    mintractorbeam;
        sh_int    maxtractorbeam;
        sh_int    minions;
        sh_int    maxions;
        sh_int    minmanuever;
        sh_int    maxmanuever;
        int       maxcargo;
        int       mincargo;
        int       maxenergy;
        int       minenergy;
        int       minshield;
        int       maxshield;
        int       minhull;
        int       maxhull;
        sh_int    minchaff;
        sh_int    maxchaff;
        int       minbattalions;
        int       maxbattalions;
        PROTOSHIP_DATA *next;
        PROTOSHIP_DATA *prev;

};

/* list structure */
struct list_data
{
        char     *name;
        char     *filename;
        char     *value1;
        char     *value2;
        char     *value3;
        char     *value4;
        char     *value5;
        LIST_DATA *next;    /* next list in list            */
        LIST_DATA *prev;    /* previous list in list        */
};


/* illness structure */
struct illness_data
{
        ILLNESS_DATA *next;
        ILLNESS_DATA *prev;
        char     *name;
        char     *social1;
        char     *social2;
        char     *social3;
        char     *message1;
        char     *message2;
        char     *message3;
        sh_int    hploss;
        sh_int    mvloss;
        sh_int    hunger;
        sh_int    thirst;
};
#endif
