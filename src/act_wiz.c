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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <crypt.h>
#if defined(__CYGWIN__)
#include <crypt.h>
#endif
#include "mud.h"
#include "changes.h"
#include "boards.h"
#include "bounty.h"
#include "account.h"
#include "channels.h"
#include "body.h"
#include "races.h"
#include "olc_bounty.h"
#include "space2.h"
#include "installations.h"

#define RESTORE_INTERVAL 21600

extern bool WEBSERVER_STATUS;

char     *const save_flag[] =
        { "death", "kill", "passwd", "drop", "put", "give", "auto", "zap",
        "auction", "get", "receive", "idle", "backup", "who", "score", "list",
        "n",
        "e", "s", "w", "ne", "se", "nw", "sw", "look", "r25", "r26", "r27",
        "r28", "r29", "r30", "r31"
};

char     *const command_flags[] = {
        "admin", "builder", "highbuilder", "enforcer", "quest", "coder",
        "all", "owner", "highenforcer", "r9", "r10",
        "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19", "r20",
        "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29",
        "r30", "r31"
};

char     *const god_flags[] = {
        "admin", "builder", "highbuilder", "enforcer", "quest", "coder",
        "all", "owner", "highenforcer", "r9", "r10",
        "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19", "r20",
        "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29",
        "r30", "r31"
};

int get_commandflag(char *flag)
{
        int       x;

        for (x = 0; x < 32; x++)
                if (!str_cmp(flag, command_flags[x]))
                        return x;
        return -1;
}

int get_godflags(char *flag)
{
        int       x;

        for (x = 0; x < 32; x++)
                if (!str_cmp(flag, god_flags[x]))
                        return x;
        return -1;
}

char     *const connection_state[] = {
        "Get Name", "Get Password",
        "Conf Name", "Get New Pass", "Conf New Pass",
		"Get Act Email", 
        "Get Sex", "MOTD", "Get Race",
        "Emulate", "Rip/ANSI",
        "Title", "Press Enter", "Wait 1",
        "Wait 2", "Wait 3", "Accepted",
        "Get PKILL", "IMOTD", "Get Email",
        "Get MSP", "Get Class", "Roll stats",
        "Stat Options", "Edit Stats", "Conf Stats",
        "Copyover Recover", "Forked", "IAForked",
        "Wizinvis", "Stat Num", "Menu"
#ifdef ACCOUNT
                , "New Acc", "Get Acc", "Old Acc Pass",
        "Get Alt", "New Acc Pass", "Conf New Acc Pass",
        "Link Alts", "Get Link Pass", "Conf New Acc Name",
        "Get old Acc Pass", "Get Acc New Pass", "Get Conf New Pass"
#endif
        , "Playing", "Editing"
};


/* from comm.c */
bool write_to_descriptor args((int desc, char *txt, int length));
bool check_parse_name args((char *name));

/*
 * Local functions.
 */
/*ROOM_INDEX_DATA * find_location	args( ( CHAR_DATA *ch, char *arg ) ); */
void save_banlist args((void));

/*void              close_area    args( ( AREA_DATA *pArea ) );*/
void save_watchlist args((void));
void lstat_keys args((CHAR_DATA * ch));
char     *number_sign args((char *txt, int num));

int       get_color(char *argument);    /* function proto */
int       get_saveflag(char *name);
CHAR_DATA *get_waiting_desc(CHAR_DATA * ch, char *name);
void      extract_area_names(char *inp, char *out);
void      remove_area_names(char *inp, char *out);
void      unlink_social(SOCIALTYPE * social);
const char *name_expand(CHAR_DATA * ch);

/*
 * Global variables.
 */

char      reboot_time[50];
time_t    new_boot_time_t;
extern struct tm new_boot_struct;
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];  /* db.c */
extern int port;    /* Port number to be used       */
HELP_DATA *get_help(CHAR_DATA * ch, char *argument);


int get_saveflag(char *name)
{
        unsigned int x;

        for (x = 0; x < sizeof(save_flag) / sizeof(save_flag[0]); x++)
                if (!str_cmp(name, save_flag[x]))
                        return x;
        return -1;
}

struct wizhelp_s
{
        struct wizhelp_s *next;
        struct wizhelp_s *prev;
        CMDTYPE  *cmd;
};

char     *itoa(int foo)
{
        static char bar[256];

        sprintf(bar, "%d", foo);
        return (bar);

}

void output_help(CHAR_DATA * ch, struct wizhelp_s **first,
                 struct wizhelp_s **last)
{
        struct wizhelp_s *curr, *next;
        int       col = 0;

        set_pager_color(AT_WHITE, ch);
        for (curr = *first; curr; curr = next)
        {
                next = curr->next;
                UNLINK(curr, *first, *last, next, prev);
                if (get_help(ch, curr->cmd->name))
                        pager_printf(ch, "&C %-17s&W", curr->cmd->name);
                else
                        pager_printf(ch, "&R %-17s&W", curr->cmd->name);
                if (++col % 5 == 0)
                        send_to_pager("\n\r", ch);
                DISPOSE(curr);
        }
        if (col % 5 != 0)
                send_to_pager("\n\r", ch);

        send_to_pager("\n\r", ch);
}

CMDF do_wizhelp(CHAR_DATA * ch, char *argument)
{
        CMDTYPE  *cmd;
        int       hash;
        struct wizhelp_s *first_general, *last_general;
        struct wizhelp_s *first_enforcer, *last_enforcer;
        struct wizhelp_s *first_builder, *last_builder;
        struct wizhelp_s *first_highbuilder, *last_highbuilder;
        struct wizhelp_s *first_coder, *last_coder;
        struct wizhelp_s *first_admin, *last_admin;
        struct wizhelp_s *first_quest, *last_quest;
        struct wizhelp_s *first_owner, *last_owner;
        struct wizhelp_s *first_highenforcer, *last_highenforcer;
        struct wizhelp_s *curr;

        argument = NULL;

        /*
         * Initalizing them 
         */
        first_general = last_general = first_enforcer = last_enforcer =
                first_builder = last_builder = first_highbuilder =
                last_highbuilder = first_coder = last_coder = first_admin =
                last_admin = first_owner = last_owner = first_quest =
                last_quest = first_highenforcer = last_highenforcer = NULL;

        if (IS_NPC(ch))
                return;


        for (hash = 0; hash < 126; hash++)
        {
                for (cmd = command_hash[hash]; cmd; cmd = cmd->next)
                {
                        if (cmd->level < LEVEL_IMMORTAL)
                                continue;
                        if (!check_command(ch, cmd))
                                continue;

                        CREATE(curr, struct wizhelp_s, 1);

                        curr->cmd = cmd;

                        if (IS_SET(cmd->perm_flags, IMM_BUILDER))
                                LINK(curr, first_builder, last_builder, next, prev);    /* add to first_builder */
                        else if (IS_SET(cmd->perm_flags, IMM_HIGHBUILDER))
                                LINK(curr, first_highbuilder, last_highbuilder, next, prev);    /* add to highbuilder */
                        else if (IS_SET(cmd->perm_flags, IMM_HIGHENFORCER))
                                LINK(curr, first_highenforcer, last_highenforcer, next, prev);  /* add to hight enforcer */
                        else if (IS_SET(cmd->perm_flags, IMM_ENFORCER))
                                LINK(curr, first_enforcer, last_enforcer, next, prev);  /* add to enforcer */
                        else if (IS_SET(cmd->perm_flags, IMM_QUEST))
                                LINK(curr, first_quest, last_quest, next, prev);    /* add to quest */
                        else if (IS_SET(cmd->perm_flags, IMM_CODER))
                                LINK(curr, first_coder, last_coder, next, prev);    /* add to coder */
                        else if (IS_SET(cmd->perm_flags, IMM_ADMIN))
                                LINK(curr, first_admin, last_admin, next, prev);    /* add to admin */
                        else if (IS_SET(cmd->perm_flags, IMM_OWNER))
                                LINK(curr, first_owner, last_owner, next, prev);    /* add to owner */
                        else
                                LINK(curr, first_general, last_general, next, prev);    /* add to general */
                }
        }


        if (first_general)
        {
                send_to_pager("&B[&cGeneral&B]&W\n\r", ch);
                output_help(ch, &first_general, &last_general);
        }
        if (first_enforcer)
        {
                send_to_pager("&B[&cEnforcer&B]&W\n\r", ch);
                output_help(ch, &first_enforcer, &last_enforcer);
        }
        if (first_highenforcer)
        {
                send_to_pager("&B[&cHigh Enforcer&B]&W\n\r", ch);
                output_help(ch, &first_highenforcer, &last_enforcer);
        }
        if (first_builder)
        {
                send_to_pager("&B[&cBuilder&B]&W\n\r", ch);
                output_help(ch, &first_builder, &last_builder);
        }
        if (first_highbuilder)
        {
                send_to_pager("&B[&cHigh Builder&B]&W\n\r", ch);
                output_help(ch, &first_highbuilder, &last_highbuilder);
        }
        if (first_coder)
        {
                send_to_pager("&B[&cCoder&B]&W\n\r", ch);
                output_help(ch, &first_coder, &last_coder);
        }
        if (first_admin)
        {
                send_to_pager("&B[&cAdmin&B]&W\n\r", ch);
                output_help(ch, &first_admin, &last_admin);
        }
        if (first_quest)
        {
                send_to_pager("&B[&cQuest&B]&W\n\r", ch);
                output_help(ch, &first_quest, &last_quest);
        }
        if (first_owner)
        {
                send_to_pager("&B[&cOwner&B]&W\n\r", ch);
                output_help(ch, &first_owner, &last_owner);
        }

        return;
}


CMDF do_restrict(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        sh_int    level, hash;
        CMDTYPE  *cmd;
        bool      found;

        found = FALSE;

        argument = one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Restrict which command?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg2);
        if (arg2[0] == '\0')
                level = get_trust(ch);
        else
                level = atoi(arg2);

        level = UMAX(UMIN(get_trust(ch), level), 0);

        hash = arg[0] % 126;
        for (cmd = command_hash[hash]; cmd; cmd = cmd->next)
        {
                if (!str_prefix(arg, cmd->name)
                    && cmd->level <= get_trust(ch))
                {
                        found = TRUE;
                        break;
                }
        }

        if (found)
        {
                if (!str_prefix(arg2, "show"))
                {
                        snprintf(buf, MSL, "%s show", cmd->name);
                        do_cedit(ch, buf);
/*    		ch_printf( ch, "%s is at level %d.\n\r", cmd->name, cmd->level );*/
                        return;
                }
                cmd->level = level;
                ch_printf(ch, "You restrict %s to level %d\n\r",
                          cmd->name, level);
                snprintf(buf, MSL, "%s restricting %s to level %d",
                         ch->name, cmd->name, level);
                log_string(buf);
        }
        else
                send_to_char("You may not restrict that command.\n\r", ch);

        return;
}

/* 
 * Check if the name prefix uniquely identifies a char descriptor
 */
CHAR_DATA *get_waiting_desc(CHAR_DATA * ch, char *name)
{
        DESCRIPTOR_DATA *d;
        CHAR_DATA *ret_char = NULL;
        static unsigned int number_of_hits;

        number_of_hits = 0;
        for (d = first_descriptor; d; d = d->next)
        {
                if (d->character && (!str_prefix(name, d->character->name)) &&
                    IS_WAITING_FOR_AUTH(d->character))
                {
                        if (++number_of_hits > 1)
                        {
                                ch_printf(ch,
                                          "%s does not uniquely identify a char.\n\r",
                                          name);
                                return NULL;
                        }
                        ret_char = d->character;    /* return current char on exit */
                }
        }
        if (number_of_hits == 1)
                return ret_char;
        else
        {
                send_to_char
                        ("No one like that waiting for authorization.\n\r",
                         ch);
                return NULL;
        }
}

CMDF do_authorize(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *victim;
        DESCRIPTOR_DATA *d;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char
                        ("Usage:  authorize <player> <yes|name|no/deny>\n\r",
                         ch);
                send_to_char("Pending authorizations:\n\r", ch);
                send_to_char(" Chosen Character Name\n\r", ch);
                send_to_char
                        ("---------------------------------------------\n\r",
                         ch);
                for (d = first_descriptor; d; d = d->next)
                        if ((victim = d->character) != NULL
                            && IS_WAITING_FOR_AUTH(victim))
                                ch_printf(ch, " %s@%s new %s...\n\r",
                                          victim->name, victim->desc->host,
                                          victim->race->name());
                return;
        }

        victim = get_waiting_desc(ch, arg1);
        if (victim == NULL)
                return;

        if (arg2[0] == '\0' || !str_cmp(arg2, "accept")
            || !str_cmp(arg2, "yes"))
        {
                victim->pcdata->auth_state = 3;
                REMOVE_BIT(victim->pcdata->flags, PCFLAG_UNAUTHED);
                if (victim->pcdata->authed_by)
                        STRFREE(victim->pcdata->authed_by);
                victim->pcdata->authed_by = QUICKLINK(ch->name);
                snprintf(buf, MSL, "%s authorized %s", ch->name,
                         victim->name);
                log_string_plus(buf, LOG_COMM, ch->top_level);
                ch_printf(ch, "You have authorized %s.\n\r", victim->name);

                /*
                 * Below sends a message to player when name is accepted - Brittany   
                 */

                ch_printf(victim,   /* B */
                          "The MUD Administrators have accepted the name %s.\n\r"   /* B */
                          "You are now fully authorized to play SWR.\n\r", victim->name);   /* B */
                return;
        }
        else if (!str_cmp(arg2, "no") || !str_cmp(arg2, "deny"))
        {
                send_to_char("You have been denied access.\n\r", victim);
                snprintf(buf, MSL, "%s denied authorization to %s", ch->name,
                         victim->name);
                ch_printf(ch, "You have denied %s.\n\r", victim->name);
                log_string_plus(buf, LOG_COMM, ch->top_level);
                do_quit(victim, "");
        }

        else if (!str_cmp(arg2, "name") || !str_cmp(arg2, "n"))
        {
                snprintf(buf, MSL, "%s has denied %s's name", ch->name,
                         victim->name);
                log_string_plus(buf, LOG_COMM, ch->top_level);
                ch_printf(victim,
                          "The MUD Administrators have found the name %s "
                          "to be unacceptable.\n\r"
                          "Use 'name' to change it to something more apropriate.\n\r",
                          victim->name);
                ch_printf(ch, "You requested %s change names.\n\r",
                          victim->name);
                victim->pcdata->auth_state = 2;
                return;
        }

        else
        {
                send_to_char("Invalid argument.\n\r", ch);
                return;
        }
}

CMDF do_bamfin(CHAR_DATA * ch, char *argument)
{
        if (!IS_NPC(ch))
        {
                smash_tilde(argument);
                STRFREE(ch->pcdata->bamfin);
                ch->pcdata->bamfin = STRALLOC(argument);
                send_to_char("Ok.\n\r", ch);
        }
        return;
}



CMDF do_bamfout(CHAR_DATA * ch, char *argument)
{
        if (!IS_NPC(ch))
        {
                smash_tilde(argument);
                STRFREE(ch->pcdata->bamfout);
                ch->pcdata->bamfout = STRALLOC(argument);
                send_to_char("Ok.\n\r", ch);
        }
        return;
}

CMDF do_rank(CHAR_DATA * ch, char *argument)
{
        if (IS_NPC(ch))
                return;

        if (!argument || argument[0] == '\0')
        {
                send_to_char("Usage: rank <string>.\n\r", ch);
                send_to_char("   or: rank none.\n\r", ch);
                return;
        }

        smash_tilde(argument);
        STRFREE(ch->pcdata->rank);
        if (!str_cmp(argument, "none"))
                ch->pcdata->rank = STRALLOC("");
        else
                ch->pcdata->rank = STRALLOC(argument);
        send_to_char("Ok.\n\r", ch);

        return;
}


/*
 *       New MUD Statistics for LOTJ ;)
 * ... The main interface command is LSTAT ...
 *           -=# Added by Ghost #=-
 *         [Modified by Greven for DW]
 */
CMDF do_mudstat(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_STRING_LENGTH];
        char      buf[MAX_STRING_LENGTH];

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || arg == NULL)
        {
                send_to_char("&RSyntax: MUDSTAT <Statistic>\n\r", ch);
                send_to_char
                        ("&R - Keys, Stats, OAverage, MAverage, Files, Homes\n\r",
                         ch);
                return;
        }

        if (!str_cmp(arg, "keys"))
        {
                lstat_keys(ch);
        }
        else if (!str_cmp(arg, "stats"))
        {
                /*
                 * ch_printf( ch, "MaxEver %5d    Topsn   %5d (%d)\n\r", sysdata.alltimemax, top_sn, MAX_SKILL );
                 * ch_printf( ch, "MaxEver time recorded at:   %s\n\r", sysdata.time_of_max ); 
                 */
                do_memory(ch, "");
        }
        else if (!str_cmp(arg, "oaverage"))
        {
        }
        else if (!str_cmp(arg, "maverage"))
        {
        }
        else if (!str_cmp(arg, "files"))
        {
                ch_printf(ch, "&zFile size tracking:\n\r");
                ch_printf(ch,
                          "&z-------------------------------------------------------\n\r");
                snprintf(buf, MSL, "%s%s", SYSTEM_DIR, BUG_FILE);
                ch_printf(ch, "&W Bug list     (bugs.txt)  - &C%s bytes\n\r",
                          num_punct(file_size(buf)));
                snprintf(buf, MSL, "%s%s", SYSTEM_DIR, BUGS_FILE);
                ch_printf(ch, "&W Bug list     (bugs.txt)  - &C%s bytes\n\r",
                          num_punct(file_size(buf)));
                snprintf(buf, MSL, "%s%s", SYSTEM_DIR, LOG_FILE);
                ch_printf(ch, "&W Speech Log   (log.txt)   - &C%s bytes\n\r",
                          num_punct(file_size(buf)));
                snprintf(buf, MSL, "%s%s", SYSTEM_DIR, IDEA_FILE);
                ch_printf(ch, "&W Idea Log     (idea.txt)  - &C%s bytes\n\r",
                          num_punct(file_size(buf)));
                snprintf(buf, MSL, "%s%s", SYSTEM_DIR, HELP_FILE);
                ch_printf(ch, "&W Help Log     (help.txt)  - &C%s bytes\n\r",
                          num_punct(file_size(buf)));
                snprintf(buf, MSL, "%s%s", LOG_DIR, "swr.lg");
                if (file_exist(buf))
                        ch_printf(ch,
                                  "&W SWR Log      (swr.log)   - &C%s bytes\n\r",
                                  num_punct(file_size(buf)));
                return;
        }
        else if (!str_cmp(arg, "homes"))
        {
                ROOM_INDEX_DATA *room;
                AREA_DATA *area;
                int       col = 0, vnum = 0;

                ch_printf(ch,
                          "&zList of homes in DW: (&WRed = Owned, Blue = Empty&z)\n\r");
                ch_printf(ch,
                          "&z-------------------------------------------------------\n\r");
                for (area = first_area; area; area = area->next)
                {
                        if (!area || area == NULL)
                                continue;

                        for (vnum = area->low_r_vnum; vnum <= area->hi_r_vnum;
                             vnum++)
                        {
                                if ((room = get_room_index(vnum)) != NULL)
                                {
                                        if (xIS_SET
                                            (room->room_flags, ROOM_PLR_HOME))
                                        {
                                                ch_printf(ch, "&R%-8d ",
                                                          room->vnum);
                                                if (++col % 5 == 0)
                                                        send_to_char("\n\r",
                                                                     ch);
                                        }
                                        else if (xIS_SET
                                                 (room->room_flags,
                                                  ROOM_EMPTY_HOME))
                                        {
                                                ch_printf(ch, "&B%-8d ",
                                                          room->vnum);
                                                if (++col % 5 == 0)
                                                        send_to_char("\n\r",
                                                                     ch);
                                        }
                                }
                        }
                }
                if (col % 3 != 0)
                        send_to_char("\n\r", ch);
        }
        else
        {
                send_to_char("&RInvalid Statistic, Try again!\n\r", ch);
                do_mudstat(ch, "");
        }

        return;
}

void lstat_keys(CHAR_DATA * ch)
{
        int       count = 0;
        OBJ_DATA *obj;

        send_to_char("\n\r&YName:           Vnum:       Lock ID:  \n\r", ch);
        set_char_color(AT_BLUE, ch);
        for (obj = first_object; obj; obj = obj->next)
        {
                if (obj->item_type == ITEM_KEY)
                {
                        ch_printf(ch, "%-15s %-12ld %-12ld\n\r",
                                  smash_color(obj->short_descr),
                                  obj->pIndexData->vnum, obj->value[0]);
                        count++;
                }
                if (count >= 300)
                        break;
        }

        if (count < 300)
                ch_printf(ch, "Objects counted: %ld\n\r", count);
        else
                ch_printf(ch, "%ld Object counted. Limit break tripped.\n\r",
                          count);

        return;
}



CMDF do_prestore(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        char      buf2[MAX_STRING_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("&w&RSyntax: PRestore <character name>\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) != NULL)
        {
                if (!str_cmp(victim->name, arg) && !IS_NPC(victim))
                {
                        ch_printf(ch, "&RNot while %s is still online.\n\r",
                                  victim->name);
                        return;
                }
        }

        snprintf(buf, MSL, "%s%c/%s", PLAYER_DIR, tolower(arg[0]),
                 capitalize(arg));
        snprintf(buf2, MSL, "%s%c/%s", BACKUP_DIR, tolower(arg[0]),
                 capitalize(arg));

        if (!file_exist(buf2))
        {
                ch_printf(ch, "&RNo backup located at '%s'.\n\r", buf2);
                return;
        }

        rename(buf2, buf);

        if (!file_exist(buf))
        {
                ch_printf(ch, "&RFailed backup restore from '%s'.\n\r", buf2);
                return;
        }

        ch_printf(ch, "&GRestored backup from '%s'.\n\r", buf2);

        return;
}

CMDF do_retire(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Retire whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (get_trust(victim) >= get_trust(ch))
        {
                send_to_char("You failed.\n\r", ch);
                return;
        }

        if (victim->top_level < LEVEL_SAVIOR)
        {
                send_to_char
                        ("The minimum level for retirement is savior.\n\r",
                         ch);
                return;
        }

        if (IS_RETIRED(victim))
        {
                REMOVE_BIT(victim->pcdata->flags, PCFLAG_RETIRED);
                ch_printf(ch, "%s returns from retirement.\n\r",
                          victim->name);
                ch_printf(victim, "%s brings you back from retirement.\n\r",
                          ch->name);
        }
        else
        {
                SET_BIT(victim->pcdata->flags, PCFLAG_RETIRED);
                ch_printf(ch, "%s is now a retired immortal.\n\r",
                          victim->name);
                ch_printf(victim,
                          "Courtesy of %s, you are now a retired immortal.\n\r",
                          ch->name);
        }
        return;
}

CMDF do_deny(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Deny whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (get_trust(victim) >= get_trust(ch))
        {
                send_to_char("You failed.\n\r", ch);
                return;
        }

        SET_BIT(victim->act, PLR_DENY);
        send_to_char("You are denied access!\n\r", victim);
        send_to_char("OK.\n\r", ch);
        do_quit(victim, "");

        return;
}



CMDF do_disconnect(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        DESCRIPTOR_DATA *d;
        CHAR_DATA *victim;

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Disconnect whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim->desc == NULL)
        {
                act(AT_PLAIN, "$N doesn't have a descriptor.", ch, NULL,
                    victim, TO_CHAR);
                return;
        }

        if (get_trust(ch) <= get_trust(victim))
        {
                send_to_char("They might not like that...\n\r", ch);
                return;
        }

        for (d = first_descriptor; d; d = d->next)
        {
                if (d == victim->desc)
                {
                        close_socket(d, FALSE);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
        }

        bug("Do_disconnect: *** desc not found ***.", 0);
        send_to_char("Descriptor not found!\n\r", ch);
        return;
}

/*
 * Force a level one player to quit.             Gorog
 * - Changed to any level character
 */
CMDF do_fquit(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        char      arg1[MAX_INPUT_LENGTH];
        ROOM_INDEX_DATA *room = NULL;

        argument = one_argument(argument, arg1);

        if (arg1[0] == '\0')
        {
                send_to_char("Force whom to quit?\n\r", ch);
                return;
        }

        if (!(victim = get_char_world(ch, arg1)))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

/*	if ( victim->top_level != 1 )  
	{ 
		send_to_char( "They are not level one!\n\r", ch ); 
		return; 
	} */

        send_to_char("The MUD administrators force you to quit\n\r", victim);
        if (!xIS_SET(victim->in_room->room_flags, ROOM_SAFE))
        {
                xSET_BIT(victim->in_room->room_flags, ROOM_SAFE);
                room = victim->in_room;
        }
        do_quit(victim, "");
        if (room)
        {
                xREMOVE_BIT(room->room_flags, ROOM_SAFE);
        }
        send_to_char("Ok.\n\r", ch);
        return;
}


CMDF do_forceclose(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        DESCRIPTOR_DATA *d;
        int       desc;

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Usage: forceclose <descriptor#>\n\r", ch);
                return;
        }
        desc = atoi(arg);

        for (d = first_descriptor; d; d = d->next)
        {
                if (d->descriptor == desc)
                {
                        if (d->character
                            && get_trust(d->character) >= get_trust(ch))
                        {
                                send_to_char
                                        ("They might not like that...\n\r",
                                         ch);
                                return;
                        }
                        close_socket(d, FALSE);
                        send_to_char("Ok.\n\r", ch);
                        return;
                }
        }

        send_to_char("Not found!\n\r", ch);
        return;
}



CMDF do_pardon(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        PLANET_DATA *planet = NULL;

        argument = one_argument(argument, arg1);

        if (arg1[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("Syntax: pardon <character> <planet>.\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg1)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if ((planet = get_planet(argument)) != NULL
            && planet->governed_by != NULL)
        {
                send_to_char("Wanted Removed.\n\r", ch);
                remove_wanted_planet(victim, planet);
                ch_printf(victim, "You are no longer wanted by %s.\n\r",
                          planet->governed_by->name);
        }
        return;
}


void echo_to_all(sh_int AT_COLOR, char *argument, sh_int tar)
{
        DESCRIPTOR_DATA *d;

        if (!argument || argument[0] == '\0')
                return;

        for (d = first_descriptor; d; d = d->next)
        {
                if (!d->character)
                        continue;
                /*
                 * Added showing echoes to players who are editing, so they won't
                 * miss out on important info like upcoming reboots. --Narn 
                 */
                if (IS_PLAYING(d) || d->connected == CON_EDITING)
                {
                        /*
                         * This one is kinda useless except for switched.. 
                         */
                        if (tar == ECHOTAR_PC && IS_NPC(d->character))
                                continue;
                        else if (tar == ECHOTAR_IMM
                                 && !IS_IMMORTAL(d->character))
                                continue;
                        set_char_color(AT_COLOR, d->character);
                        send_to_char(argument, d->character);
                        send_to_char("\n\r", d->character);
                }
        }
        return;
}

void echo_to_clan(sh_int AT_COLOR, char *argument, CLAN_DATA * clan)
{
        DESCRIPTOR_DATA *d;

        if (!argument || argument[0] == '\0')
                return;

        for (d = first_descriptor; d; d = d->next)
        {
                if (!d->character)
                        continue;
                /*
                 * Added showing echoes to players who are editing, so they won't
                 * miss out on important info like upcoming reboots. --Narn 
                 */
                if ((IS_PLAYING(d) || d->connected == CON_EDITING)
                    && (d->character->pcdata->clan
                        && d->character->pcdata->clan == clan))
                {
                        set_char_color(AT_COLOR, d->character);
                        send_to_char(argument, d->character);
                        send_to_char("\n\r", d->character);
                }
        }
        return;
}

CMDF do_echo(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        sh_int    color;
        int       target;
        char     *parg;

        if (IS_SET(ch->act, PLR_NO_EMOTE))
        {
                send_to_char("You are noemoted and can not echo.\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Echo what?\n\r", ch);
                return;
        }

        if ((color = get_color(argument)))
                argument = one_argument(argument, arg);
        parg = argument;
        argument = one_argument(argument, arg);
        if (!str_cmp(arg, "PC") || !str_cmp(arg, "player"))
                target = ECHOTAR_PC;
        else if (!str_cmp(arg, "imm"))
                target = ECHOTAR_IMM;
        else
        {
                target = ECHOTAR_ALL;
                argument = parg;
        }
        if (!color && (color = get_color(argument)))
                argument = one_argument(argument, arg);
        if (!color)
                color = AT_IMMORT;
        one_argument(argument, arg);
        if (!str_cmp(arg, "Merth") || !str_cmp(arg, "Durga"))
        {
                ch_printf(ch, "I don't think %s would like that!\n\r", arg);
                return;
        }
        echo_to_all(color, convert_newline(argument), target);
}

void echo_to_room(sh_int AT_COLOR, ROOM_INDEX_DATA * room, char *argument)
{
        CHAR_DATA *vic;

        if (room == NULL)
                return;


        for (vic = room->first_person; vic; vic = vic->next_in_room)
        {
                set_char_color(AT_COLOR, vic);
                send_to_char(argument, vic);
                send_to_char("\n\r", vic);
        }
}

CMDF do_recho(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        sh_int    color;

        if (IS_SET(ch->act, PLR_NO_EMOTE))
        {
                send_to_char("You are noemoted and can not recho.\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Recho what?\n\r", ch);
                return;
        }

        one_argument(argument, arg);
        if (!str_cmp(arg, "Thoric")
            || !str_cmp(arg, "Dominus")
            || !str_cmp(arg, "Circe")
            || !str_cmp(arg, "Haus")
            || !str_cmp(arg, "Narn")
            || !str_cmp(arg, "Scryn")
            || !str_cmp(arg, "Blodkai") || !str_cmp(arg, "Damian"))
        {
                ch_printf(ch, "I don't think %s would like that!\n\r", arg);
                return;
        }
        if ((color = get_color(argument)))
        {
                argument = one_argument(argument, arg);
                echo_to_room(color, ch->in_room, argument);
        }
        else
                echo_to_room(AT_IMMORT, ch->in_room, argument);
}


ROOM_INDEX_DATA *find_location(CHAR_DATA * ch, char *arg)
{
        CHAR_DATA *victim;
        OBJ_DATA *obj;

        if (is_number(arg))
                return get_room_index(atoi(arg));

        if ((victim = get_char_world(ch, arg)) != NULL)
                return victim->in_room;

        if ((obj = get_obj_world(ch, arg)) != NULL)
                return obj->in_room;

        return NULL;
}



CMDF do_transfer(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        ROOM_INDEX_DATA *location;
        DESCRIPTOR_DATA *d;
        CHAR_DATA *victim;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char("Transfer whom (and where)?\n\r", ch);
                return;
        }

        if (!str_cmp(arg1, "all"))
        {
                for (d = first_descriptor; d; d = d->next)
                {
                        if (IS_PLAYING(d)
                            && d->character != ch
                            && d->character->in_room
                            && d->newstate != 2 && can_see(ch, d->character))
                        {
                                char      buf[MAX_STRING_LENGTH];

                                snprintf(buf, MSL, "%s %s",
                                         d->character->name, arg2);
                                do_transfer(ch, buf);
                        }
                }
                return;
        }

        /*
         * Thanks to Grodyn for the optional location parameter.
         */
        if (arg2[0] == '\0')
        {
                location = ch->in_room;
        }
        else
        {
                if ((location = find_location(ch, arg2)) == NULL)
                {
                        send_to_char("No such location.\n\r", ch);
                        return;
                }

                if (room_is_private(ch, location)
                    && get_trust(ch) < LEVEL_GOD)
                {
                        send_to_char("That room is private right now.\n\r",
                                     ch);
                        return;
                }
        }

        if ((victim = get_char_world(ch, arg1)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (NOT_AUTHED(victim))
        {
                send_to_char("They are not authorized yet!\n\r", ch);
                return;
        }

        if (!victim->in_room)
        {
                send_to_char("They are in limbo.\n\r", ch);
                return;
        }

        if (victim->fighting)
                stop_fighting(victim, TRUE);
        act(AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim,
            NULL, NULL, TO_ROOM);
        victim->retran = victim->in_room->vnum;
        char_from_room(victim);
        char_to_room(victim, location);
        act(AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL,
            TO_ROOM);
        if (ch != victim)
                act(AT_IMMORT, "$n has transferred you.", ch, NULL, victim,
                    TO_VICT);
        do_look(victim, "auto");
        send_to_char("Ok.\n\r", ch);
        if (!IS_IMMORTAL(victim) && !IS_NPC(victim)
            && !in_hard_range(victim, location->area))
                send_to_char
                        ("Warning: the player's level is not within the area's level range.\n\r",
                         ch);
}

CMDF do_retran(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        char      buf[MAX_STRING_LENGTH];

        argument = one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Retransfer whom?\n\r", ch);
                return;
        }
        if (!(victim = get_char_world(ch, arg)))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }
        snprintf(buf, MSL, "'%s' %d", victim->name, victim->retran);
        do_transfer(ch, buf);
        return;
}

CMDF do_regoto(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];

        argument = NULL;

        snprintf(buf, MSL, "%d", ch->regoto);
        do_goto(ch, buf);
        return;
}

CMDF do_at(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        ROOM_INDEX_DATA *location;
        ROOM_INDEX_DATA *original;
        CHAR_DATA *wch;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("At where what?\n\r", ch);
                return;
        }

        if ((location = find_location(ch, arg)) == NULL)
        {
                send_to_char("No such location.\n\r", ch);
                return;
        }

        if (room_is_private(ch, location))
        {
                if (get_trust(ch) < LEVEL_GREATER)
                {
                        send_to_char("That room is private right now.\n\r",
                                     ch);
                        return;
                }
                else
                {
                        send_to_char("Overriding private flag!\n\r", ch);
                }

        }

        original = ch->in_room;
        char_from_room(ch);
        char_to_room(ch, location);
        interpret(ch, argument);

        /*
         * See if 'ch' still exists before continuing!
         * Handles 'at XXXX quit' case.
         */
        for (wch = first_char; wch; wch = wch->next)
        {
                if (wch == ch)
                {
                        char_from_room(ch);
                        char_to_room(ch, original);
                        break;
                }
        }

        return;
}

