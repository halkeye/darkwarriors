/* vim: ts=8 et ft=c sw=8
 *****************************************************************************************
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

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <crypt.h>
#if defined(__CYGWIN__)
#include <crypt.h>
#endif
#include "mud.h"
#include "changes.h"
#include "mxp.h"
#include "msp.h"
#include "hotboot.h"
#include "ban.h"
#include "editor.h"
#include "account.h"
#include "alias.h"
#include "channels.h"
#include "races.h"
#include "greet.h"

#define TELOPTS
#define TELCMDS
/*
 * Socket and TCP/IP stuff.
 */
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>

#define MAX_NEST	100

const unsigned char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const unsigned char echo_on_str[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const unsigned char go_ahead_str[] = { IAC, GA, '\0' };

#define IS		'\x00'
#define TERMINAL_TYPE	'\x18'
#define SEND            '\x01'
const unsigned char wont_eor_str[] = { IAC, WONT, EOR, '\0' };

const unsigned char do_termtype_str[] = { IAC, DO, TERMINAL_TYPE, '\0' };
const unsigned char dont_termtype_str[] = { IAC, DONT, TERMINAL_TYPE, '\0' };
const unsigned char term_call_back_str[] = { IAC, SB, TERMINAL_TYPE, IS };
const unsigned char req_termtype_str[] =
        { IAC, SB, TERMINAL_TYPE, SEND, IAC, SE, '\0' };
/* Terminal detection stuff end */

#ifdef MCCP
const unsigned char will_compress_str[] =
        { IAC, WILL, TELOPT_COMPRESS, '\0' };
const unsigned char will_compress2_str[] =
        { IAC, WILL, TELOPT_COMPRESS2, '\0' };

const unsigned char do_compress1_str[] = { IAC, DO, TELOPT_COMPRESS, '\0' };
const unsigned char dont_compress1_str[] =
        { IAC, DONT, TELOPT_COMPRESS, '\0' };
const unsigned char do_compress2_str[] = { IAC, DO, TELOPT_COMPRESS2, '\0' };
const unsigned char dont_compress2_str[] =
        { IAC, DONT, TELOPT_COMPRESS2, '\0' };

const unsigned char send_compress2_str[] =
        { IAC, SB, TELOPT_COMPRESS2, IAC, SE, '\0' };
#endif


const char *gotmail = "[MAIL]";

void save_sysdata args((SYSTEM_DATA sys));
void shutdown_mud args((char *reason));
extern bool is_ignoring(CHAR_DATA * ch, CHAR_DATA * victim);
HELP_DATA *get_help(CHAR_DATA * ch, char *argument);
bool      char_exists(char *player);
void memory_cleanup args((void));

/*  from act_info?  */
void      show_condition(CHAR_DATA * ch, CHAR_DATA * victim);

/*
 * Global variables.
 */
DESCRIPTOR_DATA *first_descriptor = NULL;   /* First descriptor     */
DESCRIPTOR_DATA *last_descriptor = NULL;    /* Last descriptor      */
DESCRIPTOR_DATA *d_next = NULL; /* Next descriptor in loop  */
int       num_descriptors;
FILE     *fpReserve = NULL; /* Reserved file handle     */
bool      mud_down; /* Shutdown         */
bool      wizlock;  /* Game is wizlocked        */
time_t    boot_time;
HOUR_MIN_SEC set_boot_time_struct;
HOUR_MIN_SEC *set_boot_time = NULL;
struct tm *new_boot_time;
struct tm new_boot_struct;
char      str_boot_time[MAX_INPUT_LENGTH];
char      lastplayercmd[MAX_INPUT_LENGTH * 2];
time_t    current_time; /* Time of this pulse       */
int       port; /* Port number to be used       */
int       control;  /* Controlling descriptor   */
int       newdesc;  /* New descriptor       */
fd_set    in_set;   /* Set of desc's for reading    */
fd_set    out_set;  /* Set of desc's for writing    */
fd_set    exc_set;  /* Set of desc's with errors    */
int       maxdesc;
bool      crashover;    /* Perform Crashover?      */


/*
 * OS-dependent local functions.
 */
void game_loop args((void));
int init_socket args((int init_port));
void new_descriptor args((int new_desc));
bool read_from_descriptor args((DESCRIPTOR_DATA * d));
bool write_to_descriptor args((int desc, char *txt, int length));
void show_stat_options args((DESCRIPTOR_DATA * d, CHAR_DATA * ch));

#if MCCP
bool write_to_descriptor_old args((int desc, char *txt, int length));
#endif
extern int maxChanges;
extern CHANGE_DATA *changes_table;
char     *current_date args((void));

/*
 * Other local functions (OS-independent).
 */
bool check_parse_name args((char *name));
bool check_reconnect args((DESCRIPTOR_DATA * d, char *name, bool fConn));
sh_int check_playing args((DESCRIPTOR_DATA * d, char *name, bool kick));
bool check_multi args((DESCRIPTOR_DATA * d, char *name));
int main  args((int argc, char **argv));
void nanny args((DESCRIPTOR_DATA * d, char *argument));
bool flush_buffer args((DESCRIPTOR_DATA * d, bool fPrompt));
void read_from_buffer args((DESCRIPTOR_DATA * d));
void stop_idling args((CHAR_DATA * ch));
void free_desc args((DESCRIPTOR_DATA * d));
void display_prompt args((DESCRIPTOR_DATA * d));
int make_color_sequence args((const char *col, char *buf,
                              DESCRIPTOR_DATA * d));
void set_pager_input args((DESCRIPTOR_DATA * d, char *argument));
bool pager_output args((DESCRIPTOR_DATA * d));

sh_int client_speed args((sh_int speed));




void mail_count args((CHAR_DATA * ch));



int main(int argc, char **argv)
{
        struct timeval now_time;
        bool      fCopyOver = !TRUE;

#ifdef IMC
        int       imcsocket = -1;
#endif

        /*
         * pid_t p = -1; 
         */

        /*
         * Memory debugging if needed.
         */
#if defined(MALLOC_DEBUG)
        malloc_debug(2);
#endif

        crashover = FALSE;
        num_descriptors = 0;
        first_descriptor = NULL;
        last_descriptor = NULL;
        sysdata.NO_NAME_RESOLVING = TRUE;
        sysdata.WAIT_FOR_AUTH = TRUE;

        /*
         * Init time.
         */
        gettimeofday(&now_time, NULL);
        current_time = (time_t) now_time.tv_sec;
/*  gettimeofday( &boot_time, NULL);   okay, so it's kludgy, sue me :) */
        boot_time = time(0);    /*  <-- I think this is what you wanted */
        mudstrlcpy(str_boot_time, ctime(&current_time), MIL);

        /*
         * Init boot time.
         */
        set_boot_time = &set_boot_time_struct;
        /*
         * set_boot_time->hour   = 6;
         * set_boot_time->min    = 0;
         * set_boot_time->sec    = 0;
         */
        set_boot_time->manual = 0;

        new_boot_time = update_time(localtime(&current_time));
        /*
         * Copies *new_boot_time to new_boot_struct, and then points
         * new_boot_time to new_boot_struct again. -- Alty 
         */
        new_boot_struct = *new_boot_time;
        new_boot_time = &new_boot_struct;
        new_boot_time->tm_mday += 1;
        if (new_boot_time->tm_hour > 12)
                new_boot_time->tm_mday += 1;
        new_boot_time->tm_sec = 0;
        new_boot_time->tm_min = 0;
        new_boot_time->tm_hour = 6;

        /*
         * Update new_boot_time (due to day increment) 
         */
        new_boot_time = update_time(new_boot_time);
        new_boot_struct = *new_boot_time;
        new_boot_time = &new_boot_struct;

        /*
         * Set reboot time string for do_time 
         */
        get_reboot_string();
        init_pfile_scan_time(); /* Pfile autocleanup initializer - Samson 5-8-99 */

        /*
         * Get the port number.
         */
        port = 4000;
        if (argc > 1)
        {
                if (!is_number(argv[1]))
                {
                        fprintf(stderr, "Usage: %s [port #]\n", argv[0]);
                        exit(1);
                }
                else if ((port = atoi(argv[1])) <= 1024)
                {
                        fprintf(stderr, "Port number must be above 1024.\n");
                        exit(1);
                }
                if (argv[2] && argv[2][0])
                {
                        fCopyOver = TRUE;
                        control = atoi(argv[3]);
#ifdef IMC
                        imcsocket = atoi(argv[4]);
#endif
                }
                else
                        fCopyOver = FALSE;

        }

#if 0
        log_string("Daemonizing");
        if ((p = fork()))
        {
                FILE     *fpid = NULL;

                if ((fpid = fopen(PID_FILE, "w")) == NULL)
                {
                        perror(NULL_FILE);
                        exit(1);
                }
                fprintf(fpid, "%d", p);
                FCLOSE(fpid);
                /*
                 * Abort parent 
                 */
                exit(0);
        }
#endif

        /*
         * Reserve two channels for our use.
         */
        if ((fpReserve = fopen(NULL_FILE, "r")) == NULL)
        {
                perror(NULL_FILE);
                exit(1);
        }
        if ((fpLOG = fopen(NULL_FILE, "r")) == NULL)
        {
                perror(NULL_FILE);
                exit(1);
        }
        /*
         * Run the game.
         */

        log_string("Booting Database");
        boot_db(fCopyOver);
        log_string("Initializing socket");
        if (!fCopyOver) /* We have already the port if copyover'ed */
                control = init_socket(port);
#ifdef WEB
        if (sysdata.web)
        {
                sprintf(log_buf, "Booting webserver on port %d.", port + 2);
                log_string(log_buf);
                init_web(port + 2);
        }
#endif
#ifdef IMC
        /*
         * Initialize and connect to IMC2 
         */
        imc_startup(FALSE, imcsocket, fCopyOver);
#endif
        if (fCopyOver)
        {
                log_string("Initiating hotboot recovery.");
                hotboot_recover();
        }
        snprintf(log_buf, MSL, "Star Wars Reality ready on port %d.", port);
        log_string(log_buf);
        game_loop();
        /*
         * That's all, folks.
         */

#ifdef IMC
        imc_shutdown(FALSE);
#endif
#ifdef WEB
        if (sysdata.web)
        {
                sprintf(log_buf, "Shutting down webserver on port %d.",
                        port + 2);
                log_string(log_buf);
                shutdown_web();
        }
#endif
        log_string("Normal termination of game.");
        log_string("Cleaning up Memory.");
        memory_cleanup();
        exit(0);
        return 0;
}



int init_socket(int init_port)
{
        char      hostname[64];
        struct sockaddr_in sa;
        struct hostent *hp;
        struct servent *sp;
        int       x = 1;
        int       fd;

        gethostname(hostname, sizeof(hostname));


        if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
                perror("Init_socket: socket");
                exit(1);
        }

        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                       (void *) &x, sizeof(x)) < 0)
        {
                perror("Init_socket: SO_REUSEADDR");
                close(fd);
                exit(1);
        }

#if defined(SO_DONTLINGER) && !defined(SYSV)
        {
                struct linger ld;

                ld.l_onoff = 1;
                ld.l_linger = 1000;

                if (setsockopt(fd, SOL_SOCKET, SO_DONTLINGER,
                               (void *) &ld, sizeof(ld)) < 0)
                {
                        perror("Init_socket: SO_DONTLINGER");
                        close(fd);
                        exit(1);
                }
        }
#endif

        hp = gethostbyname(hostname);
        sp = getservbyname("service", "mud");
        memset(&sa, '\0', sizeof(sa));
        sa.sin_family = AF_INET;    /* hp->h_addrtype; */
        sa.sin_port = htons(init_port);

        if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) == -1)
        {
                perror("Init_socket: bind");
                close(fd);
                exit(1);
        }

        if (listen(fd, 50) < 0)
        {
                perror("Init_socket: listen");
                close(fd);
                exit(1);
        }

        return fd;
}

static void SigTerm(int signum);
static void SegVio(int signum)
{
        pid_t     p;
        DESCRIPTOR_DATA *d;
        char      buf[MSL];

        signum = 0;
        log_string("SEGMENTATION VIOLATION");
        log_string(lastplayercmd);
        mudstrlcpy(lastplayercmd, "", MIL * 2);

        if (sysdata.PORT)
        {
                signal(SIGPIPE, SIG_DFL);
                signal(SIGSEGV, SIG_DFL);
                signal(SIGTERM, SIG_DFL);
                if ((p = fork()) == 0)
                {
                        abort();    /* Abort child */
                }

                if (access("core", F_OK) == 0)
                {
                        snprintf(buf, MSL, "%d.core", p);
                        rename("core", buf);
                }

                signal(SIGPIPE, SIG_IGN);
                signal(SIGSEGV, SegVio);
                signal(SIGTERM, SigTerm);   /* Catch kill signals */

                if (crashover == TRUE)
                {
                        crashover = FALSE;
                        log_string("Crashover ready. Starting crashover.");
                        crash_hotboot();
                }
                else
                {
                        log_string("Crashover not ready. Shutting down.");
                        abort();
                }

                if (crashover == TRUE)
                {
                        crashover = FALSE;
                        echo_to_all(AT_RED,
                                    "&RATTENTION!! Crash, Hold on while we try and recover.\a",
                                    ECHOTAR_ALL);
                        for (d = first_descriptor; d; d = d->next)
                                flush_buffer(d, TRUE);
                        log_string("Crashover ready. Starting crashover.");
                        crash_hotboot();
                }
                else
                {
                        log_string("Crashover not ready. Shutting down.");
                        abort();
                }

        }
        abort();
}

void init_crashover(void)
{
        if (!crashover)
        {
                log_string_plus("Crashover system is ready.", LOG_COMM, sysdata.log_level);
                crashover = TRUE;
        }
}

static void SigTerm(int signum)
{
        CHAR_DATA *vch;
        char      buf[MAX_STRING_LENGTH];

        signum = 0;

        snprintf(log_buf, MSL, "%s",
                 "&RATTENTION!! Message from game server: &YEmergency shutdown called.\a");
        echo_to_all(AT_RED, log_buf, ECHOTAR_ALL);
        snprintf(log_buf, MSL, "%s",
                 "Executing emergency shutdown proceedure.");
        echo_to_all(AT_YELLOW, log_buf, ECHOTAR_ALL);
        log_string
                ("Message from server: Executing emergency shutdown proceedure.");
        mudstrlcat(buf, "\n\r", MSL);

        if (auction->item)
                do_auction(supermob, "stop");

        log_string("Saving players....");
        for (vch = first_char; vch; vch = vch->next)
        {
                if (!IS_NPC(vch))
                {
                        save_char_obj(vch);
                        snprintf(log_buf, MSL, "%s saved.", vch->name);
                        log_string(log_buf);
                        if (vch->desc)
                        {
                                write_to_descriptor(vch->desc->descriptor,
                                                    "You have been saved to disk.\n\r",
                                                    0);
                        }
                }
        }


        fflush(stderr); /* make sure strerr is flushed */

        close(control);

        /*
         * Extra ports disabled - Samson 12-1-98 : uncomment this section if you still use them
         * close( control2 );
         * close( conclient);
         * close( conjava  ); 
         */

#ifdef IMC
        imc_shutdown(FALSE);    /* shut down IMC */
#endif
#ifdef I3
        I3_shutdown(0);
#endif

        log_string("Emergency shutdown complete.");
        shutdown_mud("Executing emergency shutdown proceedure.");

        /*
         * Using exit here instead of mud_down because the thing sometimes failed to kill when asked!! 
         */
        exit(8);
}

/*
 *
 * LAG alarm!							-Thoric
 *
static void caught_alarm( void )
{
    char buf[MAX_STRING_LENGTH];
    bug( "ALARM CLOCK!" );
    mudstrlcpy( buf, "Alas, the hideous malevalent entity known only as 'Lag' rises once more!\n\r",MSL);
    echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
    if ( newdesc )
    {
	FD_CLR( newdesc, &in_set );
	FD_CLR( newdesc, &out_set );
	log_string( "clearing newdesc" );
    }
    game_loop( );
    close( control );

    log_string( "Normal termination of game." );
    exit( 0 );
}
*/



bool check_bad_desc(int desc)
{
        if (FD_ISSET(desc, &exc_set))
        {
                FD_CLR(desc, &in_set);
                FD_CLR(desc, &out_set);
                log_string("Bad FD caught and disposed.");
                return TRUE;
        }
        return FALSE;
}

/*
 * Determine whether this player is to be watched  --Gorog
 */
#ifdef ACCOUNT
bool chk_watch(sh_int player_level, char *player_name, char *player_site,
               char *player_account)
#else
bool chk_watch(sh_int player_level, char *player_name, char *player_site)
#endif
{
        WATCH_DATA *pw;

/*
    char buf[MAX_INPUT_LENGTH];
    sprintf( buf, "che_watch entry: plev=%d pname=%s psite=%s",
                  player_level, player_name, player_site);
    log_string(buf);
*/
        if (!first_watch)
                return FALSE;

        for (pw = first_watch; pw; pw = pw->next)
        {
                if (pw->target_name)
                {
                        if (!str_cmp(pw->target_name, player_name)
                            && player_level < pw->imm_level)
                                return TRUE;
                }
                else if (pw->player_site)
                {
                        if (!str_prefix(pw->player_site, player_site)
                            && player_level < pw->imm_level)
                                return TRUE;
                }
#ifdef ACCOUNT
                else if (pw->player_account)
                {
                        if (!str_cmp(pw->player_account, player_account)
                            && player_level < pw->imm_level)
                                return TRUE;
                }
#endif
        }
        return FALSE;
}




void accept_new(int ctrl)
{
        static struct timeval null_time;
        DESCRIPTOR_DATA *d;

        /*
         * int maxdesc; Moved up for use with id.c as extern 
         */

#if defined(MALLOC_DEBUG)
        if (malloc_verify() != 1)
                abort();
#endif

        /*
         * Poll all active descriptors.
         */

        FD_ZERO(&in_set);
        FD_ZERO(&out_set);
        FD_ZERO(&exc_set);
        FD_SET(ctrl, &in_set);
        maxdesc = ctrl;
        newdesc = 0;
        for (d = first_descriptor; d; d = d->next)
        {
                maxdesc = UMAX(maxdesc, d->descriptor);
                FD_SET(d->descriptor, &in_set);
                FD_SET(d->descriptor, &out_set);
                FD_SET(d->descriptor, &exc_set);
                if (d->ifd != -1 && d->ipid != -1)
                {
                        maxdesc = UMAX(maxdesc, d->ifd);
                        FD_SET(d->ifd, &in_set);
                }
                if (d == last_descriptor)
                        break;
        }
        if (select(maxdesc + 1, &in_set, &out_set, &exc_set, &null_time) < 0)
        {
                perror("accept_new: select: poll");
                exit(1);
        }

        if (FD_ISSET(ctrl, &exc_set))
        {
                bug("Exception raise on controlling descriptor %d", ctrl);
                FD_CLR(ctrl, &in_set);
                FD_CLR(ctrl, &out_set);
        }
        else if (FD_ISSET(ctrl, &in_set))
        {
                newdesc = ctrl;
                new_descriptor(newdesc);
        }
}

