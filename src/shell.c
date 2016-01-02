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
 *                                 Pfile Pruning Module                                  *
 ****************************************************************************************/

/* Comment out this define if the child processes throw segfaults */
#define USEGLOB /* Samson 4-16-98 - For new shell command */

#include <unistd.h>
#include <sys/wait.h>   /* Samson 4-16-98 - For new shell command */
#include <fcntl.h>
#include <arpa/telnet.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>

#ifdef USEGLOB  /* Samson 4-16-98 - For new command pipe */
#include <glob.h>
#endif
#include "mud.h"

/* Global variable to protect online compiler code - Samson */
bool      compilelock = FALSE;  /* Reboot/shutdown commands locked during compiles */
bool      bootlock = FALSE; /* Protects compiler from being used during boot timers */

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

#ifndef USEGLOB
/* OLD command shell provided by Ferris - ferris@FootPrints.net Installed by Samson 4-6-98
 * For safety reasons, this is only available if the USEGLOB define is commented out.
 */
/*
 * Local functions.
 */
FILE     *popen args((const char *command, const char *type));
int pclose args((FILE * stream));

#ifdef MCCP
extern const char will_compress_str[];
extern const char will_compress2_str[];
#endif

char     *fgetf(char *s, int n, register FILE * iop)
{
        register int c;
        register char *cs;

        c = '\0';
        cs = s;
        while (--n > 0 && (c = getc(iop)) != EOF)
                if ((*cs++ = c) == '\0')
                        break;
        *cs = '\0';
        return ((c == EOF && cs == s) ? NULL : s);
}

/* NOT recommended to be used as a conventional command! */
void command_pipe(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        FILE     *fp;

#ifdef MCCP
        int       compressing = ch->desc->compressing;

        compressEnd(ch->desc);
#endif

        set_char_color(AT_RED, ch);

        fp = popen(argument, "r");

        fgetf(buf, MAX_STRING_LENGTH, fp);

        send_to_pager(buf, ch);

        pclose(fp);

#ifdef MCCP
        if (compressing)
                compressstart(ch->desc, compressing);
#endif
        return;
}

/* End OLD shell command code */
#endif

/* New command shell code by Thoric - Installed by Samson 4-16-98 */
void send_telcode(int desc, int ddww, int code)
{
        unsigned char buf[4];

        buf[0] = IAC;
        buf[1] = ddww;
        buf[2] = code;
        buf[3] = 0;
        write(desc, (char *) buf, 4);
}

CMDF do_mudexec(CHAR_DATA * ch, char *argument)
{
        int       desc;
        int       flags;
        pid_t     pid;
        bool      iafork = FALSE;

#ifdef MCCP
        int       compressing = 0;
#endif

        if (!ch->desc)
                return;

        if (argument == NULL || argument[0] == '\0')
                return;

        if (strncasecmp(argument, "ia ", 3) == 0)
        {
                argument += 3;
                iafork = TRUE;
        }

        desc = ch->desc->descriptor;

        set_char_color(AT_PLAIN, ch);

#ifdef MCCP
        compressing = ch->desc->compressing;
        compressEnd(ch->desc);
#endif

        if ((pid = fork()) == 0)
        {
                char      buf[1024];
                char     *p = argument;

#ifdef USEGLOB
                glob_t    g;
#else
                char    **argv;
                int       argc = 1;
#endif
#ifdef DEBUGGLOB
                int       argc = 0;
#endif

                flags = fcntl(desc, F_GETFL, 0);
                flags &= ~FNDELAY;
                fcntl(desc, F_SETFL, flags);
                if (iafork)
                {
                        send_telcode(desc, WILL, TELOPT_SGA);
                        /*
                         * send_telcode( desc, DO, TELOPT_NAWS );  
                         */
                        send_telcode(desc, DO, TELOPT_LFLOW);
                        send_telcode(desc, DONT, TELOPT_LINEMODE);
                        send_telcode(desc, WILL, TELOPT_STATUS);
                        send_telcode(desc, DO, TELOPT_ECHO);
                        send_telcode(desc, WILL, TELOPT_ECHO);
                        read(desc, buf, 1024);  /* read replies */
                }
                dup2(desc, STDIN_FILENO);
                dup2(desc, STDOUT_FILENO);
                dup2(desc, STDERR_FILENO);
                setenv("TERM", "vt100", 1);
                setenv("COLUMNS", "80", 1);
                setenv("LINES", "24", 1);

#ifdef USEGLOB
                g.gl_offs = 1;
                p = strtok(argument, " ");

                if (p && (p = strtok(NULL, " ")) != NULL)
                {
                        glob(p, GLOB_DOOFFS | GLOB_NOCHECK, NULL, &g);
                        if (!g.gl_pathv[g.gl_pathc - 1])
                                g.gl_pathv[g.gl_pathc - 1] = p;
                }
                else
                {
                        glob(argument, GLOB_DOOFFS | GLOB_NOCHECK, NULL, &g);
                        ++(g.gl_pathv);
                }

                while (p && (p = strtok(NULL, " ")) != NULL)
                {
                        glob(p, GLOB_DOOFFS | GLOB_NOCHECK | GLOB_APPEND,
                             NULL, &g);
                        if (!g.gl_pathv[g.gl_pathc - 1])
                                g.gl_pathv[g.gl_pathc - 1] = p;
                }
                g.gl_pathv[0] = argument;

#ifdef DEBUGGLOB
                for (argc = 0; argc < g.gl_pathc; argc++)
                        printf("arg %d: %s\n\r", argc, g.gl_pathv[argc]);
                fflush(stdout);
#endif

                execvp(g.gl_pathv[0], g.gl_pathv);
#else
                while (*p)
                {
                        while (isspace(*p))
                                ++p;
                        if (*p == '\0')
                                break;
                        ++argc;
                        while (!isspace(*p) && *p)
                                ++p;
                }
                p = argument;
                argv = CALLOC(argc + 1, sizeof(char *));

                argc = 0;
                /*
                 * make sure it has at least one space 
                 */
                argv[argc] = strtok(argument, " ");
                while ((argv[++argc] = strtok(NULL, " ")) != NULL);

                execvp(argv[0], argv);
#endif

#ifdef MCCP
                if (compressing)
                        compressStart(ch->desc, compressing);
#endif
                fprintf(stderr, "Shell process: %s failed!\n", argument);
                perror("mudexec");
                exit(0);
        }
        else if (pid < 2)
        {
                send_to_char("Process fork failed.\n\r", ch);
                fprintf(stderr, "%s", "Shell process: fork failed!\n");
                return;
        }
        else
        {
                ch->desc->process = pid;
                ch->desc->connected = iafork ? CON_IAFORKED : CON_FORKED;
        }
}