CMDF do_rat(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        ROOM_INDEX_DATA *location;
        ROOM_INDEX_DATA *original;
        DESCRIPTOR_DATA *d;
        int       Start, End, vnum;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (IS_NPC(ch))
                return;

        d = ch->desc;

        if (arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("Syntax: rat <start> <end> <command>\n\r", ch);
                return;
        }

        Start = atoi(arg1);
        End = atoi(arg2);

        if (Start < 1 || End < Start || Start > End || Start == End
            || End > MAX_VNUMS)
        {
                send_to_char("Invalid range.\n\r", ch);
                return;
        }

        if (!str_cmp(argument, "quit"))
        {
                send_to_char("I don't think so!\n\r", ch);
                return;
        }

        original = ch->in_room;
        for (vnum = Start; vnum <= End; vnum++)
        {
                if ((location = get_room_index(vnum)) == NULL)
                        continue;
                char_from_room(ch);
                char_to_room(ch, location);
                interpret(ch, argument);
                /*
                 * We buffer overflowed, so no idea what happened to characters now 
                 */
                if (!d->character)
                        break;
        }

        char_from_room(ch);
        char_to_room(ch, original);
        send_to_char("Done.\n\r", ch);
        return;
}


CMDF do_rstat(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      arg[MAX_INPUT_LENGTH];
        ROOM_INDEX_DATA *location;
        OBJ_DATA *obj;
        CHAR_DATA *rch;
        EXIT_DATA *pexit;
        int       cnt;
        static char *dir_text[] =
                { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };

        one_argument(argument, arg);

        if (get_trust(ch) < LEVEL_IMMORTAL)
        {
                AREA_DATA *pArea;

                if (!ch->pcdata || !(pArea = ch->pcdata->area))
                {
                        send_to_char
                                ("You must have an assigned area to goto.\n\r",
                                 ch);
                        return;
                }

                if (ch->in_room->vnum < pArea->low_r_vnum
                    || ch->in_room->vnum > pArea->hi_r_vnum)
                {
                        send_to_char
                                ("You can only rstat within your assigned range.\n\r",
                                 ch);
                        return;
                }

        }


        if (!str_cmp(arg, "exits"))
        {
                location = ch->in_room;

                ch_printf(ch, "Exits for room '%s.' vnum %d\n\r",
                          location->name, location->vnum);

                for (cnt = 0, pexit = location->first_exit; pexit;
                     pexit = pexit->next)
                        ch_printf(ch,
                                  "%2d) %2s to %-5d.  Key: %d  Flags: %d  Keywords: '%s'.\n\rDescription: %sExit links back to vnum: %d  Exit's RoomVnum: %d  Distance: %d\n\r",
                                  ++cnt, dir_text[pexit->vdir],
                                  pexit->to_room ? pexit->to_room->vnum : 0,
                                  pexit->key, pexit->exit_info,
                                  pexit->keyword,
                                  pexit->description[0] !=
                                  '\0' ? pexit->description : "(none).\n\r",
                                  pexit->rexit ? pexit->rexit->vnum : 0,
                                  pexit->rvnum, pexit->distance);
                return;
        }
        location = (arg[0] == '\0') ? ch->in_room : find_location(ch, arg);
        if (!location)
        {
                send_to_char("No such location.\n\r", ch);
                return;
        }

        if (ch->in_room != location && room_is_private(ch, location))
        {
                if (get_trust(ch) < LEVEL_GREATER)
                {
                        send_to_char("That room is private right now.\n\r",
                                     ch);
                        return;
                }
                else
                {
                        send_to_char("Overriding private flag!\n\r", ch);
                }

        }

        ch_printf(ch, "Name: %s.\n\rArea: %s  Filename: %s.\n\r",
                  location->name,
                  location->area ? location->area->name : "None????",
                  location->area ? location->area->filename : "None????");

        ch_printf(ch,
                  "Vnum: %d.  Sector: %d.  Light: %d.  Tunnel: %d.\n\r",
                  location->vnum,
                  location->sector_type, location->light, location->tunnel);

        ch_printf(ch, "Room flags: %s\n\r",
                  ext_flag_string(&location->room_flags, r_flags));
        ch_printf(ch, "Description:\n\r%s", location->description);

        if (location->first_extradesc)
        {
                EXTRA_DESCR_DATA *ed;

                send_to_char("Extra description keywords: '", ch);
                for (ed = location->first_extradesc; ed; ed = ed->next)
                {
                        send_to_char(ed->keyword, ch);
                        if (ed->next)
                                send_to_char(" ", ch);
                }
                send_to_char("'.\n\r", ch);
        }

        send_to_char("Characters:", ch);
        for (rch = location->first_person; rch; rch = rch->next_in_room)
        {
                if (can_see(ch, rch))
                {
                        send_to_char(" ", ch);
                        one_argument(rch->name, buf);
                        send_to_char(buf, ch);
                }
        }

        send_to_char(".\n\rObjects:   ", ch);
        for (obj = location->first_content; obj; obj = obj->next_content)
        {
                send_to_char(" ", ch);
                one_argument(obj->name, buf);
                send_to_char(buf, ch);
        }
        send_to_char(".\n\r", ch);

        if (location->first_exit)
                send_to_char
                        ("------------------- EXITS -------------------\n\r",
                         ch);
        for (cnt = 0, pexit = location->first_exit; pexit;
             pexit = pexit->next)
                ch_printf(ch,
                          "%2d) %-2s to %-5d.  Key: %d  Flags: %d  Keywords: %s.\n\r",
                          ++cnt, dir_text[pexit->vdir],
                          pexit->to_room ? pexit->to_room->vnum : 0,
                          pexit->key, pexit->exit_info,
                          pexit->keyword[0] !=
                          '\0' ? pexit->keyword : "(none)");
        return;
}

CMDF do_ostat(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        AFFECT_DATA *paf;
        OBJ_DATA *obj;
        char     *pdesc;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Ostat what?\n\r", ch);
                return;
        }
        if (arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg))
                mudstrlcpy(arg, argument, MIL);

        if ((obj = get_obj_world(ch, arg)) == NULL)
        {
                send_to_char
                        ("Nothing like that in hell, earth, or heaven.\n\r",
                         ch);
                return;
        }

        ch_printf(ch, "Name: %s.\n\r", obj->name);

        pdesc = get_extra_descr(arg, obj->first_extradesc);
        if (!pdesc)
                pdesc = get_extra_descr(arg,
                                        obj->pIndexData->first_extradesc);
        if (!pdesc)
                pdesc = get_extra_descr(obj->name, obj->first_extradesc);
        if (!pdesc)
                pdesc = get_extra_descr(obj->name,
                                        obj->pIndexData->first_extradesc);
        if (pdesc)
                send_to_char(pdesc, ch);


        ch_printf(ch, "Vnum: %d.  Type: %s.  Count: %d  Gcount: %d\n\r",
                  obj->pIndexData->vnum, item_type_name(obj),
                  obj->pIndexData->count, obj->count);

        ch_printf(ch, "Short description: %s.\n\rLong description: %s\n\r",
                  obj->short_descr, obj->description);

        if (obj->action_desc[0] != '\0')
                ch_printf(ch, "Action description: %s.\n\r",
                          obj->action_desc);

        ch_printf(ch, "Wear flags : %s\n\r",
                  flag_string(obj->wear_flags, w_flags));
        ch_printf(ch, "Extra flags: %s\n\r",
                  flag_string(obj->extra_flags, o_flags));

        ch_printf(ch, "Number: %d/%d.  Weight: %d/%d.  Layers: %d\n\r",
                  1, get_obj_number(obj),
                  obj->weight, get_obj_weight(obj), obj->pIndexData->layers);

        ch_printf(ch, "Cost: %d.  Rent: %d.  Timer: %d.  Level: %d.\n\r",
                  obj->cost, obj->pIndexData->rent, obj->timer, obj->level);

        ch_printf(ch,
                  "In room: %d.  In object: %s.  Carried by: %s.  Wear_loc: %d.\n\r",
                  obj->in_room == NULL ? 0 : obj->in_room->vnum,
                  obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr,
                  obj->carried_by == NULL ? "(none)" : obj->carried_by->name,
                  obj->wear_loc);

        ch_printf(ch, "Index Values : %d %d %d %d %d %d.\n\r",
                  obj->pIndexData->value[0], obj->pIndexData->value[1],
                  obj->pIndexData->value[2], obj->pIndexData->value[3],
                  obj->pIndexData->value[4], obj->pIndexData->value[5]);
        ch_printf(ch, "Object Values: %d %d %d %d %d %d.\n\r",
                  obj->value[0], obj->value[1], obj->value[2], obj->value[3],
                  obj->value[4], obj->value[5]);

        if (obj->pIndexData->first_extradesc)
        {
                EXTRA_DESCR_DATA *ed;

                send_to_char("Primary description keywords:   '", ch);
                for (ed = obj->pIndexData->first_extradesc; ed; ed = ed->next)
                {
                        send_to_char(ed->keyword, ch);
                        if (ed->next)
                                send_to_char(" ", ch);
                }
                send_to_char("'.\n\r", ch);
        }
        if (obj->first_extradesc)
        {
                EXTRA_DESCR_DATA *ed;

                send_to_char("Secondary description keywords: '", ch);
                for (ed = obj->first_extradesc; ed; ed = ed->next)
                {
                        send_to_char(ed->keyword, ch);
                        if (ed->next)
                                send_to_char(" ", ch);
                }
                send_to_char("'.\n\r", ch);
        }

        for (paf = obj->first_affect; paf; paf = paf->next)
                ch_printf(ch, "Affects %s by %d. (extra)\n\r",
                          affect_loc_name(paf->location), paf->modifier);

        for (paf = obj->pIndexData->first_affect; paf; paf = paf->next)
                ch_printf(ch, "Affects %s by %d.\n\r",
                          affect_loc_name(paf->location), paf->modifier);

        return;
}


CMDF do_oldmstat(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        AFFECT_DATA *paf;
        CHAR_DATA *victim;
        SKILLTYPE *skill;

        set_char_color(AT_PLAIN, ch);

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Mstat whom?\n\r", ch);
                return;
        }
        if (arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg))
                mudstrlcpy(arg, argument, MIL);

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }
        if (get_trust(ch) < get_trust(victim) && !IS_NPC(victim))
        {
                set_char_color(AT_IMMORT, ch);
                send_to_char
                        ("Their godly glow prevents you from getting a good look.\n\r",
                         ch);
                return;
        }

        if (IS_NPC(victim))
        {
                ch_printf(ch, "Name: %s     Organization: %s\n\r",
                          victim->name,
                          (!get_clan(victim->mob_clan)) ? "(none)"
                          : get_clan(victim->mob_clan)->name);
        }
        else
        {
                ch_printf(ch, "Name: %s     Organization: %s\n\r",
                          victim->name,
                          (!victim->pcdata->clan) ? "(none)"
                          : victim->pcdata->clan->name);
        }


        if (get_trust(ch) >= LEVEL_GOD && !IS_NPC(victim) && victim->desc)
                ch_printf(ch,
                          "User: %s   Descriptor: %d   Trust: %d   AuthedBy: %s\n\r",
                          victim->desc->host, victim->desc->descriptor,
                          victim->trust,
                          victim->pcdata->authed_by[0] !=
                          '\0' ? victim->pcdata->authed_by : "(unknown)");
        if (!IS_NPC(victim) && victim->pcdata->release_date != 0)
                ch_printf(ch, "Helled until %24.24s by %s.\n\r",
                          ctime(&victim->pcdata->release_date),
                          victim->pcdata->helled_by);

        ch_printf(ch,
                  "Vnum: %d   Sex: %s   Room: %d   Count: %d  Killed: %d\n\r",
                  IS_NPC(victim) ? victim->pIndexData->vnum : 0,
                  victim->sex == SEX_MALE ? "male" : victim->sex ==
                  SEX_FEMALE ? "female" : "neutral",
                  victim->in_room == NULL ? 0 : victim->in_room->vnum,
                  IS_NPC(victim) ? victim->pIndexData->count : 1,
                  IS_NPC(victim) ? victim->pIndexData->killed : victim->
                  pcdata->mdeaths + victim->pcdata->pdeaths);
        ch_printf(ch,
                  "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Cha: %d  Lck: %d  Frc: %d\n\r",
                  get_curr_str(victim), get_curr_int(victim),
                  get_curr_wis(victim), get_curr_dex(victim),
                  get_curr_con(victim), get_curr_cha(victim),
                  get_curr_lck(victim), get_curr_frc(victim));
        ch_printf(ch, "Hps: %d/%d  Endurance: %d/%d\n\r", victim->hit,
                  victim->max_hit, victim->endurance, victim->max_endurance);
        if (!IS_NPC(victim))
        {
                int       ability;

                for (ability = 0; ability < MAX_ABILITY; ability++)
                        ch_printf(ch,
                                  "%-15s   Level: %-3d   Max: %-3d   Exp: %-10ld   Next: %-10ld\n\r",
                                  ability_name[ability],
                                  victim->skill_level[ability],
                                  max_level(victim, ability),
                                  victim->experience[ability],
                                  exp_level(victim->skill_level[ability] +
                                            1));
        }
        ch_printf(ch, "Top Level: %d     Race: %s  Align: %d  Gold: %d\n\r",
                  victim->top_level,
                  victim->race->name(), victim->alignment, victim->gold);
        ch_printf(ch,
                  "Hitroll: %d   Damroll: %d   Position: %d   Wimpy: %d \n\r",
                  GET_HITROLL(victim), GET_DAMROLL(victim), victim->position,
                  victim->wimpy);
        ch_printf(ch, "Fighting: %s    Master: %s    Leader: %s\n\r",
                  victim->fighting ? victim->fighting->who->name : "(none)",
                  victim->master ? victim->master->name : "(none)",
                  victim->leader ? victim->leader->name : "(none)");
        if (!IS_NPC(victim))
                ch_printf(ch,
                          "Thirst: %d   Full: %d   Drunk: %d     Glory: %d/%d\n\r",
                          victim->pcdata->condition[COND_THIRST],
                          victim->pcdata->condition[COND_FULL],
                          victim->pcdata->condition[COND_DRUNK],
                          victim->pcdata->quest_curr,
                          victim->pcdata->quest_accum);
        else
                ch_printf(ch,
                          "Hit dice: %dd%d+%d.  Damage dice: %dd%d+%d.\n\r",
                          victim->pIndexData->hitnodice,
                          victim->pIndexData->hitsizedice,
                          victim->pIndexData->hitplus,
                          victim->pIndexData->damnodice,
                          victim->pIndexData->damsizedice,
                          victim->pIndexData->damplus);
        ch_printf(ch, "MentalState: %d   EmotionalState: %d\n\r",
                  victim->mental_state, victim->emotional_state);
        ch_printf(ch, "Saving throws: %d %d %d %d %d.\n\r",
                  victim->saving_poison_death, victim->saving_wand,
                  victim->saving_para_petri, victim->saving_breath,
                  victim->saving_spell_staff);
        ch_printf(ch,
                  "Carry figures: items (%d/%d)  weight (%d/%d)   Numattacks: %d\n\r",
                  victim->carry_number, can_carry_n(victim),
                  victim->carry_weight, can_carry_w(victim),
                  victim->numattacks);
        ch_printf(ch,
                  "Years: %d   Seconds Played: %d   Timer: %d   Act: %d\n\r",
                  get_age(victim), (int) victim->played, victim->timer,
                  victim->act);
        if (IS_NPC(victim))
        {
                ch_printf(ch, "Act flags: %s\n\r",
                          flag_string(victim->act, act_flags));
/*        ch_printf( ch, "VIP flags: %s\n\r", flag_string(victim->vip_flags, planet_flags) ); */
        }
        else
        {
                ch_printf(ch, "Player flags: %s\n\r",
                          flag_string(victim->act, plr_flags));
                ch_printf(ch, "Pcflags: %s\n\r",
                          flag_string(victim->pcdata->flags, pc_flags));
                ch_printf(ch, "God flags: %s\n\r",
                          flag_string(victim->pcdata->godflags, god_flags));
                /*
                 * Wanted information 
                 */
                if (victim->pcdata->first_wanted)
                {
                        WANTED_DATA *wanted = NULL;

                        send_to_char("Wanted: &Y", ch);
                        for (wanted = victim->pcdata->first_wanted; wanted;
                             wanted = wanted->next)
                        {
                                if (wanted->government == NULL ||
                                    wanted->government->name == NULL ||
                                    wanted->government->name[0] == '\0')
                                        continue;

                                ch_printf(ch, "%s ",
                                          wanted->government->name);
                        }
                        send_to_char("&w\n\r", ch);
                }
        }
        ch_printf(ch, "Affected by: %s\n\r",
                  affect_bit_name(victim->affected_by));
        ch_printf(ch, "Speaking: %s\n\r", victim->speaking->name);
        if (victim->pcdata && victim->pcdata->bestowments
            && victim->pcdata->bestowments[0] != '\0')
                ch_printf(ch, "Bestowments: %s\n\r",
                          victim->pcdata->bestowments);
        ch_printf(ch, "Short description: %s\n\rLong  description: %s",
                  victim->short_descr,
                  victim->long_descr[0] !=
                  '\0' ? victim->long_descr : "(none)\n\r");
        if (IS_NPC(victim) && victim->spec_fun)
                ch_printf(ch, "Mobile has spec fun: &w%s %s\n\r",
                          victim->spec_funname, victim->spec2_funname);
        ch_printf(ch, "Body Parts : %s\n\r",
                  ext_flag_string(&victim->xflags, part_flags));
        ch_printf(ch, "Resistant  : %s\n\r",
                  flag_string(victim->resistant, ris_flags));
        ch_printf(ch, "Immune     : %s\n\r",
                  flag_string(victim->immune, ris_flags));
        ch_printf(ch, "Susceptible: %s\n\r",
                  flag_string(victim->susceptible, ris_flags));
        ch_printf(ch, "Attacks    : %s\n\r",
                  flag_string(victim->attacks, attack_flags));
        ch_printf(ch, "Defenses   : %s\n\r",
                  flag_string(victim->defenses, defense_flags));
        for (paf = victim->first_affect; paf; paf = paf->next)
                if ((skill = get_skilltype(paf->type)) != NULL)
                        ch_printf(ch,
                                  "%s: '%s' modifies %s by %d for %d rounds with bits %s.\n\r",
                                  skill_tname[skill->type],
                                  skill->name,
                                  affect_loc_name(paf->location),
                                  paf->modifier,
                                  paf->duration,
                                  affect_bit_name(paf->bitvector));
        return;
}

CMDF do_mstat(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        AFFECT_DATA *paf;
        CHAR_DATA *victim;
        SKILLTYPE *skill;

        set_char_color(AT_PLAIN, ch);

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Mstat whom?\n\r", ch);
                return;
        }
        if (arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg))
                mudstrlcpy(arg, argument, MIL);

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }
        if (get_trust(ch) < get_trust(victim) && !IS_NPC(victim))
        {
                set_char_color(AT_IMMORT, ch);
                send_to_char
                        ("Their godly glow prevents you from getting a good look.\n\r",
                         ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char
                        ("&w----------------------------------------------------\n\r",
                         ch);
                send_to_char("&BMOB RELATIVE INFO:&w\n\r", ch);
                ch_printf(ch,
                          "&w----------------------------------------------------\n\r");
                ch_printf(ch, "&BName:&w %s   ", victim->name);
                ch_printf(ch,
                          "&BVnum:&w %d  &BRoom:&w %d&B   Count:&w %d\n\r",
                          IS_NPC(victim) ? victim->pIndexData->vnum : 0,
                          victim->in_room ==
                          NULL ? 0 : victim->in_room->vnum);
                send_to_char
                        ("&w----------------------------------------------------\n\r",
                         ch);
                send_to_char("&BMOB STATS:&w\n\r", ch);
                send_to_char
                        ("&w----------------------------------------------------\n\r",
                         ch);

                ch_printf(ch,
                          "&BStr:&w %d  &BInt:&w %d  &BWis:&w %d  &BDex:&w %d  &BCon:&w %d  &BCha:&w %d  &BLck:&w %d  &BFrc:&w %d\n\r",
                          get_curr_str(victim), get_curr_int(victim),
                          get_curr_wis(victim), get_curr_dex(victim),
                          get_curr_con(victim), get_curr_cha(victim),
                          get_curr_lck(victim), get_curr_frc(victim));

                ch_printf(ch,
                          "&BTop Level: &w%d  &BHps: &w%d&B/&w%d  &BEndurance: &w%d&B/&w%d   &BAC: &w%d\n\r",
                          victim->top_level, victim->hit, victim->max_hit,
                          victim->endurance, victim->max_endurance,
                          GET_AC(victim));
                ch_printf(ch,
                          "&BMain Ability:&w %s  &BRace:&w %s    &BAlign:&w %d  &BCredits:&w %d\n\r",
                          ability_name[victim->main_ability],
                          victim->race->name(), victim->alignment,
                          victim->gold);
                ch_printf(ch,
                          "&BHitroll:&w %d   &BDamroll:&w %d   &BPosition:&w %d   &BWimpy:&w %d \n\r",
                          GET_HITROLL(victim), GET_DAMROLL(victim),
                          victim->position, victim->wimpy);
                ch_printf(ch,
                          "&BHit dice: &w%d&BD&w%d&B+&w%d.  &BDamage dice: &w%d&BD&w%d&B+&w%d   &BNumattacks: &w%d\n\r",
                          victim->pIndexData->hitnodice,
                          victim->pIndexData->hitsizedice,
                          victim->pIndexData->hitplus,
                          victim->pIndexData->damnodice,
                          victim->pIndexData->damsizedice,
                          victim->pIndexData->damplus, victim->numattacks);
                ch_printf(ch,
                          "&BCarry figures: &witems &B(&w%d&B/&w%d&B)  &wweight &B(&w%d&B/&w%d&B)   \n\r",
                          victim->carry_number, can_carry_n(victim),
                          victim->carry_weight, can_carry_w(victim));
                send_to_char
                        ("&w----------------------------------------------------\n\r",
                         ch);
                send_to_char("&BCHARATER RELATED MOB DATA:&w\n\r", ch);
                send_to_char
                        ("&w----------------------------------------------------\n\r",
                         ch);
                ch_printf(ch,
                          "&BShort description: &w%s\n\r&BLong  description: &w%s",
                          victim->short_descr,
                          victim->long_descr[0] !=
                          '\0' ? victim->long_descr : "(none)\n\r");
                if (IS_NPC(victim) && victim->spec_fun)
                        ch_printf(ch,
                                  "&BSpec_fun1: &w%s &BSpec_fun2 &w%s\n\r",
                                  victim->spec_funname,
                                  victim->spec2_funname);
                ch_printf(ch, "&BClan:&w %s\n\r",
                          (!get_clan(victim->mob_clan)) ? "(none)" :
                          get_clan(victim->mob_clan)->name);
                ch_printf(ch,
                          "&BFighting: &w%s    &BMaster: &w%s    &BLeader: &w%s\n\r",
                          victim->fighting ? victim->fighting->who->
                          name : "(none)",
                          victim->master ? victim->master->name : "(none)",
                          victim->leader ? victim->leader->name : "(none)");

                if (!xIS_EMPTY(victim->xflags))
                {
                        ch_printf(ch, "&BBody Parts : &w%s\n\r",
                                  ext_flag_string(&victim->xflags,
                                                  part_flags));
                }
                if (victim->resistant > 0)
                {
                        ch_printf(ch, "&BResistant  : &w%s\n\r",
                                  flag_string(victim->resistant, ris_flags));
                }
                if (victim->immune > 0)
                {
                        ch_printf(ch, "&BImmune     : &w%s\n\r",
                                  flag_string(victim->immune, ris_flags));
                }
                if (victim->susceptible > 0)
                {
                        ch_printf(ch, "&BSusceptible: &w%s\n\r",
                                  flag_string(victim->susceptible,
                                              ris_flags));
                }
                if (victim->attacks > 0)
                {
                        ch_printf(ch, "&BAttacks    : &w%s\n\r",
                                  flag_string(victim->attacks, attack_flags));
                }
                if (victim->defenses > 0)
                {
                        ch_printf(ch, "&BDefenses   : &w%s\n\r",
                                  flag_string(victim->defenses,
                                              defense_flags));
                }
                send_to_char
                        ("&w----------------------------------------------------\n\r",
                         ch);
                send_to_char("&BMOB FLAG DATA:&w\n\r", ch);
                send_to_char
                        ("&w----------------------------------------------------\n\r",
                         ch);
                ch_printf(ch, "&BAct flags: &w%s\n\r",
                          flag_string(victim->act, act_flags));
                ch_printf(ch, "&BAffected by:&w %s\n\r",
                          affect_bit_name(victim->affected_by));
                ch_printf(ch, "&BLanguage Spoken: %s&w\n\r",
                          victim->speaking ? victim->speaking->name : "None");
                send_to_char
                        ("&w----------------------------------------------------\n\r",
                         ch);
                send_to_char("&BMOB AFFECT DATA:&w\n\r", ch);
                send_to_char
                        ("&w----------------------------------------------------\n\r",
                         ch);

                for (paf = victim->first_affect; paf; paf = paf->next)
                        if ((skill = get_skilltype(paf->type)) != NULL)
                                ch_printf(ch,
                                          "%s: '%s' modifies %s by %d for %d rounds with bits %s.\n\r",
                                          skill_tname[skill->type],
                                          skill->name,
                                          affect_loc_name(paf->location),
                                          paf->modifier,
                                          paf->duration,
                                          affect_bit_name(paf->bitvector));
                print_olc_bounties_mob(ch, victim);
                return;
        }
        else
        {
                do_immscore(ch, victim->name);
        }
}



CMDF do_mfind(CHAR_DATA * ch, char *argument)
{
/*  extern int top_mob_index; */
        char      arg[MAX_INPUT_LENGTH];
        MOB_INDEX_DATA *pMobIndex;

/*  int vnum; */
        int       hash;
        int       nMatch;
        bool      fAll;

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Mfind whom?\n\r", ch);
                return;
        }

        fAll = !str_cmp(arg, "all");
        nMatch = 0;
        set_pager_color(AT_PLAIN, ch);

        /*
         * Yeah, so iterating over all vnum's takes 10,000 loops.
         * Get_mob_index is fast, and I don't feel like threading another link.
         * Do you?
         * -- Furey
         */
/*  for ( vnum = 0; nMatch < top_mob_index; vnum++ )
    {
	if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL )
	{
	    if ( fAll || is_name( arg, pMobIndex->player_name ) )
	    {
		nMatch++;
		snprintf( buf, MSL, "[%5d] %s\n\r",
		    pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
		send_to_char( buf, ch );
	    }
	}
    }
     */

        /*
         * This goes through all the hash entry points (1024), and is therefore
         * much faster, though you won't get your vnums in order... oh well. :)
         *
         * Tests show that Furey's method will usually loop 32,000 times, calling
         * get_mob_index()... which loops itself, an average of 1-2 times...
         * So theoretically, the above routine may loop well over 40,000 times,
         * and my routine bellow will loop for as many index_mobiles are on
         * your mud... likely under 3000 times.
         * -Thoric
         */
        for (hash = 0; hash < MAX_KEY_HASH; hash++)
                for (pMobIndex = mob_index_hash[hash];
                     pMobIndex; pMobIndex = pMobIndex->next)
                        if (fAll
                            || nifty_is_name(arg, pMobIndex->player_name))
                        {
                                nMatch++;
                                pager_printf(ch, "[%5d] %s\n\r",
                                             pMobIndex->vnum,
                                             capitalize(pMobIndex->
                                                        short_descr));
                        }

        if (nMatch)
                pager_printf(ch, "Number of matches: %d\n", nMatch);
        else
                send_to_char
                        ("Nothing like that in hell, earth, or heaven.\n\r",
                         ch);

        return;
}



CMDF do_ofind(CHAR_DATA * ch, char *argument)
{
/*  extern int top_obj_index; */
        char      arg[MAX_INPUT_LENGTH];
        OBJ_INDEX_DATA *pObjIndex;

/*  int vnum; */
        int       hash;
        int       nMatch;
        bool      fAll;

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Ofind what?\n\r", ch);
                return;
        }

        set_pager_color(AT_PLAIN, ch);
        fAll = !str_cmp(arg, "all");
        nMatch = 0;