void game_loop()
{
        struct timeval last_time;
        char      cmdline[MAX_INPUT_LENGTH];
        DESCRIPTOR_DATA *d;

/*  time_t	last_check = 0;  */

        signal(SIGPIPE, SIG_IGN);
/*    signal( SIGALRM, caught_alarm ); */
        signal(SIGSEGV, SegVio);
        signal(SIGTERM, SigTerm);   /* Catch kill signals */
        gettimeofday(&last_time, NULL);
        current_time = (time_t) last_time.tv_sec;

        /*
         * Main loop 
         */
        while (!mud_down)
        {
                accept_new(control);
#ifdef WEB
                if (sysdata.web)
                        handle_web();
#endif
                /*
                 * Kick out descriptors with raised exceptions
                 * or have been idle, then check for input.
                 */
                for (d = first_descriptor; d; d = d_next)
                {
                        if (d == d->next)
                        {
                                bug("descriptor_loop: loop found & fixed");
                                d->next = NULL;
                        }
                        d_next = d->next;

                        /*
                         * Shell code - cheks for forked descriptors 
                         */
                        if (check_forks(d, cmdline))
                                continue;

                        /*
                         * Check for aliases, and do its code 
                         */
                        if (check_aliases(d))
                                continue;

                        d->idle++;  /* make it so a descriptor can idle out */
                        if (FD_ISSET(d->descriptor, &exc_set))
                        {
                                FD_CLR(d->descriptor, &in_set);
                                FD_CLR(d->descriptor, &out_set);
                                if (d->character
                                    && (d->connected == CON_PLAYING
                                        || d->connected == CON_EDITING))
                                        save_char_obj(d->character);
                                d->outtop = 0;
                                close_socket(d, TRUE);
                                continue;
                        }
                        else if ((!d->character && d->idle > 360)   /* 2 mins */
                                 || (!IS_IMMORTAL(d->character) && d->connected != CON_PLAYING && d->idle > 1200)   /* 5 mins */
                                 || (!IS_IMMORTAL(d->character) && d->idle > 28800))    /* 2 hrs  */
                        {
                                write_to_descriptor(d->descriptor,
                                                    "Idle timeout... disconnecting.\n\r",
                                                    0);
                                d->outtop = 0;
                                close_socket(d, TRUE);
                                continue;
                        }
                        else
                        {
                                d->fcommand = FALSE;

                                if (FD_ISSET(d->descriptor, &in_set))
                                {
                                        d->idle = 0;
                                        if (d->character)
                                                d->character->timer = 0;
                                        if (!read_from_descriptor(d))
                                        {
                                                FD_CLR(d->descriptor,
                                                       &out_set);
                                                if (d->character
                                                    && (d->connected ==
                                                        CON_PLAYING
                                                        || d->connected ==
                                                        CON_EDITING))
                                                        save_char_obj(d->
                                                                      character);
                                                d->outtop = 0;
                                                close_socket(d, FALSE);
                                                continue;
                                        }
                                }

                                if ((d->connected == CON_PLAYING
                                     || d->character != NULL) && d->ifd != -1
                                    && FD_ISSET(d->ifd, &in_set))
                                        process_dns(d);

                                if (d->character && d->character->wait > 0)
                                {
                                        --d->character->wait;
                                        continue;
                                }

                                read_from_buffer(d);
                                if (d->incomm[0] != '\0')
                                {
                                        d->fcommand = TRUE;
                                        stop_idling(d->character);

                                        mudstrlcpy(cmdline, d->incomm, MIL);
                                        d->incomm[0] = '\0';

                                        if (d->pagepoint)
                                                set_pager_input(d, cmdline);
                                        else
                                                switch (d->connected)
                                                {
                                                default:
                                                        nanny(d, cmdline);
                                                        break;
                                                case CON_PLAYING:
                                                        interpret(d->
                                                                  character,
                                                                  cmdline);
                                                        break;
                                                case CON_EDITING:
                                                        edit_buffer(d->
                                                                    character,
                                                                    cmdline);
                                                        break;
                                                }
                                }
                        }
                        if (d == last_descriptor)
                                break;
                }
#ifdef IMC
                imc_loop();
#endif
                /*
                 * Autonomous game motion.
                 */
                update_handler();

                /*
                 * Output.
                 */
                for (d = first_descriptor; d; d = d_next)
                {
                        d_next = d->next;

                        if ((d->fcommand || d->outtop > 0)
                            && FD_ISSET(d->descriptor, &out_set))
                        {
                                if (d->pagepoint)
                                {
                                        if (!pager_output(d))
                                        {
                                                if (d->character
                                                    && (d->connected ==
                                                        CON_PLAYING
                                                        || d->connected ==
                                                        CON_EDITING))
                                                        save_char_obj(d->
                                                                      character);
                                                d->outtop = 0;
                                                close_socket(d, FALSE);
                                        }
                                }
                                else if (!flush_buffer(d, TRUE))
                                {
                                        if (d->character
                                            && (d->connected == CON_PLAYING
                                                || d->connected ==
                                                CON_EDITING))
                                                save_char_obj(d->character);
                                        d->outtop = 0;
                                        close_socket(d, FALSE);
                                }
                        }
                        if (d == last_descriptor)
                                break;
                }

                /*
                 * Synchronize to a clock.
                 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
                 * Careful here of signed versus unsigned arithmetic.
                 */
                {
                        struct timeval now_time;
                        long      secDelta;
                        long      usecDelta;

                        gettimeofday(&now_time, NULL);
                        usecDelta =
                                ((int) last_time.tv_usec) -
                                ((int) now_time.tv_usec) +
                                1000000 / PULSE_PER_SECOND;
                        secDelta =
                                ((int) last_time.tv_sec) -
                                ((int) now_time.tv_sec);
                        while (usecDelta < 0)
                        {
                                usecDelta += 1000000;
                                secDelta -= 1;
                        }

                        while (usecDelta >= 1000000)
                        {
                                usecDelta -= 1000000;
                                secDelta += 1;
                        }

                        if (secDelta > 0 || (secDelta == 0 && usecDelta > 0))
                        {
                                struct timeval stall_time;

                                stall_time.tv_usec = usecDelta;
                                stall_time.tv_sec = secDelta;
                                if (select(0, NULL, NULL, NULL, &stall_time) <
                                    0)
                                {
                                        perror("game_loop: select: stall");
                                        exit(1);
                                }
                        }
                }

                gettimeofday(&last_time, NULL);
                current_time = (time_t) last_time.tv_sec;

                /*
                 * Check every 5 seconds...  (don't need it right now)
                 * if ( last_check+5 < current_time )
                 * {
                 * CHECK_LINKS(first_descriptor, last_descriptor, next, prev,
                 * DESCRIPTOR_DATA);
                 * last_check = current_time;
                 * }
                 */
        }
        return;
}

void init_descriptor(DESCRIPTOR_DATA * dnew, int desc)
{
        dnew->next = NULL;
        dnew->process = 0;  /* Samson 4-16-98 - For new command shell */
        dnew->descriptor = desc;
#ifndef ACCOUNT
        dnew->connected = CON_GET_NAME;
#else
        dnew->connected = CON_GET_ACCOUNT;
#endif
        dnew->outsize = 2000;
        dnew->idle = 0;
        dnew->lines = 0;
        dnew->scrlen = 24;
        dnew->auth_state = 0;
        dnew->newstate = 0;
        dnew->prevcolor = 0x07;
        dnew->speed = 3;
        /*
         * What client is the user using? 
         */
        dnew->client = STRALLOC("(unknown)");
        dnew->mxp_detected = FALSE; /* turn off MXP initaly */
        dnew->msp_detected = FALSE; /* turn off MSP initaly */
        /*
         * force ansi - Dude, as samson said, it is the 2003s - Gavin
         */
        /*
         * FIXME dnew->ansi     = TRUE; 
         */
#ifdef ACCOUNT
        dnew->account = NULL;
#endif
        dnew->ifd = -1; /* Descriptor pipes, used for DNS resolution and such */
        dnew->ipid = -1;

        CREATE(dnew->outbuf, char, dnew->outsize);
}


void new_descriptor(int new_desc)
{
        char      buf[MAX_STRING_LENGTH];
        DESCRIPTOR_DATA *dnew;
        struct hostent *from;
        struct sockaddr_in sock;
        size_t       desc, size; /* GCC4 doesn't like these as ints */

        set_alarm(20);
        size = sizeof(sock);
        if (check_bad_desc(new_desc))
        {
                set_alarm(0);
                return;
        }
        set_alarm(20);
        if ((desc =
             accept(new_desc, (struct sockaddr *) &sock,
                    (socklen_t *) & size)) < 0)
        {
                set_alarm(0);
                return;
        }
        if (check_bad_desc(new_desc))
        {
                set_alarm(0);
                return;
        }
#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif
        set_alarm(20);
        if (fcntl(desc, F_SETFL, FNDELAY) == -1)
        {
                perror("New_descriptor: fcntl: FNDELAY");
                set_alarm(0);
                return;
        }
        if (check_bad_desc(new_desc))
                return;

        CREATE(dnew, DESCRIPTOR_DATA, 1);

        init_descriptor(dnew, desc);
        from = gethostbyaddr((char *) &sock.sin_addr, sizeof(sock.sin_addr),
                             AF_INET);
        mudstrlcpy(log_buf, inet_ntoa(sock.sin_addr), MIL * 2);
        dnew->host = STRALLOC(log_buf);

        if (!sysdata.NO_NAME_RESOLVING)
        {
                mudstrlcpy(buf, in_dns_cache(log_buf), MSL);

                if (buf[0] == '\0')
                        resolve_dns(dnew, sock.sin_addr.s_addr);
                else
                {
                        STRFREE(dnew->host);
                        dnew->host = STRALLOC(buf);
                }
        }

        snprintf(log_buf, MIL * 2, "Sock.sinaddr:  %s", dnew->host);
        log_string_plus(log_buf, LOG_COMM, sysdata.log_level);

        if (check_total_bans(dnew))
        {
#ifdef MCCP
                write_to_descriptor_old(desc,
                                        "Your site has been banned from this Mud.\n\r",
                                        0);
#else
                write_to_descriptor(desc,
                                    "Your site has been banned from this Mud.\n\r",
                                    0);
#endif
                free_desc(dnew);
                set_alarm(0);
                return;
        }

        /*
         * Init descriptor data.
         */

        if (!last_descriptor && first_descriptor)
        {
                DESCRIPTOR_DATA *d;

                bug("New_descriptor: last_desc is NULL, but first_desc is not! ...fixing");
                for (d = first_descriptor; d; d = d->next)
                        if (!d->next)
                                last_descriptor = d;
        }

        LINK(dnew, first_descriptor, last_descriptor, next, prev);

        /*
         * Terminal detect 
         */
        write_to_buffer(dnew, (const char *) do_termtype_str, 0);

#ifdef MCCP
        /*
         * MCCP Compression 
         */
        write_to_buffer(dnew, (const char *) will_compress2_str, 0);
#endif

        /*
         * Mud eXtention Protocol 
         */
        /*
         * WARNING - Causes problems with older versions of zMUd that don't respond properly.
         * * As per others instructions, just tell them 'To stop using a cracked version and upgrade'
         */
        write_to_buffer(dnew, (char *) will_mxp_str, 0);

        /*
         * Mud Sound Protocol 
         */
        write_to_buffer(dnew, (char *) will_msp_str, 0);

        /*
         * Send the greeting.
         */

        {
                extern char *help_greeting;

                if (help_greeting[0] == '.')
                        send_to_desc_color(help_greeting + 1, dnew);
                else
                        send_to_desc_color(help_greeting, dnew);

        }

        if (++num_descriptors > sysdata.maxplayers)
                sysdata.maxplayers = num_descriptors;
        if (sysdata.maxplayers > sysdata.alltimemax)
        {
                if (sysdata.time_of_max)
                        DISPOSE(sysdata.time_of_max);
                snprintf(buf, MSL, "%24.24s", ctime(&current_time));
                sysdata.time_of_max = str_dup(buf);
                sysdata.alltimemax = sysdata.maxplayers;
                snprintf(log_buf, MSL,
                         "Broke all-time maximum player record: %d",
                         sysdata.alltimemax);
                log_string_plus(log_buf, LOG_COMM, sysdata.log_level);
                save_sysdata(sysdata);
        }
        set_alarm(0);
        return;
}

void free_desc(DESCRIPTOR_DATA * d)
{
        close(d->descriptor);
        STRFREE(d->host);
        DISPOSE(d->outbuf);
        if (d->pagebuf)
                DISPOSE(d->pagebuf);
        if (d->client)
                STRFREE(d->client);
#ifdef MCCP
        compressEnd(d);
#endif
        DISPOSE(d);
        --num_descriptors;
        return;
}

void close_socket(DESCRIPTOR_DATA * dclose, bool force)
{
        CHAR_DATA *ch;
        DESCRIPTOR_DATA *d;
        bool      DoNotUnlink = FALSE;

        if (dclose->ipid != -1)
        {
                int       status;

                kill(dclose->ipid, SIGKILL);
                waitpid(dclose->ipid, &status, 0);
        }
        if (dclose->ifd != -1)
                close(dclose->ifd);

        /*
         * flush outbuf 
         */
        if (!force && dclose->outtop > 0)
                flush_buffer(dclose, FALSE);

        /*
         * say bye to whoever's snooping this descriptor 
         */
        if (dclose->snoop_by)
                write_to_buffer(dclose->snoop_by,
                                "Your victim has left the game.\n\r", 0);

        /*
         * stop snooping everyone else 
         */
        for (d = first_descriptor; d; d = d->next)
                if (d->snoop_by == dclose)
                        d->snoop_by = NULL;

        /*
         * Check for switched people who go link-dead. -- Altrag 
         */
        if (dclose->original)
        {
                if ((ch = dclose->character) != NULL)
                        do_return(ch, "");
                else
                {
                        bug("Close_socket: dclose->original without character %s", (dclose->original->name ? dclose->original->name : "unknown"));
                        dclose->character = dclose->original;
                        dclose->original = NULL;
                }
        }

        ch = dclose->character;

        /*
         * sanity check :( 
         */
        if (!dclose->prev && dclose != first_descriptor)
        {
                DESCRIPTOR_DATA *dp, *dn;

                bug("Close_socket: %s desc:%p != first_desc:%p and desc->prev = NULL!", ch ? ch->name : d->host, dclose, first_descriptor);
                dp = NULL;
                for (d = first_descriptor; d; d = dn)
                {
                        dn = d->next;
                        if (d == dclose)
                        {
                                bug("Close_socket: %s desc:%p found, prev should be:%p, fixing.", ch ? ch->name : d->host, dclose, dp);
                                dclose->prev = dp;
                                break;
                        }
                        dp = d;
                }
                if (!dclose->prev)
                {
                        bug("Close_socket: %s desc:%p could not be found!.",
                            ch ? ch->name : dclose->host, dclose);
                        DoNotUnlink = TRUE;
                }
        }
        if (!dclose->next && dclose != last_descriptor)
        {
                DESCRIPTOR_DATA *dp, *dn;

                bug("Close_socket: %s desc:%p != last_desc:%p and desc->next = NULL!", ch ? ch->name : d->host, dclose, last_descriptor);
                dn = NULL;
                for (d = last_descriptor; d; d = dp)
                {
                        dp = d->prev;
                        if (d == dclose)
                        {
                                bug("Close_socket: %s desc:%p found, next should be:%p, fixing.", ch ? ch->name : d->host, dclose, dn);
                                dclose->next = dn;
                                break;
                        }
                        dn = d;
                }
                if (!dclose->next)
                {
                        bug("Close_socket: %s desc:%p could not be found!.",
                            ch ? ch->name : dclose->host, dclose);
                        DoNotUnlink = TRUE;
                }
        }

        if (dclose->character)
        {
                snprintf(log_buf, MSL, "Closing link to %s.", ch->name);
                log_string_plus(log_buf, LOG_COMM,
                                UMAX(sysdata.log_level, ch->top_level));
                if (dclose->connected == CON_PLAYING
                    || dclose->connected == CON_EDITING)
                {
                        act(AT_ACTION, "$n has lost $s link.", ch, NULL, NULL,
                            TO_ROOM);
                        ch->desc = NULL;
                }
                else
                {
                        /*
                         * clear descriptor pointer to get rid of bug message in log 
                         */
                        dclose->character->desc = NULL;
                        free_char(dclose->character);
                }
        }
#ifdef ACCOUNT
        else if (dclose->account != NULL)
        {
                free_account(dclose->account);
                dclose->account = NULL;
        }
#endif


        if (!DoNotUnlink)
        {
                /*
                 * make sure loop doesn't get messed up 
                 */
                if (d_next == dclose)
                        d_next = d_next->next;
                UNLINK(dclose, first_descriptor, last_descriptor, next, prev);
        }

        if (dclose->descriptor == maxdesc)
                --maxdesc;

#ifdef MCCP
        compressEnd(dclose);
#endif

        free_desc(dclose);
        return;
}