/* End NEW shell command code */

bool check_forks(DESCRIPTOR_DATA * d, char *cmdline)
{
        if (d->connected == CON_FORKED || d->connected == CON_IAFORKED)
        {
                int       status;

                if (d->process > 0
                    && waitpid(d->process, &status, WNOHANG) > 0)
                {
                        if (compilelock)
                        {
                                echo_to_all(AT_GREEN,
                                            "Compiler operation completed. Reboot and shutdown commands unlocked.",
                                            ECHOTAR_IMM);
                                compilelock = FALSE;
                        }
                        d->process = 0;
                }
                if (!d->process)
                {
                        if (d->connected == CON_IAFORKED)
                        {
                                send_telcode(d->descriptor, DO,
                                             TELOPT_LINEMODE);
                                send_telcode(d->descriptor, WONT,
                                             TELOPT_ECHO);
                                /*
                                 * send_telcode( d->descriptor, DO, TELOPT_ECHO ); 
                                 */
                        }
                        d->connected = CON_PLAYING;
                        fcntl(d->descriptor, F_SETFL, FNDELAY);
                        /*
                         * clean up garbage 
                         */
                        read(d->descriptor, cmdline, MAX_INPUT_LENGTH);
#ifdef MCCP
                        if (d->shellcompressing)
                                compressStart(d, d->shellcompressing);
#endif
                }
                return TRUE;
        }
        return FALSE;
}

/* This function verifies filenames during copy operations - Samson 4-7-98 */
int copy_file(CHAR_DATA * ch, char *filename)
{
        FILE     *fp;

        if ((fp = fopen(filename, "r")) == NULL)
        {
                set_char_color(AT_RED, ch);
                ch_printf(ch,
                          "The file %s does not exist, or cannot be opened. Check your spelling.\n\r",
                          filename);
                return 1;
        }
        FCLOSE(fp);
        return 0;
}

/* The guts of the compiler code, make any changes to the compiler options here - Samson 4-8-98 */
void compile_code(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];

        if (!str_cmp(argument, "cvs"))
        {
                mudstrlcpy(buf, "make -C ../src cvs", MSL);
                do_mudexec(ch, buf);
                return;
        }

        if (!str_cmp(argument, "clean"))
        {
                mudstrlcpy(buf, "make -C ../src clean", MSL);
                do_mudexec(ch, buf);
                return;
        }

        if (!str_cmp(argument, "dns"))
        {
                mudstrlcpy(buf, "make -C ../src dns", MSL);
                do_mudexec(ch, buf);
                return;
        }

        mudstrlcpy(buf, "make -C ../src", MSL);
        do_mudexec(ch, buf);
        return;
}

/* This command compiles the code on the mud, works only on code port - Samson 4-8-98 */
CMDF do_compile(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];

        if (bootlock)
        {
                send_to_char
                        ("&RThe reboot timer is running, the compiler cannot be used at this time.\n\r",
                         ch);
                return;
        }

        if (compilelock)
        {
                send_to_char
                        ("&RThe compiler is in use, please wait for the compilation to finish.\n\r",
                         ch);
                return;
        }

        compilelock = TRUE;
        set_char_color(AT_RED, ch);
        snprintf(buf, MSL,
                 "Compiler operation initiated by %s. Reboot and shutdown commands are locked.",
                 ch->name);
        echo_to_all(AT_RED, buf, ECHOTAR_IMM);

        compile_code(ch, argument);

        return;
}


/*
====================
GREP In-Game command	-Nopey
====================
*/
/* Modified by Samson to be a bit less restrictive. So one can grep anywhere the account will allow. */
CMDF do_grep(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      arg1[MAX_INPUT_LENGTH];

        argument = one_argument(argument, arg1);
        set_char_color(AT_PLAIN, ch);

        if (!argument || argument[0] == '\0' || !arg1 || arg1[0] == '\0')
                mudstrlcpy(buf, "grep --help", MSL);    /* Will cause it to forward grep's help options to you */
        else
                snprintf(buf, MSL, "grep -n %s %s", arg1, argument);    /* Line numbers are somewhat important */

#ifdef USEGLOB
        do_mudexec(ch, buf);
#else
        command_pipe(ch, buf);
#endif
        return;
}