/*  nLoop	= 0; */

        /*
         * Yeah, so iterating over all vnum's takes 10,000 loops.
         * Get_obj_index is fast, and I don't feel like threading another link.
         * Do you?
         * -- Furey
         for ( vnum = 0; nMatch < top_obj_index; vnum++ )
         {
         nLoop++;
         if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL )
         {
         if ( fAll || nifty_is_name( arg, pObjIndex->name ) )
         {
         nMatch++;
         snprintf( buf, MSL, "[%5d] %s\n\r",
         pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
         send_to_char( buf, ch );
         }
         }
         }
         */

        /*
         * This goes through all the hash entry points (1024), and is therefore
         * much faster, though you won't get your vnums in order... oh well. :)
         *
         * Tests show that Furey's method will usually loop 32,000 times, calling
         * get_obj_index()... which loops itself, an average of 2-3 times...
         * So theoretically, the above routine may loop well over 50,000 times,
         * and my routine bellow will loop for as many index_objects are on
         * your mud... likely under 3000 times.
         * -Thoric
         */
        for (hash = 0; hash < MAX_KEY_HASH; hash++)
                for (pObjIndex = obj_index_hash[hash];
                     pObjIndex; pObjIndex = pObjIndex->next)
                        if (fAll || nifty_is_name(arg, pObjIndex->name))
                        {
                                nMatch++;
                                pager_printf(ch, "[%5d] %s\n\r",
                                             pObjIndex->vnum,
                                             capitalize(pObjIndex->
                                                        short_descr));
                        }

        if (nMatch)
                pager_printf(ch, "Number of matches: %d\n", nMatch);
        else
                send_to_char
                        ("Nothing like that in hell, earth, or heaven.\n\r",
                         ch);

        return;
}



CMDF do_mwhere(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        bool      found;

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Mwhere whom?\n\r", ch);
                return;
        }

        set_pager_color(AT_PLAIN, ch);
        found = FALSE;
        for (victim = first_char; victim; victim = victim->next)
        {
                if (IS_NPC(victim)
                    && victim->in_room && nifty_is_name(arg, victim->name))
                {
                        found = TRUE;
                        pager_printf(ch, "[%5d] %-28s [%5d] %s\n\r",
                                     victim->pIndexData->vnum,
                                     victim->short_descr,
                                     victim->in_room->vnum,
                                     victim->in_room->name);
                }
        }

        if (!found)
                act(AT_PLAIN, "You didn't find any $T.", ch, NULL, arg,
                    TO_CHAR);

        return;
}


CMDF do_bodybag(CHAR_DATA * ch, char *argument)
{
        char      buf2[MAX_STRING_LENGTH];
        char      buf3[MAX_STRING_LENGTH];
        char      arg[MAX_INPUT_LENGTH];
        OBJ_DATA *obj;
        bool      found;

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Bodybag whom?\n\r", ch);
                return;
        }

        /*
         * make sure the buf3 is clear? 
         */
        snprintf(buf3, MSL, "%s", "");
        /*
         * check to see if vict is playing? 
         */
        snprintf(buf2, MSL, "the corpse of %s", arg);
        found = FALSE;
        for (obj = first_object; obj; obj = obj->next)
        {
                if (obj->in_room
                    && !str_cmp(buf2, obj->short_descr)
                    && (obj->pIndexData->vnum == 11))
                {
                        found = TRUE;
                        ch_printf(ch,
                                  "Bagging body: [%5d] %-28s [%5d] %s\n\r",
                                  obj->pIndexData->vnum, obj->short_descr,
                                  obj->in_room->vnum, obj->in_room->name);
                        obj_from_room(obj);
                        obj = obj_to_char(obj, ch);
                        obj->timer = -1;
                        save_char_obj(ch);
                }
        }

        if (!found)
                ch_printf(ch, " You couldn't find any %s\n\r", buf2);
        return;
}


/* New owhere by Altrag, 03/14/96 */
CMDF do_owhere(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      arg[MAX_INPUT_LENGTH];
        char      arg1[MAX_INPUT_LENGTH];
        OBJ_DATA *obj;
        bool      found;
        int       icnt = 0;

        argument = one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Owhere what?\n\r", ch);
                return;
        }
        argument = one_argument(argument, arg1);

        set_pager_color(AT_PLAIN, ch);
        if (arg1[0] != '\0' && !str_prefix(arg1, "nesthunt"))
        {
                if (!(obj = get_obj_world(ch, arg)))
                {
                        send_to_char("Nesthunt for what object?\n\r", ch);
                        return;
                }
                for (; obj->in_obj; obj = obj->in_obj)
                {
                        pager_printf(ch, "[%5d] %-28s in object [%5d] %s\n\r",
                                     obj->pIndexData->vnum, obj_short(obj),
                                     obj->in_obj->pIndexData->vnum,
                                     obj->in_obj->short_descr);
                        ++icnt;
                }
                snprintf(buf, MSL, "[%5d] %-28s in ", obj->pIndexData->vnum,
                         obj_short(obj));
                if (obj->carried_by)
                        snprintf(buf + strlen(buf), MSL,
                                 "invent [%5d] %s\n\r",
                                 (IS_NPC(obj->carried_by) ? obj->carried_by->
                                  pIndexData->vnum : 0), PERS(obj->carried_by,
                                                              ch));
                else if (obj->in_room)
                        snprintf(buf + strlen(buf), MSL,
                                 "room   [%5d] %s\n\r", obj->in_room->vnum,
                                 obj->in_room->name);
                else if (obj->in_obj)
                {
                        bug("do_owhere: obj->in_obj after NULL!", 0);
                        mudstrlcat(buf, "object??\n\r", MSL);
                }
                else
                {
                        bug("do_owhere: object doesnt have location!", 0);
                        mudstrlcat(buf, "nowhere??\n\r", MSL);
                }
                send_to_pager(buf, ch);
                ++icnt;
                pager_printf(ch, "Nested %d levels deep.\n\r", icnt);
                return;
        }

        found = FALSE;
        for (obj = first_object; obj; obj = obj->next)
        {
                if (!nifty_is_name(arg, obj->name))
                        continue;
                found = TRUE;

                snprintf(buf, MSL, "(%3d) [%5d] %-28s in ", ++icnt,
                         obj->pIndexData->vnum, obj_short(obj));
                if (obj->carried_by)
                        snprintf(buf + strlen(buf), MSL,
                                 "invent [%5d] %s\n\r",
                                 (IS_NPC(obj->carried_by) ? obj->carried_by->
                                  pIndexData->vnum : 0), PERS(obj->carried_by,
                                                              ch));
                else if (obj->in_room)
                        snprintf(buf + strlen(buf), MSL,
                                 "room   [%5d] %s\n\r", obj->in_room->vnum,
                                 obj->in_room->name);
                else if (obj->in_obj)
                        snprintf(buf + strlen(buf), MSL,
                                 "object [%5d] %s\n\r",
                                 obj->in_obj->pIndexData->vnum,
                                 obj_short(obj->in_obj));
                else
                {
                        bug("do_owhere: object doesnt have location!", 0);
                        mudstrlcat(buf, "nowhere??\n\r", MSL);
                }
                send_to_pager(buf, ch);
        }

        if (!found)
                act(AT_PLAIN, "You didn't find any $T.", ch, NULL, arg,
                    TO_CHAR);
        else
                pager_printf(ch, "%d matches.\n\r", icnt);

        return;
}


CMDF do_reboo(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        send_to_char("If you want to REBOOT, spell it out.\n\r", ch);
        return;
}


extern bool mud_down;
CMDF do_reboot(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *vch;

        if (compilelock)
        {
                set_char_color(AT_RED, ch);
                send_to_char
                        ("Sorry, the mud cannot be rebooted during a compiler operation.\n\rPlease wait for the compiler to finish.\n\r",
                         ch);
                return;
        }

        if (str_cmp(argument, "mud now")
            && str_cmp(argument, "nosave")
            && str_cmp(argument, "and sort skill table"))
        {
                send_to_char
                        ("Syntax: 'reboot mud now' or 'reboot nosave'\n\r",
                         ch);
                return;
        }

        if (auction->item)
                do_auction(ch, "stop");

        snprintf(buf, MSL, "Reboot by %s.", ch->name);
        do_echo(ch, buf);

        if (!str_cmp(argument, "and sort skill table"))
        {
                sort_skill_table();
                save_skill_table();
        }

        /*
         * Save all characters before booting. 
         */
        if (str_cmp(argument, "nosave"))
                for (vch = first_char; vch; vch = vch->next)
                        if (!IS_NPC(vch))
                                save_char_obj(vch);

        mud_down = TRUE;
        return;
}



CMDF do_shutdow(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        send_to_char("If you want to SHUTDOWN, spell it out.\n\r", ch);
        return;
}



CMDF do_shutdown(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *vch;

        if (compilelock)
        {
                set_char_color(AT_RED, ch);
                send_to_char
                        ("Sorry, the mud cannot be shutdown during a compiler operation.\n\rPlease wait for the compiler to finish.\n\r",
                         ch);
                return;
        }

        if (str_cmp(argument, "mud now") && str_cmp(argument, "nosave"))
        {
                send_to_char
                        ("Syntax: 'shutdown mud now' or 'shutdown nosave'\n\r",
                         ch);
                return;
        }

        if (auction->item)
                do_auction(ch, "stop");

        snprintf(buf, MSL, "Shutdown by %s.", ch->name);
        append_file(ch, SHUTDOWN_FILE, buf);
        mudstrlcat(buf, "\n\r", MSL);
        do_echo(ch, buf);

        /*
         * Save all characters before booting. 
         */
        if (str_cmp(argument, "nosave"))
                for (vch = first_char; vch; vch = vch->next)
                        if (!IS_NPC(vch))
                                save_char_obj(vch);
        mud_down = TRUE;
        return;
}


CMDF do_snoop(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        DESCRIPTOR_DATA *d;
        CHAR_DATA *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Snoop whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (!victim->desc)
        {
                send_to_char("No descriptor to snoop.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("Cancelling all snoops.\n\r", ch);
                for (d = first_descriptor; d; d = d->next)
                        if (d->snoop_by == ch->desc)
                                d->snoop_by = NULL;
                return;
        }

        if (victim->desc->snoop_by)
        {
                send_to_char("Busy already.\n\r", ch);
                return;
        }

        /*
         * Minimum snoop level... a secret mset value
         * makes the snooper think that the victim is already being snooped
         */
        if ((get_trust(ch) != MAX_LEVEL && get_trust(victim) >= get_trust(ch))
            || (victim->pcdata && victim->pcdata->min_snoop >= get_trust(ch)))
        {
                send_to_char("Busy already.\n\r", ch);
                return;
        }

        if (ch->desc)
        {
                for (d = ch->desc->snoop_by; d; d = d->snoop_by)
                        if (d->character == victim || d->original == victim)
                        {
                                send_to_char("No snoop loops.\n\r", ch);
                                return;
                        }
        }

/*  Snoop notification for higher imms, if desired, uncomment this
    if ( get_trust(victim) > LEVEL_GOD && get_trust(ch) < LEVEL_SUPREME )
      write_to_descriptor( victim->desc->descriptor, "\n\rYou feel like someone is watching your every move...\n\r", 0 );
*/
        victim->desc->snoop_by = ch->desc;
        send_to_char("Ok.\n\r", ch);
        return;
}



CMDF do_switch(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Switch into whom?\n\r", ch);
                return;
        }

        if (!ch->desc)
                return;

        if (ch->desc->original)
        {
                send_to_char("You are already switched.\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("Ok.\n\r", ch);
                return;
        }

        if (victim->desc)
        {
                send_to_char("Character in use.\n\r", ch);
                return;
        }

        if (!IS_NPC(victim) && get_trust(ch) < LEVEL_GREATER)
        {
                send_to_char("You cannot switch into a player!\n\r", ch);
                return;
        }

        ch->desc->character = victim;
        ch->desc->original = ch;
        victim->desc = ch->desc;
        ch->desc = NULL;
        ch->switched = victim;
        send_to_char("Ok.\n\r", victim);
        return;
}



CMDF do_return(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        if (!ch->desc)
                return;

        if (!ch->desc->original)
        {
                send_to_char("You aren't switched.\n\r", ch);
                return;
        }

        if (IS_SET(ch->act, ACT_POLYMORPHED))
        {
                send_to_char
                        ("Use revert to return from a polymorphed mob.\n\r",
                         ch);
                return;
        }

        send_to_char("You return to your original body.\n\r", ch);
        if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_POSSESS))
        {
                affect_strip(ch, gsn_possess);
                REMOVE_BIT(ch->affected_by, AFF_POSSESS);
        }
/*    if ( IS_NPC( ch->desc->character ) )
      REMOVE_BIT( ch->desc->character->affected_by, AFF_POSSESS );*/
        ch->desc->character = ch->desc->original;
        ch->desc->original = NULL;
        ch->desc->character->desc = ch->desc;
        ch->desc->character->switched = NULL;
        ch->desc = NULL;
        return;
}



CMDF do_minvoke(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        MOB_INDEX_DATA *pMobIndex;
        CHAR_DATA *victim;
        int       vnum;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Syntax: minvoke <vnum>.\n\r", ch);
                return;
        }

        if (!is_number(arg))
        {
                char      arg2[MAX_INPUT_LENGTH];
                int       hash, cnt;
                int       count = number_argument(arg, arg2);

                vnum = -1;
                for (hash = cnt = 0; hash < MAX_KEY_HASH; hash++)
                        for (pMobIndex = mob_index_hash[hash];
                             pMobIndex; pMobIndex = pMobIndex->next)
                                if (nifty_is_name
                                    (arg2, pMobIndex->player_name)
                                    && ++cnt == count)
                                {
                                        vnum = pMobIndex->vnum;
                                        break;
                                }
                if (vnum == -1)
                {
                        send_to_char("No such mobile exists.\n\r", ch);
                        return;
                }
        }
        else
                vnum = atoi(arg);

        if (get_trust(ch) < LEVEL_DEMI)
        {
                AREA_DATA *pArea;

                if (IS_NPC(ch))
                {
                        send_to_char("Huh?\n\r", ch);
                        return;
                }

                if (!ch->pcdata || !(pArea = ch->pcdata->area))
                {
                        send_to_char
                                ("You must have an assigned area to invoke this mobile.\n\r",
                                 ch);
                        return;
                }
                if (vnum < pArea->low_m_vnum || vnum > pArea->hi_m_vnum)
                {
                        send_to_char
                                ("That number is not in your allocated range.\n\r",
                                 ch);
                        return;
                }
        }

        if ((pMobIndex = get_mob_index(vnum)) == NULL)
        {
                send_to_char("No mobile has that vnum.\n\r", ch);
                return;
        }

        victim = create_mobile(pMobIndex);
        char_to_room(victim, ch->in_room);
        act(AT_IMMORT, "$n has created $N!", ch, NULL, victim, TO_ROOM);
        send_to_char("Ok.\n\r", ch);
        return;
}



CMDF do_oinvoke(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        OBJ_INDEX_DATA *pObjIndex;
        OBJ_DATA *obj;
        int       vnum;
        int       level;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char("Syntax: oinvoke <vnum> <level>.\n\r", ch);
                return;
        }

        if (arg2[0] == '\0')
        {
                level = get_trust(ch);
        }
        else
        {
                if (!is_number(arg2))
                {
                        send_to_char("Syntax: oinvoke <vnum> <level>.\n\r",
                                     ch);
                        return;
                }
                level = atoi(arg2);
                if (level < 0 || level > get_trust(ch))
                {
                        send_to_char("Limited to your trust level.\n\r", ch);
                        return;
                }
        }

        if (!is_number(arg1))
        {
                char      arg[MAX_INPUT_LENGTH];
                int       hash, cnt;
                int       count = number_argument(arg1, arg);

                vnum = -1;
                for (hash = cnt = 0; hash < MAX_KEY_HASH; hash++)
                        for (pObjIndex = obj_index_hash[hash];
                             pObjIndex; pObjIndex = pObjIndex->next)
                                if (nifty_is_name(arg, pObjIndex->name)
                                    && ++cnt == count)
                                {
                                        vnum = pObjIndex->vnum;
                                        break;
                                }
                if (vnum == -1)
                {
                        send_to_char("No such object exists.\n\r", ch);
                        return;
                }
        }
        else
                vnum = atoi(arg1);

        if (get_trust(ch) < LEVEL_DEMI)
        {
                AREA_DATA *pArea;

                if (IS_NPC(ch))
                {
                        send_to_char("Huh?\n\r", ch);
                        return;
                }

                if (!ch->pcdata || !(pArea = ch->pcdata->area))
                {
                        send_to_char
                                ("You must have an assigned area to invoke this object.\n\r",
                                 ch);
                        return;
                }
                if (vnum < pArea->low_o_vnum || vnum > pArea->hi_o_vnum)
                {
                        send_to_char
                                ("That number is not in your allocated range.\n\r",
                                 ch);
                        return;
                }
        }

        if ((pObjIndex = get_obj_index(vnum)) == NULL)
        {
                send_to_char("No object has that vnum.\n\r", ch);
                return;
        }

/* Commented out by Narn, it seems outdated
    if ( IS_OBJ_STAT( pObjIndex, ITEM_PROTOTYPE )
    &&	 pObjIndex->count > 5 )
    {
	send_to_char( "That object is at its limit.\n\r", ch );
	return;
    }
*/

        obj = create_object(pObjIndex, level);
        if (CAN_WEAR(obj, ITEM_TAKE))
        {
                obj = obj_to_char(obj, ch);
        }
        else
        {
                obj = obj_to_room(obj, ch->in_room);
                act(AT_IMMORT, "$n has created $p!", ch, obj, NULL, TO_ROOM);
        }
        send_to_char("Ok.\n\r", ch);
        return;
}



CMDF do_purge(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        OBJ_DATA *obj;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                /*
                 * 'purge' 
                 */
                CHAR_DATA *vnext;
                OBJ_DATA *obj_next;

                for (victim = ch->in_room->first_person; victim;
                     victim = vnext)
                {
                        vnext = victim->next_in_room;
                        if (IS_NPC(victim) && victim != ch
                            && !IS_SET(victim->act, ACT_POLYMORPHED))
                                extract_char(victim, TRUE);
                }

                for (obj = ch->in_room->first_content; obj; obj = obj_next)
                {
                        obj_next = obj->next_content;
                        if (obj->item_type == ITEM_SPACECRAFT)
                                continue;
                        extract_obj(obj);
                }

                act(AT_IMMORT, "$n purges the room!", ch, NULL, NULL,
                    TO_ROOM);
                send_to_char("Ok.\n\r", ch);
                return;
        }
        victim = NULL;
        obj = NULL;

        /*
         * fixed to get things in room first -- i.e., purge portal (obj),
         * * no more purging mobs with that keyword in another room first
         * * -- Tri 
         */
        if ((victim = get_char_room(ch, arg)) == NULL
            && (obj = get_obj_here(ch, arg)) == NULL)
        {
                if ((victim = get_char_world(ch, arg)) == NULL && (obj = get_obj_world(ch, arg)) == NULL)   /* no get_obj_room */
                {
                        send_to_char("They aren't here.\n\r", ch);
                        return;
                }
        }

/* Single object purge in room for high level purge - Scryn 8/12*/
        if (obj)
        {
                separate_obj(obj);
                act(AT_IMMORT, "$n purges $p.", ch, obj, NULL, TO_ROOM);
                act(AT_IMMORT, "You make $p disappear in a puff of smoke!",
                    ch, obj, NULL, TO_CHAR);
                extract_obj(obj);
                return;
        }


        if (!IS_NPC(victim))
        {
                send_to_char("Not on PC's.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("You cannot purge yourself!\n\r", ch);
                return;
        }

        if (IS_SET(victim->act, ACT_POLYMORPHED))
        {
                send_to_char("You cannot purge a polymorphed player.\n\r",
                             ch);
                return;
        }
        act(AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT);
        extract_char(victim, TRUE);
        return;
}


CMDF do_low_purge(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        OBJ_DATA *obj;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Purge what?\n\r", ch);
                return;
        }

        victim = NULL;
        obj = NULL;
        if ((victim = get_char_room(ch, arg)) == NULL
            && (obj = get_obj_here(ch, arg)) == NULL)
        {
                send_to_char("You can't find that here.\n\r", ch);
                return;
        }

        if (obj)
        {
                separate_obj(obj);
                act(AT_IMMORT, "$n purges $p!", ch, obj, NULL, TO_ROOM);
                act(AT_IMMORT, "You make $p disappear in a puff of smoke!",
                    ch, obj, NULL, TO_CHAR);
                extract_obj(obj);
                return;
        }

        if (!IS_NPC(victim))
        {
                send_to_char("Not on PC's.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("You cannot purge yourself!\n\r", ch);
                return;
        }

        act(AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT);
        act(AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL,
            victim, TO_CHAR);
        extract_char(victim, TRUE);
        return;
}


CMDF do_balzhur(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        char      buf2[MAX_STRING_LENGTH];
        CHAR_DATA *victim;
        int       sn;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Who is deserving of such a fate?\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't playing.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (get_trust(victim) >= get_trust(ch))
        {
                send_to_char
                        ("I wouldn't even think of that if I were you...\n\r",
                         ch);
                return;
        }

        set_char_color(AT_WHITE, ch);
        send_to_char("You summon the demon Balzhur to wreak your wrath!\n\r",
                     ch);
        send_to_char
                ("Balzhur sneers at you evilly, then vanishes in a puff of smoke.\n\r",
                 ch);
        set_char_color(AT_IMMORT, victim);
        send_to_char
                ("You hear an ungodly sound in the distance that makes your blood run cold!\n\r",
                 victim);
        snprintf(buf, MSL, "Balzhur screams, 'You are MINE %s!!!'",
                 victim->name);
        echo_to_all(AT_IMMORT, buf, ECHOTAR_ALL);
        victim->top_level = 1;
        victim->trust = 0;
        {
                int       ability;

                for (ability = 0; ability < MAX_ABILITY; ability++)
                {
                        victim->experience[ability] = 1;
                        victim->skill_level[ability] = 1;
                }
        }
        victim->max_hit = 500;
        victim->max_endurance = 1000;
        for (sn = 0; sn < top_sn; sn++)
                victim->pcdata->learned[sn] = 0;
        victim->hit = victim->max_hit;
        victim->endurance = victim->max_endurance;


        snprintf(buf, MSL, "%s%s", GOD_DIR, capitalize(victim->name));

        if (!remove(buf))
                send_to_char("Player's immortal data destroyed.\n\r", ch);
        else if (errno != ENOENT)
        {
                ch_printf(ch,
                          "Unknown error #%d - %s (immortal data).  Report to Thoric\n\r",
                          errno, strerror(errno));
                snprintf(buf2, MSL, "%s balzhuring %s", ch->name, buf);
                perror(buf2);
        }

/*
    snprintf( buf2, MSL, "%s.are", capitalize(arg) );
    for ( pArea = first_build; pArea; pArea = pArea->next )
      if ( !strcmp( pArea->filename, buf2 ) )
      {
        snprintf( buf, MSL, "%s%s", BUILD_DIR, buf2 );
        if ( IS_SET( pArea->status, AREA_LOADED ) )
          fold_area( pArea, buf, FALSE, TRUE);
        close_area( pArea );
        snprintf( buf2, MSL, "%s.bak", buf );
        set_char_color( AT_RED, ch ); 
        if ( !rename( buf, buf2 ) )
          send_to_char( "Player's area data destroyed.  Area saved as 
backup.\n\r", ch);
        else if ( errno != ENOENT )
        {
          ch_printf( ch, "Unknown error #%d - %s (area data).  Report to 
Thoric.\n\r",
                  errno, strerror( errno ) );
          snprintf( buf2, MSL, "%s destroying %s", ch->name, buf );
          perror( buf2 );   
        }
      }
 */

        make_wizlist();
        do_help(victim, "M_BALZHUR_");
        set_char_color(AT_WHITE, victim);
        send_to_char("You awake after a long period of time...\n\r", victim);
        while (victim->first_carrying)
                extract_obj(victim->first_carrying);
        return;
}

CMDF do_advance(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        int       level, ability;
        int       iLevel, iAbility;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg3);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0'
            || !is_number(arg2))
        {
                send_to_char("Syntax: advance <char> <ability> <level>.\n\r",
                             ch);
                return;
        }

        if ((victim = get_char_room(ch, arg1)) == NULL)
        {
                send_to_char("That player is not here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        /*
         * You can demote yourself but not someone else at your own trust. -- Narn 
         */
        if (get_trust(ch) <= get_trust(victim) && ch != victim)
        {
                send_to_char("You can't do that.\n\r", ch);
                return;
        }

        if ((level = atoi(arg2)) < 1 || level > 200)
        {
                send_to_char("Level must be 1 to 200.\n\r", ch);
                return;
        }

        if (!str_cmp(arg3, "all"))
        {
                for (ability = 0; ability < MAX_ABILITY; ability++)
                {
                        /*
                         * Lower level:
                         *   Reset to level 1.
                         *   Then raise again.
                         *   Currently, an imp can lower another imp.
                         *   -- Swiftest
                         */
                        if (level <= victim->skill_level[ability])
                        {
                                send_to_char("Lowering a player's level!\n\r",
                                             ch);
                                set_char_color(AT_IMMORT, victim);
                                send_to_char
                                        ("Cursed and forsaken! The gods have lowered your level.\n\r",
                                         victim);
                                if (ability == COMBAT_ABILITY)
                                        victim->max_hit = 500;
                                if (ability == FORCE_ABILITY
                                    || ability == HUNTING_ABILITY)
                                {
                                        if (ch->perm_frc > 0)
                                                victim->max_endurance = 700;
                                        else
                                                victim->max_endurance = 500;
                                        if (ability == FORCE_ABILITY)
                                                victim->max_endurance +=
                                                        victim->
                                                        skill_level
                                                        [HUNTING_ABILITY] *
                                                        victim->perm_dex;
                                        if (ability == HUNTING_ABILITY)
                                                victim->max_endurance +=
                                                        victim->
                                                        skill_level
                                                        [HUNTING_ABILITY] *
                                                        20;
                                }
                                victim->experience[ability] = 0;
                                victim->skill_level[ability] = 1;
                        }
                        else
                        {
                                send_to_char("Raising a player's level!\n\r",
                                             ch);
                                send_to_char
                                        ("The gods feel fit to raise your level!\n\r",
                                         victim);
                        }

                        for (iLevel = victim->skill_level[ability];
                             iLevel < level; iLevel++)
                        {
                                victim->experience[ability] =
                                        exp_level(iLevel + 1);
                                gain_exp_new(victim, 0, ability, FALSE);
                        }
                }
                return;
        }
        else
        {
                ability = -1;
                for (iAbility = 0; iAbility < MAX_ABILITY; iAbility++)
                {
                        if (!str_prefix(arg3, ability_name[iAbility]))
                        {
                                ability = iAbility;
                                break;
                        }
                }

                if (ability == -1)
                {
                        send_to_char("No Such Ability.\n\r", ch);
                        do_advance(ch, "");
                        return;
                }


                if (IS_NPC(victim))
                {
                        send_to_char("Not on NPC's.\n\r", ch);
                        return;
                }

                /*
                 * You can demote yourself but not someone else at your own trust. -- Narn 
                 */
                if (get_trust(ch) <= get_trust(victim) && ch != victim)
                {
                        send_to_char("You can't do that.\n\r", ch);
                        return;
                }

                if ((level = atoi(arg2)) < 1 || level > 200)
                {
                        send_to_char("Level must be 1 to 200.\n\r", ch);
                        return;
                }

                /*
                 * Lower level:
                 *   Reset to level 1.
                 *   Then raise again.
                 *   Currently, an imp can lower another imp.
                 *   -- Swiftest
                 */
                if (level <= victim->skill_level[ability])
                {
                        send_to_char("Lowering a player's level!\n\r", ch);
                        set_char_color(AT_IMMORT, victim);
                        send_to_char
                                ("Cursed and forsaken! The gods have lowered your level.\n\r",
                                 victim);
                        if (ability == COMBAT_ABILITY)
                                victim->max_hit = 500;
                        if (ability == FORCE_ABILITY
                            || ability == HUNTING_ABILITY)
                        {
                                if (ch->perm_frc > 0)
                                        victim->max_endurance = 700;
                                else
                                        victim->max_endurance = 500;
                                if (ability == FORCE_ABILITY)
                                        victim->max_endurance +=
                                                victim->
                                                skill_level[HUNTING_ABILITY] *
                                                victim->perm_dex;
                                if (ability == HUNTING_ABILITY)
                                        victim->max_endurance +=
                                                victim->
                                                skill_level[HUNTING_ABILITY] *
                                                20;
                        }
                        victim->experience[ability] = 0;
                        victim->skill_level[ability] = 1;
                }
                else
                {
                        send_to_char("Raising a player's level!\n\r", ch);
                        send_to_char
                                ("The gods feel fit to raise your level!\n\r",
                                 victim);
                }

                for (iLevel = victim->skill_level[ability]; iLevel < level;
                     iLevel++)
                {
                        victim->experience[ability] = exp_level(iLevel + 1);
                        gain_exp_new(victim, 0, ability, FALSE);
                }
                return;
        }
}

CMDF do_immortalize(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        CHANNEL_DATA *channel;
        int       sn;
        int       level, ability;
        int       iLevel, iAbility;

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);


        if (IS_NPC(ch))
                return;

        if (arg[0] == '\0')
        {
                send_to_char
                        ("Syntax: immortalize <char> <level (optional)>\n\r",
                         ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("That player is not here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }
/*
    if ( victim->top_level != LEVEL_AVATAR )
    {
	send_to_char( "This player is not worthy of immortality yet.\n\r", ch );
	return;
    }
*/
        send_to_char("Immortalizing a player...\n\r", ch);
        set_char_color(AT_IMMORT, victim);
        act(AT_IMMORT,
            "$n begins to chant softly... then raises $s arms to the sky...",
            ch, NULL, NULL, TO_ROOM);
        set_char_color(AT_WHITE, victim);
        send_to_char("You suddenly feel very strange...\n\r\n\r", victim);
        set_char_color(AT_LBLUE, victim);

        do_help(victim, "M_GODLVL1_");
        set_char_color(AT_WHITE, victim);
        send_to_char("You awake... all your possessions are gone.\n\r",
                     victim);
        while (victim->first_carrying)
                extract_obj(victim->first_carrying);

        if (arg2[0] != '\0')
                victim->top_level = atoi(arg2);

        else
                victim->top_level = LEVEL_IMMORTAL;

        for (ability = 0; ability < MAX_ABILITY; ability++)
        {

                for (iLevel = victim->skill_level[ability]; iLevel < 200;
                     iLevel++)
                {
                        victim->experience[ability] = exp_level(iLevel + 1);
                        gain_exp_new(victim, 0, ability, FALSE);
                }
        }
        for (sn = 0; sn < top_sn; sn++)
        {
                if (skill_table[sn]->guild < 0
                    || skill_table[sn]->guild >= MAX_ABILITY)
                        continue;
                if (skill_table[sn]->name
                    && (victim->skill_level[skill_table[sn]->guild] >=
                        skill_table[sn]->min_level))
                        victim->pcdata->learned[sn] = 100;
        }
        for (channel = first_channel; channel; channel = channel->next)
        {
                if (victim->top_level >= channel->level
                    && !hasname(victim->pcdata->listening, channel->name))
                        addname(&victim->pcdata->listening, channel->name);
        }

        victim->trust = 0;
        return;
}



CMDF do_trust(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        int       level;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2))
        {
                send_to_char("Syntax: trust <char> <level>.\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg1)) == NULL)
        {
                send_to_char("That player is not here.\n\r", ch);
                return;
        }

        if ((level = atoi(arg2)) < 0 || level > MAX_LEVEL)
        {
                send_to_char("Level must be 0 (reset) or 1 to 60.\n\r", ch);
                return;
        }

        if (level > get_trust(ch))
        {
                send_to_char("Limited to your own trust.\n\r", ch);
                return;
        }

        if (get_trust(victim) >= get_trust(ch))
        {
                send_to_char("You can't do that.\n\r", ch);
                return;
        }

        victim->trust = level;
        send_to_char("Ok.\n\r", ch);
        return;
}


#ifndef RESTORE
CMDF do_restore(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Restore whom?\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "all"))
        {
                CHAR_DATA *vch;
                CHAR_DATA *vch_next;

                if (!ch->pcdata)
                        return;

                if (get_trust(ch) < LEVEL_SUB_IMPLEM)
                {
                        if (IS_NPC(ch))
                        {
                                send_to_char("You can't do that.\n\r", ch);
                                return;
                        }
                        else
                        {
                                /*
                                 * Check if the player did a restore all within the last 18 hours. 
                                 */
                                if (current_time - last_restore_all_time <
                                    RESTORE_INTERVAL)
                                {
                                        send_to_char
                                                ("Sorry, you can't do a restore all yet.\n\r",
                                                 ch);
                                        do_restoretime(ch, "");
                                        return;
                                }
                        }
                }
                last_restore_all_time = current_time;
                ch->pcdata->restore_time = current_time;
                save_char_obj(ch);
                send_to_char("Ok.\n\r", ch);
                for (vch = first_char; vch; vch = vch_next)
                {
                        vch_next = vch->next;

                        if (!IS_NPC(vch) && !IS_IMMORTAL(vch))
                        {
                                vch->hit = vch->max_hit;
                                vch->endurance = vch->max_endurance;
                                vch->pcdata->condition[COND_BLOODTHIRST] =
                                        (10 + vch->top_level);
                                update_pos(vch);
                                act(AT_IMMORT, "$n has restored you.", ch,
                                    NULL, vch, TO_VICT);
                        }
                }
        }
        else
        {

                CHAR_DATA *victim;

                if ((victim = get_char_world(ch, arg)) == NULL)
                {
                        send_to_char("They aren't here.\n\r", ch);
                        return;
                }

                if (get_trust(ch) < LEVEL_LESSER
                    && victim != ch
                    && !(IS_NPC(victim)
                         && IS_SET(victim->act, ACT_PROTOTYPE)))
                {
                        send_to_char("You can't do that.\n\r", ch);
                        return;
                }

                victim->hit = victim->max_hit;
                victim->endurance = victim->max_endurance;
                if (victim->pcdata)
                        victim->pcdata->condition[COND_BLOODTHIRST] =
                                (10 + victim->top_level);
                update_pos(victim);
                if (ch != victim)
                        act(AT_IMMORT, "$n has restored you.", ch, NULL,
                            victim, TO_VICT);
                send_to_char("Ok.\n\r", ch);
                return;
        }
}
#endif
CMDF do_restoretime(CHAR_DATA * ch, char *argument)
{
        long int  time_passed;
        int       hour, minute;

        argument = NULL;
        if (!last_restore_all_time)
                ch_printf(ch,
                          "There has been no restore all since reboot\n\r");
        else
        {
                time_passed = current_time - last_restore_all_time;
                hour = (int) (time_passed / 3600);
                minute = (int) ((time_passed - (hour * 3600)) / 60);
                ch_printf(ch,
                          "The  last restore all was %d hours and %d minutes ago.\n\r",
                          hour, minute);
        }

        if (!ch->pcdata)
                return;

        if (!ch->pcdata->restore_time)
        {
                send_to_char("You have never done a restore all.\n\r", ch);
                return;
        }

        time_passed = current_time - ch->pcdata->restore_time;
        hour = (int) (time_passed / 3600);
        minute = (int) ((time_passed - (hour * 3600)) / 60);
        ch_printf(ch,
                  "Your last restore all was %d hours and %d minutes ago.\n\r",
                  hour, minute);
        return;
}

CMDF do_freeze(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Freeze whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (get_trust(victim) >= get_trust(ch))
        {
                send_to_char("You failed.\n\r", ch);
                return;
        }

        if (IS_SET(victim->act, PLR_FREEZE))
        {
                REMOVE_BIT(victim->act, PLR_FREEZE);
                send_to_char("You can play again.\n\r", victim);
                send_to_char("FREEZE removed.\n\r", ch);
        }
        else
        {
                SET_BIT(victim->act, PLR_FREEZE);
                send_to_char("You can't do ANYthing!\n\r", victim);
                send_to_char("FREEZE set.\n\r", ch);
        }

        save_char_obj(victim);

        return;
}



CMDF do_log(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Log whom?\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "all"))
        {
                if (fLogAll)
                {
                        fLogAll = FALSE;
                        send_to_char("Log ALL off.\n\r", ch);
                }
                else
                {
                        fLogAll = TRUE;
                        send_to_char("Log ALL on.\n\r", ch);
                }
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        /*
         * No level check, gods can log anyone.
         */
        if (IS_SET(victim->act, PLR_LOG))
        {
                REMOVE_BIT(victim->act, PLR_LOG);
                send_to_char("LOG removed.\n\r", ch);
        }
        else
        {
                SET_BIT(victim->act, PLR_LOG);
                send_to_char("LOG set.\n\r", ch);
        }

        return;
}