bool read_from_descriptor(DESCRIPTOR_DATA * d)
{
        int       iStart;

        /*
         * Hold horses if pending command already. 
         */
        if (d->incomm[0] != '\0')
                return TRUE;

        /*
         * Check for overflow. 
         */
        iStart = strlen(d->inbuf);
        if (iStart >= (int) (sizeof(d->inbuf) - 10))
        {
                snprintf(log_buf, MSL, "%s input overflow!", d->host);
                log_string(log_buf);
                write_to_descriptor(d->descriptor,
                                    "\n\r*** PUT A LID ON IT!!! ***\n\r", 0);
                return FALSE;
        }

        for (;;)
        {
                int       nRead;

                nRead = read(d->descriptor, d->inbuf + iStart,
                             sizeof(d->inbuf) - 10 - iStart);
                if (nRead > 0)
                {
                        iStart += nRead;
                        if (d->inbuf[iStart - 1] == '\n'
                            || d->inbuf[iStart - 1] == '\r')
                                break;
                }
                else if (nRead == 0)
                {
                        log_string_plus("EOF encountered on read.", LOG_COMM,
                                        sysdata.log_level);
                        return FALSE;
                }
                else if (errno == EWOULDBLOCK)
                        break;
                else
                {
                        perror("Read_from_descriptor");
                        return FALSE;
                }
        }

        d->inbuf[iStart] = '\0';
        return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer(DESCRIPTOR_DATA * d)
{
        int       i, j, k, iac = 0;
        unsigned char *p;

        /*
         * Hold horses if pending command already.
         */
        if (d->incomm[0] != '\0')
                return;

        /*
         * Thanks Nick! 
         */
        for (p = (unsigned char *) d->inbuf; *p; p++)
        {
                if (*p == IAC)
                {
                        if (memcmp
                            (p, term_call_back_str,
                             sizeof(term_call_back_str)) == 0)
                        {
                                int       pos = (char *) p - d->inbuf;  /* where we are in buffer */
                                int       len = sizeof(d->inbuf) - pos - sizeof(term_call_back_str);    /* how much to go */
                                char      tmp[100];
                                unsigned int x = 0;
                                unsigned char *oldp = p;

                                p += sizeof(term_call_back_str);    /* skip TERMINAL_TYPE / IS characters */

                                for (x = 0; x < (sizeof(tmp) - 1) && *p != 0    /* null marks end of buffer */
                                     && *p != IAC;  /* should terminate with IAC */
                                     x++, p++)
                                        tmp[x] = *p;
                                tmp[x] = '\0';
                                STRFREE(d->client);
                                d->client = STRALLOC(tmp);
                                p += 2; /* skip IAC and SE */
                                len -= strlen(tmp) + 2;
                                if (len < 0)
                                        len = 0;

                                /*
                                 * remove string from input buffer 
                                 */
                                memmove(oldp, p, len);
                        }   /* end of getting terminal type */
                }   /* end of finding an IAC */
        }

        /*
         * Look for at least one new line.
         */
        for (i = 0;
             d->inbuf[i] != '\n' && d->inbuf[i] != '\r' && i < MAX_INBUF_SIZE;
             i++)
        {
#ifdef DEBUG
                int       temp;
                char      buf[255];

                mudstrlcpy(buf, "IAC ", 255);
                for (temp = 1; temp < 5; temp++)
                {
                        if (d->inbuf[i + temp] == IAC)
                                break;

                        if (d->inbuf[i + temp] == '\0')
                                mudstrlcat(buf, " NULL", 255);
                        else if (TELOPT_OK(d->inbuf[i + temp]))
                                mudstrlcat(buf,
                                           telopts[(unsigned int) d->
                                                   inbuf[i + temp]], 255);
                        else if (TELCMD_OK(d->inbuf[i + temp]))
                                mudstrlcat(buf,
                                           telcmds[((unsigned int) d->
                                                    inbuf[i + temp]) - 236],
                                           255);
                        else
                        {
                                char      tempbuf[15];

                                snprintf(tempbuf, MSL, "%d",
                                         (unsigned int) d->inbuf[i + temp]);
                                mudstrlcat(buf, tempbuf, 255);
                        }

                        mudstrlcat(buf, " ", 255);
                }
                bug("TELOPT: %s", buf);
#endif
                if (d->inbuf[i] == '\0')
                        return;
        }

        /*
         * Canonical input processing.
         */
        for (i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++)
        {
                if (k >= (MAX_INBUF_SIZE-20))
                {
                        write_to_descriptor(d->descriptor,
                                            "Line too long.\n\r", 0);
                        d->inbuf[i] = '\n';
                        d->inbuf[i + 1] = '\0';
                        break;
                }
                if (d->inbuf[i] == (signed char) IAC)
                        iac = 1;
                else if (iac == 1
                         && (d->inbuf[i] == (signed char) DO
                             || d->inbuf[i] == (signed char) DONT
                             || d->inbuf[i] == (signed char) WILL))
                        iac = 2;
                else if (iac == 2)
                {
                        iac = 0;
                        if (d->inbuf[i] == (signed char) TERMINAL_TYPE)
                        {
                                if (d->inbuf[i - 1] == (signed char) WILL)
                                        write_to_buffer(d,
                                                        (const char *)
                                                        req_termtype_str, 0);
                        }
#ifdef MCCP
                        else if (d->inbuf[i] ==
                                 (signed char) TELOPT_COMPRESS2)
                        {
                                if (d->inbuf[i - 1] == (signed char) DO
                                    && !d->compressing)
                                        compressStart(d, TELOPT_COMPRESS2);
                                else if (d->compressing == TELOPT_COMPRESS2
                                         && d->inbuf[i - 1] ==
                                         (signed char) DONT)
                                        compressEnd(d);
                        }
                        else if (d->inbuf[i] ==
                                 (signed char) TELOPT_COMPRESS2)
                        {
                                if (d->inbuf[i - 1] == (signed char) DO
                                    && !d->compressing)
                                        compressStart(d, TELOPT_COMPRESS);
                                else if (d->compressing == TELOPT_COMPRESS
                                         && d->inbuf[i - 1] ==
                                         (signed char) DONT)
                                        compressEnd(d);
                        }
#endif
                        else if (d->inbuf[i] == (signed char) TELOPT_MXP)
                        {
                                if (d->inbuf[i - 1] == (signed char) DO)
                                {
                                        d->mxp_detected = TRUE; /* turn it on now */
                                        send_mxp_stylesheet(d);
/*					if (d->connected == CON_GET_ACCOUNT)
						send_to_desc_color( MXPTAG("user"), d);*/
                                }
                                else if (d->inbuf[i - 1] ==
                                         (signed char) DONT)
                                        d->mxp_detected = FALSE;
                        }
                        else if (d->inbuf[i] == (signed char) TELOPT_MSP)
                        {
                                if (d->inbuf[i - 1] == (signed char) DO)
                                        d->msp_detected = TRUE;
                                else if (d->inbuf[i - 1] ==
                                         (signed char) DONT)
                                        d->msp_detected = FALSE;
                        }
                }
                else if (d->inbuf[i] == '\b' && k > 0)
                        --k;
                else if (isascii(d->inbuf[i]) && isprint(d->inbuf[i]))
                        d->incomm[k++] = d->inbuf[i];
        }
        /*
         * Finish off the line.
         */
        if (k == 0)
                d->incomm[k++] = ' ';
        d->incomm[k] = '\0';

        /*
         * Deal with bozos with #repeat 1000 ...
         */
        if (k > 1 || d->incomm[0] == '!')
        {
                if (d->incomm[0] != '!' && strcmp(d->incomm, d->inlast))
                {
                        d->repeat = 0;
                }
                else
                {
                        if (d->repeat >= 50)
                        {
                                if (d->repeat == 50)
                                {
                                        snprintf(log_buf, MSL,
                                                 "%s (%s) input spamming!",
                                                 d->character ? d->character->
                                                 name : "unconnected",
                                                 d->host);
                                        log_string(log_buf);
                                }
                                write_to_descriptor(d->descriptor,
                                                    "\n\r*** PUT A LID ON IT!!! ***\n\r",
                                                    0);
                                close_socket(d, FALSE);
                        }
                        else if (++d->repeat >= 20)
                        {
                                if (d->repeat == 20)
                                {
                                        snprintf(log_buf, MSL,
                                                 "%s (%s) input spamming!",
                                                 d->character ? d->character->
                                                 name : "unconnected",
                                                 d->host);
                                        log_string(log_buf);
                                }
                                write_to_descriptor(d->descriptor,
                                                    "\n\r*** PUT A LID ON IT!!! ***\n\r",
                                                    0);
                        }
                }
        }
        /*
         * Do '!' substitution.
         */
        if (d->incomm[0] == '!')
                mudstrlcpy(d->incomm, d->inlast, MIL);
        else
                mudstrlcpy(d->inlast, d->incomm, MIL);

        /*
         * Shift the input buffer.
         */
        while (d->inbuf[i] == '\n' || d->inbuf[i] == '\r')
                i++;
        for (j = 0; (d->inbuf[j] = d->inbuf[i + j]) != '\0'; j++)
        {
        }
        return;
}



/*
 * Low level output function.
 */
bool flush_buffer(DESCRIPTOR_DATA * d, bool fPrompt)
{
        char      buf[MIL * 5];
        CHAR_DATA *ch;

        ch = d->original ? d->original : d->character;
        if (ch && ch->fighting && ch->fighting->who)
                show_condition(ch, ch->fighting->who);

        if (!d->speed || d->speed < 1 || d->speed > 5)
                d->speed = 2;
        /*
         * If buffer has more than their max, send max every second.
         */
        if (!mud_down && d->outtop > client_speed(d->speed))
        {
                memcpy(buf, d->outbuf, client_speed(d->speed));
                memmove(d->outbuf, d->outbuf + client_speed(d->speed),
                        d->outtop - client_speed(d->speed));
                d->outtop -= client_speed(d->speed);
                if (d->snoop_by)
                {
                        char      snoopbuf[MIL * 5];

                        buf[client_speed(d->speed)] = '\0';
                        if (d->character && d->character->name)
                        {
                                if (d->original && d->original->name)
                                        snprintf(snoopbuf, MIL * 5, "%s (%s)",
                                                 d->character->name,
                                                 d->original->name);
                                else
                                        snprintf(snoopbuf, MIL * 5, "%s",
                                                 d->character->name);
                                write_to_buffer(d->snoop_by, snoopbuf, 0);
                        }
                        write_to_buffer(d->snoop_by, "% ", 2);
                        write_to_buffer(d->snoop_by, buf, 0);
                }
                if (!write_to_descriptor
                    (d->descriptor, buf, client_speed(d->speed)))
                {
                        d->outtop = 0;
                        return FALSE;
                }
                return TRUE;
        }


        /*
         * Bust a prompt.
         */
        if (fPrompt && !mud_down && d->connected == CON_PLAYING)
        {
                ch = d->original ? d->original : d->character;
                if (IS_SET(ch->act, PLR_BLANK))
                        write_to_buffer(d, "\n\r", 2);

                if (IS_SET(ch->act, PLR_PROMPT))
                        display_prompt(d);
                if (IS_SET(ch->act, PLR_TELNET_GA))
                        write_to_buffer(d, (char *) go_ahead_str, 0);
        }

        /*
         * Short-circuit if nothing to write.
         */
        if (d->outtop == 0)
                return TRUE;

        /*
         * Snoop-o-rama.
         */
        if (d->snoop_by)
        {
                /*
                 * without check, 'force mortal quit' while snooped caused crash, -h 
                 */
                if (d->character && d->character->name)
                {
                        /*
                         * Show original snooped names. -- Altrag 
                         */
                        if (d->original && d->original->name)
                                snprintf(buf, MIL * 5, "%s (%s)",
                                         d->character->name,
                                         d->original->name);
                        else
                                snprintf(buf, MIL * 5, "%s",
                                         d->character->name);
                        write_to_buffer(d->snoop_by, buf, 0);
                }
                write_to_buffer(d->snoop_by, "% ", 2);
                write_to_buffer(d->snoop_by, d->outbuf, d->outtop);
        }

        /*
         * OS-dependent output.
         */
        if (!write_to_descriptor(d->descriptor, d->outbuf, d->outtop))
        {
                d->outtop = 0;
                return FALSE;
        }
        else
        {
                d->outtop = 0;
                return TRUE;
        }
}



/*
 * Append onto an output buffer.
 */
bool write_to_buffer(DESCRIPTOR_DATA * d, const char *txt, int length)
{
        int       origlength = 0;

        if (!d)
        {
                bug("Write_to_buffer: NULL descriptor");
                return FALSE;
        }

        /*
         * Normally a bug... but can happen if loadup is used. 
         */
        if (!d->outbuf)
                return FALSE;
        /*
         * Find length in case caller didn't. 
         */
        if (length <= 0)
                length = strlen(txt);

        origlength = length;
        /*
         * How much space do we need to expand stuff 
         */
        length += count_mxp_tags(d, txt, length);

#ifdef DEBUG
        if (length != strlen(txt))
        {
                bug("Write_to_buffer: length(%d) != strlen(txt)!", length);
                length = strlen(txt);
        }
#endif

        /*
         * Initial \n\r if needed. 
         */
        if (d->outtop == 0 && !d->fcommand)
        {
                d->outbuf[0] = '\n';
                d->outbuf[1] = '\r';
                d->outtop = 2;
        }

        /*
         * Expand the buffer as needed.
         */
        while (d->outtop + length >= (int) d->outsize)
        {
                if (d->outsize > 64000)
                {
                        /*
                         * empty buffer 
                         */
                        d->outtop = 0;
                        bug("Buffer overflow. Closing (%s).",
                            d->character ? d->character->name : "???");
                        close_socket(d, TRUE);
                        return FALSE;
                }
                d->outsize *= 2;
                RECREATE(d->outbuf, char, d->outsize);
        }

        /*
         * Copy.
         */
        convert_mxp_tags(d, d->outbuf + d->outtop, txt, origlength);
        d->outtop += length;
        d->outbuf[d->outtop] = '\0';
        return TRUE;
}


/*
* Lowest level output function. Write a block of text to the file descriptor.
* If this gives errors on very long blocks (like 'ofind all'), try lowering
* the max block size.
*
* Added block checking to prevent random booting of the descriptor. Thanks go
* out to Rustry for his suggestions. -Orion
*/
#ifdef MCCP

bool write_to_descriptor(int desc, char *txt, int length)
{
        DESCRIPTOR_DATA *d = NULL;
        int       iStart = 0;
        int       nWrite = 0;
        int       nBlock;
        int       len;

        if (length <= 0)
                length = strlen(txt);

        for (d = first_descriptor; d; d = d->next)
        {
                if (d->descriptor == desc)
                        break;
        }

		if (!d) return FALSE;

        if (d->descriptor != desc)
		{
                d = NULL;
				bug("Descriptor of d is not equal");
				return FALSE;
		}

        if (d && d->out_compress)
        {
                d->out_compress->next_in = (unsigned char *) txt;
                d->out_compress->avail_in = length;

                while (d->out_compress->avail_in)
                {
                        d->out_compress->avail_out =
                                COMPRESS_BUF_SIZE -
                                (d->out_compress->next_out -
                                 d->out_compress_buf);

                        if (d->out_compress->avail_out)
                        {
                                int       status = deflate(d->out_compress,
                                                           Z_SYNC_FLUSH);

                                if (status != Z_OK)
                                        return FALSE;
                        }

                        len = d->out_compress->next_out - d->out_compress_buf;
                        if (len > 0)
                        {
                                for (iStart = 0; iStart < len;
                                     iStart += nWrite)
                                {
                                        nBlock = UMIN(len - iStart, 4096);
                                        if ((nWrite =
                                             write(d->descriptor,
                                                   d->out_compress_buf +
                                                   iStart, nBlock)) < 0)
                                        {
                                                perror("Write_to_descriptor: compressed");
												d->outtop = 0;
												close_socket(d, TRUE);
                                                return FALSE;
                                        }

                                        if (!nWrite)
                                                break;
                                }

                                if (!iStart)
                                        break;

                                if (iStart < len)
                                        memmove(d->out_compress_buf,
                                                d->out_compress_buf + iStart,
                                                len - iStart);

                                d->out_compress->next_out =
                                        d->out_compress_buf + len - iStart;
                        }
                }
                return TRUE;
        }

        for (iStart = 0; iStart < length; iStart += nWrite)
        {
                nBlock = UMIN(length - iStart, 4096);
                if ((nWrite = write(desc, txt + iStart, nBlock)) < 0)
                {
                        perror("Write_to_descriptor");
						d->outtop = 0;
						close_socket(d, TRUE);
                        return FALSE;
                }
        }

        return TRUE;
}

bool write_to_descriptor_old(int desc, char *txt, int length)
#else
bool write_to_descriptor(int desc, char *txt, int length)
#endif
{
        int       iStart = 0;
        int       nWrite = 0;
        int       nBlock = 0;
        int       iErr = 0;

        if (length <= 0)
                length = strlen(txt);

        for (iStart = 0; iStart < length; iStart += nWrite)
        {
                nBlock = UMIN(length - iStart, 4096);
                nWrite = send(desc, txt + iStart, nBlock, 0);

                if (nWrite == -1)
                {
#ifdef WIN32
                        iErr = WSAGetLastError();
#else
                        iErr = errno;
#endif
                        if (iErr == EWOULDBLOCK)
                        {
                                /*
                                 * This is a SPAMMY little bug error. I would suggest
                                 * not using it, but I've included it in case. -Orion
                                 *
                                 perror( "Write_to_descriptor: Send is blocking" );
                                 */
                                nWrite = 0;
                                continue;
                        }
                        else
                        {
                                perror("Write_to_descriptor");
                                return FALSE;
                        }
                }
        }

        return TRUE;
}

void show_title(DESCRIPTOR_DATA * d)
{
        write_to_buffer(d, "Press enter...\n\r", 0);
        d->connected = CON_PRESS_ENTER;
}


/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny(DESCRIPTOR_DATA * d, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      arg[MAX_STRING_LENGTH];
        CHAR_DATA *ch;
        char     *pwdnew;
        char     *p;
        int       iClass;
        bool      fOld;
        sh_int    chk;
        int       i, immchanges = 0;
        int       col = 0;
        RACE_DATA *race = NULL;

#ifdef ACCOUNT
        int       count;
        char     *name;
#endif

        while (isspace(*argument))
                argument++;

        ch = d->character;

        switch (d->connected)
        {
        default:
                bug("Nanny: bad d->connected %d.", d->connected);
                close_socket(d, TRUE);
                return;
#ifdef ACCOUNT
        case CON_GET_ACCOUNT:
                d->account = NULL;
                if (argument[0] == '\0')
                {
                        close_socket(d, FALSE);
                        return;
                }

                argument[0] = UPPER(argument[0]);

                if (!str_cmp(argument, "New"))
                {
                        if (d->newstate == 0)
                        {
                                /*
                                 * New player 
                                 */
                                /*
                                 * Don't allow new players if DENY_NEW_PLAYERS is true 
                                 */
                                if (sysdata.DENY_NEW_PLAYERS == TRUE)
                                {
                                        send_to_desc_color
                                                ("The mud is currently preparing for a reboot.\n\r",
                                                 d);
                                        send_to_desc_color
                                                ("New players are not accepted during this time.\n\r",
                                                 d);
                                        send_to_desc_color
                                                ("Please try again in a few minutes.\n\r",
                                                 d);
                                        close_socket(d, FALSE);
                                        return;
                                }
                        }
                        send_to_desc_color
                                ("\n\r&BA&zn account allows you to manage and link your various characters.\n\r"
                                 "After creating your new account, you will be prompted to create your\n\r"
                                 "characters, or if you have existing characters but no account, you\n\r"
                                 "will be able to link them to your account then.\n\r"
                                 "\n\r&BP&zlease choose a login for your account: ",
                                 d);
/*				d->newstate++;
				d->connected = CON_CONFIRM_NEW_ACCOUNT_NAME; */
                        send_to_desc_color
                                ("\n\rNote: this doesn't actually work atm. FIXME PLS. For now just type in an invalid name",
                                 d);
                        return;
                }
                else if (!check_parse_name(argument))
                {
                        send_to_desc_color
                                ("&BI&zllegal name, try another.\n\r&BA&zccount &BN&zame, or &w\"&BN&zew&w\": ",
                                 d);
                        return;
                }

                if (check_playing(d, argument, FALSE) == BERR)
                {
                        send_to_desc_color("&BA&zccount &BN&zame: ", d);
                        return;
                }

                d->account = load_account(argument);
#if 0
                if (!d->account)
                {
                        mudstrlcpy(argument, "new", MIL);
                        /*
                         * FIXME - Ugly, but clean until we use functions more 
                         */
                        goto new_account;
                        return;
                }

#endif
                if (d->account)
                {
                        if (d->newstate != 0)
                        {
                                send_to_desc_color
                                        ("That account name is already taken.  Please choose another: ",
                                         d);
                                d->connected = CON_GET_ACCOUNT;
                                return;
                        }
                        /*
                         * Old player 
                         */
                        send_to_desc_color("&BA&zccount &BP&zassword: ", d);
                        send_to_desc_color((char *) echo_off_str, d);
                        if (d->mxp_detected)
                                send_to_desc_color(MXPTAG("password"), d);
                        d->connected = CON_GET_OLD_ACCOUNT_PASSWORD;
                        return;
                }
                else
                {
                        send_to_desc_color
                                ("\n\r&BT&zhat account does not appear in our database. Are you new?\n\r\n\r",
                                 d);
                        snprintf(buf, MSL,
                                 "&BI&zs this correct, %s &B(&wY&z/&wN&B)&z? ",
                                 argument);
                        send_to_desc_color(buf, d);
                        d->connected = CON_CONFIRM_NEW_ACCOUNT_NAME;
                        d->account = create_account();
                        d->account->rpcurrent = 0;
                        d->account->name = STRALLOC(argument);
                        LINK(d->account, first_account, last_account, next,
                             prev);
                        return;
                }
                break;

        case CON_GET_OLD_ACCOUNT_PASSWORD:
                write_to_buffer(d, "\n\r", 2);

                if (strcmp
                    (crypt(argument, d->account->password),
                     d->account->password))
                {
                        send_to_desc_color("&BW&zrong password.\n\r", d);
                        /*
                         * clear descriptor pointer to get rid of bug message in log 
                         */
                        close_socket(d, FALSE);
                        return;
                }
                send_to_desc_color
                        ("\n\r&BP&zlease choose one of the following\n\r", d);
                show_account_characters(d);
                send_to_desc_color("\n\r&BY&zour selection: ", d);
                d->connected = CON_GET_ALT;
                break;

        case CON_CONFIRM_NEW_ACCOUNT_NAME:
                switch (*argument)
                {
                case 'y':
                case 'Y':
                        snprintf(buf, MSL,
                                 "\n\r&BM&zake sure to use a password that won't be easily guessed by someone else.\n\r&BP&zick a good password for the account: %s",
                                 echo_off_str);
                        send_to_desc_color(buf, d);
                        d->connected = CON_GET_NEW_ACCOUNT_PASSWORD;
                        break;

                case 'n':
                case 'N':
                        send_to_desc_color("&BO&zk, what IS it, then? ", d);
                        free_account(d->account);
                        d->account = NULL;
                        d->character = NULL;
                        d->connected = CON_GET_ACCOUNT;
                        break;

                default:
                        send_to_desc_color("&BP&zlease type Yes or No. ", d);
                        break;
                }
                break;

        case CON_GET_NEW_ACCOUNT_PASSWORD:
                write_to_buffer(d, "\n\r", 2);

                if (strlen(argument) < 5)
                {
                        send_to_desc_color
                                ("&BP&zassword must be at least five characters long.\n\rPassword: ",
                                 d);
                        return;
                }

                pwdnew = crypt(argument, d->account->name);
                for (p = pwdnew; *p != '\0'; p++)
                {
                        if (*p == '~')
                        {
                                send_to_desc_color
                                        ("&BN&zew password not acceptable, try again.\n\rPassword: ",
                                         d);
                                return;
                        }
                }

                if (d->account->password)
                        STRFREE(d->account->password);
                d->account->password = STRALLOC(pwdnew);
                send_to_desc_color
                        ("\n\r&BP&zlease retype the password to confirm: ",
                         d);
                d->connected = CON_CONFIRM_NEW_ACCOUNT_PASSWORD;
                break;

        case CON_CONFIRM_NEW_ACCOUNT_PASSWORD:
                write_to_buffer(d, "\n\r", 2);

                if (strcmp
                    (crypt(argument, d->account->password),
                     d->account->password))
                {
                        send_to_desc_color
                                ("&BP&zasswords don't match.\n\rRetype password: ",
                                 d);
                        d->connected = CON_GET_NEW_ACCOUNT_PASSWORD;
                        return;
                }

                write_to_buffer(d, (char *) echo_on_str, 0);
                d->connected = CON_ACCOUNT_GET_EMAIL;
                send_to_desc_color("\n\r&BY&zour email address: ", d);
				break;

        case CON_ACCOUNT_GET_EMAIL:
				if (argument[0] == '\0' || !strstr(argument, "@")) {
					send_to_desc_color("\n\r&BI&zvalid email address. Try again\n\r", d);
					d->connected = CON_ACCOUNT_GET_EMAIL;
					send_to_desc_color("\n\r&BY&zour email address: ", d);
					break;
				}

                show_account_characters(d);
                send_to_desc_color("\n\r&BY&zour selection: ", d);
                d->connected = CON_GET_ALT;
                save_account(d->account);
                break;
/* a1 */
        case CON_GET_ACC_OLDPASS:
                if (argument[0] == '\0')
                {
                        send_to_desc_color("\n\r&BA&zborting.\n\r", d);
                        show_account_characters(d);
                        send_to_desc_color("\n\r&BY&zour selection: ", d);
                        d->connected = CON_GET_ALT;
                        return;
                }
                if (!strcmp
                    (crypt(argument, d->account->password),
                     d->account->password))
                {
                        send_to_desc_color
                                ("\n\r&BE&znter new password (or press enter to abort):&w ",
                                 d);
                        d->connected = CON_GET_ACC_NEWPASS;
                        return;
                }
                else
                {
                        send_to_desc_color
                                ("\n\rIncorrect password. Please try again.",
                                 d);
                        send_to_desc_color
                                ("\n\r&BP&zlease enter your old password (or press enter to abort):&w ",
                                 d);
                        d->connected = CON_GET_ACC_OLDPASS;
                        return;
                }

        case CON_GET_ACC_NEWPASS:
                if (argument[0] == '\0')
                {
                        send_to_desc_color("\n\r&BA&zborting.", d);
                        show_account_characters(d);
                        send_to_desc_color("\n\r&BY&zour selection: ", d);
                        d->connected = CON_GET_ALT;
                        return;
                }
                pwdnew = crypt(argument, d->account->name);
                for (p = pwdnew; *p != '\0'; p++)
                {
                        if (*p == '~')
                        {
                                send_to_desc_color
                                        ("\n\r&BI&znvalid password. Try again.",
                                         d);
                                send_to_desc_color
                                        ("\n\r&BP&zlease enter your new password (or press enter to abort):&w ",
                                         d);
                                d->connected = CON_GET_ACC_NEWPASS;
                                return;
                        }
                }
                if (strlen(argument) < 5)
                {
                        send_to_desc_color
                                ("\n\r&BP&zassword must be at least 5 characters long. Try again.",
                                 d);
                        send_to_desc_color
                                ("\n\r&BP&zlease enter your new password (or press enter to abort):&w ",
                                 d);
                        d->connected = CON_GET_ACC_NEWPASS;
                        return;
                }
                else
                {
                        d->account->password = STRALLOC(pwdnew);
                        d->connected = CON_GET_ACC_CONFIRMPASS;
                        send_to_desc_color
                                ("\n\r&BP&zlease confirm password (or press enter to abort):&w ",
                                 d);
                        return;
                }
        case CON_GET_ACC_CONFIRMPASS:
                if (argument[0] == '\0')
                {
                        send_to_desc_color("\n\r&BA&zborting.", d);
                        send_to_desc_color
                                ("\n\r&BP&zlease enter your new password (or press enter to abort):&w ",
                                 d);
                        d->connected = CON_GET_ACC_NEWPASS;
                        return;
                }
                if (strcmp
                    (crypt(argument, d->account->password),
                     d->account->password))
                {
                        send_to_desc_color("\n\r&BP&zasswords don't match.",
                                           d);
                        send_to_desc_color
                                ("\n\r&BP&zlease enter your new password (or press enter to abort):&w ",
                                 d);
                        d->connected = CON_GET_ACC_NEWPASS;
                }
                else
                {
                        send_to_desc_color("\n\r&BP&zassword changed.\n\r",
                                           d);
                        save_account(d->account);
                        send_to_desc_color
                                ("\n\r&BP&zlease choose one of the following\n\r",
                                 d);
                        show_account_characters(d);
                        send_to_desc_color("\n\r&BY&zour selection: ", d);
                        d->connected = CON_GET_ALT;
                        break;
                        return;
                }
        case CON_GET_ALT:
                if (argument[0] == '\0')
                {
                        send_to_desc_color
                                ("\n\r&BT&zhat is an invalid option, your must choose one of the following:\n\r",
                                 d);
                        show_account_characters(d);
                        d->connected = CON_GET_ALT;
                        return;
                }
/* a2 */
                if (nifty_is_name(argument, "password"))
                {
                        send_to_desc_color
                                ("\n\r&BP&zlease enter your old password (or enter to abort):&w ",
                                 d);
                        d->connected = CON_GET_ACC_OLDPASS;
                        return;
                }
                if (is_number(argument))
                {
                        count = atoi(argument);
                        if (count < 0 || count >= MAX_CHARACTERS
                            || d->account->character[count] == NULL)
                        {
                                send_to_desc_color
                                        ("\n\r&BI&znvalid Choice&W:&z ", d);
                                return;
                        }
                }
                else if (!str_cmp(argument, "?"))
                {
                        show_account_characters(d);
                        send_to_desc_color("\n\r&BY&zour selection: ", d);
                        return;
                }
                else if (!str_cmp(argument, "New"))
                {
                        if (d->newstate == 0)
                        {
                                /*
                                 * New player 
                                 */
                                /*
                                 * Don't allow new players if DENY_NEW_PLAYERS is true 
                                 */
                                if (sysdata.DENY_NEW_PLAYERS == TRUE)
                                {
                                        send_to_desc_color
                                                ("The mud is currently preparing for a reboot.\n\r",
                                                 d);
                                        send_to_desc_color
                                                ("New players are not accepted during this time.\n\r",
                                                 d);
                                        send_to_desc_color
                                                ("Please try again in a few minutes.\n\r",
                                                 d);
                                        close_socket(d, FALSE);
                                }
                                send_to_desc_color
                                        ("\n\rChoosing a name is one of the most important parts of this game...\n\r"
                                         "Make sure to pick a name appropriate to the character you are going\n\r"
                                         "to role play, and be sure that it suits our theme.\n\r"
                                         "If the name you select is not acceptable, you will be asked to choose\n\r"
                                         "another one.\n\r\n\rPlease choose a name for your character: ",
                                         d);
                                d->newstate++;
                                d->connected = CON_GET_NAME;
                                return;
                        }
                        else
                        {
                                send_to_desc_color
                                        ("&BI&zllegal name, try another.\n\r&BN&zame: ",
                                         d);
                                return;
                        }
                }
                else if (!str_cmp(argument, "link"))
                {
                        /*
                         * send_to_desc_color( "\n\r&BW&zhat is your characters name to link to this account?\n\r&BT&zype &w\"&Bn&zew&w\"&z for a new character (or hit enter to abort):&w ", d );
                         */
                        send_to_desc_color
                                ("\n\r&BN&zame of character to link (or hit enter to abort):&w ",
                                 d);
                        d->connected = CON_LINK_ALT;
                        return;
                }
                else
                {
                        for (count = 0; count < MAX_CHARACTERS; count++)
                        {
                                /*
                                 * Cleaner to use a linked list for this 
                                 */
                                /*
                                 * or a functionc all - gavin 
                                 */
                                if (d->account->character[count] == NULL)
                                        continue;
                                if (!str_cmp
                                    (argument, d->account->character[count]))
                                        break;
                        }
                        if (d->account->character[count] == NULL
                            || count == MAX_CHARACTERS)
                        {
                                send_to_desc_color("\n\rInvalid Choice: ", d);
                                return;
                        }
                }

                if (d->account->character[count] == NULL)
                {
                        send_to_desc_color
                                ("\n\r&BT&zhat is an invalid option, your options are:\n\r",
                                 d);
                        argument[0] = '\0';
                        show_account_characters(d);
                        d->connected = CON_GET_ALT;
                        return;
                }
                else
                {
                        name = d->account->character[count];
                }

                name[0] = UPPER(name[0]);

                if (check_playing(d, name, FALSE) == BERR)
                {
                        send_to_desc_color("&BN&zame: ", d);
                        return;
                }

                fOld = load_char_obj(d, name, TRUE, FALSE);
                if (!d->character)
                {
                        snprintf(log_buf, MSL, "Bad player file %s@%s.",
                                 argument, d->host);
                        log_string(log_buf);
                        write_to_buffer(d,
                                        "Your playerfile is corrupt...Please notify Thoric@mud.compulink.com.\n\r",
                                        0);
                        close_socket(d, FALSE);
                        return;
                }
                else if (check_bans(d->character, BAN_SITE))
                {
                        send_to_desc_color
                                ("&BY&zour site has been banned from this Mud.\n\r",
                                 d);
                        close_socket(d, FALSE);
                        return;
                }

                ch = d->character;

                if (IS_SET(ch->act, PLR_DENY))
                {
                        snprintf(log_buf, MSL, "Denying access to %s@%s.",
                                 argument, d->host);
                        log_string_plus(log_buf, LOG_COMM, sysdata.log_level);
                        if (d->newstate != 0)
                        {
                                send_to_desc_color
                                        ("&BT&zhat name is already taken.  Please choose another: ",
                                         d);
                                d->connected = CON_GET_NAME;
                                return;
                        }
                        send_to_desc_color("&BY&zou are denied access.\n\r",
                                           d);
                        close_socket(d, FALSE);
                        return;
                }

                chk = check_reconnect(d, name, FALSE);
                if (chk == BERR)
                        return;

                if (chk)
                {
                        fOld = TRUE;
                }
                else
                {
                        if (wizlock && !IS_IMMORTAL(ch))
                        {
                                send_to_desc_color
                                        ("&BT&zhe game is wizlocked.  Only immortals can connect now.\n\r",
                                         d);
                                send_to_desc_color
                                        ("Please try back later.\n\r", d);
                                close_socket(d, FALSE);
                                return;
                        }
                }

                if (fOld)
                {
                        if (d->newstate != 0)
                        {
                                send_to_desc_color
                                        ("That name is already taken.  Please choose another: ",
                                         d);
                                d->connected = CON_GET_NAME;
                                return;
                        }
                        send_to_desc_color
                                ("&BC&zharacter confirmed. Entering game...\n\r",
                                 d);
                        write_to_buffer(d, (char *) echo_on_str, 0);

                        if (check_playing(d, ch->name, TRUE))
                                return;

                        chk = check_reconnect(d, ch->name, TRUE);
                        if (chk == BERR)
                        {
                                if (d->character && d->character->desc)
                                        d->character->desc = NULL;
                                close_socket(d, FALSE);
                                return;
                        }
                        if (chk == TRUE)
                                return;

                        if (check_multi(d, ch->name))
                        {
                                close_socket(d, FALSE);
                                return;
                        }

                        snprintf(buf, MSL, "%s", ch->name);
                        d->character->desc = NULL;
                        free_char(d->character);
                        fOld = load_char_obj(d, buf, FALSE, FALSE);
                        ch = d->character;
                        snprintf(log_buf, MSL, "%s@%s has connected.",
                                 ch->name, d->host);
                        log_string_plus(log_buf, LOG_COMM, ch->top_level);
                        show_title(d);
                        if (ch->pcdata->area)
                                do_loadarea(ch, "");
                }
                else
                {
#ifdef ACCOUNT
                        send_to_desc_color
                                ("\n\r&BT&zhat name isn't familiar.\n\r\n\r",
                                 d);
                        if (ch)
                        {
                                ch->desc = NULL;
                                free_char(ch);
                                d->character = NULL;
                        }
                        show_account_characters(d);
                        d->connected = CON_GET_ALT;
#else
                        send_to_desc_color
                                ("\n\r&BT&zhat name isn't familiar. Are you new?\n\r\n\r",
                                 d);
                        snprintf(buf, MSL,
                                 "&BD&zid I get that right, %s &B(&wY&z/&wN&B)&z? ",
                                 name);
                        send_to_desc_color(buf, d);
                        d->connected = CON_CONFIRM_NEW_NAME;
#endif
                        return;
                }
                break;


        case CON_LINK_ALT:
                if (argument[0] == '\0')
                {
                        send_to_desc_color("\n\r&BA&zborting.\n\r", d);
                        show_account_characters(d);
                        send_to_desc_color("\n\r&BY&zour selection:&w ", d);
                        d->connected = CON_GET_ALT;
                        return;
                }

                if (!str_cmp(argument, "new"))
                {
                        d->connected = CON_GET_NAME;
                        d->newstate++;
                        send_to_desc_color
                                ("&BW&zhat name would you like?:\n\r", d);
                        return;
                }

                for (count = 0; count < MAX_CHARACTERS; count++)
                {
                        if (d->account->character[count] == NULL)
                                continue;
                        if (!str_cmp(d->account->character[count], argument))
                        {
                                send_to_desc_color
                                        ("&BT&zhat character is already part of that account, choose another:\n\r",
                                         d);
                                d->connected = CON_LINK_ALT;
                                return;
                        }

                }
                fOld = load_char_obj(d, argument, TRUE, FALSE);
                if (!d->character)
                {
                        snprintf(log_buf, MSL, "Bad player file %s@%s.",
                                 argument, d->host);
                        log_string(log_buf);
                        write_to_buffer(d,
                                        "Your playerfile is corrupt...Please notify Thoric@mud.compulink.com.\n\r",
                                        0);
                        close_socket(d, FALSE);
                        return;
                }
                ch = d->character;

                if (fOld)
                {
                        send_to_desc_color
                                ("&BP&zassword (or hit enter to abort):&w ",
                                 d);
                        send_to_desc_color((char *) echo_off_str, d);
                        if (d->mxp_detected)
                                send_to_desc_color(MXPTAG("password"), d);
                        d->connected = CON_GET_LINK_PASSWORD;
                        return;
                }
                else
                {
                        if (d->character)
                        {
                                d->character->desc = NULL;
                                free_char(d->character);
                                d->character = NULL;
                        }
                        send_to_desc_color
                                ("\n\r&BT&zhat name isn't familiar. If you want to start a new 		character, type new.\n\r\n\r",
                                 d);
                        send_to_desc_color
                                ("\n\r&BW&zhat is your characters name to link to this account?\n\r&BT&zype 	&w\"&Bn&zew&w\" for a new character\n\r",
                                 d);
                        d->connected = CON_LINK_ALT;
                        return;
                }
                break;
        case CON_GET_LINK_PASSWORD:

                if (argument[0] == '\0')
                {
                        send_to_desc_color("\n\r&BA&zborting.\n\r", d);
                        if (d->character)
                        {
                                d->character->desc = NULL;
                                free_char(d->character);
                        }
                        show_account_characters(d);
                        send_to_desc_color("\n\r&BY&zour selection:&w ", d);
                        d->connected = CON_GET_ALT;
                        return;
                }
                if (strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd))
                {
                        send_to_desc_color("&BW&zrong password.\n\r", d);
                        /*
                         * clear descriptor pointer to get rid of bug message in log 
                         */
                        d->character->desc = NULL;
                        close_socket(d, FALSE);
                        return;
                }

                if (!add_to_account(d->account, ch))
                {
                        send_to_desc_color
                                ("&BT&zhere was an error linking your character. Notify greven at 		darkwarsmud@hotmail.com.\n\r",
                                 d);
                        /*
                         * clear descriptor pointer to get rid of bug message in log 
                         */
                        d->character->desc = NULL;
                        close_socket(d, FALSE);
                        return;
                }
                else
                {
                        send_to_desc_color("&BC&zharacter linked.\n\r", d);
                        send_to_desc_color
                                ("&BW&zhich of your alts would you like to play?\n\r&BY&zou may specify their name, or choose &w\"&BL&zink&z\" to link a another character to your account:\n\r",
                                 d);
                        if (d->character)
                        {
                                d->character->desc = NULL;
                                free_char(d->character);
                        }
                        show_account_characters(d);
                        d->connected = CON_GET_ALT;
                        save_account(d->account);
                        return;
                }
                break;

#endif
        case CON_GET_NAME:
                if (argument[0] == '\0')
                {
                        close_socket(d, FALSE);
                        return;
                }

                argument[0] = UPPER(argument[0]);
                if (!check_parse_name(argument))
                {
                        send_to_desc_color
                                ("&BI&zllegal name, try another.\n\r&BN&zame: ",
                                 d);
                        return;
                }

                if (!str_cmp(argument, "New"))
                {
                        if (d->newstate == 0)
                        {
                                /*
                                 * New player 
                                 */
                                /*
                                 * Don't allow new players if DENY_NEW_PLAYERS is true 
                                 */
                                if (sysdata.DENY_NEW_PLAYERS == TRUE)
                                {
                                        write_to_buffer(d,
                                                        "New players are not accepted during this time.\n\r",
                                                        0);
                                        write_to_buffer(d,
                                                        "Please try again in a few minutes.\n\r",
                                                        0);
                                        close_socket(d, FALSE);
                                }
                                write_to_buffer(d,
                                                "\n\rChoosing a name is one of the most important parts of this game...\n\r"
                                                "Make sure to pick a name appropriate to the character you are going\n\r"
                                                "to role play, and be sure that it suits our theme.\n\r"
                                                "If the name you select is not acceptable, you will be asked to choose\n\r"
                                                "another one.\n\r\n\rPlease choose a name for your character: ",
                                                0);
                                d->newstate++;
                                d->connected = CON_GET_NAME;
                                return;
                        }
                        else
                        {
                                send_to_desc_color
                                        ("&BI&zllegal name, try another.\n\r&BN&zame: ",
                                         d);
                                return;
                        }
                }

                if (check_playing(d, argument, FALSE) == BERR)
                {
                        send_to_desc_color("&BN&zame: ", d);
                        return;
                }

                fOld = load_char_obj(d, argument, TRUE, FALSE);
                if (!d->character)
                {
                        snprintf(log_buf, MSL, "Bad player file %s@%s.",
                                 argument, d->host);
                        log_string(log_buf);
                        write_to_buffer(d,
                                        "Your playerfile is corrupt...Please notify Thoric@mud.compulink.com.\n\r",
                                        0);
                        close_socket(d, FALSE);
                        return;
                }
                ch = d->character;

                if (IS_SET(ch->act, PLR_DENY))
                {
                        snprintf(log_buf, MSL, "Denying access to %s@%s.",
                                 argument, d->host);
                        log_string_plus(log_buf, LOG_COMM, sysdata.log_level);
                        if (d->newstate != 0)
                        {
                                send_to_desc_color
                                        ("That name is already taken.  Please choose another: ",
                                         d);
                                d->connected = CON_GET_NAME;
                                return;
                        }
                        send_to_desc_color("You are denied access.\n\r", d);
                        close_socket(d, FALSE);
                        return;
                }
                chk = check_reconnect(d, argument, FALSE);
                if (chk == BERR)
                        return;

                if (chk)
                {
                        fOld = TRUE;
                }
                else
                {
                        if (wizlock && !IS_IMMORTAL(ch))
                        {
                                send_to_desc_color
                                        ("&BT&zhe game is wizlocked.  Only immortals can connect now.\n\r",
                                         d);
                                send_to_desc_color
                                        ("Please try back later.\n\r", d);
                                close_socket(d, FALSE);
                                return;
                        }
                }

                if (fOld)
                {
                        if (check_bans(d->character, BAN_CLASS))
                        {
                                write_to_buffer(d,
                                                "Your class has been banned from this Mud.\n\r",
                                                0);
                                close_socket(d, FALSE);
                                return;
                        }

                        if (check_bans(d->character, BAN_RACE))
                        {
                                write_to_buffer(d,
                                                "Your race has been banned from this Mud.\n\r",
                                                0);
                                close_socket(d, FALSE);
                                return;
                        }

                        if (d->newstate != 0)
                        {
                                send_to_desc_color
                                        ("That name is already taken.  Please choose another: ",
                                         d);
                                d->connected = CON_GET_NAME;
                                if (ch)
                                {
                                        ch->desc = NULL;
                                        free_char(ch);
                                        d->character = NULL;
                                }
                                return;
                        }
                        /*
                         * Old player 
                         */
                        send_to_desc_color("&BP&zassword: ", d);
                        write_to_buffer(d, (char *) echo_off_str, 0);
                        d->connected = CON_GET_OLD_PASSWORD;
                        if (d->mxp_detected)
                                send_to_desc_color(MXPTAG("password"), d);
                        return;
                }
                else
                {
                        send_to_desc_color
                                ("\n\r&BT&zhat name isn't familiar. Are you new?\n\r\n\r",
                                 d);
                        snprintf(buf, MSL,
                                 "&BD&zid I get that right, %s &B(&wY&z/&wN&B)&z? ",
                                 argument);
                        send_to_desc_color(buf, d);
                        d->connected = CON_CONFIRM_NEW_NAME;
                        return;
                }
                break;
        case CON_GET_OLD_PASSWORD:
                write_to_buffer(d, "\n\r", 2);

                if (strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd))
                {
                        send_to_desc_color("&BW&zrong password.\n\r", d);
                        /*
                         * clear descriptor pointer to get rid of bug message in log 
                         */
                        d->character->desc = NULL;
                        close_socket(d, FALSE);
                        return;
                }

                write_to_buffer(d, (char *) echo_on_str, 0);

                if (check_playing(d, ch->name, TRUE))
                        return;

                chk = check_reconnect(d, ch->name, TRUE);
                if (chk == BERR)
                {
                        if (d->character && d->character->desc)
                                d->character->desc = NULL;
                        close_socket(d, FALSE);
                        return;
                }
                if (chk == TRUE)
                        return;

                if (check_multi(d, ch->name))
                {
                        close_socket(d, FALSE);
                        return;
                }
                mudstrlcpy(buf, ch->name, MSL);
                d->character->desc = NULL;
                free_char(d->character);
                fOld = load_char_obj(d, buf, FALSE, FALSE);
                ch = d->character;
                snprintf(log_buf, MSL, "%s@%s has connected.", ch->name,
                         d->host);
                log_string_plus(log_buf, LOG_COMM, ch->top_level);
                show_title(d);
                if (ch->pcdata->area)
                        do_loadarea(ch, "");
                break;

        case CON_CONFIRM_NEW_NAME:
                switch (*argument)
                {
                case 'y':
                case 'Y':
                        snprintf(buf, MSL,
                                 "\n\r&BM&zake sure to use a password that won't be easily guessed by someone else."
                                 "\n\r&BP&zick a good password for %s: %s",
                                 ch->name, echo_off_str);
                        send_to_desc_color(buf, d);
                        d->connected = CON_GET_NEW_PASSWORD;
                        break;
                case 'n':
                case 'N':
                        send_to_desc_color("&BO&zk, what IS it, then? ", d);
                        /*
                         * clear descriptor pointer to get rid of bug message in log 
                         */
                        d->character->desc = NULL;
                        free_char(d->character);
                        d->character = NULL;
                        d->connected = CON_GET_NAME;
                        break;

                default:
                        send_to_desc_color("&BP&zlease type Yes or No. ", d);
                        break;
                }
                break;

        case CON_GET_NEW_PASSWORD:
                write_to_buffer(d, "\n\r", 2);

                if (strlen(argument) < 5)
                {
                        send_to_desc_color
                                ("&BP&zassword must be at least five characters long.\n\rPassword: ",
                                 d);
                        return;
                }

                pwdnew = crypt(argument, ch->name);
                for (p = pwdnew; *p != '\0'; p++)
                        if (*p == '~')
                        {
                                send_to_desc_color
                                        ("&BN&zew password not acceptable, try again.\n\rPassword: ",
                                         d);
                                return;
                        }

                DISPOSE(ch->pcdata->pwd);
                ch->pcdata->pwd = str_dup(pwdnew);
                send_to_desc_color
                        ("\n\r&BP&zlease retype the password to confirm: ",
                         d);
                d->connected = CON_CONFIRM_NEW_PASSWORD;
                break;

        case CON_CONFIRM_NEW_PASSWORD:
                write_to_buffer(d, "\n\r", 2);

                if (strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd))
                {
                        send_to_desc_color
                                ("&BP&zasswords don't match.\n\rRetype password: ",
                                 d);
                        d->connected = CON_GET_NEW_PASSWORD;
                        return;
                }
                if (check_multi(d, ch->name))
                {
                        close_socket(d, FALSE);
                }
                write_to_buffer(d, (char *) echo_on_str, 0);
                send_to_desc_color
                        ("\n\r&BW&zhat is your sex &B(&wM&z/&wF&z/&wN&B)&z? ",
                         d);
                d->connected = CON_GET_NEW_SEX;
                break;

        case CON_GET_NEW_SEX:
                switch (argument[0])
                {
                case 'm':
                case 'M':
                        ch->sex = SEX_MALE;
                        break;
                case 'f':
                case 'F':
                        ch->sex = SEX_FEMALE;
                        break;
                case 'n':
                case 'N':
                        ch->sex = SEX_NEUTRAL;
                        break;
                default:
                        send_to_desc_color
                                ("&BT&zhat's not a sex.\n\r&BW&zhat IS your sex? ",
                                 d);
                        return;
                }

                send_to_desc_color
                        ("\n\r&BY&zou may choose from the following races, or type help &B[&wrace&B]&z to learn more:\n\r",
                         d);
                buf[0] = '\0';
                send_to_desc_color
                        ("&z|-----------------------------------------------------------------------|\n\r",
                         d);
                col = 0;
                FOR_EACH_LIST(RACE_LIST, races, race)
                {
                        if (d->account->rppoints < race->rpneeded())
                                continue;
                        ++col;
                        col %= 4;
                        send_to_desc_color("&z|", d);
                        snprintf(buf, MSL, "&B[&w%-15.15s&B]&w",
                                 capitalize(race->name()));
                        send_to_desc_color(buf, d);
                        if (col == 0)
                                send_to_desc_color("&z|\n\r", d);
                }
                /*
                 * Fix up aligning up 
                 */
                if (col >= 0)
                {
                        col = 4 - (col % 4);
                        send_to_desc_color("&z|", d);
                        while (col-- > 0)
                                send_to_desc_color("&B[               &B]&z|",
                                                   d);
                        send_to_desc_color("\n\r", d);
                }
                send_to_desc_color
                        ("&z|-----------------------------------------------------------------------|\n\r",
                         d);
                send_to_desc_color("&BC&zhoice&z:&w ", d);
                d->connected = CON_GET_NEW_RACE;
                break;

        case CON_GET_NEW_RACE:
                one_argument(argument, arg);
                if (!str_cmp(arg, "help"))
                {
                        argument = one_argument(argument, arg);
                        do_help(ch, argument);
                        send_to_desc_color
                                ("&BP&zlease choose a race, or try &B'&zhelp &B[&wrace&B]&z: ",
                                 d);
                        return;
                }

                if ((race = get_race(argument)) == NULL
                    || (d->account->rppoints < race->rpneeded()))
                {
                        send_to_desc_color
                                ("&BT&zhat's not a race.\n\r&BW&zhat &wIS &zyour race? ",
                                 d);
                        return;
                }
                else
                        ch->race = race;

                if (check_bans(ch, BAN_RACE))
                {
                        send_to_char
                                ("&BT&zhat race is currently &Rnot avaiable&z.\n\r&BC&zhoice&z:&w ",
                                 ch);
                        return;
                }

                send_to_desc_color
                        ("\n\r&BP&zlease choose a main ability from the folowing classes:\n\r",
                         d);
                buf[0] = '\0';
                send_to_desc_color
                        ("&z|-----------------------------------------------------------------------|\n\r",
                         d);
                col = 0;
                for (iClass = 0; iClass < MAX_ABILITY; iClass++)
                {
                        if (ch->race
                            && IS_SET(ch->race->class_restriction(),
                                      1 << iClass))
                                continue;
                        if ((iClass == 7 && d->account->rppoints < 20)
                            || iClass == 8)
                                continue;
                        if (!ability_name[iClass]
                            || ability_name[iClass][0] == '\0')
                                continue;
                        ++col;
                        col %= 4;
                        send_to_desc_color("&z|", d);
                        snprintf(buf, MSL, "&B[&w%-15.15s&B]&w",
                                 capitalize(ability_name[iClass]));
                        send_to_desc_color(buf, d);
                        if (col == 0)
                                send_to_desc_color("&z|\n\r", d);
                }
                /*
                 * Fix up aligning up 
                 */
                if (col >= 0)
                {
                        col = 4 - (col % 4);
                        send_to_desc_color("&z|", d);
                        while (col-- > 0)
                                send_to_desc_color("&B[               &B]&z|",
                                                   d);
                        send_to_desc_color("\n\r", d);
                }
                send_to_desc_color
                        ("&z|-----------------------------------------------------------------------|\n\r",
                         d);
                send_to_desc_color("&BC&zhoice&z:&w ", d);
                d->connected = CON_GET_NEW_CLASS;
                break;

        case CON_GET_NEW_CLASS:
                argument = one_argument(argument, arg);
                if (!str_cmp(arg, "help"))
                {
                        do_help(ch, argument);
                        send_to_desc_color("Please choose an ability class: ",
                                           d);
                        return;
                }
                for (iClass = 0; iClass < MAX_ABILITY; iClass++)
                {
                        if (ch->race
                            && IS_SET(ch->race->class_restriction(),
                                      1 << iClass))
                                continue;
                        if (toupper(arg[0]) ==
                            toupper(ability_name[iClass][0])
                            && !str_prefix(arg, ability_name[iClass])
                            && iClass != 8)
                        {
                                ch->main_ability = iClass;
                                break;
                        }
                }
                if (iClass == MAX_ABILITY || !ability_name[iClass]
                    || ability_name[iClass][0] == '\0' || (iClass == 7
                                                           && d->account->
                                                           rppoints < 20)
                    || iClass == 8)
                {
                        send_to_desc_color
                                ("&BT&zhat's not a skill class.\n\r&BW&zhat IS it going to be? ",
                                 d);
                        return;
                }
                if (check_bans(ch, BAN_CLASS))
                {
                        send_to_char
                                ("&BT&zhat class is currently &Rnot avaiable&z.\n\r&BC&zhoice&z:&w ",
                                 ch);
                        return;
                }
                send_to_desc_color
                        ("\n\r&BT&zhese are your current stats. Would you like to edit them?\n\r",
                         d);
                send_to_desc_color
                        ("&BI&zf not, enter &w\"DONE\" &z or &w\"RESET\".\n\r&BT&zo add a point to a specifc stat, enter the name of the stat:\n\r",
                         d);
                ch->pcdata->statpoints = 25;
                ch->perm_str = 13;
                ch->perm_int = 13;
                ch->perm_wis = 13;
                ch->perm_dex = 13;
                ch->perm_con = 13;
                ch->perm_cha = 13;
        case CON_SHOW_STAT_OPTIONS:
                show_stat_options(d, ch);
                send_to_desc_color("&BE&znter stat name to edit:", d);
                d->connected = CON_EDIT_STATS;
                break;

        case CON_EDIT_STATS:
                if (!argument || argument[0] == '\0')
                {
                        send_to_desc_color
                                ("&BT&zhat is not a valid option, please enter which stat you'd like to edit:",
                                 d);
                        d->connected = CON_EDIT_STATS;
                        return;
                }
                if (!str_cmp(argument, "done"))
                {
                        if (ch->pcdata->statpoints > 0)
                        {
                                send_to_desc_color
                                        ("\n\r&B[&wWARNING&B] &BY&zou still have some stat points left. Please distribute them.\n\r",
                                         d);
                                show_stat_options(d, ch);
                                send_to_desc_color
                                        ("&BE&znter stat name to edit:", d);
                                d->connected = CON_EDIT_STATS;
                                return;
                        }
                        send_to_desc_color
                                ("&BA&zre you sure that these stats are alright, &B(&wY&z/&wN&B)&z? ",
                                 d);
                        d->connected = CON_STATS_OK;
                        return;
                }
                if (!str_cmp(argument, "reset"))
                {
                        send_to_desc_color("&\n\rBR&zeseting stats.", d);
                        send_to_desc_color
                                ("\n\r&BT&zhese are your current stats. Would you like to edit them?\n\r",
                                 d);
                        send_to_desc_color
                                ("&BI&zf not, enter &w\"DONE\" &z or &w\"RESET\".\n\r&BT&zo add a point to a specifc stat, enter the name of the stat:\n\r",
                                 d);
                        ch->pcdata->statpoints = 25;
                        ch->perm_str = 13;
                        ch->perm_int = 13;
                        ch->perm_wis = 13;
                        ch->perm_dex = 13;
                        ch->perm_con = 13;
                        ch->perm_cha = 13;
                        show_stat_options(d, ch);
                        d->connected = CON_EDIT_STATS;
                        return;
                }
                if (!str_prefix(argument, "strength"))
                        ch->pcdata->statedit = 1;
                else if (!str_prefix(argument, "wisdom"))
                        ch->pcdata->statedit = 2;
                else if (!str_prefix(argument, "intelligence"))
                        ch->pcdata->statedit = 3;
                else if (!str_prefix(argument, "dexterity"))
                        ch->pcdata->statedit = 4;
                else if (!str_prefix(argument, "constitution"))
                        ch->pcdata->statedit = 5;
                else if (!str_prefix(argument, "charisma"))
                        ch->pcdata->statedit = 6;
                else
                {
                        send_to_desc_color
                                ("&BT&zhat is not a valid option, please enter which stat you'd like to edit:",
                                 d);
                        d->connected = CON_EDIT_STATS;
                        return;
                }
                send_to_desc_color("&BH&zow much do you want to edit it by? ",
                                   d);
                if (ch->pcdata->statedit == 1)
                {
                        snprintf(buf, MSL,
                                 "&BC&zurrent range is &B[&w%02d&B] &z- &B[&W%02d&B]&z:\n\r",
                                 UMAX((1 - ch->perm_str),
                                      (3 - ch->perm_str -
                                       ch->race->
                                       attr_modifier(ATTR_STRENGTH))),
                                 (20 - ch->perm_str));
                        send_to_desc_color(buf, d);
                }
                if (ch->pcdata->statedit == 2)
                {
                        snprintf(buf, MSL,
                                 "&BC&zurrent range is &B[&w%02d&B] &z- &B[&W%02d&B]&z:\n\r",
                                 UMAX((1 - ch->perm_wis),
                                      (3 - ch->perm_wis -
                                       ch->race->attr_modifier(ATTR_WISDOM))),
                                 (20 - ch->perm_wis));
                        send_to_desc_color(buf, d);
                }
                if (ch->pcdata->statedit == 3)
                {
                        snprintf(buf, MSL,
                                 "&BC&zurrent range is &B[&w%02d&B] &z- &B[&W%02d&B]&z:\n\r",
                                 UMAX((1 - ch->perm_int),
                                      (3 - ch->perm_int -
                                       ch->race->
                                       attr_modifier(ATTR_INTELLIGENCE))),
                                 (20 - ch->perm_int));
                        send_to_desc_color(buf, d);
                }
                if (ch->pcdata->statedit == 4)
                {
                        snprintf(buf, MSL,
                                 "&BC&zurrent range is &B[&w%02d&B] &z- &B[&W%02d&B]&z:\n\r",
                                 UMAX((1 - ch->perm_dex),
                                      (3 - ch->perm_dex -
                                       ch->race->
                                       attr_modifier(ATTR_DEXTERITY))),
                                 (20 - ch->perm_dex));
                        send_to_desc_color(buf, d);
                }
                if (ch->pcdata->statedit == 5)
                {
                        snprintf(buf, MSL,
                                 "&BC&zurrent range is &B[&w%02d&B] &z- &B[&W%02d&B]&z:\n\r",
                                 UMAX((1 - ch->perm_con),
                                      (3 - ch->perm_con -
                                       ch->race->
                                       attr_modifier(ATTR_CONSTITUTION))),
                                 (20 - ch->perm_con));
                        send_to_desc_color(buf, d);
                }
                if (ch->pcdata->statedit == 6)
                {
                        snprintf(buf, MSL,
                                 "&BC&zurrent range is &B[&w%02d&B] &z- &B[&W%02d&B]&z:\n\r",
                                 UMAX((1 - ch->perm_cha),
                                      (3 - ch->perm_cha -
                                       ch->race->
                                       attr_modifier(ATTR_CHARISMA))),
                                 (20 - ch->perm_cha));
                        send_to_desc_color(buf, d);
                }

                d->connected = CON_EDIT_STAT_NUM;
                break;

        case CON_EDIT_STAT_NUM:
                if (!(argument[0] == '-' && is_number(argument + 1))
                    && !is_number(argument))
                {
                        send_to_desc_color
                                ("&BT&zhat is not a valid option, please enter a number:",
                                 d);
                        d->connected = CON_EDIT_STAT_NUM;
                        return;
                }
                i = atoi(argument);
                if (i > ch->pcdata->statpoints)
                {
                        send_to_desc_color
                                ("&BY&zou do not have enough stat points. Please enter a new number.",
                                 d);
                        d->connected = CON_EDIT_STAT_NUM;
                        return;
                }
                if (ch->pcdata->statedit == 1)
                {
                        if ((i + ch->perm_str) > 20 || (i + ch->perm_str) < 1
                            || (i + ch->perm_str +
                                ch->race->attr_modifier(ATTR_STRENGTH)) < 3)
                        {
                                snprintf(buf, MSL,
                                         "&BT&zhat is not a valid amount. Current range is &B[&w%02d&B] &z- &B[&W%02d&B]&z:\n\r",
                                         UMAX((1 - ch->perm_str),
                                              (3 - ch->perm_str -
                                               ch->race->
                                               attr_modifier(ATTR_STRENGTH))),
                                         (20 - ch->perm_str));
                                send_to_desc_color(buf, d);
                                d->connected = CON_EDIT_STAT_NUM;
                                return;
                        }
                        else
                                ch->perm_str += i;
                }
                else if (ch->pcdata->statedit == 2)
                {
                        if ((i + ch->perm_wis) > 20 || (i + ch->perm_wis) < 1
                            || (i + ch->perm_wis +
                                ch->race->attr_modifier(ATTR_WISDOM)) < 3)
                        {
                                snprintf(buf, MSL,
                                         "&BT&zhat is not a valid amount. Current range is &B[&w%02d&B] &z- &B[&W%02d&B]&z:\n\r",
                                         UMAX((1 - ch->perm_wis),
                                              (3 - ch->perm_wis -
                                               ch->race->
                                               attr_modifier(ATTR_WISDOM))),
                                         (20 - ch->perm_wis));
                                send_to_desc_color(buf, d);
                                d->connected = CON_EDIT_STAT_NUM;
                                return;
                        }
                        else
                                ch->perm_wis += i;
                }
                else if (ch->pcdata->statedit == 3)
                {
                        if ((i + ch->perm_int) > 20 || (i + ch->perm_int) < 1
                            || (i + ch->perm_int +
                                ch->race->attr_modifier(ATTR_INTELLIGENCE)) <
                            3)
                        {
                                snprintf(buf, MSL,
                                         "&BT&zhat is not a valid amount. Current range is &B[&w%02d&B] &z- &B[&W%02d&B]&z:\n\r",
                                         UMAX((1 - ch->perm_int),
                                              (3 - ch->perm_int -
                                               ch->race->
                                               attr_modifier
                                               (ATTR_INTELLIGENCE))),
                                         (20 - ch->perm_int));
                                send_to_desc_color(buf, d);
                                d->connected = CON_EDIT_STAT_NUM;
                                return;
                        }
                        else
                                ch->perm_int += i;
                }
                else if (ch->pcdata->statedit == 4)
                {
                        if ((i + ch->perm_dex) > 20 || (i + ch->perm_dex) < 1
                            || (i + ch->perm_dex +
                                ch->race->attr_modifier(ATTR_DEXTERITY)) < 3)
                        {
                                snprintf(buf, MSL,
                                         "&BT&zhat is not a valid amount. Current range is &B[&w%02d&B] &z- &B[&W%02d&B]&z:\n\r",
                                         UMAX((1 - ch->perm_dex),
                                              (3 - ch->perm_dex -
                                               ch->race->
                                               attr_modifier
                                               (ATTR_DEXTERITY))),
                                         (20 - ch->perm_dex));
                                send_to_desc_color(buf, d);
                                d->connected = CON_EDIT_STAT_NUM;
                                return;
                        }
                        else
                                ch->perm_dex += i;
                }
                else if (ch->pcdata->statedit == 5)
                {
                        if ((i + ch->perm_con) > 20 || (i + ch->perm_con) < 1
                            || (i + ch->perm_con +
                                ch->race->attr_modifier(ATTR_CONSTITUTION)) <
                            3)
                        {
                                snprintf(buf, MSL,
                                         "&BT&zhat is not a valid amount. Current range is &B[&w%02d&B] &z- &B[&W%02d&B]&z:\n\r",
                                         UMAX((1 - ch->perm_con),
                                              (3 - ch->perm_con -
                                               ch->race->
                                               attr_modifier
                                               (ATTR_CONSTITUTION))),
                                         (20 - ch->perm_con));
                                send_to_desc_color(buf, d);
                                d->connected = CON_EDIT_STAT_NUM;
                                return;
                        }
                        else
                                ch->perm_con += i;
                }
                else if (ch->pcdata->statedit == 6)
                {
                        if ((i + ch->perm_cha) > 20 || (i + ch->perm_cha) < 1
                            || (i + ch->perm_cha +
                                ch->race->attr_modifier(ATTR_CHARISMA)) < 3)
                        {
                                snprintf(buf, MSL,
                                         "&BT&zhat is not a valid amount. Current range is &B[&w%02d&B] &z- &B[&W%02d&B]&z:\n\r",
                                         UMAX((1 - ch->perm_cha),
                                              (3 - ch->perm_cha -
                                               ch->race->
                                               attr_modifier(ATTR_CHARISMA))),
                                         (20 - ch->perm_cha));
                                send_to_desc_color(buf, d);
                                d->connected = CON_EDIT_STAT_NUM;
                                return;
                        }
                        else
                                ch->perm_cha += i;
                }
                ch->pcdata->statpoints -= i;
                send_to_desc_color("Done.\n\r", d);
                send_to_desc_color
                        ("&BT&zhese are your current stats. Would you like to edit them?\n\r",
                         d);
                send_to_desc_color
                        ("&BI&zf not, enter &w\"DONE\" &z or &w\"RESET\".\n\r&BT&zo add a point to a specifc stat, enter the name of the stat:\n\r",
                         d);
                show_stat_options(d, ch);
                send_to_desc_color("&BE&znter stat name to edit:", d);
                d->connected = CON_EDIT_STATS;
                return;


        case CON_STATS_OK:
                switch (argument[0])
                {
                case 'y':
                case 'Y':
                        ch->perm_str +=
                                ch->race->attr_modifier(ATTR_STRENGTH);
                        ch->perm_wis += ch->race->attr_modifier(ATTR_WISDOM);
                        ch->perm_int +=
                                ch->race->attr_modifier(ATTR_INTELLIGENCE);
                        ch->perm_dex +=
                                ch->race->attr_modifier(ATTR_DEXTERITY);
                        ch->perm_con +=
                                ch->race->attr_modifier(ATTR_CONSTITUTION);
                        ch->perm_cha +=
                                ch->race->attr_modifier(ATTR_CHARISMA);
                        break;
                case 'n':
                case 'N':
                        send_to_desc_color
                                ("\n\r&BT&zhese are your current stats. Would you like to edit them?\n\r",
                                 d);
                        send_to_desc_color
                                ("&BI&zf not, enter &w\"DONE\" &z or &w\"RESET\".\n\r&BT&zo add a point to a specifc stat, enter the name of the stat:\n\r",
                                 d);
                        show_stat_options(d, ch);
                        d->connected = CON_EDIT_STATS;
                        return;
                default:
                        send_to_desc_color
                                ("&BI&znvalid selection.\n\r&wYES &zor &wNO&z? ",
                                 d);
                        return;
                }

                send_to_desc_color
                        ("\n\r&BW&zould you like &wANSI&z graphic/color support, &B(&wY&z/&wN&B)&z? ",
                         d);
                d->connected = CON_GET_WANT_RIPANSI;
                break;

        case CON_GET_WANT_RIPANSI:
                switch (argument[0])
                {
                case 'y':
                case 'Y':
                        SET_BIT(ch->act, PLR_ANSI);
                        break;
                case 'n':
                case 'N':
                        break;
                default:
                        send_to_desc_color
                                ("&BI&znvalid selection.\n\r&wYES &zor &wNO&z? ",
                                 d);
                        return;
                }
                send_to_desc_color
                        ("&BD&zoes your mud client have the Mud Sound Protocol?  &B(&wY&z/&wN&B)&z?",
                         d);
                d->connected = CON_GET_MSP;
                break;


        case CON_GET_MSP:
                switch (argument[0])
                {
                case 'y':
                case 'Y':
                        SET_BIT(ch->act, PLR_SOUND);
                        break;
                case 'n':
                case 'N':
                        break;
                default:
                        write_to_buffer(d,
                                        "Invalid selection.\n\rYES or NO? ",
                                        0);
                        return;
                }

                snprintf(buf, MSL, " %s@%s new %s %s %s\n\r",
                         ch->name, d->host,
                         ch->sex == SEX_MALE ? "Male" : ch->sex ==
                         SEX_FEMALE ? "Female" : "Neutra", ch->race->name(),
                         ability_name[ch->main_ability]);
                log_string_plus(buf, LOG_COMM, sysdata.log_level);
                send_to_desc_color("&BP&zress &B[&wENTER&B] ", d);
                show_title(d);
                {
                        int       ability;

                        for (ability = 0; ability < MAX_ABILITY; ability++)
                                ch->skill_level[ability] = 0;
                }
                ch->top_level = 0;
                ch->position = POS_STANDING;
                d->connected = CON_PRESS_ENTER;
                return;
                break;

        case CON_PRESS_ENTER:
