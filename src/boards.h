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
 *                              SWR Note and Board module                                *
 ****************************************************************************************/


typedef struct note_data NOTE_DATA;
typedef struct board_data BOARD_DATA;

#define VNUM_IDEA_BOARD 10123
#define VNUM_MAIL_BOARD 38
#define OBJ_VNUM_NOTE		     36

/* Board Types */
typedef enum
{ BOARD_NOTE, BOARD_MAIL, BOARD_IDEA, BOARD_GLOBAL, BOARD_MAX } e_board_types;
typedef enum
{ VOTE_NONE, VOTE_OPEN, VOTE_CLOSED, VOTE_MAX } e_vote_types;
extern char *const board_types[BOARD_MAX];
extern char *const vote_types[VOTE_MAX];
extern BOARD_DATA *first_board;
extern BOARD_DATA *last_board;

/*
 * Data structure for notes.
 */
struct note_data
{
        NOTE_DATA *next;
        NOTE_DATA *prev;
        char     *sender;
        char     *date;
        char     *to_list;
        char     *subject;
        int       voting;
        char     *yesvotes;
        char     *novotes;
        char     *abstentions;
        char     *text;
};

struct board_data
{
        BOARD_DATA *next;   /* Next board in list          */
        BOARD_DATA *prev;   /* Previous board in list      */
        NOTE_DATA *first_note;  /* First note on board         */
        NOTE_DATA *last_note;   /* Last note on board          */
        char     *note_file;    /* Filename to save notes to       */
        char     *board_name;   /* Name of the board.              */
        char     *read_group;   /* Can restrict a board to a       */
        char     *post_group;   /* council, clan, guild etc        */
        char     *extra_readers;    /* Can give read rights to players */
        char     *extra_removers;   /* Can give remove rights to players */
        int       board_obj;    /* Vnum of board object        */
        sh_int    num_posts;    /* Number of notes on this board   */
        sh_int    min_read_level;   /* Minimum level to read a note    */
        sh_int    min_post_level;   /* Minimum level to post a note    */
        sh_int    min_remove_level; /* Minimum level to remove a note  */
        sh_int    max_posts;    /* Maximum amount of notes allowed */
        int       type; /* Normal board or mail board? */
};

bool is_note_to args((CHAR_DATA * ch, NOTE_DATA * pnote));
bool is_note_to_def args((CHAR_DATA * ch, NOTE_DATA * pnote));
void note_attach args((CHAR_DATA * ch));
void      note_remove
args((CHAR_DATA * ch, BOARD_DATA * board, NOTE_DATA * pnote));
void note args((CHAR_DATA * ch, char *arg_passed, BOARD_DATA * board));
void write_board args((BOARD_DATA * board));

/* boards.c */
void load_boards args((void));
BOARD_DATA *get_board args((OBJ_DATA * obj));
void free_note args((NOTE_DATA * pnote));