CMDF do_litterbug(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Set litterbug flag on whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (get_trust(victim) >= get_trust(ch))
        {
                send_to_char("You failed.\n\r", ch);
                return;
        }

        if (IS_SET(victim->act, PLR_LITTERBUG))
        {
                REMOVE_BIT(victim->act, PLR_LITTERBUG);
                send_to_char("You can drop items again.\n\r", victim);
                send_to_char("LITTERBUG removed.\n\r", ch);
        }
        else
        {
                SET_BIT(victim->act, PLR_LITTERBUG);
                send_to_char
                        ("You a strange force prevents you from dropping any more items!\n\r",
                         victim);
                send_to_char("LITTERBUG set.\n\r", ch);
        }

        return;
}


CMDF do_noemote(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Noemote whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (get_trust(victim) >= get_trust(ch))
        {
                send_to_char("You failed.\n\r", ch);
                return;
        }

        if (IS_SET(victim->act, PLR_NO_EMOTE))
        {
                REMOVE_BIT(victim->act, PLR_NO_EMOTE);
                send_to_char("You can emote again.\n\r", victim);
                send_to_char("NO_EMOTE removed.\n\r", ch);
        }
        else
        {
                SET_BIT(victim->act, PLR_NO_EMOTE);
                send_to_char("You can't emote!\n\r", victim);
                send_to_char("NO_EMOTE set.\n\r", ch);
        }

        return;
}



CMDF do_notell(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Notell whom?", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (get_trust(victim) >= get_trust(ch))
        {
                send_to_char("You failed.\n\r", ch);
                return;
        }

        if (IS_SET(victim->act, PLR_NO_TELL))
        {
                REMOVE_BIT(victim->act, PLR_NO_TELL);
                send_to_char("You can tell again.\n\r", victim);
                send_to_char("NO_TELL removed.\n\r", ch);
        }
        else
        {
                SET_BIT(victim->act, PLR_NO_TELL);
                send_to_char("You can't tell!\n\r", victim);
                send_to_char("NO_TELL set.\n\r", ch);
        }

        return;
}


CMDF do_notitle(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Notitle whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (get_trust(victim) >= get_trust(ch))
        {
                send_to_char("You failed.\n\r", ch);
                return;
        }

        if (IS_SET(victim->pcdata->flags, PCFLAG_NOTITLE))
        {
                REMOVE_BIT(victim->pcdata->flags, PCFLAG_NOTITLE);
                send_to_char("You can set your own title again.\n\r", victim);
                send_to_char("NOTITLE removed.\n\r", ch);
        }
        else
        {
                SET_BIT(victim->pcdata->flags, PCFLAG_NOTITLE);
                snprintf(buf, MSL, "%s", victim->name);
                set_title(victim, buf);
                send_to_char("You can't set your own title!\n\r", victim);
                send_to_char("NOTITLE set.\n\r", ch);
        }

        return;
}

CMDF do_silence(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Silence whom?", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (get_trust(victim) >= get_trust(ch))
        {
                send_to_char("You failed.\n\r", ch);
                return;
        }

        if (IS_SET(victim->act, PLR_SILENCE))
        {
                send_to_char
                        ("Player already silenced, use unsilence to remove.\n\r",
                         ch);
        }
        else
        {
                SET_BIT(victim->act, PLR_SILENCE);
                send_to_char("You can't use channels!\n\r", victim);
                send_to_char("SILENCE set.\n\r", ch);
        }

        return;
}

CMDF do_unsilence(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Unsilence whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (get_trust(victim) >= get_trust(ch))
        {
                send_to_char("You failed.\n\r", ch);
                return;
        }

        if (IS_SET(victim->act, PLR_SILENCE))
        {
                REMOVE_BIT(victim->act, PLR_SILENCE);
                send_to_char("You can use channels again.\n\r", victim);
                send_to_char("SILENCE removed.\n\r", ch);
        }
        else
        {
                send_to_char("That player is not silenced.\n\r", ch);
        }

        return;
}




CMDF do_peace(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *rch;

        argument = NULL;
        act(AT_IMMORT, "$n booms, 'PEACE!'", ch, NULL, NULL, TO_ROOM);
        for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
        {
                if (rch->fighting)
                {
                        stop_fighting(rch, TRUE);
                        do_sit(rch, "");
                }

                /*
                 * Added by Narn, Nov 28/95 
                 */
                stop_hating(rch);
                stop_hunting(rch);
                stop_fearing(rch);
        }

        send_to_char("Ok.\n\r", ch);
        return;
}

extern bool wizlock;
CMDF do_wizlock(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        wizlock = !wizlock;

        if (wizlock)
                send_to_char("Game wizlocked.\n\r", ch);
        else
                send_to_char("Game un-wizlocked.\n\r", ch);

        return;
}


CMDF do_noresolve(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        sysdata.NO_NAME_RESOLVING = !sysdata.NO_NAME_RESOLVING;

        if (sysdata.NO_NAME_RESOLVING)
                send_to_char("Name resolving disabled.\n\r", ch);
        else
                send_to_char("Name resolving enabled.\n\r", ch);

        return;
}


CMDF do_users(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        DESCRIPTOR_DATA *d;
        int       count;
        char      arg[MAX_INPUT_LENGTH];

        set_pager_color(AT_PLAIN, ch);

        one_argument(argument, arg);
        count = 0;
        buf[0] = '\0';

        snprintf(buf, MSL,
                 "\n\rDesc|Curr Connection State|Idle| Player      @HostIP           ");
        mudstrlcat(buf, "\n\r", MSL);
        mudstrlcat(buf,
                   "----+---------------------+----+------+-------------------------------",
                   MSL);
        mudstrlcat(buf, "\n\r", MSL);
        send_to_pager(buf, ch);

        for (d = first_descriptor; d; d = d->next)
        {
                if (arg[0] == '\0')
                {
                        if (get_trust(ch) >= LEVEL_SUPREME
                            || (d->character && can_see(ch, d->character)))
                        {
                                count++;
                                snprintf(buf, MSL,
                                         " %3d| %20.20s|%4d| %-12s %-16s ",
                                         d->descriptor,
                                         center_str(connection_state
                                                    [d->connected], 20),
                                         d->idle / 4,
                                         d->original ? d->original->name : d->
                                         character ? d->character->
                                         name : "(none)", d->host);
                                mudstrlcat(buf, "\n\r", MSL);
                                send_to_pager(buf, ch);
                        }
                }
                else
                {
                        if ((get_trust(ch) >= LEVEL_SUPREME
                             || (d->character && can_see(ch, d->character)))
                            && (!str_prefix(arg, d->host)
                                || (d->character
                                    && !str_prefix(arg, d->character->name))))
                        {
                                count++;
                                pager_printf(ch,
                                             " %3d| %20.20s|%4d| %-12s@%-16s ",
                                             d->descriptor,
                                             center_str(connection_state
                                                        [d->connected], 20),
                                             d->idle / 4,
                                             d->original ? d->original->
                                             name : d->character ? d->
                                             character->name : "(none)",
                                             d->host);
                                buf[0] = '\0';
                                mudstrlcat(buf, "\n\r", MSL);
                                send_to_pager(buf, ch);
                        }
                }
        }
        pager_printf(ch, "%d user%s.\n\r", count, count == 1 ? "" : "s");
        return;
}


/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
CMDF do_force(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        bool      mobsonly;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char("Force whom to do what?\n\r", ch);
                return;
        }

        mobsonly = get_trust(ch) < sysdata.level_forcepc;

        if (!str_cmp(arg, "all"))
        {
                CHAR_DATA *vch;
                CHAR_DATA *vch_next;

                if (mobsonly)
                {
                        send_to_char("Force whom to do what?\n\r", ch);
                        return;
                }

                for (vch = first_char; vch; vch = vch_next)
                {
                        vch_next = vch->next;

                        if (!IS_NPC(vch) && get_trust(vch) < get_trust(ch))
                        {
                                act(AT_IMMORT, "$n forces you to '$t'.", ch,
                                    argument, vch, TO_VICT_OOC);
                                interpret(vch, argument);
                        }
                }
        }
        else
        {
                CHAR_DATA *victim;

                if ((victim = get_char_world(ch, arg)) == NULL)
                {
                        send_to_char("They aren't here.\n\r", ch);
                        return;
                }

                if (victim == ch)
                {
                        send_to_char("Aye aye, right away!\n\r", ch);
                        return;
                }

                if ((get_trust(victim) > get_trust(ch))
                    || (mobsonly && !IS_NPC(victim)))
                {
                        send_to_char("Do it yourself!\n\r", ch);
                        return;
                }

                act(AT_IMMORT, "$n forces you to '$t'.", ch, argument, victim,
                    TO_VICT_OOC);
                interpret(victim, argument);
        }
        send_to_char("Ok.\n\r", ch);
        return;
}


CMDF do_invis(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        sh_int    level;

        /*
         * if ( IS_NPC(ch))
         * return;
         */

        argument = one_argument(argument, arg);
        if (arg && arg[0] != '\0')
        {
                if (!is_number(arg))
                {
                        send_to_char("Usage: invis | invis <level>\n\r", ch);
                        return;
                }
                level = atoi(arg);
                if (level < 2 || level > get_trust(ch))
                {
                        send_to_char("Invalid level.\n\r", ch);
                        return;
                }

                if (!IS_NPC(ch))
                {
                        ch->pcdata->wizinvis = level;
                        ch_printf(ch, "Wizinvis level set to %d.\n\r", level);
                }

                if (IS_NPC(ch))
                {
                        ch->mobinvis = level;
                        ch_printf(ch, "Mobinvis level set to %d.\n\r", level);
                }
                return;
        }

        if (!IS_NPC(ch))
        {
                if (ch->pcdata->wizinvis < 2)
                        ch->pcdata->wizinvis = ch->top_level;
        }

        if (IS_NPC(ch))
        {
                if (ch->mobinvis < 2)
                        ch->mobinvis = ch->top_level;
        }

        if (IS_SET(ch->act, PLR_WIZINVIS))
        {
                REMOVE_BIT(ch->act, PLR_WIZINVIS);
                act(AT_IMMORT, "$n slowly fades into existence.", ch, NULL,
                    NULL, TO_ROOM);
                send_to_char("You slowly fade back into existence.\n\r", ch);
        }
        else
        {
                SET_BIT(ch->act, PLR_WIZINVIS);
                act(AT_IMMORT, "$n slowly fades into thin air.", ch, NULL,
                    NULL, TO_ROOM);
                send_to_char("You slowly vanish into thin air.\n\r", ch);
        }

        return;
}



CMDF do_holylight(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        if (IS_NPC(ch))
                return;

        if (IS_SET(ch->act, PLR_HOLYLIGHT))
        {
                REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
                send_to_char("Holy light mode off.\n\r", ch);
        }
        else
        {
                SET_BIT(ch->act, PLR_HOLYLIGHT);
                send_to_char("Holy light mode on.\n\r", ch);
        }

        return;
}


CMDF do_passdoor(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        if (IS_NPC(ch))
                return;

        if (IS_AFFECTED(ch, AFF_PASS_DOOR))
        {
                REMOVE_BIT(ch->affected_by, AFF_PASS_DOOR);
                send_to_char("Pass Door mode off.\n\r", ch);
        }
        else
        {
                SET_BIT(ch->affected_by, AFF_PASS_DOOR);
                send_to_char("Pass Door mode on.\n\r", ch);
        }

        return;
}

CMDF do_flying(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        if (IS_NPC(ch))
                return;

        if (IS_AFFECTED(ch, AFF_FLYING))
        {
                REMOVE_BIT(ch->affected_by, AFF_FLYING);
                send_to_char("Flying mode off.\n\r", ch);
        }
        else
        {
                SET_BIT(ch->affected_by, AFF_FLYING);
                send_to_char("Flying mode on.\n\r", ch);
        }

        return;
}

CMDF do_rassign(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        int       r_lo, r_hi;
        CHAR_DATA *victim;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);
        r_lo = atoi(arg2);
        r_hi = atoi(arg3);

        if (arg1[0] == '\0' || r_lo < 0 || r_hi < 0)
        {
                send_to_char("Syntax: assign <who> <low> <high>\n\r", ch);
                return;
        }
        if ((victim = get_char_world(ch, arg1)) == NULL)
        {
                send_to_char("They don't seem to be around.\n\r", ch);
                return;
        }
        if (IS_NPC(victim) || get_trust(victim) < LEVEL_AVATAR)
        {
                send_to_char
                        ("They wouldn't know what to do with a room range.\n\r",
                         ch);
                return;
        }
        if (r_lo > r_hi)
        {
                send_to_char("Unacceptable room range.\n\r", ch);
                return;
        }
        if (r_lo == 0)
                r_hi = 0;
        victim->pcdata->r_range_lo = r_lo;
        victim->pcdata->r_range_hi = r_hi;
        assign_area(victim);
        send_to_char("Done.\n\r", ch);
        ch_printf(victim, "%s has assigned you the room range %d - %d.\n\r",
                  ch->name, r_lo, r_hi);
        assign_area(victim);    /* Put back by Thoric on 02/07/96 */
        if (!victim->pcdata->area)
        {
                bug("rassign: assign_area failed", 0);
                return;
        }

        if (r_lo == 0)  /* Scryn 8/12/95 */
        {
                REMOVE_BIT(victim->pcdata->area->status, AREA_LOADED);
                SET_BIT(victim->pcdata->area->status, AREA_DELETED);
        }
        else
        {
                SET_BIT(victim->pcdata->area->status, AREA_LOADED);
                REMOVE_BIT(victim->pcdata->area->status, AREA_DELETED);
        }
        return;
}

CMDF do_vassign(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        int       r_lo, r_hi;
        CHAR_DATA *victim;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);
        r_lo = atoi(arg2);
        r_hi = atoi(arg3);

        if (arg1[0] == '\0' || r_lo < 0 || r_hi < 0)
        {
                send_to_char("Syntax: vassign <who> <low> <high>\n\r", ch);
                return;
        }
        if ((victim = get_char_world(ch, arg1)) == NULL)
        {
                send_to_char("They don't seem to be around.\n\r", ch);
                return;
        }
        if (IS_NPC(victim) || get_trust(victim) < LEVEL_CREATOR)
        {
                send_to_char
                        ("They wouldn't know what to do with a vnum range.\n\r",
                         ch);
                return;
        }
        if (r_lo > r_hi)
        {
                send_to_char("Unacceptable room range.\n\r", ch);
                return;
        }
        if (r_lo == 0)
                r_hi = 0;
        victim->pcdata->r_range_lo = r_lo;
        victim->pcdata->r_range_hi = r_hi;
        victim->pcdata->o_range_lo = r_lo;
        victim->pcdata->o_range_hi = r_hi;
        victim->pcdata->m_range_lo = r_lo;
        victim->pcdata->m_range_hi = r_hi;

        assign_area(victim);
        send_to_char("Done.\n\r", ch);
        ch_printf(victim, "%s has assigned you the vnum range %d - %d.\n\r",
                  ch->name, r_lo, r_hi);
        assign_area(victim);    /* Put back by Thoric on 02/07/96 */
        if (!victim->pcdata->area)
        {
                bug("rassign: assign_area failed", 0);
                return;
        }

        if (r_lo == 0)  /* Scryn 8/12/95 */
        {
                REMOVE_BIT(victim->pcdata->area->status, AREA_LOADED);
                SET_BIT(victim->pcdata->area->status, AREA_DELETED);
        }
        else
        {
                SET_BIT(victim->pcdata->area->status, AREA_LOADED);
                REMOVE_BIT(victim->pcdata->area->status, AREA_DELETED);
        }
        return;
}

CMDF do_oassign(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        int       o_lo, o_hi;
        CHAR_DATA *victim;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);
        o_lo = atoi(arg2);
        o_hi = atoi(arg3);

        if (arg1[0] == '\0' || o_lo < 0 || o_hi < 0)
        {
                send_to_char("Syntax: oassign <who> <low> <high>\n\r", ch);
                return;
        }
        if ((victim = get_char_world(ch, arg1)) == NULL)
        {
                send_to_char("They don't seem to be around.\n\r", ch);
                return;
        }
        if (IS_NPC(victim) || get_trust(victim) < LEVEL_SAVIOR)
        {
                send_to_char
                        ("They wouldn't know what to do with an object range.\n\r",
                         ch);
                return;
        }
        if (o_lo > o_hi)
        {
                send_to_char("Unacceptable object range.\n\r", ch);
                return;
        }
        victim->pcdata->o_range_lo = o_lo;
        victim->pcdata->o_range_hi = o_hi;
        assign_area(victim);
        send_to_char("Done.\n\r", ch);
        ch_printf(victim,
                  "%s has assigned you the object vnum range %d - %d.\n\r",
                  ch->name, o_lo, o_hi);
        return;
}

CMDF do_massign(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        int       m_lo, m_hi;
        CHAR_DATA *victim;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);
        m_lo = atoi(arg2);
        m_hi = atoi(arg3);

        if (arg1[0] == '\0' || m_lo < 0 || m_hi < 0)
        {
                send_to_char("Syntax: massign <who> <low> <high>\n\r", ch);
                return;
        }
        if ((victim = get_char_world(ch, arg1)) == NULL)
        {
                send_to_char("They don't seem to be around.\n\r", ch);
                return;
        }
        if (IS_NPC(victim) || get_trust(victim) < LEVEL_SAVIOR)
        {
                send_to_char
                        ("They wouldn't know what to do with a monster range.\n\r",
                         ch);
                return;
        }
        if (m_lo > m_hi)
        {
                send_to_char("Unacceptable monster range.\n\r", ch);
                return;
        }
        victim->pcdata->m_range_lo = m_lo;
        victim->pcdata->m_range_hi = m_hi;
        assign_area(victim);
        send_to_char("Done.\n\r", ch);
        ch_printf(victim,
                  "%s has assigned you the monster vnum range %d - %d.\n\r",
                  ch->name, m_lo, m_hi);
        return;
}

CMDF do_cmdtable(CHAR_DATA * ch, char *argument)
{
        int       hash, cnt;
        CMDTYPE  *cmd;

        argument = NULL;

        set_pager_color(AT_PLAIN, ch);
        send_to_pager("Commands and Number of Uses This Run\n\r", ch);

        for (cnt = hash = 0; hash < 126; hash++)
                for (cmd = command_hash[hash]; cmd; cmd = cmd->next)
                {
                        if ((++cnt) % 4)
                                pager_printf(ch, "%-6.6s %4d\t", cmd->name,
                                             cmd->userec.num_uses);
                        else
                                pager_printf(ch, "%-6.6s %4d\n\r", cmd->name,
                                             cmd->userec.num_uses);
                }
        send_to_pager("\n\r", ch);
        return;
}

/*
 * Load up a player file
 */
CMDF do_loadup(CHAR_DATA * ch, char *argument)
{
        char      fname[1024];
        char      name[256];
        struct stat fst;
        bool      loaded;
        DESCRIPTOR_DATA *d;
        int       old_room_vnum;
        char      buf[MAX_STRING_LENGTH];

        one_argument(argument, name);
        if (name[0] == '\0')
        {
                send_to_char("&RSyntax: &Gloadup &C<&cplayername&C>&w\n\r",
                             ch);
                return;
        }

        name[0] = UPPER(name[0]);

        snprintf(fname, MSL, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
                 capitalize(name));
        if (stat(fname, &fst) != -1)
        {
                CREATE(d, DESCRIPTOR_DATA, 1);
                d->next = NULL;
                d->prev = NULL;
                d->connected = CON_GET_NAME;
                d->outsize = 2000;
                CREATE(d->outbuf, char, d->outsize);

                loaded = load_char_obj(d, name, FALSE, FALSE);
                add_char(d->character);
                old_room_vnum = d->character->in_room->vnum;
                char_to_room(d->character, ch->in_room);
                if (get_trust(d->character) > get_trust(ch))
                {
                        do_say(d->character, "Do *NOT* disturb me again!");
                        send_to_char
                                ("I think you'd better leave that player alone!\n\r",
                                 ch);
                        d->character->desc = NULL;
                        do_quit(d->character, "");
                        return;
                }
                d->character->desc = NULL;
                d->character->retran = old_room_vnum;
                d->character = NULL;
                DISPOSE(d->outbuf);
                DISPOSE(d);
                ch_printf(ch, "Player %s loaded from room %d.\n\r",
                          capitalize(name), old_room_vnum);
                snprintf(buf, MSL,
                         "%s appears from nowhere, eyes glazed over.\n\r",
                         capitalize(name));
                act(AT_IMMORT, buf, ch, NULL, NULL, TO_ROOM);
                send_to_char("Done.\n\r", ch);
                return;
        }
        /*
         * else no player file 
         */
        send_to_char("No such player.\n\r", ch);
        return;
}

CMDF do_fixchar(CHAR_DATA * ch, char *argument)
{
        char      name[MAX_STRING_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, name);
        if (name[0] == '\0')
        {
                send_to_char("Usage: fixchar <playername>\n\r", ch);
                return;
        }
        victim = get_char_room(ch, name);
        if (!victim)
        {
                send_to_char("They're not here.\n\r", ch);
                return;
        }
        fix_char(victim);
/*  victim->armor	= 100;
    victim->mod_str	= 0;
    victim->mod_dex	= 0;
    victim->mod_wis	= 0;
    victim->mod_int	= 0;
    victim->mod_con	= 0;
    victim->mod_cha	= 0;
    victim->mod_lck	= 0;
    victim->damroll	= 0;
    victim->hitroll	= 0;
    victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
    victim->saving_spell_staff = 0; */



        send_to_char("Done.\n\r", ch);
}

CMDF do_newbieset(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        OBJ_DATA *obj;
        CHAR_DATA *victim;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char("Syntax: newbieset <char>.\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg1)) == NULL)
        {
                send_to_char("That player is not here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if ((victim->top_level < 1) || (victim->top_level > 5))
        {
                send_to_char("Level of victim must be 1 to 5.\n\r", ch);
                return;
        }
        obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 1);
        obj_to_char(obj, victim);

        obj = create_object(get_obj_index(OBJ_VNUM_SCHOOL_DAGGER), 1);
        obj_to_char(obj, victim);

        /*
         * Added by Brittany, on Nov. 24, 1996. The object is the adventurer's 
         * guide to the realms of despair, part of academy.are. 
         */
        {
                OBJ_INDEX_DATA *obj_ind = get_obj_index(10333);

                if (obj_ind != NULL)
                {
                        obj = create_object(obj_ind, 1);
                        obj_to_char(obj, victim);
                }
        }

/* Added the burlap sack to the newbieset.  The sack is part of sgate.are
   called Spectral Gate.  Brittany */

        {

                OBJ_INDEX_DATA *obj_ind = get_obj_index(123);

                if (obj_ind != NULL)
                {
                        obj = create_object(obj_ind, 1);
                        obj_to_char(obj, victim);
                }
        }

        act(AT_IMMORT, "$n has equipped you with a newbieset.", ch, NULL,
            victim, TO_VICT);
        ch_printf(ch, "You have re-equipped %s.\n\r", victim->name);
        return;
}

/*
 * Extract area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "joe.are susan.are"
 * - Gorog
 */
void extract_area_names(char *inp, char *out)
{
        char      buf[MAX_INPUT_LENGTH], *pbuf = buf;
        int       len;

        *out = '\0';
        while (inp && *inp)
        {
                inp = one_argument(inp, buf);
                if ((len = strlen(buf)) >= 5
                    && !strcmp(".are", pbuf + len - 4))
                {
                        if (*out)
                                mudstrlcat(out, " ", MSL);
                        mudstrlcat(out, buf, MSL);
                }
        }
}

/*
 * Remove area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "aset sedit cset"
 * - Gorog
 */
void remove_area_names(char *inp, char *out)
{
        char      buf[MAX_INPUT_LENGTH], *pbuf = buf;
        int       len;

        *out = '\0';
        while (inp && *inp)
        {
                inp = one_argument(inp, buf);
                if ((len = strlen(buf)) < 5 || strcmp(".are", pbuf + len - 4))
                {
                        if (*out)
                                mudstrlcat(out, " ", MSL);
                        mudstrlcat(out, buf, MSL);
                }
        }
}

CMDF do_bestowarea(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        CHAR_DATA *victim;
        int       arg_len;

        argument = one_argument(argument, arg);

        if (get_trust(ch) < LEVEL_SUB_IMPLEM)
        {
                send_to_char("Sorry...\n\r", ch);
                return;
        }

        if (!*arg)
        {
                send_to_char("Syntax:\n\r"
                             "bestowarea <victim> <filename>.are\n\r"
                             "bestowarea <victim> none             removes bestowed areas\n\r"
                             "bestowarea <victim> list             lists bestowed areas\n\r"
                             "bestowarea <victim>                  lists bestowed areas\n\r",
                             ch);
                return;
        }

        if (!(victim = get_char_world(ch, arg)))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("You can't give special abilities to a mob!\n\r",
                             ch);
                return;
        }

        if (get_trust(victim) < LEVEL_IMMORTAL)
        {
                send_to_char("They aren't an immortal.\n\r", ch);
                return;
        }

        if (!victim->pcdata->bestowments)
                victim->pcdata->bestowments = STRALLOC("");

        if (!*argument || !str_cmp(argument, "list"))
        {
                extract_area_names(victim->pcdata->bestowments, buf);
                ch_printf(ch, "Bestowed areas: %s\n\r", buf);
                return;
        }

        if (!str_cmp(argument, "none"))
        {
                remove_area_names(victim->pcdata->bestowments, buf);
                STRFREE(victim->pcdata->bestowments);
                victim->pcdata->bestowments = STRALLOC(buf);
                send_to_char("Done.\n\r", ch);
                return;
        }

        arg_len = strlen(argument);
        if (arg_len < 5
            || argument[arg_len - 4] != '.' || argument[arg_len - 3] != 'a'
            || argument[arg_len - 2] != 'r' || argument[arg_len - 1] != 'e')
        {
                send_to_char("You can only bestow an area name\n\r", ch);
                send_to_char("E.G. bestow joe sam.are\n\r", ch);
                return;
        }

        snprintf(buf, MSL, "%s %s", victim->pcdata->bestowments, argument);
        STRFREE(victim->pcdata->bestowments);
        victim->pcdata->bestowments = STRALLOC(buf);
        ch_printf(victim, "%s has bestowed on you the area: %s\n\r",
                  ch->name, argument);
        send_to_char("Done.\n\r", ch);
}

CMDF do_bestow(CHAR_DATA * ch, char *argument)
{
        char      arg[MIL], buf[MAX_STRING_LENGTH],
                arg_buf[MAX_STRING_LENGTH];
        CHAR_DATA *victim;
        CMDTYPE  *cmd;
        bool      fComm = FALSE;

        set_char_color(AT_IMMORT, ch);

        argument = one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Bestow whom with what?\n\r", ch);
                return;
        }
        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }
        if (IS_NPC(victim))
        {
                send_to_char("You can't give special abilities to a mob!\n\r",
                             ch);
                return;
        }
        if (victim == ch || get_trust(victim) >= get_trust(ch))
        {
                send_to_char("You aren't powerful enough...\n\r", ch);
                return;
        }

        if (!victim->pcdata->bestowments)
                victim->pcdata->bestowments = STRALLOC("");

        if (argument[0] == '\0' || !str_cmp(argument, "show list"))
        {
                ch_printf(ch, "Current bestowed commands on %s: %s.\n\r",
                          victim->name, victim->pcdata->bestowments);
                return;
        }

        if (!str_cmp(argument, "none"))
        {
                STRFREE(victim->pcdata->bestowments);
                victim->pcdata->bestowments = STRALLOC("");
                ch_printf(ch, "Bestowments removed from %s.\n\r",
                          victim->name);
                ch_printf(victim,
                          "%s has removed your bestowed commands.\n\r",
                          ch->name);
                return;
        }

        arg_buf[0] = '\0';
        argument = one_argument(argument, arg);

        while (arg && arg[0] != '\0')
        {
                char     *cmd_buf, cmd_tmp[MAX_INPUT_LENGTH];
                bool      cFound = FALSE;

                if (!(cmd = find_command(arg)))
                {
                        ch_printf(ch, "No such command as %s!\n\r", arg);
                        argument = one_argument(argument, arg);
                        continue;
                }
                else if (cmd->level > get_trust(ch))
                {
                        ch_printf(ch, "You can't bestow the %s command!\n\r",
                                  arg);
                        argument = one_argument(argument, arg);
                        continue;
                }

                cmd_buf = victim->pcdata->bestowments;
                cmd_buf = one_argument(cmd_buf, cmd_tmp);
                while (cmd_tmp && cmd_tmp[0] != '\0')
                {
                        if (!str_cmp(cmd_tmp, arg))
                        {
                                cFound = TRUE;
                                break;
                        }

                        cmd_buf = one_argument(cmd_buf, cmd_tmp);
                }

                if (cFound == TRUE)
                {
                        argument = one_argument(argument, arg);
                        continue;
                }

                mudstrlcat(arg, " ", MIL);
                mudstrlcat(arg_buf, arg, MSL);
                argument = one_argument(argument, arg);
                fComm = TRUE;
        }
        if (!fComm)
        {
                send_to_char
                        ("Good job, knucklehead... you just bestowed them with that master command called 'NOTHING!'\n\r",
                         ch);
                return;
        }

        if (arg_buf[strlen(arg_buf) - 1] == ' ')
                arg_buf[strlen(arg_buf) - 1] = '\0';

        snprintf(buf, MSL, "%s %s", victim->pcdata->bestowments, arg_buf);
        STRFREE(victim->pcdata->bestowments);
        smash_tilde(buf);
        victim->pcdata->bestowments = STRALLOC(buf);
        set_char_color(AT_IMMORT, victim);
        ch_printf(victim, "%s has bestowed on you the command(s): %s\n\r",
                  ch->name, arg_buf);
        send_to_char("Done.\n\r", ch);
}