#ifdef ACCOUNT
                if (chk_watch(get_trust(ch), ch->name, d->host, d->account->name))  /*  --Gorog */
#else
                if (chk_watch(get_trust(ch), ch->name, d->host))    /*  --Gorog */
#endif
                        SET_BIT(ch->pcdata->flags, PCFLAG_WATCH);
                else
                        REMOVE_BIT(ch->pcdata->flags, PCFLAG_WATCH);
                if (IS_SET(ch->act, PLR_ANSI))
                        send_to_pager("\033[2J", ch);
                else
                        send_to_pager("\014", ch);
#ifdef ACCOUNT
                if (IS_SET(ch->act, PLR_SOUND) && ch->pcdata->account)
                {
                        SET_BIT(ch->pcdata->account->flags, ACCOUNT_SOUND);
                        REMOVE_BIT(ch->act, PLR_SOUND);
                }
#endif
                if (IS_IMMORTAL(ch))
                {
                        HELP_DATA *pHelp = get_help(ch, "imotd");

                        send_to_pager("&WImmortal Message of the Day&w\n\r",
                                      ch);
                        if (pHelp)
                        {
                                if (pHelp->text[0] == '.')
                                        send_to_pager(pHelp->text + 1, ch);
                                else
                                        send_to_pager(pHelp->text, ch);
                        }
                }
                if (ch->top_level > 0)
                {
                        HELP_DATA *pHelp = get_help(ch, "motd");

                        send_to_pager("\n\r&WMessage of the Day&w\n\r", ch);
                        if (pHelp)
                        {
                                if (pHelp->text[0] == '.')
                                        send_to_pager(pHelp->text + 1, ch);
                                else
                                        send_to_pager(pHelp->text, ch);
                        }
                }
                if (ch->top_level >= LEVEL_AVATAR)
                {
                        HELP_DATA *pHelp = get_help(ch, "amotd");

                        send_to_pager("\n\r&WAvatar Message of the Day&w\n\r",
                                      ch);
                        if (pHelp)
                        {
                                if (pHelp->text[0] == '.')
                                        send_to_pager(pHelp->text + 1, ch);
                                else
                                        send_to_pager(pHelp->text, ch);
                        }
                }
                if (ch->top_level == 0)
                {
                        HELP_DATA *pHelp = get_help(ch, "nmotd");

                        if (pHelp)
                        {
                                if (pHelp->text[0] == '.')
                                        send_to_pager(pHelp->text + 1, ch);
                                else
                                        send_to_pager(pHelp->text, ch);
                        }
                }
				if (!IS_CLANNED(ch))
				{
                        HELP_DATA *pHelp = get_help(ch, "citmotd");

                        send_to_pager("\n\r&BA&zttention Galactic Citizens:&W\n\r",ch);

                        if (pHelp)
                        {
                                if (pHelp->text[0] == '.')
                                        send_to_pager(pHelp->text + 1, ch);
                                else
                                        send_to_pager(pHelp->text, ch);
                        }
                        send_to_pager("\n\r",ch);
				}
                if (!IS_NPC(ch) && IS_SET(ch->pcdata->flags, PCFLAG_GOTMAIL))
                        send_to_char
                                ("&R[&YYou've got mail waiting for you!&R]&W",
                                 ch);
                if (maxChanges > 0)
                {
                        for (i = 0; i < maxChanges; i++)
                                if (changes_table[i].immchange == 1
                                    && !IS_IMMORTAL(ch))
                                        immchanges++;
                        ch_printf(ch,
                                  "&BT&zhere is a total of &B[&w %3d &B]&z changes in the database.\n\r",
                                  maxChanges - immchanges);

                }
                if (IS_IMMORTAL(ch))
                {
                        send_to_pager
                                ("\n\r&BD&zo you wish to enter &wWIZINVIS&z? &B[&wY&z/&wN&B]&R&W    ",
                                 ch);
                        d->connected = CON_WIZINVIS;
                }
                else
                {
                        send_to_pager("\n\r&WPress [ENTER] &Y", ch);
                        d->connected = CON_READ_MOTD;
                }
                break;

        case CON_WIZINVIS:
                switch (argument[0])
                {
                case 'y':
                case 'Y':
                        SET_BIT(ch->act, PLR_WIZINVIS);
                        send_to_desc_color("Wizinvis ON ", d);
                        break;
                case 'n':
                case 'N':
                        REMOVE_BIT(ch->act, PLR_WIZINVIS);
                        send_to_desc_color("Wizinvis OFF ", d);
                        break;
                default:
                        break;
                }
                send_to_pager("\n\r&WPress [ENTER] &Y", ch);
                d->connected = CON_READ_MOTD;
                break;

        case CON_READ_MOTD:
                write_to_buffer(d, "\n\r\n\r", 0);
                add_char(ch);
                d->connected = CON_PLAYING;
