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
 *                           ^     +----- |  / ^     ^ |     | +-\                       *
 *                          / \    |      | /  |\   /| |     | |  \                      *
 *                         /   \   +---   |<   | \ / | |     | |  |                      *
 *                        /-----\  |      | \  |  v  | |     | |  /                      *
 *                       /       \ |      |  \ |     | +-----+ +-/                       *
 *****************************************************************************************
 *                                                                                       *
 * AFKMud Copyright 1997-2003 by Roger Libiez (Samson), Levi Beckerson (Whir),           *
 * Michael Ward (Tarl), Erik Wolfe (Dwip), Cameron Carroll (Cam), Cyberfox, Karangi,     *
 * Rathian, Raine, and Adjani. All Rights Reserved.                                      *
 *                                                                                       *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag, Blodkai, Haus, Narn,*
 * Scryn, Swordbearer, Tricops, Gorog, Rennard, Grishnakh, Fireblade, and Nivek.         *
 *                                                                                       *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                                   *
 *                                                                                       *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen, Michael Seifert,  *
 * and Sebastian Hammer.                                                                 *
 *****************************************************************************************
 *                         Internal server shell command module                          *
 ****************************************************************************************/

/* Change this line to the home directory for the server - Samson */
#define HOST_DIR 	"/home/mud/darkwars/dwadmins/"

/* Change this line to the name of your compiled binary - Samson */
#define BINARYFILE "swr"

/* Change each of these to reflect your directory structure - Samson */

#define CODEZONEDIR	HOST_DIR "dist3/area/"  /* Used in do_copyzone - Samson 8-22-98 */
#define BUILDZONEDIR	HOST_DIR "dist2/area/"  /* Used in do_copyzone - Samson 4-7-98 */
#define MAINZONEDIR	HOST_DIR "dist/area/"   /* Used in do_copyzone - Samson 4-7-98 */
#define TESTCODEDIR     HOST_DIR "dist3/src/"   /* Used in do_copycode - Samson 4-7-98 */
#define BUILDCODEDIR    HOST_DIR "dist2/src/"   /* Used in do_copycode - Samson 8-22-98 */
#define MAINCODEDIR	HOST_DIR "dist/src/"    /* Used in do_copycode - Samson 4-7-98 */
#define CODESYSTEMDIR   HOST_DIR "dist3/system/"    /* Used in do_copysocial - Samson 5-2-98 */
#define BUILDSYSTEMDIR  HOST_DIR "dist2/system/"    /* Used in do_copysocial - Samson 5-2-98 */
#define MAINSYSTEMDIR   HOST_DIR "dist/system/" /* Used in do_copysocial - Samson 5-2-98 */
#define CODECLASSDIR	HOST_DIR "dist3/classes/"   /* Used in do_copyclass - Samson 9-17-98 */
#define BUILDCLASSDIR	HOST_DIR "dist2/classes/"   /* Used in do_copyclass - Samson 9-17-98 */
#define MAINCLASSDIR	HOST_DIR "dist/classes/"    /* Used in do_copyclass - Samson 9-17-98 */
#define CODERACEDIR	HOST_DIR "dist3/races/" /* Used in do_copyrace - Samson 10-13-98 */
#define BUILDRACEDIR	HOST_DIR "dist2/races/" /* Used in do_copyrace - Samson 10-13-98 */
#define MAINRACEDIR	HOST_DIR "dist/races/"  /* Used in do_copyrace - Samson 10-13-98 */
#define MAIN_STARTUP "swr"
#define PORT_STARTUP "startup"
#define MAIN_PORT    HOST_DIR
#define CODING_PORT  HOST_DIR "port/"
#define SOURCE_DIR   "src/"

#ifdef OVERLANDCODE
#define MAINMAPDIR	HOST_DIR "dist/maps/"   /* Used in do_copymap - Samson 8-2-99 */
#define BUILDMAPDIR	HOST_DIR "dist2/maps/"  /* Used in do_copymap - Samson 8-2-99 */
#define CODEMAPDIR	HOST_DIR "dist3/maps/"  /* Used in do_copymap - Samson 8-2-99 */
#endif

void      send_telcode(int desc, int ddww, int code);
bool      check_forks(DESCRIPTOR_DATA * d, char *cmdline);

extern bool compilelock;
extern bool bootlock;