struct tm *update_time(struct tm *old_time)
{
        time_t    _time;

        _time = mktime(old_time);
        return localtime(&_time);
}

CMDF do_set_boot_time(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg1[MAX_INPUT_LENGTH];
        bool      check;

        check = FALSE;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char
                        ("Syntax: setboot time {hour minute <day> <month> <year>}\n\r",
                         ch);
                send_to_char("        setboot manual {0/1}\n\r", ch);
                send_to_char("        setboot default\n\r", ch);
                ch_printf(ch,
                          "Boot time is currently set to %s, manual bit is set to %d\n\r",
                          reboot_time, set_boot_time->manual);
                return;
        }

        if (!str_cmp(arg, "time"))
        {
                struct tm *now_time;

                argument = one_argument(argument, arg);
                argument = one_argument(argument, arg1);
                if (!*arg || !*arg1 || !is_number(arg) || !is_number(arg1))
                {
                        send_to_char
                                ("You must input a value for hour and minute.\n\r",
                                 ch);
                        return;
                }
                now_time = localtime(&current_time);

                if ((now_time->tm_hour = atoi(arg)) < 0
                    || now_time->tm_hour > 23)
                {
                        send_to_char("Valid range for hour is 0 to 23.\n\r",
                                     ch);
                        return;
                }

                if ((now_time->tm_min = atoi(arg1)) < 0
                    || now_time->tm_min > 59)
                {
                        send_to_char("Valid range for minute is 0 to 59.\n\r",
                                     ch);
                        return;
                }

                argument = one_argument(argument, arg);
                if (*arg != '\0' && is_number(arg))
                {
                        if ((now_time->tm_mday = atoi(arg)) < 1
                            || now_time->tm_mday > 31)
                        {
                                send_to_char
                                        ("Valid range for day is 1 to 31.\n\r",
                                         ch);
                                return;
                        }
                        argument = one_argument(argument, arg);
                        if (*arg != '\0' && is_number(arg))
                        {
                                if ((now_time->tm_mon = atoi(arg)) < 1
                                    || now_time->tm_mon > 12)
                                {
                                        send_to_char
                                                ("Valid range for month is 1 to 12.\n\r",
                                                 ch);
                                        return;
                                }
                                now_time->tm_mon--;
                                argument = one_argument(argument, arg);
                                if ((now_time->tm_year = atoi(arg) - 1900) < 0
                                    || now_time->tm_year > 199)
                                {
                                        send_to_char
                                                ("Valid range for year is 1900 to 2099.\n\r",
                                                 ch);
                                        return;
                                }
                        }
                }
                now_time->tm_sec = 0;
                if (mktime(now_time) < current_time)
                {
                        send_to_char
                                ("You can't set a time previous to today!\n\r",
                                 ch);
                        return;
                }
                if (set_boot_time->manual == 0)
                        set_boot_time->manual = 1;
                new_boot_time = update_time(now_time);
                new_boot_struct = *new_boot_time;
                new_boot_time = &new_boot_struct;
                reboot_check(mktime(new_boot_time));
                get_reboot_string();

                ch_printf(ch, "Boot time set to %s\n\r", reboot_time);
                check = TRUE;
        }
        else if (!str_cmp(arg, "manual"))
        {
                argument = one_argument(argument, arg1);
                if (arg1[0] == '\0')
                {
                        send_to_char
                                ("Please enter a value for manual boot on/off\n\r",
                                 ch);
                        return;
                }

                if (!is_number(arg1))
                {
                        send_to_char
                                ("Value for manual must be 0 (off) or 1 (on)\n\r",
                                 ch);
                        return;
                }

                if (atoi(arg1) < 0 || atoi(arg1) > 1)
                {
                        send_to_char
                                ("Value for manual must be 0 (off) or 1 (on)\n\r",
                                 ch);
                        return;
                }

                set_boot_time->manual = atoi(arg1);
                ch_printf(ch, "Manual bit set to %s\n\r", arg1);
                check = TRUE;
                get_reboot_string();
                return;
        }

        else if (!str_cmp(arg, "default"))
        {
                set_boot_time->manual = 0;
                /*
                 * Reinitialize new_boot_time 
                 */
                new_boot_time = localtime(&current_time);
                new_boot_time->tm_mday += 1;
                if (new_boot_time->tm_hour > 12)
                        new_boot_time->tm_mday += 1;
                new_boot_time->tm_hour = 6;
                new_boot_time->tm_min = 0;
                new_boot_time->tm_sec = 0;
                new_boot_time = update_time(new_boot_time);

                sysdata.DENY_NEW_PLAYERS = FALSE;

                send_to_char("Reboot time set back to normal.\n\r", ch);
                check = TRUE;
        }

        if (!check)
        {
                send_to_char("Invalid argument for setboot.\n\r", ch);
                return;
        }

        else
        {
                get_reboot_string();
                new_boot_time_t = mktime(new_boot_time);
        }
}

/* Online high level immortal command for displaying what the encryption
 * of a name/password would be, taking in 2 arguments - the name and the
 * password - can still only change the password if you have access to 
 * pfiles and the correct password
 */
CMDF do_form_password(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_STRING_LENGTH];

        argument = one_argument(argument, arg);

        ch_printf(ch, "Those two arguments encrypted would result in: %s",
                  crypt(arg, argument));
        return;
}

/*
 * Purge a player file.  No more player.  -- Altrag
 */
CMDF do_destro(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        set_char_color(AT_RED, ch);
        send_to_char("If you want to destroy a character, spell it out!\n\r",
                     ch);
        return;
}

/*
 * This could have other applications too.. move if needed. -- Altrag
 */
/*extern OBJ_INDEX_DATA   *obj_index_hash[MAX_KEY_HASH]; - Already declared - Gavin */
/* extern MOB_INDEX_DATA   *mob_index_hash[MAX_KEY_HASH]; - Already declared - Gavin */
void close_area(AREA_DATA * pArea)
{
        CHAR_DATA *ech;
        CHAR_DATA *ech_next;
        OBJ_DATA *eobj;
        OBJ_DATA *eobj_next;
        int       icnt;
        ROOM_INDEX_DATA *rid;
        ROOM_INDEX_DATA *rid_next;
        OBJ_INDEX_DATA *oid;
        OBJ_INDEX_DATA *oid_next;
        MOB_INDEX_DATA *mid;
        MOB_INDEX_DATA *mid_next;
        RESET_DATA *ereset;
        RESET_DATA *ereset_next;
        EXTRA_DESCR_DATA *eed;
        EXTRA_DESCR_DATA *eed_next;
        EXIT_DATA *pexit;
        EXIT_DATA *exit_next;
        MPROG_ACT_LIST *mpact;
        MPROG_ACT_LIST *mpact_next;
        MPROG_DATA *mprog;
        MPROG_DATA *mprog_next;
        AFFECT_DATA *paf;
        AFFECT_DATA *paf_next;

        for (ech = first_char; ech; ech = ech_next)
        {
                ech_next = ech->next;

                if (ech->fighting)
                        stop_fighting(ech, TRUE);
                if (IS_NPC(ech))
                {
                        /*
                         * if mob is in area, or part of area. 
                         */
                        if (URANGE(pArea->low_m_vnum, ech->pIndexData->vnum,
                                   pArea->hi_m_vnum) == ech->pIndexData->vnum
                            || (ech->in_room && ech->in_room->area == pArea))
                                extract_char(ech, TRUE);
                        continue;
                }
                if (ech->in_room && ech->in_room->area == pArea)
                        do_recall(ech, "");
        }
        for (eobj = first_object; eobj; eobj = eobj_next)
        {
                eobj_next = eobj->next;
                /*
                 * if obj is in area, or part of area. 
                 */
                if (URANGE(pArea->low_o_vnum, eobj->pIndexData->vnum,
                           pArea->hi_o_vnum) == eobj->pIndexData->vnum ||
                    (eobj->in_room && eobj->in_room->area == pArea))
                        extract_obj(eobj);
        }
        for (icnt = 0; icnt < MAX_KEY_HASH; icnt++)
        {
                for (rid = room_index_hash[icnt]; rid; rid = rid_next)
                {
                        rid_next = rid->next;

                        for (pexit = rid->first_exit; pexit;
                             pexit = exit_next)
                        {
                                exit_next = pexit->next;
                                if (rid->area == pArea
                                    || pexit->to_room->area == pArea)
                                {
                                        STRFREE(pexit->keyword);
                                        STRFREE(pexit->description);
                                        UNLINK(pexit, rid->first_exit,
                                               rid->last_exit, next, prev);
                                        DISPOSE(pexit);
                                }
                        }
                        if (rid->area != pArea)
                                continue;
                        STRFREE(rid->name);
                        STRFREE(rid->description);
                        if (rid->first_person)
                        {
                                bug("close_area: room with people #%d",
                                    rid->vnum);
                                for (ech = rid->first_person; ech;
                                     ech = ech_next)
                                {
                                        ech_next = ech->next_in_room;
                                        if (ech->fighting)
                                                stop_fighting(ech, TRUE);
                                        if (IS_NPC(ech))
                                                extract_char(ech, TRUE);
                                        else
                                                do_recall(ech, "");
                                }
                        }
                        if (rid->first_content)
                        {
                                bug("close_area: room with contents #%d",
                                    rid->vnum);
                                for (eobj = rid->first_content; eobj;
                                     eobj = eobj_next)
                                {
                                        eobj_next = eobj->next_content;
                                        extract_obj(eobj);
                                }
                        }
                        for (eed = rid->first_extradesc; eed; eed = eed_next)
                        {
                                eed_next = eed->next;
                                STRFREE(eed->keyword);
                                STRFREE(eed->description);
                                DISPOSE(eed);
                        }
                        for (mpact = rid->mpact; mpact; mpact = mpact_next)
                        {
                                mpact_next = mpact->next;
                                STRFREE(mpact->buf);
                                DISPOSE(mpact);
                        }
                        for (mprog = rid->mudprogs; mprog; mprog = mprog_next)
                        {
                                mprog_next = mprog->next;
                                STRFREE(mprog->arglist);
                                STRFREE(mprog->comlist);
                                DISPOSE(mprog);
                        }
                        if (rid == room_index_hash[icnt])
                                room_index_hash[icnt] = rid->next;
                        else
                        {
                                ROOM_INDEX_DATA *trid;

                                for (trid = room_index_hash[icnt]; trid;
                                     trid = trid->next)
                                        if (trid->next == rid)
                                                break;
                                if (!trid)
                                        bug("Close_area: rid not in hash list %d", rid->vnum);
                                else
                                        trid->next = rid->next;
                        }
                        DISPOSE(rid);
                }

                for (mid = mob_index_hash[icnt]; mid; mid = mid_next)
                {
                        mid_next = mid->next;

                        if (mid->vnum < pArea->low_m_vnum
                            || mid->vnum > pArea->hi_m_vnum)
                                continue;

                        STRFREE(mid->player_name);
                        STRFREE(mid->short_descr);
                        STRFREE(mid->long_descr);
                        STRFREE(mid->description);
                        if (mid->pShop)
                        {
                                UNLINK(mid->pShop, first_shop, last_shop,
                                       next, prev);
                                DISPOSE(mid->pShop);
                        }
                        if (mid->rShop)
                        {
                                UNLINK(mid->rShop, first_repair, last_repair,
                                       next, prev);
                                DISPOSE(mid->rShop);
                        }
                        for (mprog = mid->mudprogs; mprog; mprog = mprog_next)
                        {
                                mprog_next = mprog->next;
                                STRFREE(mprog->arglist);
                                STRFREE(mprog->comlist);
                                DISPOSE(mprog);
                        }
                        if (mid == mob_index_hash[icnt])
                                mob_index_hash[icnt] = mid->next;
                        else
                        {
                                MOB_INDEX_DATA *tmid;

                                for (tmid = mob_index_hash[icnt]; tmid;
                                     tmid = tmid->next)
                                        if (tmid->next == mid)
                                                break;
                                if (!tmid)
                                        bug("Close_area: mid not in hash list %s", mid->vnum);
                                else
                                        tmid->next = mid->next;
                        }
                        DISPOSE(mid);
                }

                for (oid = obj_index_hash[icnt]; oid; oid = oid_next)
                {
                        oid_next = oid->next;

                        if (oid->vnum < pArea->low_o_vnum
                            || oid->vnum > pArea->hi_o_vnum)
                                continue;

                        STRFREE(oid->name);
                        STRFREE(oid->short_descr);
                        STRFREE(oid->description);
                        STRFREE(oid->action_desc);

                        for (eed = oid->first_extradesc; eed; eed = eed_next)
                        {
                                eed_next = eed->next;
                                STRFREE(eed->keyword);
                                STRFREE(eed->description);
                                DISPOSE(eed);
                        }
                        for (paf = oid->first_affect; paf; paf = paf_next)
                        {
                                paf_next = paf->next;
                                DISPOSE(paf);
                        }
                        for (mprog = oid->mudprogs; mprog; mprog = mprog_next)
                        {
                                mprog_next = mprog->next;
                                STRFREE(mprog->arglist);
                                STRFREE(mprog->comlist);
                                DISPOSE(mprog);
                        }
                        if (oid == obj_index_hash[icnt])
                                obj_index_hash[icnt] = oid->next;
                        else
                        {
                                OBJ_INDEX_DATA *toid;

                                for (toid = obj_index_hash[icnt]; toid;
                                     toid = toid->next)
                                        if (toid->next == oid)
                                                break;
                                if (!toid)
                                        bug("Close_area: oid not in hash list %s", oid->vnum);
                                else
                                        toid->next = oid->next;
                        }
                        DISPOSE(oid);
                }
        }
        for (ereset = pArea->first_reset; ereset; ereset = ereset_next)
        {
                ereset_next = ereset->next;
                DISPOSE(ereset);
        }
        DISPOSE(pArea->name);
        DISPOSE(pArea->filename);
        STRFREE(pArea->author);
        if( IS_SET( pArea->flags, AFLAG_PROTOTYPE ))
        {
            UNLINK( pArea, first_build, last_build, next, prev );
            UNLINK( pArea, first_bsort, last_bsort, next_sort, prev_sort );
        }
        else
        {
            UNLINK( pArea, first_area, last_area, next, prev );
            UNLINK( pArea, first_asort, last_asort, next_sort, prev_sort );
        }
        DISPOSE(pArea);
}

CMDF do_destroy(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        char      buf[MAX_STRING_LENGTH];
        char      buf2[MAX_STRING_LENGTH];
        char      arg[MAX_INPUT_LENGTH];
        struct stat fst;
        char     *name;


        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Destroy what player file?\n\r", ch);
                return;
        }

        /*
         * Set the file points.
         */
        name = capitalize(arg);
        snprintf(buf, MSL, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), name);
        snprintf(buf2, MSL, "%s%c/%s", BACKUP_DIR, tolower(arg[0]), name);

        /*
         * This check makes sure the name is valid and that the file is there, else there
         * is no need to go on. -Orion
         */
        if (lstat(buf, &fst) == -1)
        {
                ch_printf(ch, "No player exists by the name %s.\n\r", name);
                return;
        }

        for (victim = first_char; victim; victim = victim->next)
                if (!IS_NPC(victim) && !str_cmp(victim->name, arg))
                        break;
        if (!victim)
        {
                DESCRIPTOR_DATA *d;

                /*
                 * Make sure they aren't halfway logged in. 
                 */
                for (d = first_descriptor; d; d = d->next)
                        if ((victim = d->character) && !IS_NPC(victim) &&
                            !str_cmp(victim->name, arg))
                                break;
                if (d)
                        close_socket(d, TRUE);
        }
        else
        {
                int       x, y;

                quitting_char = victim;
                save_char_obj(victim);
                saving_char = NULL;
                extract_char(victim, TRUE);
                for (x = 0; x < MAX_WEAR; x++)
                        for (y = 0; y < MAX_LAYERS; y++)
                                save_equipment[x][y] = NULL;
        }

        if (!rename(buf, buf2))
        {
                AREA_DATA *pArea;

                set_char_color(AT_RED, ch);
                send_to_char
                        ("Player destroyed.  Pfile saved in backup directory.\n\r",
                         ch);
                snprintf(buf, MSL, "%s%s", GOD_DIR, capitalize(arg));
                if (!remove(buf))
                        send_to_char("Player's immortal data destroyed.\n\r",
                                     ch);
                else if (errno != ENOENT)
                {
                        ch_printf(ch,
                                  "Unknown error #%d - %s (immortal data).  Report to Thoric.\n\r",
                                  errno, strerror(errno));
                        snprintf(buf2, MSL, "%s destroying %s", ch->name,
                                 buf);
                        perror(buf2);
                }

                snprintf(buf2, MSL, "%s.are", capitalize(arg));
                for (pArea = first_build; pArea; pArea = pArea->next)
                        if (!strcmp(pArea->filename, buf2))
                        {
                                snprintf(buf, MSL, "%s%s", BUILD_DIR, buf2);
                                if (IS_SET(pArea->status, AREA_LOADED))
                                        fold_area(pArea, buf, FALSE, FALSE);
                                close_area(pArea);

                                close_area(pArea);
                                snprintf(buf2, MSL, "%s.bak", buf);
                                set_char_color(AT_RED, ch); /* Log message changes colors */
                                if (!rename(buf, buf2))
                                        send_to_char
                                                ("Player's area data destroyed.  Area saved as backup.\n\r",
                                                 ch);
                                else if (errno != ENOENT)
                                {
                                        ch_printf(ch,
                                                  "Unknown error #%d - %s (area data).  Report to Thoric.\n\r",
                                                  errno, strerror(errno));
                                        snprintf(buf2, MSL,
                                                 "%s destroying %s", ch->name,
                                                 buf);
                                        perror(buf2);
                                }
                        }
        }
        else if (errno == ENOENT)
        {
                set_char_color(AT_PLAIN, ch);
                send_to_char("Player does not exist.\n\r", ch);
        }
        else
        {
                set_char_color(AT_WHITE, ch);
                ch_printf(ch,
                          "Unknown error #%d - %s.  Report to Thoric.\n\r",
                          errno, strerror(errno));
                snprintf(buf, MSL, "%s destroying %s", ch->name, arg);
                perror(buf);
        }
        return;
}

/* Super-AT command:

FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>


Executes action several times, either on ALL players (not including yourself),
MORTALS (including trusted characters), GODS (characters with level higher than
L_HERO), MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containg
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example: 

FOR ALL SMILE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with 
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/
const char *name_expand(CHAR_DATA * ch)
{
        int       count = 1;
        CHAR_DATA *rch;
        char      name[MAX_INPUT_LENGTH];   /*  HOPEFULLY no mob has a name longer than THAT */

        static char outbuf[MAX_INPUT_LENGTH];

        if (!IS_NPC(ch))
                return ch->name;

        one_argument(ch->name, name);   /* copy the first word into name */

        if (!name[0])   /* weird mob .. no keywords */
        {
                mudstrlcpy(outbuf, "", MIL);    /* Do not return NULL, just an empty buffer */
                return outbuf;
        }

        /*
         * ->people changed to ->first_person -- TRI 
         */
        for (rch = ch->in_room->first_person; rch && (rch != ch); rch =
             rch->next_in_room)
                if (is_name(name, rch->name))
                        count++;


        snprintf(outbuf, MSL, "%d.%s", count, name);
        return outbuf;
}