#ifdef ACCOUNT
                ch->pcdata->account = d->account;
#endif

                if (!xIS_EMPTY(ch->race->body_parts())
                    && xIS_EMPTY(ch->xflags))
                        ch->xflags = ch->race->body_parts();
                if (!xIS_EMPTY(ch->race->body_parts()))
                {
                        sh_int    p = 0;
                        int       sn;

                        for (sn = 0; sn < top_sn; sn++)
                        {
                                if (!xIS_EMPTY(skill_table[sn]->body_parts))
                                        /*
                                         * Forgot to initilize, tsk tsk grev 
                                         */
                                        for (p = 0; p < MAX_BITS; p++)
                                                if (xIS_SET(ch->xflags, p)
                                                    &&
                                                    xIS_SET(skill_table[sn]->
                                                            body_parts, p))
                                                        ch->pcdata->
                                                                learned[sn] =
                                                                100;
                        }
                }

                if (ch->top_level == 0)
                {
                        OBJ_DATA *obj;
                        int       iLang;
                        int       chance = number_percent();

                        if (ch->desc->mxp_detected)
                                SET_BIT(ch->act, PLR_MXP);
                        ch->pcdata->clan = NULL;
                        ch->pcdata->full_name = QUICKLINK(ch->name);
                        ch->pcdata->spouse = STRALLOC("");

                        ch->perm_lck = number_range(6, 18);
                        if (ch->main_ability == FORCE_ABILITY)
                                ch->perm_frc = number_range(3, 20);
                        /*
                         * 1 in 100 chance. To increase, check more random numbers of chance 
                         */
                        else if (chance == 2)
                                ch->perm_frc = number_range(1, 20);
                        else
                                ch->perm_frc = 0;

                        ch->affected_by = ch->race->affected();
                        ch->perm_lck += ch->race->attr_modifier(ATTR_LUCK);
                        ch->perm_frc =
                                URANGE(0,
                                       ch->perm_frc +
                                       ch->race->attr_modifier(ATTR_FORCE),
                                       20);

                        ch->pcdata->age = ch->race->start_age();

                        if ((ch->perm_frc -
                             ch->race->attr_modifier(ATTR_FORCE)) > 0
                            && str_cmp(ch->race->name(), "droid"))
					    {
                                snprintf(buf, MSL, "%s is starting with natural force of %d.", ch->name,  ch->perm_frc);
								log_string_plus(buf, LOG_COMM, sysdata.log_level);
                        }
                        if ((iLang =
                             skill_lookup(ch->race->language()->name)) < 0)
                                bug("Nanny: cannot find racial language.");
                        else
                        {
                                ch->pcdata->learned[iLang] = 100;
                                ch->speaking = ch->race->language();
                        }

                        /*
                         * ch->resist           += race_table[ch->race].resist;    drats 
                         */
                        /*
                         * ch->susceptible     += race_table[ch->race].suscept;    drats 
                         */
                        reset_colors(ch);
                        name_stamp_stats(ch);

                        {
                                int       ability;

                                for (ability = 0; ability < MAX_ABILITY;
                                     ability++)
                                {
                                        ch->skill_level[ability] = 1;
                                        ch->experience[ability] = 0;
                                }

                        }

                        ch->bonus_str = 0;
                        ch->bonus_dex = 0;
                        ch->bonus_wis = 0;
                        ch->bonus_int = 0;
                        ch->bonus_con = 0;
                        ch->bonus_cha = 0;
                        ch->bonus_frc = 0;
                        ch->bonus_lck = 0;
                        ch->top_level = 1;
                        ch->hit = ch->max_hit;
                        ch->max_hit += ch->race->hit();
                        if (ch->perm_frc > 0)
                                ch->max_endurance = 700;
                        else
                                ch->max_endurance = 500;
                        ch->endurance = ch->max_endurance;
                        snprintf(buf, MSL, "%s the %s", ch->name,
                                 ch->race->name());
                        set_title(ch, buf);
                        {
                                CHANNEL_DATA *channel;

                                for (channel = first_channel; channel;
                                     channel = channel->next)
                                {
                                        if (ch->top_level >=
                                            channel->level
                                            && !hasname(ch->pcdata->
                                                        listening,
                                                        channel->name))
                                                addname(&ch->pcdata->
                                                        listening,
                                                        channel->name);
                                }
                        }

                        /*
                         * Added by Narn.  Start new characters with autoexit and autgold
                         * * already turned on.  Very few people don't use those. 
                         */
                        SET_BIT(ch->act, PLR_AUTOGOLD);
                        SET_BIT(ch->act, PLR_AUTOEXIT);

                        /*
                         * New players don't have to earn some eq 
                         */

                        obj = create_object(get_obj_index
                                            (OBJ_VNUM_SCHOOL_BANNER), 0);
                        obj_to_char(obj, ch);
                        equip_char(ch, obj, WEAR_LIGHT);

                        /*
                         * armor they do though
                         * * obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );
                         * * obj_to_char( obj, ch );
                         * * equip_char( ch, obj, WEAR_BODY );
                         * * 
                         * * obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
                         * * obj_to_char( obj, ch );
                         * * equip_char( ch, obj, WEAR_SHIELD );
                         * * 
                         */

                        obj = create_object(get_obj_index
                                            (OBJ_VNUM_SCHOOL_DAGGER), 0);
                        obj_to_char(obj, ch);
                        ch->gold = 5000;
                        equip_char(ch, obj, WEAR_WIELD);

                        /*
                         * comlink 
                         */

                        {
                                OBJ_INDEX_DATA *obj_ind =
                                        get_obj_index(10424);
                                if (obj_ind != NULL)
                                {
                                        obj = create_object(obj_ind, 0);
                                        obj_to_char(obj, ch);
                                }
                        }


                        if (!sysdata.WAIT_FOR_AUTH)
                        {
                                char_to_room(ch,
                                             get_room_index
                                             (ROOM_VNUM_SCHOOL));
                                ch->pcdata->auth_state = 3;
                        }
                        else
                        {
                                char_to_room(ch,
                                             get_room_index
                                             (ROOM_VNUM_SCHOOL));
                                ch->pcdata->auth_state = 1;
                                SET_BIT(ch->pcdata->flags, PCFLAG_UNAUTHED);
                        }
                        add_to_account(d->account, ch);
                        if (!ch->pcdata->birthday.hour)
                                ch->pcdata->birthday.hour = time_info.hour;
                        if (!ch->pcdata->birthday.day)
                                ch->pcdata->birthday.day = time_info.day;
                        if (!ch->pcdata->birthday.month)
                                ch->pcdata->birthday.month = time_info.month;
                        if (!ch->pcdata->birthday.year)
                                ch->pcdata->birthday.year = time_info.year;
                        if (ch->desc->msp_detected)
                        {
                                SET_BIT(ch->act, PLR_SOUND);
                        }
                }
                else
                {
                        if (!IS_IMMORTAL(ch)
                            && ch->pcdata->release_date > current_time)
                        {
                                char_to_room(ch, get_room_index(6));
                        }
                        else if (ch->in_room && !IS_IMMORTAL(ch)
                                 && ch->in_room->vnum != 6)
                        {
                                char_to_room(ch, ch->in_room);
                        }
                        else
                        {
                                ch->in_room = get_room_index(wherehome(ch));
                                char_to_room(ch, ch->in_room);
                        }
                }

                if (get_timer(ch, TIMER_SHOVEDRAG) > 0)
                        remove_timer(ch, TIMER_SHOVEDRAG);

                if (get_timer(ch, TIMER_PKILLED) > 0)
                        remove_timer(ch, TIMER_PKILLED);

                if (ch->plr_home != NULL)
                {
                        load_home(ch);
/*                        char      filename[256];
                        FILE     *fph;
                        ROOM_INDEX_DATA *storeroom = ch->plr_home;
                        OBJ_DATA *obj;
                        OBJ_DATA *obj_next;
                        for (obj = storeroom->first_content; obj;
                             obj = obj_next)
                        {
                                obj_next = obj->next_content;
                                extract_obj(obj);
                        }

                        snprintf(filename, MSL, "%s%c/%s.home", PLAYER_DIR,
                                 tolower(ch->name[0]), capitalize(ch->name));
                        if ((fph = fopen(filename, "r")) != NULL)
                        {
                                int       iNest;
                                bool      found;
                                OBJ_DATA *tobj, *tobj_next;

                                rset_supermob(storeroom);
                                for (iNest = 0; iNest < MAX_NEST; iNest++)
                                        rgObjNest[iNest] = NULL;

                                found = TRUE;
                                for (;;)
                                {
                                        char      letter;
                                        char     *word;

                                        letter = fread_letter(fph);
                                        if (letter == '*')
                                        {
                                                fread_to_eol(fph);
                                                continue;
                                        }

                                        if (letter != '#')
                                        {
                                                bug("Load_plr_home: # not found.", 0);
                                                bug(ch->name, 0);
                                                break;
                                        }

                                        word = fread_word(fph);
                                        if (!str_cmp(word, "OBJECT"))   
                                                fread_obj(supermob, fph,
                                                          OS_CARRY);
                                        else if (!str_cmp(word, "END"))
                                                break;
                                        else
                                        {
                                                bug("Load_plr_home: bad section.", 0);
                                                bug(ch->name, 0);
                                                break;
                                        }
                                }

                                FCLOSE(fph);

                                for (tobj = supermob->first_carrying; tobj;
                                     tobj = tobj_next)
                                {
                                        tobj_next = tobj->next_content;
                                        obj_from_char(tobj);
                                        obj_to_room(tobj, storeroom);
                                }
                                release_supermob();

                        } */
                }
                if (!IS_SET(ch->act, PLR_WIZINVIS))
                {
                        snprintf(buf, MSL, "%s has entered %s", ch->name,
                                 sysdata.mud_name);
                        info_chan(buf);
                }
                act(AT_ACTION, "$n has entered the game.", ch, NULL, NULL,
                    TO_ROOM);
                do_look(ch, "auto");
                mail_count(ch);
                if (ch->top_level > 1)
                {
                        char      motdbuf[MAX_STRING_LENGTH];
                        FILE     *fp;

                        snprintf(motdbuf, MSL, "%s%s", MAIL_DIR,
                                 capitalize(ch->name));
                        if ((fp = fopen(motdbuf, "r")) != NULL)
                        {
                                SET_BIT(ch->pcdata->flags, PCFLAG_GOTMAIL);
                                FCLOSE(fp);
                        }
                }