CMDF do_for(CHAR_DATA * ch, char *argument)
{
        char      range[MAX_INPUT_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        bool      fGods = FALSE, fMortals = FALSE, fMobs =
                FALSE, fEverywhere = FALSE, found;
        ROOM_INDEX_DATA *room, *old_room;
        CHAR_DATA *p, *p_prev;  /* p_next to p_prev -- TRI */
        int       i;

        argument = one_argument(argument, range);

        if (!range[0] || !argument[0])  /* invalid usage? */
        {
                do_help(ch, "for");
                return;
        }

        if (!str_prefix("quit", argument))
        {
                send_to_char
                        ("Are you trying to crash the MUD or something?\n\r",
                         ch);
                return;
        }


        if (!str_cmp(range, "all"))
        {
                fMortals = TRUE;
                fGods = TRUE;
        }
        else if (!str_cmp(range, "gods"))
                fGods = TRUE;
        else if (!str_cmp(range, "mortals"))
                fMortals = TRUE;
        else if (!str_cmp(range, "mobs"))
                fMobs = TRUE;
        else if (!str_cmp(range, "everywhere"))
                fEverywhere = TRUE;
        else
                do_help(ch, "for"); /* show syntax */

        /*
         * do not allow # to make it easier 
         */
        if (fEverywhere && strchr(argument, '#'))
        {
                send_to_char
                        ("Cannot use FOR EVERYWHERE with the # thingie.\n\r",
                         ch);
                return;
        }

        if (strchr(argument, '#'))  /* replace # ? */
        {
                /*
                 * char_list - last_char, p_next - gch_prev -- TRI 
                 */
                for (p = last_char; p; p = p_prev)
                {
                        p_prev = p->prev;   /* TRI */
                        /*
                         * p_next = p->next; 
                         *//*
                         * In case someone DOES try to AT MOBS SLAY # 
                         */
                        found = FALSE;

                        if (!(p->in_room) || room_is_private(p, p->in_room)
                            || (p == ch))
                                continue;

                        if (IS_NPC(p) && fMobs)
                                found = TRUE;
                        else if (!IS_NPC(p) && get_trust(p) >= LEVEL_IMMORTAL
                                 && fGods)
                                found = TRUE;
                        else if (!IS_NPC(p) && get_trust(p) < LEVEL_IMMORTAL
                                 && fMortals)
                                found = TRUE;

                        /*
                         * It looks ugly to me.. but it works :) 
                         */
                        if (found)  /* p is 'appropriate' */
                        {
                                char     *pSource = argument;   /* head of buffer to be parsed */
                                char     *pDest = buf;  /* parse into this */

                                while (*pSource)
                                {
                                        if (*pSource == '#')    /* Replace # with name of target */
                                        {
                                                const char *namebuf =
                                                        name_expand(p);

                                                if (namebuf)    /* in case there is no mob name ?? */
                                                        while (*namebuf)    /* copy name over */
                                                                *(pDest++) =
                                                                        *
                                                                        (namebuf++);

                                                pSource++;
                                        }
                                        else
                                                *(pDest++) = *(pSource++);
                                }   /* while */
                                *pDest = '\0';  /* Terminate */

                                /*
                                 * Execute 
                                 */
                                old_room = ch->in_room;
                                char_from_room(ch);
                                char_to_room(ch, p->in_room);
                                interpret(ch, buf);
                                char_from_room(ch);
                                char_to_room(ch, old_room);

                        }   /* if found */
                }   /* for every char */
        }
        else    /* just for every room with the appropriate people in it */
        {
                for (i = 0; i < MAX_KEY_HASH; i++)  /* run through all the buckets */
                        for (room = room_index_hash[i]; room;
                             room = room->next)
                        {
                                found = FALSE;

                                /*
                                 * Anyone in here at all? 
                                 */
                                if (fEverywhere)    /* Everywhere executes always */
                                        found = TRUE;
                                else if (!room->first_person)   /* Skip it if room is empty */
                                        continue;
                                /*
                                 * ->people changed to first_person -- TRI 
                                 */

                                /*
                                 * Check if there is anyone here of the requried type 
                                 */
                                /*
                                 * Stop as soon as a match is found or there are no more ppl in room 
                                 */
                                /*
                                 * ->people to ->first_person -- TRI 
                                 */
                                for (p = room->first_person; p && !found;
                                     p = p->next_in_room)
                                {

                                        if (p == ch)    /* do not execute on oneself */
                                                continue;

                                        if (IS_NPC(p) && fMobs)
                                                found = TRUE;
                                        else if (!IS_NPC(p)
                                                 && (get_trust(p) >=
                                                     LEVEL_IMMORTAL) && fGods)
                                                found = TRUE;
                                        else if (!IS_NPC(p)
                                                 && (get_trust(p) <=
                                                     LEVEL_IMMORTAL)
                                                 && fMortals)
                                                found = TRUE;
                                }   /* for everyone inside the room */

                                if (found && !room_is_private(p, room)) /* Any of the required type here AND room not private? */
                                {
                                        /*
                                         * This may be ineffective. Consider moving character out of old_room
                                         * once at beginning of command then moving back at the end.
                                         * This however, is more safe?
                                         */

                                        old_room = ch->in_room;
                                        char_from_room(ch);
                                        char_to_room(ch, room);
                                        interpret(ch, argument);
                                        char_from_room(ch);
                                        char_to_room(ch, old_room);
                                }   /* if found */
                        }   /* for every room in a bucket */
        }   /* if strchr */
}   /* do_for */

void save_sysdata args((SYSTEM_DATA sys));

CMDF do_cset(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_STRING_LENGTH];
        sh_int    level;

        set_char_color(AT_IMMORT, ch);

        if (argument[0] == '\0')
        {
                ch_printf(ch,
                          "&BS&zystem:\n\r  Current Port: &w%-22s&z Maximum Log Size: &w%d&z\n\r",
                          sysdata.PORT ? "Main" : "Coding", sysdata.log_size);
                ch_printf(ch, "  Mud Name: &w%-26s&z Mud Email:&w%s&z\n\r",
                          sysdata.mud_name, sysdata.mud_email);
                ch_printf(ch, "  Mud Url: &w%-27s&z Mail Path: &w%s&z\n\r",
                          sysdata.mud_url, sysdata.mail_path);
                ch_printf(ch,
                          "  Channel log: &w%23.23d&z Name Resolving: &w%s&z\n\r",
                          sysdata.channellog,
                          !sysdata.
                          NO_NAME_RESOLVING ? "Enabled" : "Disabled");
#ifdef WEB
                ch_printf(ch,
                          "  Web server:  &w%23.23s&z Status: &w%s&z\n\r",
                          sysdata.
                          web ? "Enabled" : "Disabled",
                          WEBSERVER_STATUS ? "On" : "Off");
#endif
                ch_printf(ch,
                          "&BM&zail:\n\r  Read all mail: &w%-21.21d&z Read mail for free: &w%d&z.\n\r",
                          sysdata.read_all_mail, sysdata.read_mail_free);
                ch_printf(ch,
                          "  Take all mail: &w%-20.20d&z  Write mail for free: &w%d&z.\n\r",
                          sysdata.take_others_mail, sysdata.write_mail_free);
                ch_printf(ch,
                          "&BP&zfiles:\n\r  Autocleanup status: &w%s&z\n\r  Days before purging newbies: &w%d&z\n\r",
                          sysdata.CLEANPFILES ? "On" : "Off",
                          sysdata.newbie_purge);
                ch_printf(ch, "  Days before purging players: &w%d&z\n\r",
                          sysdata.regular_purge);
                ch_printf(ch,
                          "&BB&zuilding:\n\r  Prototype modification: &w%-.12d&z Player msetting: &w%d&z.\n\r",
                          sysdata.level_modify_proto,
                          sysdata.level_mset_player);
                ch_printf(ch,
                          "&BO&zther:\n\r  Force on players: &w%-18.18d&z ",
                          sysdata.level_forcepc);
                ch_printf(ch, "Private room override: &w%d&z.\n\r",
                          sysdata.level_override_private);
                ch_printf(ch, "  Penalty to regular stun chance: &w%-4.4d&z ",
                          sysdata.stun_regular);
                ch_printf(ch, "Penalty to stun plr vs. plr: &w%d&z\n\r",
                          sysdata.stun_plr_vs_plr);
                ch_printf(ch, "  Percent damage plr vs. plr: &w%-8.8d&z ",
                          sysdata.dam_plr_vs_plr);
                ch_printf(ch, "Percent damage plr vs. mob: &w%d&z.\n\r",
                          sysdata.dam_plr_vs_mob);
                ch_printf(ch, "  Percent damage mob vs. plr: &w%-8.8d&z ",
                          sysdata.dam_mob_vs_plr);
                ch_printf(ch, "Percent damage mob vs. mob: &w%d&z.\n\r",
                          sysdata.dam_mob_vs_mob);
                ch_printf(ch, "  Get object without take flag: &w%-6.6d&z ",
                          sysdata.level_getobjnotake);
                ch_printf(ch, "Autosave frequency (minutes): &w%d&z.\n\r",
                          sysdata.save_frequency);
                ch_printf(ch, "  Debug Output: &w%s&z.\n\r",
                          true_false[sysdata.DEBUG]);
                ch_printf(ch, "  Greet System: &w%s&z.\n\r",
                          true_false[sysdata.GREET]);
                ch_printf(ch, "  Save flags: &w%s&z\n\r\n\r&W",
                          flag_string(sysdata.save_flags, save_flag));
                return;
        }

        argument = one_argument(argument, arg);
        level = (sh_int) atoi(argument);

        if (!str_cmp(arg, "help"))
        {
                do_help(ch, "controls");
                return;
        }
		else if (!str_cmp(arg, "pfiles"))
        {

                sysdata.CLEANPFILES = !sysdata.CLEANPFILES;

                if (sysdata.CLEANPFILES)
                        send_to_char("Pfile autocleanup enabled.\n\r", ch);
                else
                        send_to_char("Pfile autocleanup disabled.\n\r", ch);
                return;
        }
#ifdef WEB
		else if (!str_cmp(arg, "web"))
        {

                sysdata.web = !sysdata.web;

                if (sysdata.web)
                {
                        send_to_char("Web server enabled.\n\r", ch);
                        sprintf(log_buf, "Booting webserver on port %d.",
                                port + 2);
                        log_string(log_buf);
                        init_web(port + 2);
                }
                else
                {
                        send_to_char("Web server disasbled.\n\r", ch);
                        sprintf(log_buf,
                                "Shutting down webserver on port %d.",
                                port + 2);
                        log_string(log_buf);
                        shutdown_web();
                }
                return;
        }
#endif
		else if (!str_cmp(arg, "save"))
        {
                save_sysdata(sysdata);
                return;
        }
		else if (!str_cmp(arg, "debug"))
        {

                sysdata.DEBUG = !sysdata.DEBUG;

                if (sysdata.DEBUG)
                        send_to_char("Debug enabled.\n\r", ch);
                else
                        send_to_char("Debug disabled.\n\r", ch);
                return;
        }
		else if (!str_cmp(arg, "greet"))
        {

                sysdata.GREET = !sysdata.GREET;

                if (sysdata.GREET)
                        send_to_char("Greet enabled.\n\r", ch);
                else
                        send_to_char("Greet disabled.\n\r", ch);
                return;
        }
		else if (!str_cmp(arg, "saveflag"))
        {
                char      buf[MSL];
                int       value;

                if (!argument || argument[0] == '\0')
                {
                        send_to_char
                                ("Usage: cset saveflag <flag> [flag]...\n\r",
                                 ch);
                        send_to_char(wordwrap(show_ext_flag_string
                                              (NUMITEMS(save_flag),
                                               save_flag), 78), ch);
                        send_to_char("\n\r", ch);
                        return;
                }
                while (argument[0] != '\0')
                {
                        argument = one_argument(argument, buf);
                        value = get_saveflag(buf);
                        if (value < 0 || value > 31)
                                ch_printf(ch, "Unknown flag: %s\n\r", buf);
                        else
                                TOGGLE_BIT(sysdata.save_flags, 1 << value);
                }
                return;
        }


        else if (!str_prefix(arg, "savefrequency"))
        {
                sysdata.save_frequency = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }

        else if (!str_cmp(arg, "log_size"))
        {
                sysdata.log_size = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }

        else if (!str_cmp(arg, "newbie_purge"))
        {
                if (level < 1)
                {
                        send_to_char
                                ("You must specify a period of at least 1 day.\n\r",
                                 ch);
                        return;
                }

                sysdata.newbie_purge = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }

        else if (!str_cmp(arg, "regular_purge"))
        {
                if (level < 1)
                {
                        send_to_char
                                ("You must specify a period of at least 1 day.\n\r",
                                 ch);
                        return;
                }

                sysdata.regular_purge = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }

        else if (!str_cmp(arg, "stun"))
        {
                sysdata.stun_regular = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }

        else if (!str_cmp(arg, "stun_pvp"))
        {
                sysdata.stun_plr_vs_plr = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }

        else if (!str_cmp(arg, "dam_pvp"))
        {
                sysdata.dam_plr_vs_plr = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }

        else if (!str_cmp(arg, "get_notake"))
        {
                sysdata.level_getobjnotake = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }

        else if (!str_cmp(arg, "dam_pvm"))
        {
                sysdata.dam_plr_vs_mob = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }

        else if (!str_cmp(arg, "dam_mvp"))
        {
                sysdata.dam_mob_vs_plr = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }

        else if (!str_cmp(arg, "dam_mvm"))
        {
                sysdata.dam_mob_vs_mob = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }

        if (level < 0 || level > MAX_LEVEL)
        {
                send_to_char("Invalid value for new control.\n\r", ch);
                return;
        }

        else if (!str_cmp(arg, "read_all"))
        {
                sysdata.read_all_mail = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }

        else if (!str_cmp(arg, "read_free"))
        {
                sysdata.read_mail_free = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg, "write_free"))
        {
                sysdata.write_mail_free = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg, "take_all"))
        {
                sysdata.take_others_mail = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg, "muse"))
        {
                sysdata.muse_level = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg, "think"))
        {
                sysdata.think_level = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg, "log"))
        {
                sysdata.log_level = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg, "build"))
        {
                sysdata.build_level = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg, "proto_modify"))
        {
                sysdata.level_modify_proto = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg, "override_private"))
        {
                sysdata.level_override_private = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg, "forcepc"))
        {
                sysdata.level_forcepc = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg, "mudurl"))
        {
                if (sysdata.mud_url != NULL)
                        STRFREE(sysdata.mud_url);
                sysdata.mud_url = STRALLOC(argument);
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg, "mailpath"))
        {
                if (sysdata.mail_path != NULL)
                        STRFREE(sysdata.mail_path);
                sysdata.mail_path = STRALLOC(argument);
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg, "mudname"))
        {
                if (sysdata.mud_name != NULL)
                        STRFREE(sysdata.mud_name);
                sysdata.mud_name = STRALLOC(argument);
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg, "mudemail"))
        {
                if (sysdata.mud_email != NULL)
                        STRFREE(sysdata.mud_email);
                sysdata.mud_email = STRALLOC(argument);
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg, "channellog"))
        {
                sh_int    oldsize;
                CHANNEL_DATA *channel;

                for (channel = first_channel; channel;
                     channel = channel->next)
                {
                        oldsize = sysdata.channellog;
                        sysdata.channellog = level;

                        /*
                         * If we are not setting it to the same size 
                         */
                        if (channel->log != NULL && level != oldsize)
                        {
                                LOG_DATA *new_log;
                                sh_int    i;

                                CREATE(new_log, LOG_DATA, level);
                                /*
                                 * if the old one was smaller, its ok to start at 0 and copy 
                                 */
                                if (oldsize < level)
                                {
                                        for (i = 0; i < oldsize; i++)
                                        {
                                                if (!channel->log[i].name
                                                    || !channel->log[i].
                                                    message)
                                                        continue;
                                                new_log[i] = channel->log[i];
                                        }
                                }
                                /*
                                 * if the new one is smaller, we need to just copy the last X messages in 
                                 */
                                else
                                {
                                        for (i = level; i >= 0; i--)
                                                new_log[i] =
                                                        channel->log[oldsize -
                                                                     (level -
                                                                      i)];

                                        for (i = 0; i < level; i++)
                                        {
                                                if (!channel->log[i].name
                                                    || !channel->log[i].
                                                    message)
                                                        continue;
                                                DISPOSE(channel->log[i].name);
                                                DISPOSE(channel->log[i].
                                                        message);
                                        }
                                }
                                DISPOSE(channel->log);
                                channel->log = new_log;
                                channel->logpos = level;
                        }
                }
                send_to_char("Ok.\n\r", ch);
                return;
        }
		else if (!str_cmp(arg, "port"))
        {

                sysdata.PORT = !sysdata.PORT;

                if (sysdata.PORT)
                        send_to_char("Main Port system status enabled.\n\r",
                                     ch);
                else
                        send_to_char("Coding Port system status enabled.\n\r",
                                     ch);
                return;
        }

		else if (!str_cmp(arg, "mset_player"))
        {
                sysdata.level_mset_player = level;
                send_to_char("Ok.\n\r", ch);
                return;
        }
        else
        {
                send_to_char("Invalid argument.\n\r", ch);
                send_to_char
                        ("Valid arguments:\n\rmset_player, port, mudemail, mudname, mailpath\n\r",
                         ch);
                send_to_char
                        ("mudurl, forcepc, override_private, proto_modify, build, log, think\n\r",
                         ch);
                send_to_char
                        ("mude, take_all, write_free, read_free, read_all, dam_mvm, dam_mvp\n\r",
                         ch);
                send_to_char
                        ("dam_pvm, dam_pvp, get_notake, stun_pvp, stun, regular_purge\n\r",
                         ch);
                send_to_char
                        ("newbie_purge, log_size, savefrequency,\n\r", ch);
                send_to_char("saveflag, save, pfiles, channellog\n\r", ch);
                return;
        }
}

void get_reboot_string(void)
{
        snprintf(reboot_time, MSL, "%s", asctime(new_boot_time));
}


CMDF do_orange(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        send_to_char("Function under construction.\n\r", ch);
        return;
}

CMDF do_mrange(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        send_to_char("Function under construction.\n\r", ch);
        return;
}

CMDF do_hell(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        char      arg[MAX_INPUT_LENGTH];
        sh_int    amt_of_time;
        bool      h_d = FALSE;
        struct tm *tms;

        argument = one_argument(argument, arg);
        if (!*arg)
        {
                send_to_char("Hell who, and for how long?\n\r", ch);
                return;
        }
        if (!(victim = get_char_world(ch, arg)) || IS_NPC(victim))
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }
        if (IS_IMMORTAL(victim))
        {
                send_to_char("There is no point in helling an immortal.\n\r",
                             ch);
                return;
        }
        if (victim->pcdata->release_date != 0)
        {
                ch_printf(ch,
                          "They are already in hell until %24.24s, by %s.\n\r",
                          ctime(&victim->pcdata->release_date),
                          victim->pcdata->helled_by);
                return;
        }
        argument = one_argument(argument, arg);
        if (!*arg || !is_number(arg))
        {
                send_to_char("Hell them for how long?\n\r", ch);
                return;
        }
        amt_of_time = atoi(arg);
        if (amt_of_time <= 0)
        {
                send_to_char("You cannot hell for zero or negative time.\n\r",
                             ch);
                return;
        }
        argument = one_argument(argument, arg);
        if (!*arg || !str_prefix(arg, "hours"))
                h_d = TRUE;
        else if (str_prefix(arg, "days"))
        {
                send_to_char("Is that value in hours or days?\n\r", ch);
                return;
        }
        else if (amt_of_time > 30)
        {
                send_to_char
                        ("You may not hell a person for more than 30 days at a time.\n\r",
                         ch);
                return;
        }
        tms = localtime(&current_time);
        if (h_d)
                tms->tm_hour += amt_of_time;
        else
                tms->tm_mday += amt_of_time;
        victim->pcdata->release_date = mktime(tms);
        victim->pcdata->helled_by = STRALLOC(ch->name);
        ch_printf(ch, "%s will be released from hell at %24.24s.\n\r",
                  victim->name, ctime(&victim->pcdata->release_date));
        act(AT_MAGIC, "$n disappears in a cloud of hellish light.", victim,
            NULL, ch, TO_NOTVICT);
        char_from_room(victim);
        char_to_room(victim, get_room_index(6));
        act(AT_MAGIC, "$n appears in a could of hellish light.", victim, NULL,
            ch, TO_NOTVICT);
        do_look(victim, "auto");
        ch_printf(victim,
                  "The immortals are not pleased with your actions.\n\r"
                  "You shall remain in hell for %d %s%s.\n\r", amt_of_time,
                  (h_d ? "hour" : "day"), (amt_of_time == 1 ? "" : "s"));
        save_char_obj(victim);  /* used to save ch, fixed by Thoric 09/17/96 */
        return;
}

CMDF do_unhell(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        char      arg[MAX_INPUT_LENGTH];
        ROOM_INDEX_DATA *location;

        argument = one_argument(argument, arg);
        if (!*arg)
        {
                send_to_char("Unhell whom..?\n\r", ch);
                return;
        }
        location = ch->in_room;
        ch->in_room = get_room_index(6);
        victim = get_char_room(ch, arg);
        ch->in_room = location; /* The case of unhell self, etc. */
        if (!victim || IS_NPC(victim) || victim->in_room->vnum != 6)
        {
                send_to_char("No one like that is in hell.\n\r", ch);
                return;
        }
        location = get_room_index(wherehome(victim));
        if (!location)
                location = ch->in_room;
        MOBtrigger = FALSE;
        act(AT_MAGIC, "$n disappears in a cloud of godly light.", victim,
            NULL, ch, TO_NOTVICT);
        char_from_room(victim);
        char_to_room(victim, location);
        send_to_char
                ("The gods have smiled on you and released you from hell early!\n\r",
                 victim);
        do_look(victim, "auto");
        send_to_char("They have been released.\n\r", ch);

        if (victim->pcdata->helled_by)
        {
                if (str_cmp(ch->name, victim->pcdata->helled_by))
                        ch_printf(ch,
                                  "(You should probably write a note to %s, explaining the early release.)\n\r",
                                  victim->pcdata->helled_by);
                STRFREE(victim->pcdata->helled_by);
                victim->pcdata->helled_by = NULL;
        }

        MOBtrigger = FALSE;
        act(AT_MAGIC, "$n appears in a cloud of godly light.", victim, NULL,
            ch, TO_NOTVICT);
        victim->pcdata->release_date = 0;
        save_char_obj(victim);
        return;
}

/* Vnum search command by Swordbearer */
CMDF do_vsearch(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        bool      found = FALSE;
        OBJ_DATA *obj;
        OBJ_DATA *in_obj;
        int       obj_counter = 1;
        int       argi;

        one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Syntax:  vsearch <vnum>.\n\r", ch);
                return;
        }

        set_pager_color(AT_PLAIN, ch);
        argi = atoi(arg);
        if (argi < 0 && argi > MAX_VNUMS)
        {
                send_to_char("Vnum out of range.\n\r", ch);
                return;
        }
        for (obj = first_object; obj != NULL; obj = obj->next)
        {
                if (!can_see_obj(ch, obj) || !(argi == obj->pIndexData->vnum))
                        continue;

                found = TRUE;
                for (in_obj = obj; in_obj->in_obj != NULL;
                     in_obj = in_obj->in_obj);

                if (in_obj->carried_by != NULL)
                        pager_printf(ch,
                                     "[%2d] Level %d %s carried by %s.\n\r",
                                     obj_counter, obj->level, obj_short(obj),
                                     PERS(in_obj->carried_by, ch));
                else
                        pager_printf(ch, "[%2d] [%-5d] %s in %s.\n\r",
                                     obj_counter,
                                     ((in_obj->in_room) ? in_obj->in_room->
                                      vnum : 0), obj_short(obj),
                                     (in_obj->in_room ==
                                      NULL) ? "somewhere" : in_obj->in_room->
                                     name);

                obj_counter++;
        }

        if (!found)
                send_to_char
                        ("Nothing like that in hell, earth, or heaven.\n\r",
                         ch);

        return;
}

/* 
 * Simple function to let any imm make any player instantly sober.
 * Saw no need for level restrictions on this.
 * Written by Narn, Apr/96 
 */
CMDF do_sober(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        char      arg1[MAX_INPUT_LENGTH];

        smash_tilde(argument);
        argument = one_argument(argument, arg1);
        if ((victim = get_char_room(ch, arg1)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on mobs.\n\r", ch);
                return;
        }

        if (victim->pcdata)
                victim->pcdata->condition[COND_DRUNK] = 0;
        send_to_char("Ok.\n\r", ch);
        send_to_char("You feel sober again.\n\r", victim);
        return;
}


/*
 * Free a social structure					-Thoric
 */
void free_social(SOCIALTYPE * social)
{
        if (social->name)
                DISPOSE(social->name);
        if (social->char_no_arg)
                DISPOSE(social->char_no_arg);
        if (social->others_no_arg)
                DISPOSE(social->others_no_arg);
        if (social->char_found)
                DISPOSE(social->char_found);
        if (social->others_found)
                DISPOSE(social->others_found);
        if (social->vict_found)
                DISPOSE(social->vict_found);
        if (social->char_auto)
                DISPOSE(social->char_auto);
        if (social->others_auto)
                DISPOSE(social->others_auto);
        DISPOSE(social);
}

/*
 * Remove a social from it's hash index				-Thoric
 */
void unlink_social(SOCIALTYPE * social)
{
        SOCIALTYPE *tmp, *tmp_next;
        int       hash;

        if (!social)
        {
                bug("Unlink_social: NULL social", 0);
                return;
        }

        if (social->name[0] < 'a' || social->name[0] > 'z')
                hash = 0;
        else
                hash = (social->name[0] - 'a') + 1;

        if (social == (tmp = social_index[hash]))
        {
                social_index[hash] = tmp->next;
                return;
        }
        for (; tmp; tmp = tmp_next)
        {
                tmp_next = tmp->next;
                if (social == tmp_next)
                {
                        tmp->next = tmp_next->next;
                        return;
                }
        }
}

/*
 * Add a social to the social index table			-Thoric
 * Hashed and insert sorted
 */
void add_social(SOCIALTYPE * social)
{
        int       hash, x;
        SOCIALTYPE *tmp, *prev;

        if (!social)
        {
                bug("Add_social: NULL social", 0);
                return;
        }

        if (!social->name)
        {
                bug("Add_social: NULL social->name", 0);
                return;
        }

        if (!social->char_no_arg)
        {
                bug("Add_social: NULL social->char_no_arg", 0);
                return;
        }

        /*
         * make sure the name is all lowercase 
         */
        for (x = 0; social->name[x] != '\0'; x++)
                social->name[x] = LOWER(social->name[x]);

        if (social->name[0] < 'a' || social->name[0] > 'z')
                hash = 0;
        else
                hash = (social->name[0] - 'a') + 1;

        if ((prev = tmp = social_index[hash]) == NULL)
        {
                social->next = social_index[hash];
                social_index[hash] = social;
                return;
        }

        for (; tmp; tmp = tmp->next)
        {
                if ((x = strcmp(social->name, tmp->name)) == 0)
                {
                        bug("Add_social: trying to add duplicate name to bucket %d", hash);
                        free_social(social);
                        return;
                }
                else if (x < 0)
                {
                        if (tmp == social_index[hash])
                        {
                                social->next = social_index[hash];
                                social_index[hash] = social;
                                return;
                        }
                        prev->next = social;
                        social->next = tmp;
                        return;
                }
                prev = tmp;
        }

        /*
         * add to end 
         */
        prev->next = social;
        social->next = NULL;
        return;
}

/*
 * Social editor/displayer/save/delete				-Thoric
 */
CMDF do_sedit(CHAR_DATA * ch, char *argument)
{
        SOCIALTYPE *social;
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];

        smash_tilde(argument);
        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        set_char_color(AT_SOCIAL, ch);

        if (arg1[0] == '\0')
        {
                send_to_char("Syntax: sedit <social> [field]\n\r", ch);
                send_to_char("Syntax: sedit <social> create\n\r", ch);
                if (get_trust(ch) > LEVEL_GOD)
                        send_to_char("Syntax: sedit <social> delete\n\r", ch);
                if (get_trust(ch) > LEVEL_LESSER)
                        send_to_char("Syntax: sedit <save>\n\r", ch);
                send_to_char("\n\rField being one of:\n\r", ch);
                send_to_char
                        ("  cnoarg onoarg cfound ofound vfound cauto oauto arousal minarousal\n\r",
                         ch);
                return;
        }

        if (IS_IMM_ENFORCER(ch) && !str_cmp(arg1, "save"))
        {
                save_socials();
                send_to_char("Saved.\n\r", ch);
                return;
        }

        social = find_social(arg1);

        if (!str_cmp(arg2, "create"))
        {
                if (social)
                {
                        send_to_char("That social already exists!\n\r", ch);
                        return;
                }
                CREATE(social, SOCIALTYPE, 1);
                social->name = str_dup(arg1);
                snprintf(arg2, MSL, "You %s.", arg1);
                social->char_no_arg = str_dup(arg2);
                add_social(social);
                send_to_char("Social added.\n\r", ch);
                return;
        }

        if (!social)
        {
                send_to_char("Social not found.\n\rChecking Xsocials...\n\r",
                             ch);
                social = find_xsocial(arg1);
        }

        if (!social)
        {
                send_to_char("XSocial not found.\n\r", ch);
                return;
        }

        if (arg2[0] == '\0' || !str_cmp(arg2, "show"))
        {
                ch_printf(ch, "Social: %s\n\r\n\rCNoArg: %s\n\r",
                          social->name, social->char_no_arg);
                ch_printf(ch, "ONoArg: %s\n\rCFound: %s\n\rOFound: %s\n\r",
                          social->others_no_arg ? social->
                          others_no_arg : "(not set)",
                          social->char_found ? social->
                          char_found : "(not set)",
                          social->others_found ? social->
                          others_found : "(not set)");
                ch_printf(ch, "VFound: %s\n\rCAuto : %s\n\rOAuto : %s\n\r",
                          social->vict_found ? social->
                          vict_found : "(not set)",
                          social->char_auto ? social->char_auto : "(not set)",
                          social->others_auto ? social->
                          others_auto : "(not set)");
                ch_printf(ch, "Sex: %s\n\r",
                          social->sex == 0 ? "Neutral" : social->sex ==
                          1 ? "Male" : social->sex ==
                          2 ? "Female" : social->sex ==
                          3 ? "Any" : "Not Set");
                ch_printf(ch, "Arousal: %d\n\rMinimum Arousal : %d\n\r",
                          social->arousal, social->minarousal);
                return;
        }

        if (get_trust(ch) > LEVEL_GOD && !str_cmp(arg2, "delete"))
        {
                unlink_social(social);
                free_social(social);
                send_to_char("Deleted.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "cnoarg"))
        {
                if (argument[0] == '\0' || !str_cmp(argument, "clear"))
                {
                        send_to_char
                                ("You cannot clear this field.  It must have a message.\n\r",
                                 ch);
                        return;
                }
                if (social->char_no_arg)
                        DISPOSE(social->char_no_arg);
                social->char_no_arg = str_dup(argument);
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "onoarg"))
        {
                if (social->others_no_arg)
                        DISPOSE(social->others_no_arg);
                if (argument[0] != '\0' && str_cmp(argument, "clear"))
                        social->others_no_arg = str_dup(argument);
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "cfound"))
        {
                if (social->char_found)
                        DISPOSE(social->char_found);
                if (argument[0] != '\0' && str_cmp(argument, "clear"))
                        social->char_found = str_dup(argument);
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "ofound"))
        {
                if (social->others_found)
                        DISPOSE(social->others_found);
                if (argument[0] != '\0' && str_cmp(argument, "clear"))
                        social->others_found = str_dup(argument);
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "vfound"))
        {
                if (social->vict_found)
                        DISPOSE(social->vict_found);
                if (argument[0] != '\0' && str_cmp(argument, "clear"))
                        social->vict_found = str_dup(argument);
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "cauto"))
        {
                if (social->char_auto)
                        DISPOSE(social->char_auto);
                if (argument[0] != '\0' && str_cmp(argument, "clear"))
                        social->char_auto = str_dup(argument);
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "oauto"))
        {
                if (social->others_auto)
                        DISPOSE(social->others_auto);
                if (argument[0] != '\0' && str_cmp(argument, "clear"))
                        social->others_auto = str_dup(argument);
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "arousal"))
        {
                social->arousal = atoi(argument);
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "minarousal"))
        {
                social->minarousal = atoi(argument);
                send_to_char("Done.\n\r", ch);
                return;
        }


        if (!str_cmp(arg2, "sex"))
        {
                if (str_cmp(argument, "male") && str_cmp(argument, "female")
                    && str_cmp(argument, "neutral")
                    && str_cmp(argument, "any"))
                {
                        send_to_char
                                ("&CChoices&R:&W male, female, neutral, any.\n\r",
                                 ch);
                        return;
                }
                if (!str_cmp(argument, "neutral"))
                {
                        social->sex = 0;
                }
                if (!str_cmp(argument, "male"))
                {
                        social->sex = 1;
                }

                if (!str_cmp(argument, "female"))
                {
                        social->sex = 2;
                }
                if (!str_cmp(argument, "any"))
                {
                        social->sex = 3;
                }

                send_to_char("Done.\n\r", ch);
                return;
        }

        if (get_trust(ch) > LEVEL_GREATER && !str_cmp(arg2, "name"))
        {
                bool      relocate;

                one_argument(argument, arg1);
                if (arg1[0] == '\0')
                {
                        send_to_char("Cannot clear name field!\n\r", ch);
                        return;
                }
                if (arg1[0] != social->name[0])
                {
                        unlink_social(social);
                        relocate = TRUE;
                }
                else
                        relocate = FALSE;
                if (social->name)
                        DISPOSE(social->name);
                social->name = str_dup(arg1);
                if (relocate)
                        add_social(social);
                send_to_char("Done.\n\r", ch);
                return;
        }

        /*
         * display usage message 
         */
        do_sedit(ch, "");
}

/*
 * Free a command structure					-Thoric
 */
void free_command(CMDTYPE * command)
{
        if (command->name)
                DISPOSE(command->name);
        if (command->fun_name)
                DISPOSE(command->fun_name);
        DISPOSE(command);
}

/*
 * Remove a command from it's hash index			-Thoric
 */
void unlink_command(CMDTYPE * command)
{
        CMDTYPE  *tmp, *tmp_next;
        int       hash;

        if (!command)
        {
                bug("Unlink_command NULL command", 0);
                return;
        }

        hash = command->name[0] % 126;

        if (command == (tmp = command_hash[hash]))
        {
                command_hash[hash] = tmp->next;
                return;
        }
        for (; tmp; tmp = tmp_next)
        {
                tmp_next = tmp->next;
                if (command == tmp_next)
                {
                        tmp->next = tmp_next->next;
                        return;
                }
        }
}

/*
 * Add a command to the command hash table			-Thoric
 */
void add_command(CMDTYPE * command)
{
        int       hash, x;
        CMDTYPE  *tmp, *prev;

        if (!command)
        {
                bug("Add_command: NULL command", 0);
                return;
        }

        if (!command->name)
        {
                bug("Add_command: NULL command->name", 0);
                return;
        }

        if (!command->do_fun)
        {
                bug("Add_command: NULL command->do_fun", 0);
                return;
        }

        /*
         * make sure the name is all lowercase 
         */
        for (x = 0; command->name[x] != '\0'; x++)
                command->name[x] = LOWER(command->name[x]);

        hash = command->name[0] % 126;

        if ((prev = tmp = command_hash[hash]) == NULL)
        {
                command->next = command_hash[hash];
                command_hash[hash] = command;
                return;
        }

        /*
         * add to the END of the list 
         */
        for (; tmp; tmp = tmp->next)
                if (!tmp->next)
                {
                        tmp->next = command;
                        command->next = NULL;
                }
        return;
}

/*
 * Command editor/displayer/save/delete				-Thoric
 */
CMDF do_cedit(CHAR_DATA * ch, char *argument)
{
        CMDTYPE  *command;
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];

        smash_tilde(argument);
        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        set_char_color(AT_IMMORT, ch);

        if (arg1[0] == '\0')
        {
                send_to_char("Syntax: cedit save\n\r", ch);
                if (get_trust(ch) > LEVEL_SUB_IMPLEM)
                {
                        send_to_char
                                ("Syntax: cedit <command> create [code]\n\r",
                                 ch);
                        send_to_char("Syntax: cedit <command> delete\n\r",
                                     ch);
                        send_to_char("Syntax: cedit <command> show\n\r", ch);
                        send_to_char("Syntax: cedit <command> [field]\n\r",
                                     ch);
                        send_to_char("\n\rField being one of:\n\r", ch);
                        send_to_char
                                ("  level position log code ooc held list raise lower flags\n\r",
                                 ch);
                }
                return;
        }

        if (get_trust(ch) > LEVEL_GREATER && !str_cmp(arg1, "save"))
        {
                save_commands();
                send_to_char("Saved.\n\r", ch);
                return;
        }

        command = find_command(arg1);

        if (get_trust(ch) > LEVEL_SUB_IMPLEM && !str_cmp(arg2, "create"))
        {
                if (command)
                {
                        send_to_char("That command already exists!\n\r", ch);
                        return;
                }
                CREATE(command, CMDTYPE, 1);
                command->name = str_dup(arg1);
                command->level = get_trust(ch);
                if (*argument)
                        one_argument(argument, arg2);
                else
                        snprintf(arg2, MSL, "do_%s", arg1);
                command->do_fun = skill_function(arg2);
                command->fun_name = str_dup(arg2);
                add_command(command);
                send_to_char("Command added.\n\r", ch);
                if (command->do_fun == skill_notfound)
                        ch_printf(ch,
                                  "Code %s not found.  Set to no code.\n\r",
                                  arg2);
                return;
        }

        if (!command)
        {
                send_to_char("Command not found.\n\r", ch);
                return;
        }
        else if (command->level > get_trust(ch))
        {
                send_to_char("You cannot touch this command.\n\r", ch);
                return;
        }

        if (arg2[0] == '\0' || !str_cmp(arg2, "show"))
        {
                ch_printf(ch,
                          "Command:  %s\n\rLevel:    %d\n\rPosition: %s\n\r"
                          "Log:      %s\n\rCode:     %s\n\rOOC:      %s\n\r"
                          "Held:     %s\n\rFull Name:%s\n\rFlags:    %s\n\r", 
						  command->name, command->level, npc_position[command->position],
                          log_flag[command->log], command->fun_name, true_false[!!IS_SET(command->flags, CMD_OOC)],
                          true_false[!!IS_SET(command->flags, CMD_HELD)], true_false[!!IS_SET(command->flags, CMD_FULLNAME)],
						  flag_string(command->perm_flags, command_flags));
                if (command->userec.num_uses)
                        send_timer(&command->userec, ch);
                return;
        }

        if (get_trust(ch) <= LEVEL_SUB_IMPLEM)
        {
                do_cedit(ch, "");
                return;
        }

        if (get_trust(ch) <= LEVEL_SUB_IMPLEM)
        {
                do_cedit(ch, "");
                return;
        }

        if (!str_cmp(arg2, "raise"))
        {
                CMDTYPE  *tmp, *tmp_next;
                int       hash = command->name[0] % 126;

                if ((tmp = command_hash[hash]) == command)
                {
                        send_to_char
                                ("That command is already at the top.\n\r",
                                 ch);
                        return;
                }
                if (tmp->next == command)
                {
                        command_hash[hash] = command;
                        tmp_next = tmp->next;
                        tmp->next = command->next;
                        command->next = tmp;
                        ch_printf(ch, "Moved %s above %s.\n\r", command->name,
                                  command->next->name);
                        return;
                }
                for (; tmp; tmp = tmp->next)
                {
                        tmp_next = tmp->next;
                        if (tmp_next->next == command)
                        {
                                tmp->next = command;
                                tmp_next->next = command->next;
                                command->next = tmp_next;
                                ch_printf(ch, "Moved %s above %s.\n\r",
                                          command->name, command->next->name);
                                return;
                        }
                }
                send_to_char("ERROR -- Not Found!\n\r", ch);
                return;
        }
        if (!str_cmp(arg2, "lower"))
        {
                CMDTYPE  *tmp, *tmp_next;
                int       hash = command->name[0] % 126;

                if (command->next == NULL)
                {
                        send_to_char
                                ("That command is already at the bottom.\n\r",
                                 ch);
                        return;
                }
                tmp = command_hash[hash];
                if (tmp == command)
                {
                        tmp_next = tmp->next;
                        command_hash[hash] = command->next;
                        command->next = tmp_next->next;
                        tmp_next->next = command;

                        ch_printf(ch, "Moved %s below %s.\n\r", command->name,
                                  tmp_next->name);
                        return;
                }
                for (; tmp; tmp = tmp->next)
                {
                        if (tmp->next == command)
                        {
                                tmp_next = command->next;
                                tmp->next = tmp_next;
                                command->next = tmp_next->next;
                                tmp_next->next = command;

                                ch_printf(ch, "Moved %s below %s.\n\r",
                                          command->name, tmp_next->name);
                                return;
                        }
                }
                send_to_char("ERROR -- Not Found!\n\r", ch);
                return;
        }
        if (!str_cmp(arg2, "list"))
        {
                CMDTYPE  *tmp;
                int       hash = command->name[0] % 126;

                pager_printf(ch, "Priority placement for [%s]:\n\r",
                             command->name);
                for (tmp = command_hash[hash]; tmp; tmp = tmp->next)
                {
                        if (tmp == command)
                                set_pager_color(AT_GREEN, ch);
                        else
                                set_pager_color(AT_PLAIN, ch);
                        pager_printf(ch, "  %s\n\r", tmp->name);
                }
                return;
        }
        if (!str_cmp(arg2, "delete"))
        {
                unlink_command(command);
                free_command(command);
                send_to_char("Deleted.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "code"))
        {
                DO_FUN   *fun = skill_function(argument);

                if (fun == skill_notfound)
                {
                        send_to_char("Code not found.\n\r", ch);
                        return;
                }
                command->do_fun = fun;
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "level"))
        {
                int       level = atoi(argument);

                if (level < 0 || level > get_trust(ch))
                {
                        send_to_char("Level out of range.\n\r", ch);
                        return;
                }
                command->level = level;
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "log"))
        {
                int       ilog = atoi(argument);

                if (ilog < 0 || ilog > LOG_COMM)
                {
                        send_to_char("Log out of range.\n\r", ch);
                        return;
                }
                command->log = ilog;
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "position"))
        {
                int       position = atoi(argument);

                if (position < 0 || position > POS_DRAG)
                {
                        send_to_char("Position out of range.\n\r", ch);
                        return;
                }
                command->position = position;
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "ooc"))
        {
                int       ooc = atoi(argument);

                if (ooc < 0 || ooc > 1)
                {
                        send_to_char
                                ("Improper format. Format: cedit <command> ooc <1 for ooc/0 for ic>\n\r",
                                 ch);
                        return;
                }

                if (ooc)
                {
                        SET_BIT(command->flags, CMD_OOC);
                }
                else
                {
                        REMOVE_BIT(command->flags, CMD_OOC);
                }

                send_to_char("Done.\n\r", ch);
                return;
        }
        if (!str_cmp(arg2, "fullname"))
        {
			TOGGLE_BIT(command->flags, CMD_FULLNAME);
			send_to_char("Done.\n\r", ch);
			return;
        }
        if (!str_cmp(arg2, "held"))
        {
                int       held = atoi(argument);

                if (held < 0 || held > 1)
                {
                        send_to_char
                                ("Improper format. Format: cedit <command> held <1 for held/0 for non-held>\n\r",
                                 ch);
                        return;
                }
                if (held)
                {
                        SET_BIT(command->flags, CMD_HELD);
                }
                else
                {
                        REMOVE_BIT(command->flags, CMD_HELD);
                }
                send_to_char("Done.\n\r", ch);
                return;
        }
        if (!str_cmp(arg2, "flags"))
        {
                char      arg3[MSL];
                sh_int    tempnum;

                if (!argument || argument[0] == '\0')
                {
                        send_to_char
                                ("Usage: cedit <command> flags <flag> [flag]...\n\r",
                                 ch);
                        send_to_char
                                ("Valid flags are: admin, builder, highbuilder, highenforcer, enforcer, quest\n\r",
                                 ch);
                        send_to_char("                 coder, all, owner\n\r",
                                     ch);
                        return;
                }

                while (argument[0] != '\0')
                {
                        argument = one_argument(argument, arg3);
                        tempnum = get_commandflag(arg3);

                        if (tempnum < 0 || tempnum > 31)
                        {
                                ch_printf(ch, "Unknown flag: %s\n\r", arg3);
                                return;
                        }
                        TOGGLE_BIT(command->perm_flags, 1 << tempnum);
                }
                send_to_char("Done.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "test"))
        {
                if (check_command(ch, command))
                        send_to_char
                                ("You are allowed to use that command\n\r",
                                 ch);
                else
                        send_to_char
                                ("You are NOT allowed to use that command\n\r",
                                 ch);
                return;
        }

        if (!str_cmp(arg2, "name"))
        {
                bool      relocate;

                one_argument(argument, arg1);
                if (arg1[0] == '\0')
                {
                        send_to_char("Cannot clear name field!\n\r", ch);
                        return;
                }
                if (arg1[0] != command->name[0])
                {
                        unlink_command(command);
                        relocate = TRUE;
                }
                else
                        relocate = FALSE;
                if (command->name)
                        DISPOSE(command->name);
                command->name = str_dup(arg1);
                if (relocate)
                        add_command(command);
                send_to_char("Done.\n\r", ch);
                return;
        }

        /*
         * display usage message 
         */
        do_cedit(ch, "");
}

CMDF do_feed(CHAR_DATA * ch, char *argument)
{
        char      name[MAX_STRING_LENGTH];
        CHAR_DATA *victim;
        int       amount, condition;

        one_argument(argument, name);
        if (name[0] == '\0')
        {
                send_to_char("Usage: fixchar <playername>\n\r", ch);
                return;
        }
        victim = get_char_world(ch, name);
        if (!victim || IS_NPC(victim))
        {
                send_to_char("They're not here.\n\r", ch);
                return;
        }


        condition = victim->pcdata->condition[COND_FULL];
        amount = 48 - condition;

        gain_condition(victim, COND_FULL, amount);

        condition = victim->pcdata->condition[COND_THIRST];
        amount = 48 - condition;

        gain_condition(victim, COND_THIRST, amount);
        send_to_char("You've been fed by the gods!\n\r", victim);
        send_to_char("Done.\n\r", ch);

}

void comment_add_comment(CHAR_DATA * from, ACCOUNT_DATA * victim, char * subject, char * text);
CMDF do_reward(CHAR_DATA * ch, char *argument)
{
        char      name[MAX_STRING_LENGTH];
        char      buf[MAX_STRING_LENGTH];
		int       increase;
        CHAR_DATA *victim;

        argument = one_argument(argument, name);
        if (name[0] == '\0')
        {
                send_to_char("Usage: reward <playername> [<increase amt>]\n\r", ch);
                return;
        }
		increase = 1;
		if (argument[0] != '\0' && is_number(argument))
			increase = atoi(argument);

        victim = get_char_world(ch, name);
        if (!victim || !victim->pcdata)
        {
                send_to_char("They're not here.\n\r", ch);
                return;
        }

        /*if (victim->pcdata->account == ch->pcdata->account)
        {
                send_to_char("You cannot reward yourself!\n\r", ch);
                return;
        }
	*/
        if (victim->pcdata->account)
        {
                victim->pcdata->account->rppoints += increase;
                victim->pcdata->account->rpcurrent += increase;
        }
        else
                victim->pcdata->rp += increase;

        if (str_cmp(argument, "silent"))
                send_to_char
                        ("You've been rewarded by the gods for your role-playing skill!\n\r",
                         victim);
        snprintf(buf, MSL,"[AUTO COMMENT] I gave %s a reward of %d points\n\r",victim->name, increase);
		comment_add_comment(ch,victim->pcdata->account, "Been rewarded for role-playing", buf);
        save_account(victim->pcdata->account);
        send_to_char("Done.\n\r", ch);

}

CMDF do_punish(CHAR_DATA * ch, char *argument)
{
        char      name[MAX_STRING_LENGTH];
        CHAR_DATA *victim;

        one_argument(argument, name);
        if (name[0] == '\0')
        {
                send_to_char("Usage: punish <playername>\n\r", ch);
                return;
        }
        victim = get_char_world(ch, name);
        if (!victim || !victim->pcdata)
        {
                send_to_char("They're not here.\n\r", ch);
                return;
        }


#ifndef ACCOUNT
        victim->pcdata->rp = victim->pcdata->rp--;
#else
        if (victim->pcdata->account)
        {
                victim->pcdata->account->rppoints--;
                victim->pcdata->account->rpcurrent--;
        }
        else
                victim->pcdata->rp--;
#endif
        send_to_char
                ("You've been punished by the gods for your poor role-playing skill!\n\r",
                 victim);
        save_account(victim->pcdata->account);
        send_to_char("Done.\n\r", ch);
}

CMDF do_freevnums(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_STRING_LENGTH];
        char      arg2[MAX_STRING_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        AREA_DATA *pArea;
        bool      a_conflict;
        int       low_v = 0, high_v = 0, count = 0;
        int       l = 0, h = 0, curr = 0, total_c = 0, total_r = 0;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0')
        {
                send_to_char("Syntax: FREEVNUMS [low vnum] [high vnum]\n\r",
                             ch);
                return;
        }
        else
        {
                low_v = atoi(arg1);
                high_v = atoi(arg2);
        }

        if (low_v < 1 || low_v > MAX_VNUMS)
        {
                send_to_char("Invalid argument for bottom of range.\n\r", ch);
                return;
        }

        if (high_v < 1 || high_v > MAX_VNUMS)
        {
                send_to_char("Invalid argument for top of range.\n\r", ch);
                return;
        }

        if (high_v <= low_v)
        {
                send_to_char
                        ("Bottom of range must be below top of range.\n\r",
                         ch);
                return;
        }

        if ((high_v - low_v) > 1000000)
        {
                send_to_char
                        ("Sorry, You can only check up to 1000000 rooms at a time!\n\r",
                         ch);
                return;
        }

        snprintf(buf, MSL,
                 "&zFree vnum ranges in the &C%d &zto &C%d &zRANGE: &C------ \n\r\n\r",
                 low_v, high_v);
        send_to_char(buf, ch);

        /*
         * Check the range room-by-room. This is going to SUCK processer time... 
         */
        l = low_v;
        h = low_v;
        for (curr = low_v; curr <= high_v; curr++)
        {
                a_conflict = FALSE;
                for (pArea = first_asort; pArea; pArea = pArea->next_sort)
                {
                        if (IS_SET(pArea->status, AREA_DELETED))
                                continue;
                        if ((curr >= pArea->low_r_vnum
                             && curr <= pArea->hi_r_vnum)
                            || (curr >= pArea->low_m_vnum
                                && curr <= pArea->hi_m_vnum)
                            || (curr >= pArea->low_o_vnum
                                && curr <= pArea->hi_o_vnum))
                        {
                                a_conflict = TRUE;
                        }
                }
                if (a_conflict != TRUE)
                {
                        for (pArea = first_bsort; pArea;
                             pArea = pArea->next_sort)
                        {
                                if (IS_SET(pArea->status, AREA_DELETED))
                                        continue;
                                if ((curr >= pArea->low_r_vnum
                                     && curr <= pArea->hi_r_vnum)
                                    || (curr >= pArea->low_m_vnum
                                        && curr <= pArea->hi_m_vnum)
                                    || (curr >= pArea->low_o_vnum
                                        && curr <= pArea->hi_o_vnum))
                                {
                                        a_conflict = TRUE;
                                }
                        }
                }
                if (a_conflict || curr == high_v)
                {
                        count = curr - l - 1;
                        h = count + l;
                        if (curr == high_v)
                                h++;
                        if (count > 1)
                        {
                                snprintf(buf, MSL,
                                         "&BVNums: &C%7d &B- &C%7d   &B|   Count: &C%8d&B\n\r",
                                         l, h, count);
                                send_to_char(buf, ch);
                                total_c++;
                                total_r += count;
                        }
                        l = curr + 1;
                        h = curr + 1;
                }
        }
        snprintf(buf, MSL,
                 "\n\r&zThere are &C%d &zfree rooms in &C%d &zset(s) of vnums.\n\r",
                 total_r, total_c);
        send_to_char(buf, ch);
        return;
}

CMDF do_compute(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_STRING_LENGTH];
        char      arg2[MAX_STRING_LENGTH];
        char      arg3[MAX_STRING_LENGTH];
        float     a = 0, b = 0, c = 0;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);

        if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
        {
                send_to_char
                        ("Syntax: COMPUTE [first number] [expression] [second number]\n\r",
                         ch);
                send_to_char("Valid Expressions: + - * \\\n\r", ch);
                return;
        }
        else
        {
                a = atoi(arg1);
                b = atoi(arg3);
        }

        if (!str_cmp(arg2, "+") || !str_cmp(arg2, "-") || !str_cmp(arg2, "\\")
            || !str_cmp(arg2, "/") || !str_cmp(arg2, "*"))
        {
                if (a < -999999 || a > 999999 || b < -999999 || b > 999999)
                {
                        send_to_char
                                ("Sorry, One of your numbers is too large.\n\r",
                                 ch);
                        return;
                }
        }
        else
        {
                send_to_char("&RInvalid expression for formula!\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "+"))
                c = a + b;
        if (!str_cmp(arg2, "-"))
                c = a - b;
        if (!str_cmp(arg2, "*"))
                c = a * b;
        if (!str_cmp(arg2, "\\"))
                c = a / b;
        if (!str_cmp(arg2, "/"))
                c = a / b;

        ch_printf(ch, "&zComputed Answer: &C%-0.2f\n\r", c);
        return;
}

CMDF do_cycle(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        int       start, stop, x;

        if (argument[0] == '\0')
        {
                send_to_char
                        ("Syntax: cycle <low> <high> <command string>  \n\r",
                         ch);
                return;
        }
        argument = one_argument(argument, arg);

        if ((start = atoi(arg)) < 0)
        {
                send_to_char("You must start on a zero or higher.\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg);

        if ((stop = atoi(arg)) <= start)
        {
                ch_printf(ch,
                          "You must stop at a point higher than you've started. %d is %s than %d.\n\r",
                          stop, stop == start ? "equal to" : "less than",
                          start);
                return;
        }

        for (x = start; x <= stop; x++)
                interpret(ch, number_sign(argument, x));
}

/* This is only required if you do not already have it, you need it for both loop and cycle */
char     *number_sign(char *txt, int num)
{
        static char newstring[MAX_INPUT_LENGTH];
        int       i;

        if (!txt)
                return "";

        newstring[0] = '\0';
        for (i = 0; txt[i] != '\0'; i++)
        {
                if (txt[i] == '#')
                        mudstrlcat(newstring, itoa(num), MIL);
                else
                        sprintf(newstring, "%s%c", newstring, txt[i]);
/*      add_letter(newstring, txt[i]);*/
        }
        return newstring;
}


CMDF do_fdcheck(CHAR_DATA * ch, char *argument)
{
        struct stat fs;
        int       i, j = 0;

        argument = NULL;
        send_to_char("FD's in use:\n\r", ch);
        for (i = 0; i < 256; ++i)
                if (!fstat(i, &fs))
                {
                        ch_printf(ch, "%03d ", i);
                        if (!(++j % 15))
                                send_to_char("\n\r", ch);
                }
        if (j % 15)
                send_to_char("\n\r", ch);
        ch_printf(ch, "%d descriptors in use.\n\r", j);
        return;
}

CMDF do_numsize(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        ch_printf(ch, "Size of bool:               %d\n\r", sizeof(bool));
        ch_printf(ch, "Size of sh_int:             %d\n\r", sizeof(sh_int));
        ch_printf(ch, "Size of int:                %d\n\r", sizeof(int));
        ch_printf(ch, "Size of long int:           %d\n\r", sizeof(long int));
        /*
         * Gavin - ISO C90 does not support `long long'
         * ch_printf(ch, "Size of ULL:                %d\n\r", sizeof( unsigned long long int ));
         */
        ch_printf(ch, "Size of char:               %d\n\r", sizeof(char));
		ch_printf(ch, "Size of unsigned char:      %d\n\r", sizeof(unsigned char));
        ch_printf(ch, "Size of CHAR_DATA:          %d\n\r", sizeof(CHAR_DATA));
        ch_printf(ch, "Size of INSTALLATION_DATA:  %d\n\r", sizeof(INSTALLATION_DATA));
        ch_printf(ch, "Size of PC_DATA:            %d\n\r", sizeof(PC_DATA));
        ch_printf(ch, "Size of GOD_DATA:           %d\n\r", sizeof(GOD_DATA));
}

/*
 * Command to rename a player - Brought to SWR (and fixed) by Gavin
 */

CMDF do_pcrename(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      newname[MAX_STRING_LENGTH];
        char      oldname[MAX_STRING_LENGTH];
        char      backname[MAX_STRING_LENGTH];
        char      buf[MAX_STRING_LENGTH];
        NOTE_DATA *pnote;
        BOARD_DATA *tboard;

        argument = one_argument(argument, arg1);
        one_argument(argument, arg2);
        smash_tilde(arg2);


        if (IS_NPC(ch))
                return;

        if (arg1[0] == '\0' || arg2[0] == '\0')
        {
                send_to_char("Syntax: rename <victim> <new name>\n\r", ch);
                return;
        }

        if (!check_parse_name(arg2))
        {
                send_to_char("Illegal name.\n\r", ch);
                return;
        }

        /*
         * Just a security precaution so you don't rename someone you don't mean 
         * * too --Shaddai
         */
        if ((victim = get_char_room(ch, arg1)) == NULL)
        {
                send_to_char("That person is not in the room.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("You can't rename NPC's.\n\r", ch);
                return;
        }

        if (get_trust(ch) < get_trust(victim))
        {
                send_to_char("I don't think they would like that!\n\r", ch);
                return;
        }

        snprintf(newname, 255, "%s%c/%s", PLAYER_DIR, tolower(arg2[0]),
                 capitalize(arg2));
        snprintf(oldname, 255, "%s%c/%s", PLAYER_DIR,
                 tolower(victim->name[0]), capitalize(victim->name));
        snprintf(backname, 255, "%s%c/%s", BACKUP_DIR,
                 tolower(victim->name[0]), capitalize(victim->name));

        if (access(newname, F_OK) == 0)
        {
                send_to_char("That name already exists.\n\r", ch);
                return;
        }

        /*
         * Have to remove the old god entry in the directories 
         */
        if (IS_IMMORTAL(victim))
        {
                char      godname[MAX_STRING_LENGTH];

                snprintf(godname, 255, "%s%s", GOD_DIR,
                         capitalize(victim->name));
                remove(godname);
        }

        /*
         * Remember to change the names of the areas 
         */
        if (victim->pcdata->area)
        {
                char      filename[MAX_STRING_LENGTH];
                char      newfilename[MAX_STRING_LENGTH];

                snprintf(filename, 255, "%s%s.are", BUILD_DIR, victim->name);
                snprintf(newfilename, 255, "%s%s.are", BUILD_DIR,
                         capitalize(arg2));
                rename(filename, newfilename);
                snprintf(filename, 255, "%s%s.are.bak", BUILD_DIR,
                         victim->name);
                snprintf(newfilename, 255, "%s%s.are.bak", BUILD_DIR,
                         capitalize(arg2));
                rename(filename, newfilename);
        }

        if (remove(oldname))
        {
                snprintf(buf, MSL,
                         "Error: Couldn't delete file %s in do_rename.",
                         oldname);
                send_to_char("Couldn't delete the old file!\n\r", ch);
                log_string(oldname);
        }
        /*
         * Lets Do The Finger Files 
         */
        {
                /*
                 * Should we just delete these? or will they eventually be updated? 
                 */
                snprintf(newname, 255, "%s%c/%s.F", PLAYER_DIR,
                         tolower(victim->name[0]), capitalize(victim->name));
                remove(newname);
        }

#ifdef CHANGES_SNIPPET
        if (IS_IMMORTAL(victim))
        {
                /*
                 * original name = victim->name 
                 */
                /*
                 * new name = arg2 
                 */
                int       i = 0;

                for (i = 0; i < maxChanges; i++)
                {
                        if (!strcmp(changes_table[i].coder, victim->name))
                        {
                                STRFREE(changes_table[i].coder);
                                changes_table[i].coder =
                                        STRALLOC(capitalize(arg2));
                        }
                }
        }
#endif

        /*
         * Lets fix any ships they own 
         */
        {
                bool      changed = FALSE;
                SHIP_DATA *ship = NULL;

                for (ship = first_ship; ship; ship = ship->next)
                {
                        changed = FALSE;
                        if (!str_cmp(victim->name, ship->owner))
                        {
                                STRFREE(ship->owner);
                                ship->owner = STRALLOC(arg2);
                                changed = TRUE;
                        }
                        if (!str_cmp(victim->name, ship->pilot))
                        {
                                STRFREE(ship->pilot);
                                ship->pilot = STRALLOC(arg2);
                                changed = TRUE;
                        }
                        if (!str_cmp(victim->name, ship->copilot))
                        {
                                STRFREE(ship->copilot);
                                ship->copilot = STRALLOC(arg2);
                                changed = TRUE;
                        }
                        if (changed)
                                save_ship(ship);
                }
        }
        /*
         * Lets do clans now 
         */
        {
                if (victim->pcdata->clan)
                {
                        CLAN_DATA *clan = victim->pcdata->clan;

                        if (!str_cmp(victim->name, clan->leader))
                        {
                                STRFREE(clan->leader);
                                clan->leader = STRALLOC(arg2);
                        }
                        if (!str_cmp(victim->name, clan->number1))
                        {
                                STRFREE(clan->number1);
                                clan->number1 = STRALLOC(arg2);
                        }

                        if (!str_cmp(victim->name, clan->number2))
                        {
                                STRFREE(clan->number2);
                                clan->number2 = STRALLOC(arg2);
                        }
                        save_clan(clan);
                }
        }
        /*
         * Lets do clones now 
         */
        {
                snprintf(newname, 255, "%s%c/%s.clone", PLAYER_DIR,
                         tolower(victim->name[0]), capitalize(victim->name));
                remove(newname);
        }
        /*
         * Bounties 
         */
        {
                BOUNTY_DATA *bounty = NULL;

                for (bounty = first_disintigration; bounty;
                     bounty = bounty->next)
                {
                        if (!str_cmp(bounty->target, victim->name))
                        {
                                STRFREE(bounty->target);
                                bounty->target = STRALLOC(capitalize(arg2));
                        }
                }
                save_disintigrations();
        }

        /*
         * Lets Do board stuff too 
         */
        for (tboard = first_board; tboard; tboard = tboard->next)
        {
                bool      change = FALSE;

                for (pnote = tboard->first_note; pnote; pnote = pnote->next)
                {
                        if (!str_cmp(victim->name, pnote->sender))
                        {
                                STRFREE(pnote->sender);
                                pnote->sender = STRALLOC(capitalize(arg2));
                                change = TRUE;
                        }
                        if (!str_cmp(victim->name, pnote->to_list))
                        {
                                STRFREE(pnote->to_list);
                                pnote->to_list = STRALLOC(capitalize(arg2));
                                change = TRUE;
                        }
                        /*
                         * Update lists? 
                         */
                }
                if (change)
                        write_board(tboard);
        }

        /*
         * Lets do homes now 
         */
        {
                snprintf(newname, 255, "%s%c/%s.home", PLAYER_DIR,
                         tolower(victim->name[0]), capitalize(victim->name));
                remove(newname);
        }

        /*
         * Lets do helpfiles now 
         */
        {
                HELP_DATA *help;
                bool      found = FALSE;

                for (help = first_help; help; help = help->next)
                        if (!str_cmp(help->author, victim->name))
                        {
                                if (help->author)
                                        STRFREE(help->author);
                                help->author = STRALLOC(capitalize(arg2));
                                found = TRUE;
                        }
                if (found)
                        save_help();
        }

#ifdef ACCOUNT
        if (victim->pcdata->account)
        {
                del_from_account(victim->pcdata->account, victim);
                save_account(victim->pcdata->account);
        }
#endif

        STRFREE(victim->name);
        victim->name = STRALLOC(capitalize(arg2));
        STRFREE(victim->pcdata->full_name);
        victim->pcdata->full_name = STRALLOC(capitalize(arg2));
        remove(backname);
        /*
         * Time to save to force the affects to take place 
         */
        save_char_obj(victim);
        save_finger(victim);
        save_home(victim);
#ifdef ACCOUNT
        if (victim->pcdata->account)
        {
                add_to_account(victim->pcdata->account, victim);
                save_account(victim->pcdata->account);
        }
#endif
        /*
         * Now lets update the wizlist 
         */
        if (IS_IMMORTAL(victim))
                make_wizlist();

        send_to_char("Character was renamed.\n\r", ch);
        return;
}

/*
 * Fairly useless command to list the sexes of all who is online
 */
CMDF do_sexes(CHAR_DATA * ch, char *argument)
{
        int       count = 0;
        DESCRIPTOR_DATA *d;
        CHAR_DATA *victim;
        char      buf[MAX_STRING_LENGTH];

        argument = NULL;

        for (d = first_descriptor; d; d = d->next)
        {
                if (IS_PLAYING(d) || d->connected == CON_EDITING)
                {
                        if (count == 0)
                                ch_printf(ch,
                                          "\n\r&B[&WName       &B] &B[&WSex            &B]&W\n\r");
                        victim = d->character;
                        mudstrlcpy(buf, "ERROR", MSL);
                        if (victim->sex == SEX_MALE)
                        {
                                mudstrlcpy(buf, "male       ", MSL);
                        }
                        else if (victim->sex == SEX_FEMALE)
                        {
                                mudstrlcpy(buf, "female     ", MSL);
                        }
                        else if (victim->sex == SEX_NEUTRAL)
                        {
                                mudstrlcpy(buf, "neutral    ", MSL);
                        }
                        ch_printf(ch, "&B[&W%-11s&B] &B[&W%-15s&B]&W\n\r",
                                  victim->name, buf);
                        count++;
                }
        }
        return;
}

CMDF do_listships(CHAR_DATA * ch, char *argument)
{
        SHIP_DATA *ship;
        SPACE_DATA *starsystem;
        DOCK_DATA *dock;
        int       count = 0;
        CHAR_DATA *victim;
        char      arg[MAX_INPUT_LENGTH];
        CLAN_DATA *clan;

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Syntax: listship <player>\n\r", ch);
                send_to_char("Syntax: listship clans\n\r", ch);
                send_to_char("Syntax: listship count\n\r", ch);
                send_to_char("Syntax: listship starsystems\n\r", ch);
                send_to_char("Syntax: listship docks\n\r", ch);
                return;
        }

        if (!str_cmp(argument, "clans"))
        {
                for (clan = first_clan; clan; clan = clan->next)
                {
                        int       clancount = 0;

                        for (ship = first_ship; ship; ship = ship->next)
                                if (!str_cmp(ship->owner, clan->name))
                                        clancount++;
                        ch_printf(ch, "&B[&w%-20s&B] &BS&zhips: &w%d\n\r",
                                  clan->name, clancount);
                }
                return;
        }
        if (!str_cmp(argument, "count"))
        {
                for (ship = first_ship; ship; ship = ship->next)
                        count++;
                ch_printf(ch, "&BT&wotal ships:%d\n\r", count);
                return;
        }

        if (!str_cmp(argument, "starsystems"))
        {
                for (starsystem = first_starsystem; starsystem;
                     starsystem = starsystem->next)
                {
                        count = 0;
                        for (ship = starsystem->first_ship; ship;
                             ship = ship->next_in_starsystem)
                                count++;
                        ch_printf(ch, "&B[&z%-20s&B]: %d\n\r",
                                  starsystem->name, count);
                }
                return;
        }
        if (!str_cmp(argument, "docks"))
        {
                for (dock = first_dock; dock; dock = dock->next)
                {
                        count = 0;
                        for (ship = first_ship; ship; ship = ship->next)
                                if (ship->in_room
                                    && ship->in_room->vnum == dock->vnum)
                                        count++;
                        ch_printf(ch, "&B[&z%-20s&B]: %d\n\r", dock->name,
                                  count);
                }
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        count = 0;
        ch_printf(ch,
                  "The following ships are owned by %s or his organization:\n\r",
                  victim->name);
        send_to_char("\n\r&WShip                               Location\n\r",
                     ch);

        for (ship = first_ship; ship; ship = ship->next)
        {
                if (ship->ship_class > SHIP_PLATFORM)
                        continue;
                if (IS_SET(ship->flags, MOB_SHIP))
                        continue;
                set_char_color(AT_BLUE, ch);

                if (!str_cmp(ship->owner, victim->name))
                {
                        if (ship->in_room)
                                ch_printf(ch,
                                          "&R[&WOwner&R]&B%-35s     %s [%d].\n\r",
                                          ship->name, ship->in_room->name,
                                          ship->location);
                        else if (ship->starsystem && ship->starsystem->name)
                                ch_printf(ch,
                                          "&R[&WOwner&R]&B%-35s     %s.\n\r",
                                          ship->name, ship->starsystem->name);
                }
                else if (!str_cmp(ship->pilot, victim->name))
                {
                        if (ship->in_room)
                                ch_printf(ch,
                                          "&R[&WPilot&R]&B%-35s     %s [%d].\n\r",
                                          ship->name, ship->in_room->name,
                                          ship->location);
                        else if (ship->starsystem && ship->starsystem->name)
                                ch_printf(ch,
                                          "&R[&WPilot&R]&B%-35s     %s.\n\r",
                                          ship->name, ship->starsystem->name);
                }
                else if (!str_cmp(ship->copilot, victim->name))
                {
                        if (ship->in_room)
                                ch_printf(ch,
                                          "&R[&WCo-Pilot&R]&B%-35s     %s [%d].\n\r",
                                          ship->name, ship->in_room->name,
                                          ship->location);
                        else if (ship->starsystem && ship->starsystem->name)
                                ch_printf(ch,
                                          "&R[&WCo-Pilot&R]&B%-35s     %s.\n\r",
                                          ship->name, ship->starsystem->name);
                }
                else if (victim->pcdata->clan
                         && !str_cmp(ship->owner, victim->pcdata->clan->name))
                {
                        if (ship->in_room)
                                ch_printf(ch,
                                          "&R[&WClan&R]&B%-35s     %s [%d].\n\r",
                                          ship->name, ship->in_room->name,
                                          ship->location);
                        else if (ship->starsystem && ship->starsystem->name)
                                ch_printf(ch,
                                          "&R[&WClan&R]&B%-35s     %s.\n\r",
                                          ship->name, ship->starsystem->name);
                }
                else
                        continue;

                count++;
        }

        if (!count)
        {
                send_to_char("They don't own any ships.\n\r", ch);
                return;
        }
}

CMDF do_gfighting(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        DESCRIPTOR_DATA *d;
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        bool      found = FALSE;
        int       low = 1, high = 105, count = 0;

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        if (arg1[0] != '\0')
        {
                if (arg1[0] == '\0' || arg2[0] == '\0')
                {
                        send_to_pager
                                ("\n\r&wSyntax:  gfighting | gfighting <low> <high> | gfighting <low> <high> mobs\n\r",
                                 ch);
                        return;
                }
                low = atoi(arg1);
                high = atoi(arg2);
        }
        if (low < 1 || high < low || low > high || high > 105)
        {
                send_to_pager("&wInvalid level range.\n\r", ch);
                return;
        }

        send_to_char("&cGlobal conflicts:\n\r", ch);
        for (d = first_descriptor; d; d = d->next)
        {
                /*
                 * How can someone be fighting and editing? 
                 */
                if ((IS_PLAYING(d) || d->connected == CON_EDITING)
                    && (victim = d->character) != NULL && !IS_NPC(victim)
                    && victim->in_room && can_see(ch, victim)
                    && victim->fighting && victim->top_level >= low
                    && victim->top_level <= high)
                {
                        found = TRUE;
                        pager_printf(ch,
                                     "&w%-12.12s &C|%2d &wvs &C%2d| &w%-16.16s [%5d]  &c%-20.20s [%5d]\n\r",
                                     victim->name, victim->top_level,
                                     victim->fighting->who->top_level,
                                     IS_NPC(victim->fighting->who) ? victim->
                                     fighting->who->short_descr : victim->
                                     fighting->who->name,
                                     IS_NPC(victim->fighting->who) ? victim->
                                     fighting->who->pIndexData->vnum : 0,
                                     victim->in_room->area->name,
                                     victim->in_room ==
                                     NULL ? 0 : victim->in_room->vnum);
                        count++;
                }
        }
        pager_printf(ch, "&c%d conflicts located.\n\r", count);
        return;
}

CMDF do_bones(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *vch;

        argument = NULL;
        send_to_pager("&BB&zone status of players:\n\r", ch);
        set_pager_color(AT_WHITE, ch);
        for (vch = first_char; vch; vch = vch->next)
        {
                /*
                 * Apparently the char list has npcs too 
                 */
                if (IS_NPC(vch))
                        continue;
                pager_printf(ch, "%s: %-60s\n\r", vch->name,
                             vch->bodyparts ? flag_string(vch->bodyparts,
                                                          body_parts) :
                             "none");
        }
        return;
}

int       get_bodypart(char *flag);
CMDF do_break(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        char      arg[MIL];
        int       flag = -1;

        if (argument[0] == '\0')
        {
                send_to_char("&BS&zyntax: break <player> <bone> [<bone>...]",
                             ch);
                return;
        }

        argument = one_argument(argument, arg);

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("&BN&zo such person in the game.\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("&BN&zo bones specified.", ch);
                return;
        }

        argument = one_argument(argument, arg);

        while (arg[0] != '\0')
        {
                flag = get_bodypart(arg);
                if (flag == -1)
                {
                        send_to_char("&BN&zo such bone.\n\r", ch);
                }
                else
                {
                        TOGGLE_BIT(victim->bodyparts, 1 << flag);
                        if (IS_SET(victim->bodyparts, 1 << flag))
                                send_to_char("&BB&zone broken.\n\r", ch);
                        else
                                send_to_char("&BB&zone set.\n\r", ch);

                }
                argument = one_argument(argument, arg);
        }
}


CMDF do_forsaken(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;

        if (argument[0] == '\0')
        {
                send_to_char("&BS&zyntax: forsaken <player>", ch);
                return;
        }

        if ((victim = get_char_world(ch, argument)) == NULL)
        {
                send_to_char("&BN&zo such person in the game.\n\r", ch);
                return;
        }

        if (IS_SET(victim->act, PLR_FORSAKEN))
        {
                send_to_char("They have been forgiven.\n\r", ch);
                send_to_char("You have been forgiven.\n\r", victim);
        }
        else
        {
                send_to_char("They have sinned.\n\r", ch);
                send_to_char("You have sinned and have been punished.\n\r",
                             victim);
        }
        TOGGLE_BIT(victim->act, PLR_FORSAKEN);
        return;
}

CMDF do_updatepships(CHAR_DATA * ch, char *argument)
{
        char      buf[MSL];
        SHIP_DATA *ship;

        argument = NULL;
        for (ship = first_ship; ship; ship = ship->next)
        {
                if (!ship->owner || ship->owner[0] == '\0')
                        continue;
                snprintf(buf, MSL, "%s%c/%s", PLAYER_DIR,
                         tolower(ship->owner[0]), capitalize(ship->owner));

                ch_printf(ch, "Checking ship \"%s\", owner: %s\n\r",
                          ship->name, ship->owner);
                ch_printf(ch, "buf: %s\n\r", buf);

                if (access(buf, F_OK) != 0 && (get_clan(ship->owner) == NULL))
                {
                        ch_printf(ch,
                                  "Player %s doesn't exist for ship %s, wiping owners\n\r",
                                  ship->owner, ship->name);
                        STRFREE(ship->owner);
                        ship->owner = STRALLOC("");
                        STRFREE(ship->pilot);
                        ship->pilot = STRALLOC("");
                        STRFREE(ship->copilot);
                        ship->copilot = STRALLOC("");
                        save_ship(ship);
                }
        }
}

CMDF do_setrecall(CHAR_DATA * ch, char *argument)
{
        int       vnum = 0;

        if (IS_NPC(ch))
                return;

        if (!argument || argument[0] == '\0')
        {
                if (!ch->in_room)
                {
                        send_to_char
                                ("Apparently you are not in a room, please contact someone.\n\r",
                                 ch);
                        return;
                }

                vnum = ch->in_room->vnum;
        }

        else if (!is_number(argument))
        {
                send_to_char("&RSyntax: setrecall <vnum>\n\r", ch);
                return;
        }
        else
                vnum = atoi(argument);

        ch->pcdata->recall = vnum;
        ch_printf(ch, "&GRecall vnum set to %d.\n\r", vnum);
}

CMDF do_doas(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim = NULL;
        DESCRIPTOR_DATA *orig = NULL;
        char      arg[MIL];

        if (IS_NPC(ch))
                return;

        if (!ch->desc)
                return;
        argument = one_argument(argument, arg);

        if (NULLSTR(argument) || NULLSTR(arg))
        {
                send_to_char("&RSyntax:&c doas <victim> <command>", ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL)
        {
                send_to_char("&BN&zo such person in the game.\n\r", ch);
                return;
        }

        ch_printf(ch, "You switch bodies with %s momentarily\n\r",
                  victim->name);
        orig = victim->desc;

        victim->desc = ch->desc;
        ch->desc = NULL;

        interpret(victim, argument);

        ch->desc = victim->desc;
        victim->desc = orig;
        send_to_char("You revert back to your own body\n\r", ch);
}

void do_pwipe(CHAR_DATA * ch, char *argument)
{
        DIR      *dp;
        struct dirent *dentry;
        char      directory_name[100], buf[MSL];
        sh_int    alpha_loop;

        ch = NULL;
        argument = NULL;
        nice(20);
        for (alpha_loop = 0; alpha_loop <= 25; alpha_loop++)
        {
                snprintf(directory_name, MSL, "%s%c", PLAYER_DIR,
                         'a' + alpha_loop);
                dp = opendir(directory_name);
                dentry = readdir(dp);
                while (dentry)
                {
                        if (dentry->d_name[0] != '.')
                        {
                                snprintf(buf, MSL, "%s%c/%s", PLAYER_DIR,
                                         'a' + alpha_loop, dentry->d_name);
                                unlink(buf);
                        }
                        dentry = readdir(dp);
                }
                closedir(dp);
        }
        log_string("Pwipe complete.\n\r");
        return;
}

/*
 * The "watch" facility allows imms to specify the name of a player or
 * the name of a site to be watched. It is like "logging" a player except
 * the results are written to a file in the "watch" directory named with
 * the same name as the imm. The idea is to allow lower level imms to 
 * watch players or sites without having to have access to the log files.
 */
CMDF do_watch(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        WATCH_DATA *pw;

        if (IS_NPC(ch))
                return;

        argument = one_argument(argument, arg);
        set_pager_color(AT_IMMORT, ch);

        if (arg[0] == '\0' || !str_cmp(arg, "help"))
        {
                send_to_pager("Syntax Examples:\n\r", ch);
                /*
                 * Only IMP+ can see all the watches. The rest can just see their own.
                 */
                if (get_trust(ch) >= LEVEL_IMPLEMENTOR)
                        send_to_pager
                                ("   watch show all          show all watches\n\r",
                                 ch);
                send_to_pager
                        ("   watch show              show all my watches\n\r"
                         "   watch size              show the size of my watch file\n\r"
                         "   watch player joe        add a new player watch\n\r"
#ifdef ACCOUNT
                         "   watch account joe       add a new account watch\n\r"
#endif
                         "   watch site 2.3.123      add a new site watch\n\r"
                         "   watch command make      add a new command watch\n\r"
                         "   watch site 2.3.12       matches 2.3.12x\n\r"
                         "   watch site 2.3.12.      matches 2.3.12.x\n\r"
                         "   watch delete n          delete my nth watch\n\r"
                         "   watch print 500         print watch file starting at line 500\n\r"
                         "   watch print 500 1000    print 1000 lines starting at line 500\n\r"
                         "   watch clear             clear my watch file\n\r",
                         ch);
                return;
        }

        set_pager_color(AT_PLAIN, ch);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);
        if (!str_cmp(arg, "save"))
        {
                save_watchlist();
                send_to_pager("Saved.\n\r", ch);
                return;
        }
/*
 * Clear watch file
 */
        if (!str_cmp(arg, "clear"))
        {
                char      fname[MAX_INPUT_LENGTH];

                sprintf(fname, "%s%s", WATCH_DIR, strlower(ch->name));
                if (0 == remove(fname))
                {
                        send_to_pager
                                ("Ok. Your watch file has been cleared.\n\r",
                                 ch);
                        return;
                }
                send_to_pager("You have no valid watch file to clear.\n\r",
                              ch);
                return;
        }

/*
 * Display size of watch file
 */
        if (!str_cmp(arg, "size"))
        {
                FILE     *fp;
                char      fname[MAX_INPUT_LENGTH], s[MAX_STRING_LENGTH];
                int       rec_count = 0;

                sprintf(fname, "%s%s", WATCH_DIR, strlower(ch->name));

                if (!(fp = fopen(fname, "r")))
                {
                        send_to_pager
                                ("You have no watch file. Perhaps you cleared it?\n\r",
                                 ch);
                        return;
                }

                fgets(s, MAX_STRING_LENGTH, fp);
                while (!feof(fp))
                {
                        rec_count++;
                        fgets(s, MAX_STRING_LENGTH, fp);
                }
                pager_printf(ch, "You have %d lines in your watch file.\n\r",
                             rec_count);
                fclose(fp);
                return;
        }

/*
 * Print watch file
 */
        if (!str_cmp(arg, "print"))
        {
                FILE     *fp;
                char      fname[MAX_INPUT_LENGTH], s[MAX_STRING_LENGTH];
                const int MAX_DISPLAY_LINES = 1000;
                int       start, limit, disp_count = 0, rec_count = 0;

                if (arg2[0] == '\0')
                {
                        send_to_pager
                                ("Sorry. You must specify a starting line number.\n\r",
                                 ch);
                        return;
                }

                start = atoi(arg2);
                limit = (arg3[0] == '\0') ? MAX_DISPLAY_LINES : atoi(arg3);
                limit = UMIN(limit, MAX_DISPLAY_LINES);

                sprintf(fname, "%s%s", WATCH_DIR, strlower(ch->name));
                if (!(fp = fopen(fname, "r")))
                        return;
                fgets(s, MAX_STRING_LENGTH, fp);

                while ((disp_count < limit) && (!feof(fp)))
                {
                        if (++rec_count >= start)
                        {
                                send_to_pager(s, ch);
                                disp_count++;
                        }
                        fgets(s, MAX_STRING_LENGTH, fp);
                }
                send_to_pager("\n\r", ch);
                if (disp_count >= MAX_DISPLAY_LINES)
                        send_to_pager
                                ("Maximum display lines exceeded. List is terminated.\n\r"
                                 "Type 'help watch' to see how to print the rest of the list.\n\r"
                                 "\n\r"
                                 "Your watch file is large. Perhaps you should clear it?\n\r",
                                 ch);

                fclose(fp);
                return;
        }

/*
 * Display all watches
 * Only IMP+ can see all the watches. The rest can just see their own.
 */
        if (get_trust(ch) >= LEVEL_IMPLEMENTOR
            && !str_cmp(arg, "show") && !str_cmp(arg2, "all"))
        {
                pager_printf(ch, "%-12s %-14s %-15s\n\r",
                             "Imm Name", "Player/Command/Account", "Type");
                if (first_watch)
                {
                        for (pw = first_watch; pw; pw = pw->next)
                        {
                                if (get_trust(ch) >= pw->imm_level)
                                {
                                        char     *name = NULL;
                                        char      type[250];

                                        if (pw->target_name)
                                        {
                                                strcpy(type, "Player");
                                                name = pw->target_name;
                                        }
                                        else if (pw->player_account)
                                        {
                                                strcpy(type, "Account");
                                                name = pw->player_account;
                                        }
                                        else if (pw->player_account)
                                        {
                                                strcpy(type, "Site");
                                                name = pw->player_site;
                                        }
                                        else
                                        {
                                                bug("Invalid watch type, not player, account, nor site", 0);
                                                continue;
                                        }
                                        pager_printf(ch,
                                                     "%-12s %-14s %-15s\n\r",
                                                     pw->imm_name,
                                                     name, type);
                                }
                        }
                }
                return;
        }

/*
 * Display only those watches belonging to the requesting imm 
 */
        if (!str_cmp(arg, "show") && arg2[0] == '\0')
        {
                int       cou = 0;

                pager_printf(ch, "%-3s %-12s %-14s %-15s\n\r",
                             " ", "Imm Name",
                             "Player/Command/Account/Site", "Watch Type");
                if (first_watch)
                {
                        for (pw = first_watch; pw; pw = pw->next)
                        {
                                if (!str_cmp(ch->name, pw->imm_name))
                                {
                                        char     *name = NULL;
                                        char      type[250];

                                        if (pw->target_name)
                                        {
                                                strcpy(type, "Player");
                                                name = pw->target_name;
                                        }
                                        else if (pw->player_account)
                                        {
                                                strcpy(type, "Account");
                                                name = pw->player_account;
                                        }
                                        else if (pw->player_account)
                                        {
                                                strcpy(type, "Site");
                                                name = pw->player_site;
                                        }
                                        else
                                        {
                                                bug("Invalid watch type, not player, account, nor site", 0);
                                                continue;
                                        }
                                        pager_printf(ch,
                                                     "%3d %-12s %-14s %-15s\n\r",
                                                     ++cou, pw->imm_name,
                                                     name, type);
                                }
                        }
                }
                return;
        }

/*
 * Delete a watch belonging to the requesting imm
 */
        if (!str_cmp(arg, "delete") && isdigit(*arg2))
        {
                int       cou = 0;
                int       num;

                num = atoi(arg2);
                if (first_watch)
                        for (pw = first_watch; pw; pw = pw->next)
                                if (!str_cmp(ch->name, pw->imm_name))
                                        if (num == ++cou)
                                        {
                                                /*
                                                 * Oops someone forgot to clear up the memory --Shaddai 
                                                 */
                                                if (pw->imm_name)
                                                        DISPOSE(pw->imm_name);
                                                if (pw->player_site)
                                                        DISPOSE(pw->
                                                                player_site);
                                                if (pw->player_account)
                                                        DISPOSE(pw->
                                                                player_account);
                                                if (pw->target_name)
                                                        DISPOSE(pw->
                                                                target_name);
                                                /*
                                                 * Now we can unlink and then clear up that final
                                                 * * pointer -- Shaddai 
                                                 */
                                                UNLINK(pw, first_watch,
                                                       last_watch, next,
                                                       prev);
                                                DISPOSE(pw);
                                                save_watchlist();
                                                send_to_pager("Deleted.\n\r",
                                                              ch);
                                                return;
                                        }
                send_to_pager("Sorry. I found nothing to delete.\n\r", ch);
                return;
        }

/*
 * Watch a specific player
 */
        if (!str_cmp(arg, "player") && *arg2)
        {
                WATCH_DATA *pinsert;
                CHAR_DATA *vic;
                char      buf[MAX_INPUT_LENGTH];

                if (first_watch)    /* check for dups */
                        for (pw = first_watch; pw; pw = pw->next)
                                if (!str_cmp(ch->name, pw->imm_name)
                                    && pw->target_name
                                    && !str_cmp(arg2, pw->target_name))
                                {
                                        send_to_pager
                                                ("You are already watching that player.\n\r",
                                                 ch);
                                        return;
                                }

                CREATE(pinsert, WATCH_DATA, 1); /* create new watch */
                pinsert->imm_level = get_trust(ch);
                pinsert->imm_name = str_dup(strlower(ch->name));
                pinsert->target_name = str_dup(strlower(arg2));
                pinsert->player_site = NULL;
                pinsert->player_account = NULL;

                /*
                 * stupid get_char_world returns ptr to "samantha" when given "sam" 
                 */
                /*
                 * so I do a str_cmp to make sure it finds the right player --Gorog 
                 */

                sprintf(buf, "0.%s", arg2);
                if ((vic = get_char_world(ch, buf)))    /* if vic is in game now */
                        if ((!IS_NPC(vic)) && !str_cmp(arg2, vic->name))
                                SET_BIT(vic->pcdata->flags, PCFLAG_WATCH);

                if (first_watch)    /* ins new watch if app */
                        for (pw = first_watch; pw; pw = pw->next)
                                if (strcmp(pinsert->imm_name, pw->imm_name) <
                                    0)
                                {
                                        INSERT(pinsert, pw, first_watch, next,
                                               prev);
                                        save_watchlist();
                                        send_to_pager
                                                ("Ok. That player will be watched.\n\r",
                                                 ch);
                                        return;
                                }

                LINK(pinsert, first_watch, last_watch, next, prev); /* link new watch */
                save_watchlist();
                send_to_pager("Ok. That player will be watched.\n\r", ch);
                return;
        }

        if (!str_cmp(arg, "account") && *arg2)
        {
                WATCH_DATA *pinsert;
                CHAR_DATA *vic;
                DESCRIPTOR_DATA *d;
                char      buf[MAX_INPUT_LENGTH];

                if (first_watch)    /* check for dups */
                        for (pw = first_watch; pw; pw = pw->next)
                                if (!str_cmp(ch->name, pw->imm_name)
                                    && pw->player_account
                                    && !str_cmp(arg2, pw->player_account))
                                {
                                        send_to_pager
                                                ("You are already watching that account.\n\r",
                                                 ch);
                                        return;
                                }

                CREATE(pinsert, WATCH_DATA, 1); /* create new watch */
                pinsert->imm_level = get_trust(ch);
                pinsert->imm_name = str_dup(strlower(ch->name));
                pinsert->target_name = NULL;
                pinsert->player_account = str_dup(strlower(arg2));
                pinsert->player_site = NULL;

                /*
                 * stupid get_char_world returns ptr to "samantha" when given "sam" 
                 */
                /*
                 * so I do a str_cmp to make sure it finds the right player --Gorog 
                 */

                for (d = first_descriptor; d; d = d->next)
                {
                        vic = d->character;
                        if (!vic)
                                continue;
                        if (!vic->pcdata)
                                continue;
                        if (!vic->pcdata->account)
                                continue;
                        if (!str_cmp
                            (vic->pcdata->account->name,
                             pinsert->player_account))
                                SET_BIT(vic->pcdata->flags, PCFLAG_WATCH);
                }

                if (first_watch)    /* ins new watch if app */
                        for (pw = first_watch; pw; pw = pw->next)
                                if (strcmp(pinsert->imm_name, pw->imm_name) <
                                    0)
                                {
                                        INSERT(pinsert, pw, first_watch, next,
                                               prev);
                                        save_watchlist();
                                        send_to_pager
                                                ("Ok. That account will be watched.\n\r",
                                                 ch);
                                        return;
                                }

                LINK(pinsert, first_watch, last_watch, next, prev); /* link new watch */
                save_watchlist();
                send_to_pager("Ok. That account will be watched.\n\r", ch);
                return;
        }

/*
 * Watch a specific site
 */
        if (!str_cmp(arg, "site") && *arg2)
        {
                WATCH_DATA *pinsert;
                CHAR_DATA *vic;

                if (first_watch)    /* check for dups */
                        for (pw = first_watch; pw; pw = pw->next)
                                if (!str_cmp(ch->name, pw->imm_name)
                                    && pw->player_site
                                    && !str_cmp(arg2, pw->player_site))
                                {
                                        send_to_pager
                                                ("You are already watching that site.\n\r",
                                                 ch);
                                        return;
                                }

                CREATE(pinsert, WATCH_DATA, 1); /* create new watch */
                pinsert->imm_level = get_trust(ch);
                pinsert->imm_name = str_dup(strlower(ch->name));
                pinsert->player_site = str_dup(strlower(arg2));
                pinsert->target_name = NULL;
                pinsert->player_account = NULL;

                for (vic = first_char; vic; vic = vic->next)
                        if (!IS_NPC(vic) && vic->desc && *pinsert->player_site
                            && !str_prefix(pinsert->player_site,
                                           vic->desc->host)
                            && get_trust(vic) < pinsert->imm_level)
                                SET_BIT(vic->pcdata->flags, PCFLAG_WATCH);

                if (first_watch)    /* ins new watch if app */
                        for (pw = first_watch; pw; pw = pw->next)
                                if (strcmp(pinsert->imm_name, pw->imm_name) <
                                    0)
                                {
                                        INSERT(pinsert, pw, first_watch, next,
                                               prev);
                                        save_watchlist();
                                        send_to_pager
                                                ("Ok. That site will be watched.\n\r",
                                                 ch);
                                        return;
                                }

                LINK(pinsert, first_watch, last_watch, next, prev);
                save_watchlist();
                send_to_pager("Ok. That site will be watched.\n\r", ch);
                return;
        }

/*
 * Watch a specific command - FB
 */
        if (!str_cmp(arg, "command") && *arg2)
        {
                WATCH_DATA *pinsert;
                CMDTYPE  *cmd;
                bool      found = FALSE;

                for (pw = first_watch; pw; pw = pw->next)
                {
                        if (!str_cmp(ch->name, pw->imm_name)
                            && pw->target_name
                            && !str_cmp(arg2, pw->target_name))
                        {
                                send_to_pager
                                        ("You are already watching that command.\n\r",
                                         ch);
                                return;
                        }
                }

                for (cmd = command_hash[LOWER(arg2[0]) % 126]; cmd;
                     cmd = cmd->next)
                {
                        if (!strcmp(arg2, cmd->name))
                        {
                                found = TRUE;
                                break;
                        }
                }

                if (!found)
                {
                        send_to_pager("No such command exists.\n\r", ch);
                        return;
                }
                else
                {
                        SET_BIT(cmd->flags, CMD_WATCH);
                }

                CREATE(pinsert, WATCH_DATA, 1);
                pinsert->imm_level = get_trust(ch);
                pinsert->imm_name = str_dup(strlower(ch->name));
                pinsert->player_site = NULL;
                pinsert->player_account = NULL;
                pinsert->target_name = str_dup(arg2);

                for (pw = first_watch; pw; pw = pw->next)
                {
                        if (strcmp(pinsert->imm_name, pw->imm_name) < 0)
                        {
                                INSERT(pinsert, pw, first_watch, next, prev);
                                save_watchlist();
                                send_to_pager
                                        ("Ok, That command will be watched.\n\r",
                                         ch);
                                return;
                        }
                }

                LINK(pinsert, first_watch, last_watch, next, prev);
                save_watchlist();
                send_to_pager("Ok. That site will be watched.\n\r", ch);
                return;
        }

        send_to_pager("Sorry. I can't do anything with that. "
                      "Please read the help file.\n\r", ch);
        return;
}

WATCH_DATA *first_watch;
WATCH_DATA *last_watch;

void save_watchlist(void)
{
        WATCH_DATA *pwatch;
        FILE     *fp;

        fclose(fpReserve);
        if (!(fp = fopen(SYSTEM_DIR WATCH_LIST, "w")))
        {
                bug("Save_watchlist: Cannot open " WATCH_LIST, 0);
                perror(WATCH_LIST);
                fpReserve = fopen(NULL_FILE, "r");
                return;
        }

        for (pwatch = first_watch; pwatch; pwatch = pwatch->next)
                fprintf(fp, "%d %s~%s~%s~%s~\n", pwatch->imm_level,
                        pwatch->imm_name,
                        pwatch->target_name ? pwatch->target_name : " ",
                        pwatch->player_site ? pwatch->player_site : " ",
                        pwatch->player_account ? pwatch->
                        player_account : " ");
        fprintf(fp, "-1\n");
        fclose(fp);
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

CMDF do_listdir(CHAR_DATA * ch, char *argument)
{

        char      buf[MSL];
        DIR      *dp;
        struct dirent *dentry;
        int       count = 0, dircount = 0;

        snprintf(buf, MSL, "../%s", argument);

        if ((dp = opendir(buf)) == NULL)
        {
                send_to_char("That directory does not exist\n\r", ch);
                return;
        }

        dentry = readdir(dp);

        ch_printf(ch, "&RDirectory listing for: %s\n\r", buf);

        for (dentry = readdir(dp); dentry; dentry = readdir(dp), count++)
        {
                if (dentry->d_name[0] == '.')
                {
                        dircount++;
                        continue;
                }

                ch_printf(ch, "&G  %s\n\r", dentry->d_name);
        }

        ch_printf(ch, "&pTotal number of files: %d\n\r", count - dircount);
        closedir(dp);
}

CMDF do_makebuilder(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;
        char      buf[MSL];
        char      arg[MAX_INPUT_LENGTH];
        sh_int    level;

        if (IS_NPC(ch))
                return;

        argument = one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char
                        ("Syntax: makebuilder <char> <level (optional)>\n\r",
                         ch);
                return;
        }

        if (argument[0] == '\0')
                level = LEVEL_BUILDER;
        else
                level = atoi(argument);

        if (level > get_trust(ch) || level <= LEVEL_IMMORTAL)
                level = get_trust(ch);

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("That player is not here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (level > victim->top_level)
        {
                victim->top_level = level;
                while (victim->first_carrying)
                        extract_obj(victim->first_carrying);
        }

        SET_BIT(victim->pcdata->godflags, IMM_BUILDER);
        send_to_char("Character made into a builder now.", ch);
        snprintf(buf, MSL, "%s was made into a builder", victim->name);
        log_string_plus(buf, LOG_BUILD, victim->top_level);
}

   CMDF do_list_teachers(CHAR_DATA * ch, char *argument)
   {
       CHAR_DATA *wch;
       int lc=0,alt = 0;
	   char buff[MSL];
       for (wch = first_char; wch; wch = wch->next)
       {
		   lc = 8;
           if (IS_NPC(wch) && IS_SET(wch->act, ACT_PRACTICE))
           {
               send_to_char( "\n\r&b----------------------------------------------------------------------------------\n\r",ch);
               ch_printf(ch, "&BT&zeacher&z: &W%-40s &bin &BR&zoom: &W%-7d &BV&znum&z: &W%-7d\n\r",wch->name, wch->in_room->vnum, wch->pIndexData->vnum );
               send_to_char( "&BT&zeachs: &W \n\r        ",ch);
               for ( int sn = 0; sn < top_sn; sn++ )
               {                
                   if ( skill_table[sn]->teachers && skill_table[sn]->teachers[0] != '\0' )
                   {
                       sprintf( buff, "%d", wch->pIndexData->vnum );

					   if( buff[0] == '\0' ) continue;
                       if ( is_name( buff, skill_table[sn]->teachers ) )
                       {
         			       if ((lc + strlen(skill_table[sn]->name)) > 80)
			               {
				               send_to_char("\n\r        ",ch);
              				   lc = 8;
			               }
           
                           if( alt == 0 )
                           {
                               alt = 1;
                               ch_printf(ch, "&W%s ", skill_table[sn]->name );
			                   lc += strlen(skill_table[sn]->name) + 1;
                           }
                           else
						   {
                               if( alt == 1 )
                               {
                                   alt = 0;
                                   ch_printf(ch, "&z%s ", skill_table[sn]->name );
 			                       lc += strlen(skill_table[sn]->name) + 1;
                               }
						   }
                       }
                   }    
               }
               send_to_char("\n\r",ch);
               send_to_char( "&b----------------------------------------------------------------------------------\n\r",ch);
           }
           buff[0]='\0';
       }
       return;
    }

CMDF do_qpreward(CHAR_DATA * ch, char *argument)
{
        char      name[MAX_STRING_LENGTH];
        char      buf[MAX_STRING_LENGTH];
		int       increase;
        CHAR_DATA *victim;

        argument = one_argument(argument, name);
        if (name[0] == '\0')
        {
                send_to_char("Usage:qpreward <playername> <increase amt>\n\r", ch);
                return;
        }
		increase = 1;
		if (!argument || argument[0] == '\0')
	    {
		    send_to_char("You must specify how many quest points they gain", ch);
			return;
        }

    	increase = atoi(argument);
		if (!is_number(argument) || increase < 1)
		{
			send_to_char("Please specify a numeric amount to increate between 0 and 10000", ch);
			return;
		}
        victim = get_char_world(ch, name);
        if (!victim || !victim->pcdata)
        {
                send_to_char("They're not here.\n\r", ch);
                return;
        }


        victim->pcdata->quest_curr += increase;

        send_to_char("You've been rewarded by the gods for your role-playing skill!\n\r", victim);
        snprintf(buf, MSL,"[AUTO COMMENT] I gave %s a reward of %d quest points\n\r",victim->name, increase);
		comment_add_comment(ch,victim->pcdata->account, "Been rewarded", buf);
        save_account(victim->pcdata->account);
        send_to_char("Done.\n\r", ch);

}


CMDF do_stripaffects(CHAR_DATA * ch, char *argument)
{
        char      name[MAX_STRING_LENGTH];
        char      buf[MAX_STRING_LENGTH];
		int       increase;
        CHAR_DATA *victim;

        argument = one_argument(argument, name);
        if (name[0] == '\0')
        {
                send_to_char("Usage:stripaffects <playername>\n\r", ch);
                return;
        }

        victim = get_char_world(ch, name);
        if (!victim || !victim->pcdata)
        {
                send_to_char("They're not here.\n\r", ch);
                return;
        }

		if (IS_NPC(victim))
		{
                send_to_char("Only on mortals.\n\r", ch);
                return;
		}


        act(AT_MAGIC, "You pass your hands over $S body and remove all magical auras", ch, NULL, victim, TO_CHAR);
        act(AT_MAGIC, "$n passes $s hands over you, and you feel less magical.", ch, NULL, victim, TO_VICT);
        while (victim->first_affect)
            affect_remove(victim, victim->first_affect);
        victim->affected_by = victim->race->affected();
}