#ifdef ACCOUNT
                if (IS_SET(ch->act, PLR_SOUND) && ch->pcdata->account)
                {
                        SET_BIT(ch->pcdata->account->flags, ACCOUNT_SOUND);
                        REMOVE_BIT(ch->act, PLR_SOUND);
                }
#endif
                break;

        }
        return;
}


/*
 * Parse a name for acceptability.
 */
bool check_parse_name(char *name)
{
        /*
         * Reserved words.
         */
        if (is_name
            (name,
             "all auto someone immortal self god supreme demigod dog guard cityguard cat cornholio spock hicaine hithoric death ass fuck shit piss crap quit public link new"))
                return FALSE;

        if (is_reserved_name(name))
                return FALSE;
        /*
         * Length restrictions.
         */
        if (strlen(name) < 3)
                return FALSE;

        if (strlen(name) > 12)
                return FALSE;

        /*
         * Alphanumerics only.
         * Lock out IllIll twits.
         */
        {
                char     *pc;
                bool      fIll;

                fIll = TRUE;
                for (pc = name; *pc != '\0'; pc++)
                {
                        if (!isalpha(*pc))
                                return FALSE;
                        if (LOWER(*pc) != 'i' && LOWER(*pc) != 'l')
                                fIll = FALSE;
                }

                if (fIll)
                        return FALSE;
        }

        /*
         * Code that followed here used to prevent players from naming
         * themselves after mobs... this caused much havoc when new areas
         * would go in...
         */

        return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect(DESCRIPTOR_DATA * d, char *name, bool fConn)
{
        CHAR_DATA *ch;

        for (ch = first_char; ch; ch = ch->next)
        {
                if (!IS_NPC(ch)
                    && (!fConn || !ch->desc)
                    && ch->name && !str_cmp(name, ch->name))
                {
                        if (check_multi(d, ch->name))
                        {
                                close_socket(d, FALSE);
                                break;
                        }
                        if (fConn && ch->switched)
                        {
                                write_to_buffer(d,
                                                "Already playing.\n\rName: ",
                                                0);
                                d->connected = CON_GET_NAME;
                                if (d->character)
                                {
                                        /*
                                         * clear descriptor pointer to get rid of bug message in log 
                                         */
                                        d->character->desc = NULL;
                                        free_char(d->character);
                                        d->character = NULL;
                                }
                                return BERR;
                        }
                        if (fConn == FALSE && d->character)
                        {
                                DISPOSE(d->character->pcdata->pwd);
                                d->character->pcdata->pwd =
                                        str_dup(ch->pcdata->pwd);
                        }
                        else
                        {
                                /*
                                 * clear descriptor pointer to get rid of bug message in log 
                                 */
                                if (d->character)
                                {
                                        d->character->desc = NULL;
                                        free_char(d->character);
                                }
                                d->character = ch;
                                ch->desc = d;
                                ch->timer = 0;
                                send_to_char("Reconnecting.\n\r", ch);
                                act(AT_ACTION, "$n has reconnected.", ch,
                                    NULL, NULL, TO_ROOM);
                                snprintf(log_buf, MSL, "%s@%s reconnected.",
                                         ch->name, d->host);
                                log_string_plus(log_buf, LOG_COMM,
                                                UMAX(sysdata.log_level,
                                                     ch->top_level));
#ifdef ACCOUNT
                                ch->pcdata->account = d->account;
                                ch->pcdata->account->inuse--;
#endif
                                d->connected = CON_PLAYING;
                        }
                        return TRUE;
                }
        }

        return FALSE;
}



/*
 * Check if already playing.
 */

bool check_multi(DESCRIPTOR_DATA * d, char *name)
{
        DESCRIPTOR_DATA *dold;

        for (dold = first_descriptor; dold; dold = dold->next)
        {
                if (dold != d && (dold->character || dold->original)
                    && str_cmp(name,
                               dold->original ? dold->original->name : dold->
                               character->name)
                    && !str_cmp(dold->host, d->host))
                {
                        const char *ok = "194.234.177";
                        const char *ok2 = "209.183.133.229";
                        int       iloop;

                        if (get_trust(d->character) >= LEVEL_GREATER
                            || get_trust(dold->original ? dold->
                                         original : dold->character) >=
                            LEVEL_GREATER)
                                return FALSE;
                        for (iloop = 0; iloop < 11; iloop++)
                        {
                                if (ok[iloop] != d->host[iloop])
                                        break;
                        }
                        if (iloop >= 10)
                                return FALSE;
                        for (iloop = 0; iloop < 11; iloop++)
                        {
                                if (ok2[iloop] != d->host[iloop])
                                        break;
                        }
                        if (iloop >= 10)
                                return FALSE;
                        write_to_buffer(d,
                                        "Sorry multi-playing is not allowed ... have you other character quit.\n\r",
                                        0);
                        snprintf(log_buf, MSL,
                                 "%s attempting to multiplay with %s.",
                                 dold->original ? dold->original->
                                 name : dold->character->name,
                                 d->character->name);
                        log_string_plus(log_buf, LOG_COMM, sysdata.log_level);
                        d->character = NULL;
                        free_char(d->character);
                        return TRUE;
                }
        }

        return FALSE;

}

sh_int check_playing(DESCRIPTOR_DATA * d, char *name, bool kick)
{
        CHAR_DATA *ch;

        DESCRIPTOR_DATA *dold;
        int       cstate;

        for (dold = first_descriptor; dold; dold = dold->next)
        {
                if (dold != d
                    && (dold->character || dold->original)
                    && !str_cmp(name, dold->original
                                ? dold->original->name : dold->character->
                                name))
                {
                        cstate = dold->connected;
                        ch = dold->original ? dold->original : dold->
                                character;
                        if (!ch->name
                            || (cstate != CON_PLAYING
                                && cstate != CON_EDITING))
                        {
                                write_to_buffer(d,
                                                "Already connected - try again.\n\r",
                                                0);
                                snprintf(log_buf, MSL,
                                         "%s already connected.", ch->name);
                                log_string_plus(log_buf, LOG_COMM,
                                                sysdata.log_level);
                                return BERR;
                        }
                        if (!kick)
                                return TRUE;
                        write_to_buffer(d,
                                        "Already playing... Kicking off old connection.\n\r",
                                        0);
                        write_to_buffer(dold,
                                        "Kicking off old connection... bye!\n\r",
                                        0);
                        close_socket(dold, FALSE);
                        /*
                         * clear descriptor pointer to get rid of bug message in log 
                         */
                        d->character->desc = NULL;
                        free_char(d->character);
                        d->character = ch;
                        ch->desc = d;
                        ch->timer = 0;
                        if (ch->switched)
                                do_return(ch->switched, "");
                        ch->switched = NULL;
                        ch->pcdata->account->inuse--;
                        send_to_char("Reconnecting.\n\r", ch);
                        act(AT_ACTION,
                            "$n has reconnected, kicking off old link.", ch,
                            NULL, NULL, TO_ROOM);
                        snprintf(log_buf, MSL,
                                 "%s@%s reconnected, kicking off old link.",
                                 ch->name, d->host);
                        log_string_plus(log_buf, LOG_COMM,
                                        UMAX(sysdata.log_level,
                                             ch->top_level));
                        d->connected = cstate;
                        return TRUE;
                }
        }

        return FALSE;
}



void stop_idling(CHAR_DATA * ch)
{
        if (!ch
            || !ch->desc
            || ch->desc->connected != CON_PLAYING
            || !ch->was_in_room
            || ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
                return;

        ch->timer = 0;
        char_from_room(ch);
        char_to_room(ch, ch->was_in_room);
        ch->was_in_room = NULL;
        act(AT_ACTION, "$n has returned from the void.", ch, NULL, NULL,
            TO_ROOM);
        return;
}



void center_to_char(char *argument, CHAR_DATA * ch, int columns)
{
        char      centered[MAX_INPUT_LENGTH];
        int       spaces;

        columns = (columns < 2) ? 80 : columns;
        spaces = (columns - strlen(argument)) / 2;

        snprintf(centered, MSL, "%*c%s", spaces, ' ', argument);
        send_to_char(centered, ch);

        return;
}

char     *center_str(char *argument, int columns)
{
        static char centered[MAX_INPUT_LENGTH];
        int       spaces;

        columns = (columns < 2) ? 80 : columns;
        spaces = (columns - strlen(argument)) / 2;

        snprintf(centered, MSL, "%*c%s%*c", spaces, ' ', argument, spaces,
                 ' ');
        return centered;
}

void log_printf(char *fmt, ...)
{
        char      buf[MAX_STRING_LENGTH * 2];
        va_list   args;

        va_start(args, fmt);
        vsnprintf(buf, MSL, fmt, args);
        va_end(args);

        log_string(buf);
}


char     *obj_short(OBJ_DATA * obj)
{
        static char buf[MAX_STRING_LENGTH];

        if (obj->count > 1)
        {
                snprintf(buf, MSL, "%s (%d)", obj->short_descr, obj->count);
                return buf;
        }
        return obj->short_descr;
}

/*
 * The primary output interface for formatted output.
 */
/* Major overhaul. -- Alty */
#define NAME(ch)	(IS_NPC((ch)) ? (ch)->short_descr : (ch)->name)
#define NAME2(ch)	(IS_NPC((ch)) ? (ch)->short_descr : (ch)->pcdata->full_name )
#define CAN_SEE(ch, vict) ( (OOC && can_see_ooc((vict), (ch))) || (!OOC && can_see((vict), (ch))))
char     *act_string(const char *format, CHAR_DATA * to, CHAR_DATA * ch,
                     void *arg1, void *arg2, bool OOC)
{
        static char *const he_she[] = { "it", "he", "she" };
        static char *const him_her[] = { "it", "him", "her" };
        static char *const his_her[] = { "its", "his", "her" };
        static char buf[MAX_STRING_LENGTH];
        char      fname[MAX_INPUT_LENGTH];
        char     *point = buf;
        const char *str = format;
        const char *i;
        int       room = 0;
        CHAR_DATA *vch = (CHAR_DATA *) arg2;
        OBJ_DATA *obj1 = (OBJ_DATA *) arg1;
        OBJ_DATA *obj2 = (OBJ_DATA *) arg2;
        SHIP_DATA *ship = NULL;

        if (str == NULL or * str == '\0')
                return "";

        while (*str != '\0')
        {
                if (*str != '$')
                {
                        *point++ = *str++;
                        continue;
                }
                ++str;
                if (!arg2 && *str >= 'A' && *str <= 'Z')
                {
                        bug("Act: missing arg2 for code %c:", *str);
                        bug(format);
                        i = " <@@@> ";
                }
                else
                {
                        switch (*str)
                        {
                        default:
                                bug("Act: bad code %c.", *str);
                                i = " <@@@> ";
                                break;
                        case 'x':
                                room = ch->in_room ? ch->in_room->vnum : -1;
                                ship = ship_from_room(room);
                                i = ship ? ship->name : "";
                                break;
                        case 't':
                                i = (char *) arg1;
                                break;
                        case 'T':
                                i = (char *) arg2;
                                break;
                        case 'g':
                                i = (can_see_ooc(to, ch) ? NAME(ch) :
                                     "Someone");
                                break;
                        case 'G':
                                i = (can_see_ooc(to, vch) ? NAME(vch) :
                                     "Someone");
                                break;
                        case 'n':
                                {
                                        if (!to)
                                        {
                                                i =  get_char_desc(ch, to);
                                                break;
                                        }
                                        if (CAN_SEE(ch, to))
                                                i = (OOC ? NAME(ch) : get_char_desc(ch, to) );
                                        else if (OOC && IS_IMMORTAL(ch))
                                                i = "An Immortal";
                                        else
                                                i = "Someone";
                                        break;
                                }
                        case 'N':
                                {
                                        if (!to)
                                        {
                                                i =  get_char_desc(vch, NULL);
                                                break;
                                        }
                                        if (CAN_SEE(vch, to))
                                                i = (OOC ? NAME(vch) : get_char_desc(vch, to) );
                                        else if (OOC && IS_IMMORTAL(vch))
                                                i = "An Immortal";
                                        else
                                                i = "Someone";
                                        break;
                                }
                        case 'a':
                                {
                                        i = npc_sex[ch->sex];
                                        break;
                                }
                        case 'e':
                                if (ch->sex > 2 || ch->sex < 0)
                                {
                                        bug("act_string: player %s has sex set at %d!", ch->name, ch->sex);
                                        i = "it";
                                }
                                else
                                        i = he_she[URANGE(0, ch->sex, 2)];
                                break;
                        case 'E':
                                if (vch->sex > 2 || vch->sex < 0)
                                {
                                        bug("act_string: player %s has sex set at %d!", vch->name, vch->sex);
                                        i = "it";
                                }
                                else
                                        i = he_she[URANGE(0, vch->sex, 2)];
                                break;
                        case 'm':
                                if (ch->sex > 2 || ch->sex < 0)
                                {
                                        bug("act_string: player %s has sex set at %d!", ch->name, ch->sex);
                                        i = "it";
                                }
                                else
                                        i = him_her[URANGE(0, ch->sex, 2)];
                                break;
                        case 'M':
                                if (vch->sex > 2 || vch->sex < 0)
                                {
                                        bug("act_string: player %s has sex set at %d!", vch->name, vch->sex);
                                        i = "it";
                                }
                                else
                                        i = him_her[URANGE(0, vch->sex, 2)];
                                break;
                        case 's':
                                if (ch->sex > 2 || ch->sex < 0)
                                {
                                        bug("act_string: player %s has sex set at %d!", ch->name, ch->sex);
                                        i = "its";
                                }
                                else
                                        i = his_her[URANGE(0, ch->sex, 2)];
                                break;
                        case 'S':
                                if (vch->sex > 2 || vch->sex < 0)
                                {
                                        bug("act_string: player %s has sex set at %d!", vch->name, vch->sex);
                                        i = "its";
                                }
                                else
                                        i = his_her[URANGE(0, vch->sex, 2)];
                                break;
                        case 'q':
                                i = (to == ch) ? "" : "s";
                                break;
                        case 'Q':
                                i = (to == ch) ? "your" :
                                        his_her[URANGE(0, ch->sex, 2)];
                                break;
                        case 'p':
                                i = (!to || can_see_obj(to, obj1)
                                     ? obj_short(obj1) : "something");
                                break;
                        case 'P':
                                i = (!to || can_see_obj(to, obj2)
                                     ? obj_short(obj2) : "something");
                                break;
                        case 'd':
                                if (!arg2 || ((char *) arg2)[0] == '\0')
                                        i = "door";
                                else
                                {
                                        one_argument((char *) arg2, fname);
                                        i = fname;
                                }
                                break;
                        }
                }
                ++str;
                while ((*point = *i) != '\0')
                        ++point, ++i;
        }
        mudstrlcpy(point, "\n\r", MSL);
        buf[0] = UPPER(buf[0]);
        return buf;
}

#undef NAME
#undef NAME2

void act(sh_int AType, const char *format, CHAR_DATA * ch, void *arg1,
         void *arg2, int type)
{
        char     *txt;
        CHAR_DATA *to;
        CHAR_DATA *vch = (CHAR_DATA *) arg2;
        bool      OOC = IS_OOC_ACT(type);

        if (OOC)
                type -= OOC_TO;
        /*
         * Discard null and zero-length messages.
         */
        if (!format || format[0] == '\0')
                return;

        if (!ch)
        {
                bug("Act: null ch. (%s)", format);
                return;
        }

        if (!ch->in_room)
                to = NULL;
        else if (type == TO_CHAR)
                to = ch;
        else if (type == TO_MUD)
                to = first_char;
        else
                to = ch->in_room->first_person;


        /*
         * ACT_SECRETIVE and PLR_SECRETIVE handling
         */
        if (IS_NPC(ch) && IS_SET(ch->act, ACT_SECRETIVE) && type != TO_CHAR
            && !OOC)
                return;

        if (!IS_NPC(ch)
            && (IS_SET(ch->act, PLR_SECRETIVE)
                || IS_AFFECTED(ch, AFF_SECRETIVE)) && type != TO_CHAR && !OOC)
                return;

        if (type == TO_VICT)
        {
                if (!vch)
                {
                        bug("Act: null vch with TO_VICT.");
                        bug("%s (%s)", ch->name, format);
                        return;
                }
                if (!vch->in_room)
                {
                        bug("Act: vch in NULL room!");
                        bug("%s -> %s (%s)", ch->name, vch->name, format);
                        return;
                }
                to = vch;
        }

        if (MOBtrigger && type != TO_CHAR && type != TO_VICT && to
            && !IS_OOC_ACT(type))
        {
                OBJ_DATA *to_obj;

                txt = act_string(format, NULL, ch, arg1, arg2, OOC);
                if (!to->in_room)
                        return;
                if (IS_SET(to->in_room->progtypes, ACT_PROG))
                        rprog_act_trigger(txt, to->in_room, ch,
                                          (OBJ_DATA *) arg1, (void *) arg2);
                for (to_obj = to->in_room->first_content; to_obj;
                     to_obj = to_obj->next_content)
                        if (IS_SET(to_obj->pIndexData->progtypes, ACT_PROG))
                                oprog_act_trigger(txt, to_obj, ch,
                                                  (OBJ_DATA *) arg1,
                                                  (void *) arg2);
        }

        /*
         * Anyone feel like telling me the point of looping through the whole
         * * room when we're only sending to one char anyways..? -- Alty 
         */

        for (; to;
             to = (type == TO_MUD) ? to->next : (type == TO_CHAR
                                                 || type ==
                                                 TO_VICT) ? NULL : to->
             next_in_room)
        {
                if ((!to->desc
                     && (IS_NPC(to)
                         && !IS_SET(to->pIndexData->progtypes, ACT_PROG)))
                    || (!OOC && !IS_AWAKE(to)))
                        continue;

                if (type == TO_CHAR && to != ch)
                        continue;
                if (type == TO_VICT && (to != vch || to == ch))
                        continue;
                if (type == TO_ROOM && to == ch)
                        continue;
                if (type == TO_NOTVICT && (to == ch || to == vch))
                        continue;
                if (type == TO_MUD && (to == ch || to == vch))
                        continue;

                /*
                 * Hear no evil, see no evil, do no evil 
                 */
                if (is_ignoring(ch, to))
                        continue;

                txt = act_string(format, to, ch, arg1, arg2, OOC);
                if (to->desc && !is_ignoring(to, ch))
                {
                        set_char_color(AType, to);
                        send_to_char(txt, to);
                }
                if (MOBtrigger && !IS_OOC_ACT(type))
                {
                        /*
                         * Note: use original string, not string with ANSI. -- Alty 
                         */
                        mprog_act_trigger(txt, to, ch, (OBJ_DATA *) arg1,
                                          (void *) arg2);
                }
        }
        MOBtrigger = TRUE;
        return;
}

CMDF do_name(CHAR_DATA * ch, char *argument)
{
        char      fname[1024];
        struct stat fst;
        CHAR_DATA *tmp;

        if (!NOT_AUTHED(ch) || ch->pcdata->auth_state != 2)
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument[0] = UPPER(argument[0]);

        if (!check_parse_name(argument))
        {
                send_to_char("Illegal name, try another.\n\r", ch);
                return;
        }

        if (!str_cmp(ch->name, argument))
        {
                send_to_char("That's already your name!\n\r", ch);
                return;
        }

        for (tmp = first_char; tmp; tmp = tmp->next)
        {
                if (!str_cmp(argument, tmp->name))
                        break;
        }

        if (tmp || char_exists(argument))
        {
                send_to_char
                        ("That name is already taken.  Please choose another.\n\r",
                         ch);
                return;
        }

        snprintf(fname, MSL, "%s%c/%s", PLAYER_DIR, tolower(argument[0]),
                 capitalize(argument));
        if (stat(fname, &fst) != -1)
        {
                send_to_char
                        ("That name is already taken.  Please choose another.\n\r",
                         ch);
                return;
        }

        STRFREE(ch->name);
        ch->name = STRALLOC(argument);
        STRFREE(ch->pcdata->full_name);
        ch->pcdata->full_name = STRALLOC(argument);
        send_to_char("Your name has been changed.  Please apply again.\n\r",
                     ch);
        ch->pcdata->auth_state = 1;
        SET_BIT(ch->pcdata->flags, PCFLAG_UNAUTHED);
        return;
}

char     *default_prompt(CHAR_DATA * ch)
{
        static char buf[MAX_STRING_LENGTH];

        mudstrlcpy(buf, "", MSL);
        if (ch->skill_level[FORCE_ABILITY] > 1
            || get_trust(ch) >= LEVEL_IMMORTAL)
                mudstrlcat(buf, "&pAlign:&P%a ", MSL);
        mudstrlcat(buf, "&BHealth:&C%h&B/%H  &pEndurance:&P%m/&p%M", MSL);
        mudstrlcat(buf, "&C > &w", MSL);
        return buf;
}

char     *gav_prompt(CHAR_DATA * ch)
{
        static char buf[MAX_STRING_LENGTH];

        ch = NULL;

        mudstrlcpy(buf,
                   "&B[&Y%T&B] &BH&zealth&B: &c%h&B/&C%H &BE&zndurance&B: &c%v&B/&C%V",
                   MSL);

        mudstrlcat(buf, "&C> &w", MSL);
        /*
         * mudstrlcpy( buf,"",MSL);
         * if (ch->skill_level[FORCE_ABILITY] > 1 || get_trust(ch) >= LEVEL_IMMORTAL )
         * mudstrlcat(buf, "&pForce:&P%m/&p%M  &pAlign:&P%a\n\r", MSL);      
         * mudstrlcat(buf, "&B<Health>&C%h&B/%H  &B<Movement>&C%v&B/%V", MSL);
         * mudstrlcat(buf, " &YGold:&O%g", MSL);
         * mudstrlcat(buf, "&C > &w", MSL);
         */
        return buf;
}

int getcolor(char clr)
{
        static const char colors[17] = "xrgObpcwzRGYBPCW";
        int       r;

        for (r = 0; r < 16; r++)
                if (clr == colors[r])
                        return r;
        return -1;
}

void display_prompt(DESCRIPTOR_DATA * d)
{
        CHAR_DATA *ch = d->character;
        CHAR_DATA *och = (d->original ? d->original : d->character);
        CHAR_DATA *victim;
        bool      ansi = (!IS_NPC(och) && IS_SET(och->act, PLR_ANSI));
        const char *prompt;
        char      buf[MAX_STRING_LENGTH];
        char     *pbuf = buf;
        int       pstat, percent;
		static char * no_email_prompt = "Please set your email using setself realemail <your email address>";


        if (!ch)
        {
                bug("display_prompt: NULL ch");
                return;
        }

        if (IS_MXP(ch))
                send_to_char(MXPTAG("Prompt"), ch);

        if (!IS_NPC(ch) && ch->substate != SUB_NONE && ch->pcdata->subprompt
            && ch->pcdata->subprompt[0] != '\0')
                prompt = ch->pcdata->subprompt;
        else if (IS_NPC(ch))
                prompt = gav_prompt(ch);
		else if (ch->pcdata->account && (!ch->pcdata->account->email || ch->pcdata->account->email[0] == '\0'))
				prompt = no_email_prompt;
        else if (ch->fighting && ch->pcdata->fprompt
                 && ch->pcdata->fprompt[0] != '\0')
                prompt = ch->pcdata->fprompt;
        else if (!ch->pcdata->prompt || !*ch->pcdata->prompt)
                prompt = gav_prompt(ch);
        else if (ch->pcdata && !str_cmp(ch->pcdata->prompt, "old_default"))
                prompt = default_prompt(ch);
        else
                prompt = ch->pcdata->prompt;

        if (ansi)
        {
                mudstrlcpy(pbuf, ANSI_RESET, MSL);
                d->prevcolor = 0x08;
                pbuf += 4;
        }
        /*
         * Clear out old color stuff 
         */
/*  make_color_sequence(NULL, NULL, NULL);*/
        for (; *prompt; prompt++)
        {
                /*
                 * '%' = prompt commands
                 * Note: foreground changes will revert background to 0 (black)
                 */
                if (*prompt != '%')
                {
                        *(pbuf++) = *prompt;
                        continue;
                }
                ++prompt;
                if (!*prompt)
                        break;
                if (*prompt == *(prompt - 1))
                {
                        *(pbuf++) = *prompt;
                        continue;
                }
                switch (*(prompt - 1))
                {
                default:
                        bug("Display_prompt: bad command char '%c'.",
                            *(prompt - 1));
                        break;
                case '%':
                        *pbuf = '\0';
                        pstat = 65543;
                        switch (*prompt)
                        {
                        case '%':
                                *pbuf++ = '%';
                                *pbuf = '\0';
                                break;
                        case 'a':
                                if (ch->top_level >= 10)
                                        pstat = ch->alignment;
                                else if (IS_GOOD(ch))
                                        mudstrlcpy(pbuf, "good", MSL);
                                else if (IS_EVIL(ch))
                                        mudstrlcpy(pbuf, "evil", MSL);
                                else
                                        mudstrlcpy(pbuf, "neutral", MSL);
                                break;
                        case 'b': /* Combat XP needed till next level */
                                snprintf(pbuf, MSL, "%d", (exp_level(ch->skill_level[COMBAT_ABILITY] + 1) - ch->experience[COMBAT_ABILITY]));                                
                                break;
                        case 'C':
                                if (ch->max_hit > 0)
                                        percent =
                                                (100 * ch->hit) / ch->max_hit;
                                else
                                        percent = -1;
                                if (percent >= 60)
                                        snprintf(pbuf, MSL, "&G%d&w",
                                                 percent);
                                else if (percent >= 40 && percent < 60)
                                        snprintf(pbuf, MSL, "&Y%d&w",
                                                 percent);
                                else if (percent >= 20 && percent < 40)
                                        snprintf(pbuf, MSL, "&B%d&w",
                                                 percent);
                                else
                                        snprintf(pbuf, MSL, "&R%d&w",
                                                 percent);
                                break;

                        case 'c':
                                if ((victim = who_fighting(ch)) != NULL)
                                {
                                        if (victim->max_hit > 0)
                                                percent =
                                                        (100 * victim->hit) /
                                                        victim->max_hit;
                                        else
                                                percent = -1;
                                        if (percent >= 60)
                                                snprintf(pbuf, MSL,
                                                         "&G%d&w",
                                                         percent);
                                        else if (percent >= 40
                                                 && percent < 60)
                                                snprintf(pbuf, MSL,
                                                         "&Y%d&w",
                                                         percent);
                                        else if (percent >= 20
                                                 && percent < 40)
                                                snprintf(pbuf, MSL,
                                                         "&B%d&w",
                                                         percent);
                                        else
                                                snprintf(pbuf, MSL,
                                                         "&R%d&w",
                                                         percent);
                                }
                                break;

                        case 'e':
                                if ((victim = who_fighting(ch)) != NULL)
                                {
                                        if (victim->max_hit > 0)
                                                percent =
                                                        (100 * victim->hit) /
                                                        victim->max_hit;
                                        else
                                                percent = -1;
                                        if (percent >= 100)
                                                snprintf(pbuf, MSL,
                                                         "[&R||&B||&Y||&G||||&w]");
                                        else if (percent >= 90)
                                                snprintf(pbuf, MSL,
                                                         "[&R||&B||&Y||&G|||&w ]");
                                        else if (percent >= 80)
                                                snprintf(pbuf, MSL,
                                                         "[&R||&B||&Y||&G||&w  ]");
                                        else if (percent >= 70)
                                                snprintf(pbuf, MSL,
                                                         "[&R||&B||&Y||&G|&w   ]");
                                        else if (percent >= 60)
                                                snprintf(pbuf, MSL,
                                                         "[&R||&B||&Y||&G&w    ]");
                                        else if (percent >= 50)
                                                snprintf(pbuf, MSL,
                                                         "[&R||&B||&Y|&G&w      ]");
                                        else if (percent >= 40)
                                                snprintf(pbuf, MSL,
                                                         "[&R||&B||&Y&G&w       ]");
                                        else if (percent >= 30)
                                                snprintf(pbuf, MSL,
                                                         "[&R||&B|&Y&G&w        ]");
                                        else if (percent >= 20)
                                                snprintf(pbuf, MSL,
                                                         "[&R||&B&Y&G&w         ]");
                                        else if (percent >= 10)
                                                snprintf(pbuf, MSL,
                                                         "[&R|&B&Y&G&w          ]");
                                        else
                                                snprintf(pbuf, MSL,
                                                         "[&R    %d%%&w    ]",
                                                         percent);
                                }
                                break;

                        case 'E':
                                if (ch->max_hit > 0)
                                        percent =
                                                (100 * ch->hit) / ch->max_hit;
                                else
                                        percent = -1;
                                if (percent >= 100)
                                        snprintf(pbuf, MSL,
                                                 "[&R||&B||&Y||&G||||&w]");
                                else if (percent >= 90)
                                        snprintf(pbuf, MSL,
                                                 "[&R||&B||&Y||&G|||&w ]");
                                else if (percent >= 80)
                                        snprintf(pbuf, MSL,
                                                 "[&R||&B||&Y||&G||&w  ]");
                                else if (percent >= 70)
                                        snprintf(pbuf, MSL,
                                                 "[&R||&B||&Y||&G|&w   ]");
                                else if (percent >= 60)
                                        snprintf(pbuf, MSL,
                                                 "[&R||&B||&Y||&G&w    ]");
                                else if (percent >= 50)
                                        snprintf(pbuf, MSL,
                                                 "[&R||&B||&Y|&G&w      ]");
                                else if (percent >= 40)
                                        snprintf(pbuf, MSL,
                                                 "[&R||&B||&Y&G&w       ]");
                                else if (percent >= 30)
                                        snprintf(pbuf, MSL,
                                                 "[&R||&B|&Y&G&w        ]");
                                else if (percent >= 20)
                                        snprintf(pbuf, MSL,
                                                 "[&R||&B&Y&G&w         ]");
                                else if (percent >= 10)
                                        snprintf(pbuf, MSL,
                                                 "[&R|&B&Y&G&w          ]");
                                else
                                        snprintf(pbuf, MSL,
                                                 "[&R    %d%%&w    ]",
                                                 percent);
                                break;
                        case 'h':
                                pstat = ch->hit;
                                break;
                        case 'H':
                                pstat = ch->max_hit;
                                break;
                        case '_':
                                mudstrlcpy(pbuf, "\n\r", MSL);
                                break;
                        case 'T':
                                if (time_info.hour < 5)
                                        mudstrlcpy(pbuf, "night", MSL);
                                else if (time_info.hour < 6)
                                        mudstrlcpy(pbuf, "dawn", MSL);
                                else if (time_info.hour < 19)
                                        mudstrlcpy(pbuf, "day", MSL);
                                else if (time_info.hour < 21)
                                        mudstrlcpy(pbuf, "dusk", MSL);
                                else
                                        mudstrlcpy(pbuf, "night", MSL);
                                break;
                        case 'u':
                                {
                                        sh_int    count = 0;

                                        for (d = first_descriptor; d;
                                             d = d->next)
                                                if (d->connected ==
                                                    CON_PLAYING
                                                    && d->character
                                                    && !IS_SET(d->character->
                                                               act,
                                                               PLR_WIZINVIS))
                                                        count++;
                                        pstat = count;
                                }
                                break;
                        case 'U':
                                pstat = sysdata.maxplayers;
                                break;
                        case 'v':
                                pstat = ch->endurance;
                                break;
                        case 'V':
                                pstat = ch->max_endurance;
                                break;
                        case 'm':
                                pstat = ch->endurance;
                                break;
                        case 'M':
                                pstat = ch->max_endurance;
                                break;
                        case 'g':
                                pstat = ch->gold;
                                break;
                        case 'r':
                                if (IS_IMMORTAL(och))
                                        pstat = ch->in_room->vnum;
                                break;
                        case 'R':
                                if (IS_SET(och->act, PLR_ROOMVNUM))
                                        snprintf(pbuf, MSL, "<#%d> ",
                                                 ch->in_room->vnum);
                                break;
                        case 'i':
                                if ((!IS_NPC(ch)
                                     && IS_SET(ch->act, PLR_WIZINVIS))
                                    || (IS_NPC(ch)
                                        && IS_SET(ch->act, ACT_MOBINVIS)))
                                        snprintf(pbuf, MSL, "(Invis %d) ",
                                                 (IS_NPC(ch) ? ch->
                                                  mobinvis : ch->pcdata->
                                                  wizinvis));
                                else if (IS_AFFECTED(ch, AFF_INVISIBLE))
                                        snprintf(pbuf, MSL, "%s", "(Invis) ");
                                break;
                        case 'I':
                                pstat = (IS_NPC(ch)
                                         ? (IS_SET(ch->act, ACT_MOBINVIS) ?
                                            ch->mobinvis : 0) : (IS_SET(ch->
                                                                        act,
                                                                        PLR_WIZINVIS)
                                                                 ? ch->
                                                                 pcdata->
                                                                 wizinvis :
                                                                 0));
                                break;
                        }
                        if (pstat != 65543)
                                snprintf(pbuf, MSL, "%d", pstat);
                        pbuf += strlen(pbuf);
                        break;
                }
        }
        *pbuf = '\0';
        send_to_char(buf, ch);
        if (IS_MXP(ch))
                send_to_char(MXPTAG("/Prompt"), ch);
        return;
}


void set_pager_input(DESCRIPTOR_DATA * d, char *argument)
{
        while (isspace(*argument))
                argument++;
        d->pagecmd = *argument;
        return;
}

bool pager_output(DESCRIPTOR_DATA * d)
{
        register char *last;
        CHAR_DATA *ch;
        int       pclines;
        register int lines;
        bool      ret;

        if (!d || !d->pagepoint || d->pagecmd == -1)
                return TRUE;
        ch = d->original ? d->original : d->character;
        pclines = UMAX(ch->pcdata->pagerlen, 5) - 1;
        switch (LOWER(d->pagecmd))
        {
        default:
                lines = 0;
                break;
        case 'b':
                lines = -1 - (pclines * 2);
                break;
        case 'r':
                lines = -1 - pclines;
                break;
        case 'q':
                d->pagetop = 0;
                d->pagepoint = NULL;
                flush_buffer(d, TRUE);
                DISPOSE(d->pagebuf);
                d->pagesize = MAX_STRING_LENGTH;
                return TRUE;
        }
        while (lines < 0 && d->pagepoint >= d->pagebuf)
                if (*(--d->pagepoint) == '\n')
                        ++lines;
        if (*d->pagepoint == '\n' && *(++d->pagepoint) == '\r')
                ++d->pagepoint;
        if (d->pagepoint < d->pagebuf)
                d->pagepoint = d->pagebuf;
        for (lines = 0, last = d->pagepoint; lines < pclines; ++last)
                if (!*last)
                        break;
                else if (*last == '\n')
                        ++lines;
        if (*last == '\r')
                ++last;
        if (last != d->pagepoint)
        {
                if (!write_to_descriptor
                    (d->descriptor, d->pagepoint, (last - d->pagepoint)))
                        return FALSE;
                d->pagepoint = last;
        }
        while (isspace(*last))
                ++last;
        if (!*last)
        {
                d->pagetop = 0;
                d->pagepoint = NULL;
                flush_buffer(d, TRUE);
                DISPOSE(d->pagebuf);
                d->pagesize = MAX_STRING_LENGTH;
                return TRUE;
        }
        d->pagecmd = -1;
        if (IS_SET(ch->act, PLR_ANSI))
                if (write_to_descriptor(d->descriptor, ANSI_LBLUE, 0) ==
                    FALSE)
                        return FALSE;
        if ((ret =
             write_to_descriptor(d->descriptor,
                                 "(C)ontinue, (R)efresh, (B)ack, (Q)uit: [C] ",
                                 0)) == FALSE)
                return FALSE;
        if (IS_SET(ch->act, PLR_ANSI))
        {
                char      buf[32];

                snprintf(buf, MSL, "%s", color_str(d->pagecolor, ch));
                ret = write_to_descriptor(d->descriptor, buf, 0);
        }
        return ret;
}


CMDF do_speed(CHAR_DATA * ch, char *argument)
{
        sh_int    speed = atoi(argument);

        if (!ch->desc)
                return;

        if (argument[0] == '\0')
        {
                ch_printf(ch,
                          "Your present speed is a %d, which equates to %d bytes per second.\n\r",
                          ch->desc->speed, client_speed(ch->desc->speed));
                return;
        }

        if (speed > 5 || speed < 0)
        {
                send_to_char("Speed is between 0 and 5.\n\r", ch);
                return;
        }
        ch->desc->speed = speed;
        ch->speed = speed;
        ch_printf(ch,
                  "The MUD will now send output to you at %d bytes per second.\n\r",
                  client_speed(speed));
        if (client_speed(speed) > 2048)
                ch_printf(ch,
                          "You should be aware %d is fast enough to lag you if you have a slow connection.\n\r",
                          client_speed(speed));
        return;
}

sh_int client_speed(sh_int speed)
{
        switch (speed)
        {

        case 1:
                return 512;
        case 2:
                return 1024;
        case 3:
                return 2048;
        case 4:
                return 3584;
        case 5:
                return 5120;
        default:
                break;
        }
        return 512;
}

void show_stat_options(DESCRIPTOR_DATA * d, CHAR_DATA * ch)
{
        char      buf[MSL];

        if (d == NULL || ch == NULL)
                return;

        send_to_desc_color
                ("&z|-----------------------------------------------------------------------|\n\r",
                 d);
        snprintf(buf, MSL,
                 "&z|&B[&wStrength             %-2d&B]&z|&B[&wRacial Bonus        &w%+2d&B]&z|&B[ &wTotal:         &w%-2d&B]&z|\n\r",
                 ch->perm_str, ch->race->attr_modifier(ATTR_STRENGTH),
                 ch->perm_str + ch->race->attr_modifier(ATTR_STRENGTH));
        send_to_desc_color(buf, d);
        snprintf(buf, MSL,
                 "&z|&B[&wWisdom               %-2d&B]&z|&B[&wRacial Bonus        &w%+2d&B]&z|&B[ &wTotal:         &w%-2d&B]&z|\n\r",
                 ch->perm_wis, ch->race->attr_modifier(ATTR_WISDOM),
                 ch->perm_wis + ch->race->attr_modifier(ATTR_WISDOM));
        send_to_desc_color(buf, d);
        snprintf(buf, MSL,
                 "&z|&B[&wIntelligence         %-2d&B]&z|&B[&wRacial Bonus        &w%+2d&B]&z|&B[ &wTotal:         &w%-2d&B]&z|\n\r",
                 ch->perm_int, ch->race->attr_modifier(ATTR_INTELLIGENCE),
                 ch->perm_int + ch->race->attr_modifier(ATTR_INTELLIGENCE));
        send_to_desc_color(buf, d);
        snprintf(buf, MSL,
                 "&z|&B[&wDexterity            %-2d&B]&z|&B[&wRacial Bonus        &w%+2d&B]&z|&B[ &wTotal:         &w%-2d&B]&z|\n\r",
                 ch->perm_dex, ch->race->attr_modifier(ATTR_DEXTERITY),
                 ch->perm_dex + ch->race->attr_modifier(ATTR_DEXTERITY));
        send_to_desc_color(buf, d);
        snprintf(buf, MSL,
                 "&z|&B[&wConstitution         %-2d&B]&z|&B[&wRacial Bonus        &w%+2d&B]&z|&B[ &wTotal:         &w%-2d&B]&z|\n\r",
                 ch->perm_con, ch->race->attr_modifier(ATTR_CONSTITUTION),
                 ch->perm_con + ch->race->attr_modifier(ATTR_CONSTITUTION));
        send_to_desc_color(buf, d);
        snprintf(buf, MSL,
                 "&z|&B[&wCharisma             %-2d&B]&z|&B[&wRacial Bonus        &w%+2d&B]&z|&B[ &wTotal:         &w%-2d&B]&z|\n\r",
                 ch->perm_cha, ch->race->attr_modifier(ATTR_CHARISMA),
                 ch->perm_cha + ch->race->attr_modifier(ATTR_CHARISMA));
        send_to_desc_color(buf, d);
        send_to_desc_color
                ("&z|-----------------------------------------------------------------------|\n\r",
                 d);
        snprintf(buf, MSL,
                 "&z|&B[&wMin: 1         &wMax:  20&B]&z|&B[      &BY&zou have &B[&w%02d&B] &zstat points left.      &B]&z|\n\r",
                 ch->pcdata->statpoints);
        send_to_desc_color(buf, d);
        send_to_desc_color
                ("&z|-----------------------------------------------------------------------|\n\r",
                 d);
}
