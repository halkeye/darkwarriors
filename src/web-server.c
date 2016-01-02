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
 *                $Id: web-server.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
/*--------------------------------------------------------------------------*
 *                         ** WolfPaw 1.0 **                                *
 *--------------------------------------------------------------------------*
 *               WolfPaw 1.0 (c) 1997,1998 by Dale Corse                    *
 *--------------------------------------------------------------------------*
 *            The WolfPaw Coding Team is headed by: Greywolf                *
 *  With the Assitance from: Callinon, Dhamon, Sentra, Wyverns, Altrag      *
 *  Scryn, Thoric, Justice, Tricops and Mask.                               *
 *--------------------------------------------------------------------------*
 *              Wolfpaw Integrated Web Server Protocol Module               *
 *--------------------------------------------------------------------------*/

/* Code Based On:
 * ROM 2.4 Integrated Web Server - Version 1.0
 * Copyright 1998 -- Defiant -- Rob Siemborski -- mud@towers.crusoe.net
 * His Copyright states to please include him in your mud credits.
 * Modifications from that release Copyright Greywolf (Dale Corse) 1998
 * My copyright says:
 *
 * [ABSOULUTLY NOTHING!]
 *
 * Except this:
 *
 * I will answer questions, and help you fix bugs.. with MY CODE
 * not you other stuff, and i am not resposible for anything that
 * happens to you as a result of using this code, not even if it
 * makes you lose your girlfriend, melts your ice cream, formats
 * you hard drive, or causes nuclear holocost (sp?). Also, please
 * don't use this code, and say you did it, cause Defiant, Altrag, and I
 * did. So doing that would piss us off.
 *
 * --GW 
 */

/*
 * Took out a lot of the code that was commented out, a couple of unused
 * variables, wrote an installation README, and took out a bunch of references
 * to Age of the Ancients (it now points to sysdata.mud_name) and overall just
 * cleaned it up a bit. All the hard work was done by GW so don't give me any
 * credit other then telling you how to install it.
 *
 * LrdElder (Hope this helps)
 *
 * P. S. The reason I have to use this code is because I cannot write a CGI
 *      due to the fact that my web space is actually on a different computer
 *      then my mud.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <stdarg.h>
#include <fcntl.h>
#include <vector>
#include <string>
#include <arpa/inet.h>
#include "mud.h"
#include "races.h"
#include "channels.h"
#include "web-server.h"
#include "installations.h"

/* Moved the Struct to Mud.h */

void      web_header(WEB_DESCRIPTOR * wdesc, char *title);
void      web_footer(WEB_DESCRIPTOR * wdesc);

/* FUNCTION DEFS */
int       send_buf(int fd, char *buf, bool filter);
void      handle_web_request(WEB_DESCRIPTOR * wdesc);
void      handle_web_who_request(WEB_DESCRIPTOR * wdesc);
void      handle_web_wwwwho_request(WEB_DESCRIPTOR * wdesc);
void      handle_web_wizlist_request(WEB_DESCRIPTOR * wdesc);
void      handle_web_help_request(WEB_DESCRIPTOR * wdesc, int hmin, int hmax);
void      handle_web_skill_request(WEB_DESCRIPTOR * wdesc);
void      handle_web_clan_request(WEB_DESCRIPTOR * wdesc);
void      handle_web_planet_request(WEB_DESCRIPTOR * wdesc);
void      handle_web_race_request(WEB_DESCRIPTOR * wdesc);
void      handle_web_message_request(WEB_DESCRIPTOR * wdesc);
void      handle_web_empty_request(WEB_DESCRIPTOR * wdesc);
void      handle_edithelp_message_request(WEB_DESCRIPTOR * wdesc);
void      print_ooc_history(WEB_DESCRIPTOR * wdesc);
char     *strip_web_content(WEB_DESCRIPTOR * wdesc);
bool check_help_net args((WEB_DESCRIPTOR * wdesc, int hmin, int hmax));
bool check_skill_net args((WEB_DESCRIPTOR * wdesc));
bool check_clan_net args((WEB_DESCRIPTOR * wdesc));
bool check_planet_net args((WEB_DESCRIPTOR * wdesc));
bool check_race_net args((WEB_DESCRIPTOR * wdesc));
char     *text2html(const char *ip);
char     *parse_quotes(char *arg);
extern char *clan_type(CLAN_DATA * clan);
long      get_taxes(PLANET_DATA * planet);
std::vector < std::string > explodestring(char *oldstring, char *delim);

char conv_result[MAX_STRING_LENGTH];    /* Color Token Filtering */

/* Some of these variables were scattered throughout the code with externs
   defined here, why keep all the web stuff together? -- LrdElder */

/* Locals */
int sockfd;
int portid = 4850;
bool WEBSERVER_STATUS;
WEB_DESCRIPTOR *first_webdesc;
WEB_DESCRIPTOR *last_webdesc;
int top_web_desc;


int web_colour(char type, char *string)
{
        char code[50];  /* Either this is MSL, or all the things below are 50, i chose this, faster */
        char     *p = '\0';

        switch (type)
        {
        default:
                break;
        case '&':
                mudstrlcpy(code, "&", 50);
                break;
        case 'x':
                mudstrlcpy(code, "</font><font color=#000000>", 50);
                break;
        case 'b':
                mudstrlcpy(code, "</font><font color=#00007F>", 50);
                break;
        case 'c':
                mudstrlcpy(code, "</font><font color=#007F7F>", 50);
                break;
        case 'g':
                mudstrlcpy(code, "</font><font color=#007F00>", 50);
                break;
        case 'r':
                mudstrlcpy(code, "</font><font color=#7F0000>", 50);
                break;
        case 'w':
                mudstrlcpy(code, "</font><font color=#BFBFBF>", 50);
                break;
        case 'y':
                mudstrlcpy(code, "</font><font color=#FFFF00>", 50);
                break;
        case 'Y':
                mudstrlcpy(code, "</font><font color=#FFFF00>", 50);
                break;
        case 'B':
                mudstrlcpy(code, "</font><font color=#0000FF>", 50);
                break;
        case 'C':
                mudstrlcpy(code, "</font><font color=#00FFFF>", 50);
                break;
        case 'G':
                mudstrlcpy(code, "</font><font color=#00FF00>", 50);
                break;
        case 'R':
                mudstrlcpy(code, "</font><font color=#FF0000>", 50);
                break;
        case 'W':
                mudstrlcpy(code, "</font><font color=#FFFFFF>", 50);
                break;
        case 'z':
                mudstrlcpy(code, "</font><font color=#7F7F7F>", 50);
                break;
        case 'o':
                mudstrlcpy(code, "</font><font color=#FFFF00>", 50);
                break;
        case 'O':
                mudstrlcpy(code, "</font><font color=#7F7F00>", 50);
                break;
        case 'p':
                mudstrlcpy(code, "</font><font color=#7F007F>", 50);
                break;
        case 'P':
                mudstrlcpy(code, "</font><font color=#FF00FF>", 50);
                break;

        case '/':
                mudstrlcpy(code, "<br>", 50);
                break;
        case '{':
                snprintf(code, 50, "%c", '{');
                break;
        case '-':
                snprintf(code, 50, "%c", '~');
                break;
        }
        p = code;
        while (*p != '\0')
        {
                *string = *p++;
                *++string = '\0';
        }
        return (strlen(code));
}

void web_colourconv(char *buffer, const char *txt)
{
        const char *point;
        int skip = 0;

        if (txt == NULL || buffer == NULL)
        {
                bug("Null txt or buffer", 0);
                return;
        }
        for (point = txt; *point; point++)
        {
                if (*point == '&')
                {
                        point++;
                        skip = web_colour(*point, buffer);
                        while (skip-- > 0)
                                ++buffer;
                        continue;
                }
                *buffer = *point;
                *++buffer = '\0';
        }
        *buffer = '\0';
        return;
}


HELP_DATA *get_web_help(char *argument)
{
        char argall[MAX_INPUT_LENGTH];
        char argone[MAX_INPUT_LENGTH];
        char argnew[MAX_INPUT_LENGTH];
        HELP_DATA *pHelp;
        int lev;

        if (argument[0] == '\0')
                argument = "summary";

        if (isdigit(argument[0]))
        {
                lev = number_argument(argument, argnew);
                argument = argnew;
        }
        else
                lev = -2;
        /*
         * Tricky argument handling so 'help a b' doesn't match a.
         */
        argall[0] = '\0';
        while (argument[0] != '\0')
        {
                argument = one_argument(argument, argone);
                if (argall[0] != '\0')
                        mudstrlcat(argall, " ", MIL);
                mudstrlcat(argall, argone, MIL);
        }

        for (pHelp = first_help; pHelp; pHelp = pHelp->next)
        {
                if (lev != -2 && pHelp->level != lev)
                        continue;

                if (is_name(argall, pHelp->keyword))
                        return pHelp;
        }

        return NULL;
}

void web_broadcast(char *argument)
{
        char buf[MAX_STRING_LENGTH];

        snprintf(buf, MSL, "&B[&zWeb Broadcast&B] &w%s&R&W", argument);
        echo_to_all(AT_GOSSIP, buf, ECHOTAR_ALL);
}

/*
 * Changes spaces into %20 for URL Addresses
 */
char     *convert_sp(char *str)
{
        static char newstr[MAX_STRING_LENGTH];
        int i, j;

        for (i = j = 0; str[i] != '\0'; i++)
                if (str[i] != ' ')
                {
                        newstr[j++] = str[i];
                }
                else
                {
                        newstr[j++] = '%';
                        newstr[j++] = '2';
                        newstr[j++] = '0';
                }
        newstr[j] = '\0';
        return newstr;
}

/*
 * Changes spaces into %20 for URL Addresses
 */
char     *convert_sp_reverse(char *str)
{
        static char newstr[MAX_STRING_LENGTH];
        int i, j;

        for (i = j = 0; str[i] != '\0'; i++)
                if (str[i] != '%' && str[i + 1] != '2' && str[i + 2] != '0')
                {
                        newstr[j++] = str[i];
                }
                else
                {
                        newstr[j++] = ' ';
                        i += 2;
                }
        newstr[j] = '\0';
        return newstr;
}

/*
 * Changes carriage returns into <br> for HTML
 * Changes spaces into '&nbsp;'
 */
char     *conv_tag(char *str)
{
        static char newstr[MAX_STRING_LENGTH];
        int i, j;
        int itag = 0;

        for (i = j = 0; str[i] != '\0'; i++)
        {
                if (str[i] == '\n')
                {
                        newstr[j++] = '<';
                        newstr[j++] = 'b';
                        newstr[j++] = 'r';
                        newstr[j++] = '>';
                }
                else if (str[i] == '<')
                {
                        itag++;
                        newstr[j++] = str[i];
                }
                else if (str[i] == '>')
                {
                        itag--;
                        newstr[j++] = str[i];
                }

                else if (str[i] != '\r')
                {
                        newstr[j++] = str[i];
                }
        }
        newstr[j] = '\0';
        return newstr;
}


void init_web(int port)
{
        struct sockaddr_in my_addr;
        char buf[MAX_STRING_LENGTH];

        sprintf(buf, "Web features starting on port: %d", port);
        log_string(buf);

        WEBSERVER_STATUS = TRUE;

        /*
         * Lets clear these out .. --GW 
         */
        first_webdesc = NULL;
        last_webdesc = NULL;
        top_web_desc = 0;

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
                perror("web-socket");
                exit(1);
        }

        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(port);
        my_addr.sin_addr.s_addr = htons(INADDR_ANY);
        bzero(&(my_addr.sin_zero), 8);

        if ((bind
             (sockfd, (struct sockaddr *) &my_addr,
              sizeof(struct sockaddr))) == -1)
        {
                perror("web-bind");
                log_string("WebServer Disabled.");
                WEBSERVER_STATUS = FALSE;
                return;
        }

        /*
         * Only listen for 5 connects at once, do we really need more? 
         */
        /*
         * Nah .. but thanks for asking! =P *smirk* --GW 
         */
        listen(sockfd, 5);

}

struct timeval ZERO_TIME = { 0, 0 };

void handle_web(void)
{
        int max_fd;
        WEB_DESCRIPTOR *current, *next;
        fd_set readfds;

        if (WEBSERVER_STATUS == FALSE)
                return;

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        /*
         * it *will* be atleast sockfd 
         */
        max_fd = sockfd;

        /*
         * add in all the current web descriptors 
         */
        /*
         * Linked stuff again --GW 
         */
        for (current = first_webdesc; current; current = current->next)
        {
                FD_SET(current->fd, &readfds);
                if (max_fd < current->fd)
                        max_fd = current->fd;
        }

        /*
         * Wait for ONE descriptor to have activity 
         */
        select(max_fd + 1, &readfds, NULL, NULL, &ZERO_TIME);

        if (FD_ISSET(sockfd, &readfds))
        {
                socklen_t temp;

                /*
                 * NEW CONNECTION -- INIT & ADD TO LIST 
                 */

                /*
                 * Ok .. so we dont really need those memory functions
                 * included in the original release, a function for 1 line
                 * of code .. no. --GW 
                 */
                CREATE(current, WEB_DESCRIPTOR, 1);
                current->sin_size = sizeof(struct sockaddr_in);
                current->request[0] = '\0';
                temp = current->sin_size;

                if ((current->fd =
                     accept(sockfd,
                            (struct sockaddr *) &(current->their_addr),
                            &temp)) == -1)
                {
                        perror("web-accept");
                        exit(1);
                }

                /*
                 * Ugh .. lets just use LINK here .. --GW 
                 */
                LINK(current, first_webdesc, last_webdesc, next, prev);

                /*
                 * END ADDING NEW DESC 
                 */
        }

        /*
         * DATA IN! 
         */
        /*
         * Nother change for Linked List stuff --GW 
         */
        for (current = first_webdesc; current; current = current->next)
        {
                if (FD_ISSET(current->fd, &readfds))    /* We Got Data! */
                {
                        char buf[1024];
                        int numbytes;

                        if ((numbytes =
                             read(current->fd, buf, sizeof(buf))) == -1)
                        {
                                perror("web-read");
                                exit(1);
                        }

                        buf[numbytes] = '\0';

                        strcat(current->request, buf);
                }
        }   /* DONE WITH DATA IN */

        /*
         * DATA OUT 
         */
        /*
         * Again .... --GW 
         */
        for (current = first_webdesc; current; current = next)
        {
                next = current->next;

                if (strstr(current->request, "HTTP/1.") /* 1.x request (vernum on FIRST LINE) */
                    && strstr(current->request, ENDREQUEST))
                        handle_web_request(current);
                else if (!strstr(current->request, "HTTP/1.") && strchr(current->request, '\n'))    /* HTTP/0.9 (no ver number) */
                        handle_web_request(current);
                else
                {
                        continue;   /* Don't have full request yet! */
                }

                close(current->fd);
                /*
                 * Again, no function needed! 
                 * also moved this up to here, were done with it, so
                 * nuke away!  --GW 
                 */
                UNLINK(current, first_webdesc, last_webdesc, next, prev);
                DISPOSE(current);
        }
        /*
         * Removed a whack of stuff here, we dont need it. --GW 
         */

}   /* END DATA-OUT */


/* Generic Utility Function */

int send_buf(int fd, char *buf, int filter)
{
        char string[MSL * 10];

        if (filter == 1)
        {
                send(fd, "<CODE>", 6, 0);
                buf = smash_color(buf);
                buf = text2html(buf);
                send(fd, "</CODE>", 7, 0);
        }
        if (filter == 2)
        {
                web_colourconv(string, buf);
                return send(fd, string, strlen(string), 0);
        }
        return send(fd, buf, strlen(buf), 0);
}

void handle_web_request(WEB_DESCRIPTOR * wdesc)
{
        /*
         * process request 
         */
        /*
         * are we using HTTP/1.x? If so, write out header stuff.. 
         */
        if (!strstr(wdesc->request, "GET"))
        {
                send_buf(wdesc->fd, "HTTP/1.0 501 Not Implemented", FALSE);
                return;
        }
        else if (strstr(wdesc->request, "HTTP/1."))
        {
                send_buf(wdesc->fd, "HTTP/1.0 200 OK\n", FALSE);
                send_buf(wdesc->fd, "Content-type: text/html\n\n", FALSE);
        }

        /*
         * Handle the actual request 
         */
        if (strstr(wdesc->request, "/wholist"))
        {
//                log_string("Web Hit: WHOLIST");
                handle_web_who_request(wdesc);
                return;
        }

        else if (strstr(wdesc->request, "/wizlist"))
        {
//                log_string("Web Hit: WIZ-LIST");
                handle_web_wizlist_request(wdesc);
                return;
        }
        else if (strstr(wdesc->request, "/help.htm "))
        {
//                log_string("Web Hit: HELP-LIST");
                handle_web_help_request(wdesc, -1, 100);
                return;
        }
        else if (strstr(wdesc->request, "/skill.htm "))
        {
//                log_string("Web Hit: SKILL-LIST");
                handle_web_skill_request(wdesc);
                return;
        }
        else if (strstr(wdesc->request, "/clans.htm "))
        {
//                log_string("Web Hit: CLAN-LIST");
                handle_web_clan_request(wdesc);
                return;
        }
        else if (strstr(wdesc->request, "/printooc.htm "))
        {
//                log_string("Web Hit: CLAN-LIST");
                print_ooc_history(wdesc);
                return;
        }
        else if (strstr(wdesc->request, "/planets.htm "))
        {
//                log_string("Web Hit: PLANET-LIST");
                handle_web_planet_request(wdesc);
                return;
        }
        else if (strstr(wdesc->request, "/races.htm "))
        {
//                log_string("Web Hit: RACES-LIST");
                handle_web_race_request(wdesc);
                return;
        }
        else if (strstr(wdesc->request, "/test/"))
        {
                strip_web_content(wdesc);
                handle_web_message_request(wdesc);
                return;
        }

        else if (strstr(wdesc->request, "/edithelp/edithelp.htm"))
        {
                handle_edithelp_message_request(wdesc);
                return;
        }

        else if (strstr(wdesc->request, "/help/"))
        {
                if (check_help_net(wdesc, -1, 100))
                        return;

                send_buf(wdesc->fd,
                         "Dark Warriors Web help: INVALID HELP FILE", FALSE);
                return;
        }
        else if (strstr(wdesc->request, "/clan/"))
        {
                if (check_clan_net(wdesc))
                        return;

                send_buf(wdesc->fd,
                         "Dark Warriors Web clan listing: INVALID CLAN FILE",
                         FALSE);
                return;
        }
        else if (strstr(wdesc->request, "/planet/"))
        {
                if (check_planet_net(wdesc))
                        return;

                send_buf(wdesc->fd,
                         "Dark Warriors Web planet listing: INVALID PLANET FILE",
                         FALSE);
                return;
        }
        else if (strstr(wdesc->request, "/races/"))
        {
                if (check_race_net(wdesc))
                        return;

                send_buf(wdesc->fd,
                         "Dark Warriors Web race listing: INVALID RACE FILE",
                         FALSE);
                return;
        }
/*      else if ( strstr(wdesc->request, "/~immhelp/" ) )
        {
                if ( check_help_net(wdesc, -2, 200) )
                return;

                send_buf(wdesc->fd, "Dark Warriors Web help: ILLEGAL ACCESS ATTEMPT", FALSE);
                return;
        }*/
        else
        {
//                log_string("Web Hit: INVALID URL");
                handle_web_empty_request(wdesc);
/*                send_buf(wdesc->fd,
                         "Sorry, the webserver 1.0 only supports /wholist and /wizlist",
                         FALSE);*/
                return;
        }
}

void shutdown_web(void)
{
        WEB_DESCRIPTOR *current, *next;

        /*
         * Close All Current Connections 
         */
        /*
         * lets change this around ... blah blah --GW 
         */
        for (current = first_webdesc; current; current = next)
        {
                next = current->next;
                close(current->fd);
                /*
                 * Again, no function needed! --GW 
                 */
                UNLINK(current, first_webdesc, last_webdesc, next, prev);
                DISPOSE(current);
        }

        /*
         * Stop Listening 
         */
        close(sockfd);
        WEBSERVER_STATUS = FALSE;
}

void handle_web_empty_request(WEB_DESCRIPTOR * wdesc)
{
        web_header(wdesc, "Index");
        send_buf(wdesc->fd, "<a href='/wholist'>Who list</a><br>", FALSE);
        send_buf(wdesc->fd, "<a href='/wizlist'>Wiz list</a><br>", FALSE);
        send_buf(wdesc->fd, "<a href='/help.htm'>Help</a><br>", FALSE);
        send_buf(wdesc->fd, "<a href='/skill.htm'>Skill</a><br>", FALSE);
        send_buf(wdesc->fd, "<a href='/clans.htm'>Clans</a><br>", FALSE);
        send_buf(wdesc->fd, "<a href='/planets.htm'>Planets</a><br>", FALSE);
        send_buf(wdesc->fd, "<a href='/races.htm'>Races</a><br>", FALSE);
        web_footer(wdesc);
        return;
}

void handle_web_who_request(WEB_DESCRIPTOR * wdesc)
{
        FILE     *fp;
        char buf[MAX_STRING_LENGTH];
        int c;
        int num = 0;


        /*
         * Well .. why have 2 copies of your who? Smaug already supports
         * webwho, however wont give anyone the cgi for it .. so here we go!
         * --GW 
         */

/* Send the Basic Html Config, hard coded background .. sue me. --GW */
        do_who(NULL, "");

/*
 * Modified version of Show File, used in here --GW
 */

        if ((fp = fopen(WHO_FILE, "r")) != NULL)
        {
                while (!feof(fp))
                {
                        while ((buf[num] = fgetc(fp)) != EOF
                               && buf[num] != '\n'
                               && buf[num] != '\r'
                               && num < (MAX_STRING_LENGTH - 2))
                                num++;
                        c = fgetc(fp);
                        if ((c != '\n' && c != '\r') || c == buf[num])
                                ungetc(c, fp);
                        buf[num++] = '\n';
                        buf[num++] = '\r';
                        buf[num] = '\0';

                        if (strlen(buf) > 32000)
                        {
                                bug("Strlen Greater then 32000: show_file",
                                    0);
                                buf[32000] = '\0';
                        }
                        num = 0;
                        send_buf(wdesc->fd, buf, FALSE);
//    send_buf(wdesc->fd,"<BR>",FALSE);
                }
                FCLOSE(fp);
        }

        return;
}


#define WEBWIZLIST_FILE SYSTEM_DIR "WIZLIST"

void handle_web_wizlist_request(WEB_DESCRIPTOR * wdesc)
{
        FILE     *fp;
        char buf[MAX_STRING_LENGTH];
        int c;
        int num = 0;


        make_wizlist();

/*
 * Modified version of Show File, used in here --GW
 */
        snprintf(buf, MSL, "%swizlist.html", HTML_MUDINFO_WRITE_DIR);
        if ((fp = fopen(buf, "r")) != NULL)
        {
                send_buf(wdesc->fd, "<CENTER>", FALSE);
                while (!feof(fp))
                {
                        while ((buf[num] = fgetc(fp)) != EOF
                               && buf[num] != '\n'
                               && buf[num] != '\r'
                               && num < (MAX_STRING_LENGTH - 2))
                                num++;
                        c = fgetc(fp);
                        if ((c != '\n' && c != '\r') || c == buf[num])
                                ungetc(c, fp);
                        buf[num++] = '\n';
                        buf[num++] = '\r';
                        buf[num] = '\0';

                        if (strlen(buf) > 32000)
                        {
                                bug("Strlen Greater then 32000: show_file",
                                    0);
                                buf[32000] = '\0';
                        }
                        num = 0;
                        send_buf(wdesc->fd, buf, FALSE);
                }
                send_buf(wdesc->fd, "</CENTER>", FALSE);
                FCLOSE(fp);
        }
        return;
}

/*
 * Many thanks to Altrag who contributed this function! --GW
 */
char     *text2html(const char *ip)
{
        static struct
        {
                const char *text;
                const char *html;
                int tlen, hlen;
        } convert_table[] =
        {
                {
                "<", "&lt;", 0, 0},
                {
                ">", "&gt;", 0, 0},
                {
                "&", "&amp;", 0, 0},
                {
                "\"", "&quot;", 0, 0},
                {
                " ", "&nbsp;", 0, 0},
                {
        NULL, NULL, 0, 0}};

        static char buf[MAX_STRING_LENGTH * 2]; /* Safety here .. --GW */
        char     *bp = buf;
        int i;

        if (!convert_table[0].tlen)
        {
                for (i = 0; convert_table[i].text; ++i)
                {
                        convert_table[i].tlen = strlen(convert_table[i].text);
                        convert_table[i].hlen = strlen(convert_table[i].html);
                }
        }
        while (*ip)
        {
                for (i = 0; convert_table[i].text; ++i)
                        if (!strncmp
                            (ip, convert_table[i].text,
                             convert_table[i].tlen))
                                break;
                if (convert_table[i].text)
                {
                        strcpy(bp, convert_table[i].html);
                        bp += convert_table[i].hlen;
                        ip += convert_table[i].tlen;
                }
                else
                        *bp++ = *ip++;
        }
        *bp = '\0';
        return buf;
}

char     *parse_quotes(char *arg)
{
        int str;

        for (str = 0; arg[str] != '\0'; str++)
        {
                if (arg[str] == '*')
                        arg[str] = '"';
        }

        return arg;
}

/*
 * ####################################################
 *   Web HELP Request (Or not)
 * ####################################################
 */
bool check_help_net(WEB_DESCRIPTOR * wdesc, int hmin, int hmax)
{
        HELP_DATA *help;
        char buf[MAX_STRING_LENGTH];
        char buf2[MAX_STRING_LENGTH];
        char buf3[MAX_STRING_LENGTH];
        char buf4[MAX_STRING_LENGTH];
        char buf5[MAX_STRING_LENGTH];
        char color[MAX_STRING_LENGTH];

        for (help = first_help; help; help = help->next)
        {
                if (help->level >= hmax || help->level <= hmin)
                        continue;

                sprintf(buf4, "/~delete_help/%s.htm ",
                        convert_sp(strlower(help->keyword)));
                one_argument(help->keyword, buf3);
                sprintf(buf5, "/~delete_help/%s.htm ", strlower(buf3));

                if (help->level >= 100 || hmax > 100)
                {
                        sprintf(buf, "/~immhelp/%s.htm ",
                                convert_sp(strlower(help->keyword)));
                        one_argument(help->keyword, buf3);
                        sprintf(buf2, "/~immhelp/%s.htm ", strlower(buf3));
                }
                else
                {
                        sprintf(buf, "/help/%s.htm ",
                                convert_sp(strlower(help->keyword)));
                        one_argument(help->keyword, buf3);
                        sprintf(buf2, "/help/%s.htm ", strlower(buf3));
                }

                if (strstr(wdesc->request, buf)
                    || strstr(wdesc->request, buf2))
                {
                        /*
                         * handle_web_help(wdesc); 
                         */
                        send_buf(wdesc->fd, "<html>\n", FALSE);
                        send_buf(wdesc->fd, "<head>\n", FALSE);
                        send_buf(wdesc->fd,
                                 "<title>Dark Warriors - Help Listing</title>\n",
                                 FALSE);
                        send_buf(wdesc->fd,
                                 "<style> pre { display: inline; }  a:link { background-color: inherit; ",
                                 FALSE);
                        send_buf(wdesc->fd,
                                 "text-decoration: none; color: inherit; }\n\r",
                                 FALSE);
                        send_buf(wdesc->fd,
                                 "a:visited { background-color: inherit; text-decoration: none; color: inherit; }",
                                 FALSE);
                        send_buf(wdesc->fd,
                                 "\n\ra:hover { background-color: inherit; text-decoration: none; ",
                                 FALSE);
                        send_buf(wdesc->fd,
                                 "color: inherit; }\n\ra:active { background-color: inherit; text-decoration: none; ",
                                 FALSE);
                        send_buf(wdesc->fd,
                                 "color: inherit; }</style></head>\n", FALSE);
                        send_buf(wdesc->fd,
                                 "<body bgcolor=black text=white topmargin=0 rightmargin=0 bottommargin=0 leftmargin=0>\n\r",
                                 FALSE);
                        send_buf(wdesc->fd, "<FONT FACE=" "courier" ">\n",
                                 FALSE);
                        if (help->level >= 100)
                                send_buf(wdesc->fd,
                                         "Dark Warriors Immortal-Only Web help: ",
                                         FALSE);
                        if (help->level < 100)
                                send_buf(wdesc->fd,
                                         "Dark Warriors Web help: ", FALSE);
                        send_buf(wdesc->fd, help->keyword, FALSE);
                        send_buf(wdesc->fd, "<br><br>\n", FALSE);
                        if (help->text[0] == '.')
                                web_colourconv(color, help->text + 1);
                        else
                                web_colourconv(color, help->text);
                        send_buf(wdesc->fd, conv_tag(color), FALSE);
                        send_buf(wdesc->fd, "<br><br>\n", FALSE);
                        send_buf(wdesc->fd, "Last edited by ", FALSE);
                        send_buf(wdesc->fd, help->author, FALSE);
                        send_buf(wdesc->fd, "<br><br>\n", FALSE);
                        if (hmax > 100)
                        {
                                sprintf(buf2, "/~delete_help/%s.htm ",
                                        convert_sp(strlower(help->keyword)));
                                sprintf(buf,
                                        "<br><a href=" "%s"
                                        ">[Delete this helpfile]</a><br>\n",
                                        buf2);
                                send_buf(wdesc->fd, buf, FALSE);
                        }
                        send_buf(wdesc->fd, "</font>\n", FALSE);
                        send_buf(wdesc->fd, "</body>\n", FALSE);
                        return TRUE;
                }
                else if (strstr(wdesc->request, buf4)
                         || strstr(wdesc->request, buf5))
                {
                        /*
                         * handle_web_help(wdesc); 
                         */
                        send_buf(wdesc->fd, "<html>\n", FALSE);
                        send_buf(wdesc->fd, "<head>\n", FALSE);
                        send_buf(wdesc->fd,
                                 "<title>Dark Warriors - Help Listing</title>\n",
                                 FALSE);
                        send_buf(wdesc->fd,
                                 "<style> pre { display: inline; }  a:link { background-color: inherit; ",
                                 FALSE);
                        send_buf(wdesc->fd,
                                 "text-decoration: none; color: inherit; }\n\r",
                                 FALSE);
                        send_buf(wdesc->fd,
                                 "a:visited { background-color: inherit; text-decoration: none; color: inherit; }",
                                 FALSE);
                        send_buf(wdesc->fd,
                                 "\n\ra:hover { background-color: inherit; text-decoration: none; ",
                                 FALSE);
                        send_buf(wdesc->fd,
                                 "color: inherit; }\n\ra:active { background-color: inherit; text-decoration: none; ",
                                 FALSE);
                        send_buf(wdesc->fd,
                                 "color: inherit; }</style></head>\n", FALSE);
                        send_buf(wdesc->fd,
                                 "<body bgcolor=black text=white topmargin=0 rightmargin=0 bottommargin=0 leftmargin=0>\n\r",
                                 FALSE);
                        send_buf(wdesc->fd, "<FONT FACE=" "courier" ">\n",
                                 FALSE);
                        if (help->level >= 100)
                                send_buf(wdesc->fd,
                                         "Dark Warriors Immortal-Only Web help: ",
                                         FALSE);
                        if (help->level < 100)
                                send_buf(wdesc->fd,
                                         "Dark Warriors Web help: ", FALSE);
                        send_buf(wdesc->fd, help->keyword, FALSE);
                        send_buf(wdesc->fd, "<br><br>\n", FALSE);
                        web_colourconv(color,
                                       "&RHelpfile has been successfully deleted.");
                        send_buf(wdesc->fd, conv_tag(color), FALSE);
                        send_buf(wdesc->fd, "</font>\n", FALSE);
                        send_buf(wdesc->fd, "</body>\n", FALSE);
                        UNLINK(help, first_help, last_help, next, prev);
                        delete_help(help);
                        return TRUE;
                }

        }

        return FALSE;
}

/*
 * ####################################################
 *   Web HELP LISTING Request
 * ####################################################
 */
void handle_web_help_request(WEB_DESCRIPTOR * wdesc, int hmin, int hmax)
{
        HELP_DATA *help;
        char buf[MAX_STRING_LENGTH];
        char buf2[MAX_STRING_LENGTH];
        char tmp = ' ', chk = ' ';
        int cnt = 0, inqt = 0, first = 0;

        send_buf(wdesc->fd, "<html>\n", FALSE);
        send_buf(wdesc->fd, "<head>\n", FALSE);
        send_buf(wdesc->fd, "<title>Dark Warriors - Help Listing</title>\n",
                 FALSE);
        send_buf(wdesc->fd,
                 "<style> pre { display: inline; }  a:link { background-color: inherit; ",
                 FALSE);
        send_buf(wdesc->fd, "text-decoration: none; color: inherit; }\n\r",
                 FALSE);
        send_buf(wdesc->fd,
                 "a:visited { background-color: inherit; text-decoration: none; color: inherit; }",
                 FALSE);
        send_buf(wdesc->fd,
                 "\n\ra:hover { background-color: inherit; text-decoration: none; ",
                 FALSE);
        send_buf(wdesc->fd,
                 "color: inherit; }\n\ra:active { background-color: inherit; text-decoration: none; ",
                 FALSE);
        send_buf(wdesc->fd, "color: inherit; }</style></head>\n", FALSE);
        send_buf(wdesc->fd,
                 "<body bgcolor=black text=white topmargin=0 rightmargin=0 bottommargin=0 leftmargin=0>\n\r",
                 FALSE);
        send_buf(wdesc->fd,
                 "<h1><center>Summary of Dark Warrior Help Topics</center></h1>\n",
                 FALSE);
        send_buf(wdesc->fd, "<b><font size=" "2" ">\n", FALSE);
        send_buf(wdesc->fd, "<br><hr color=" "#FFFFFF" "><br>\n", FALSE);

        for (help = first_help; help; help = help->next)
        {
                if (help->level >= hmax || help->level <= hmin)
                        continue;

                chk = help->keyword[0];

                if (help->keyword[0] == '!')
                {
                        continue;   /* Skip the ! Command */
                }
                else if (help->keyword[0] == '\'' || help->keyword[0] == '"')
                {
                        chk = help->keyword[1];
                        if (help->keyword[0] == '"')
                                inqt = 1;
                }
                else
                {
                        if (inqt == 1)
                                send_buf(wdesc->fd,
                                         "<br><hr color=" "#FFFFFF" "><br>\n",
                                         FALSE);
                        inqt = 0;
                }

                if (tmp != chk)
                {
                        tmp = chk;
                        if (first == 0)
                                first = 1;
                        else
                                send_buf(wdesc->fd, "</table>", FALSE);
                        if (inqt == 1)
                                sprintf(buf,
                                        "<br></font><font size=" "4"
                                        ">'%c'</font><font size=" "2"
                                        "><br>\n", tmp);
                        if (inqt == 0)
                                sprintf(buf,
                                        "<br></font><font size=" "4"
                                        ">%c</font><font size=" "2" "><br>\n",
                                        tmp);
                        send_buf(wdesc->fd, buf, FALSE);
                        send_buf(wdesc->fd,
                                 "<table cellpadding=0 cellspacing=0 border = 1  width=500>",
                                 FALSE);
                        send_buf(wdesc->fd,
                                 "<tr><th width=\"315\">Name</th><th width=\"185\">Date laste modified</th></tr>",
                                 FALSE);
                        send_buf(wdesc->fd, "</table>", FALSE);
                        send_buf(wdesc->fd,
                                 "<table cellpadding=0 cellspacing=0 width=500>",
                                 FALSE);
                }

                send_buf(wdesc->fd, "<tr><td width=\"315\">", FALSE);
                if (help->level >= 100)
                {
                        sprintf(buf2, "/~immhelp/%s.htm ",
                                convert_sp(strlower(help->keyword)));
                        sprintf(buf, "<a href=" "%s" ">%s *</a><br>\n", buf2,
                                help->keyword);
                }
                else
                {
                        if (hmax > 100)
                        {
                                sprintf(buf2, "/~immhelp/%s.htm ",
                                        convert_sp(strlower(help->keyword)));
                                sprintf(buf, "<a href=" "%s" ">%s</a><br>\n",
                                        buf2, help->keyword);
                        }
                        else
                        {
                                sprintf(buf2, "/help/%s.htm ",
                                        convert_sp(strlower(help->keyword)));
                                sprintf(buf, "<a href=" "%s" ">%s</a><br>\n",
                                        buf2, help->keyword);
                        }
                }

                send_buf(wdesc->fd, buf, FALSE);
                send_buf(wdesc->fd, "</td><td width=\"185\"><center>", FALSE);
                send_buf(wdesc->fd, help->date, FALSE);
                send_buf(wdesc->fd, "</center></td></tr>", FALSE);
                cnt++;
        }

        send_buf(wdesc->fd, "<br><br><hr color=" "#FFFFFF" "><br>\n", FALSE);

        send_buf(wdesc->fd, "<center><font face=" "Times New Roman" ">\n",
                 FALSE);
        if (cnt > 0)
        {
                if (hmax >= 100)
                        send_buf(wdesc->fd,
                                 "Links marked with a '*' means they are over level 100.<br>\n",
                                 FALSE);
                sprintf(buf,
                        "-There are [ %d ] help files currently on Dark Warriors-<br>\n",
                        cnt);
                send_buf(wdesc->fd, buf, FALSE);
        }
        else
                send_buf(wdesc->fd,
                         "-There are no help files on Dark Warriors right now-<br>\n",
                         FALSE);

        sprintf(buf, "<br>This file last updated at %s Eastern Time.\n",
                ((char *) ctime(&current_time)));
        send_buf(wdesc->fd, buf, FALSE);
        send_buf(wdesc->fd, "</center></font>\n", FALSE);
        send_buf(wdesc->fd, "</body>\n", FALSE);
        send_buf(wdesc->fd, "</html>\n", FALSE);
        return;
}

void handle_web_skill_request(WEB_DESCRIPTOR * wdesc)
{
        HELP_DATA *help;
        char buf[MAX_STRING_LENGTH];
        char buf2[MAX_STRING_LENGTH];
        char buf3[MAX_STRING_LENGTH];
        int ability;
        int sn, i, col = 0;

        send_buf(wdesc->fd, "<html>\n", FALSE);
        send_buf(wdesc->fd, "<head>\n", FALSE);
        send_buf(wdesc->fd, "<title>Dark Warriors - Skills Listing</title>\n",
                 FALSE);
        send_buf(wdesc->fd,
                 "<style> pre { display: inline; }  a:link { background-color: inherit; ",
                 FALSE);
        send_buf(wdesc->fd, "text-decoration: none; color: inherit; }\n\r",
                 FALSE);
        send_buf(wdesc->fd,
                 "a:visited { background-color: inherit; text-decoration: none; color: inherit; }",
                 FALSE);
        send_buf(wdesc->fd,
                 "\n\ra:hover { background-color: inherit; text-decoration: none; ",
                 FALSE);
        send_buf(wdesc->fd,
                 "color: inherit; }\n\ra:active { background-color: inherit; text-decoration: none; ",
                 FALSE);
        send_buf(wdesc->fd, "color: inherit; }</style></head>\n", FALSE);
        send_buf(wdesc->fd,
                 "<body bgcolor=black text=white topmargin=0 rightmargin=0 bottommargin=0 leftmargin=0>\n\r",
                 FALSE);
        send_buf(wdesc->fd,
                 "<h1><center>Summary of Dark Warrior Skill Table</center></h1>\n",
                 FALSE);
        send_buf(wdesc->fd, "<b><font size=" "2" ">\n", FALSE);
        send_buf(wdesc->fd, "<br><hr color=" "#FFFFFF" "><br>\n", FALSE);

        send_buf(wdesc->fd, "&BS&zPELL &w& &BS&zKILL &BL&zIST\n\r", 2);
        send_buf(wdesc->fd, "<br>", 2);
        send_buf(wdesc->fd, "------------------\n\r", 2);
        send_buf(wdesc->fd, "<br><pre>", 2);

        for (ability = -1; ability < MAX_ABILITY; ability++)
        {
                if (ability == 7)
                        ability++;

                if (ability >= 0)
                        snprintf(buf, MSL, "\n\r&B[&z%s&B]\n\r",
                                 capitalize(ability_name[ability]));
                else
                        snprintf(buf, MSL, "%s",
                                 "\n\r&B[&zGeneral Skills&B]\n\r");

                send_buf(wdesc->fd, buf, 2);

                for (i = 0; i <= 150; i++)
                {
                        for (sn = 0; sn < top_sn; sn++)
                        {
                                if (!skill_table[sn]->name)
                                        break;

                                if (skill_table[sn]->guild != ability)
                                        continue;

                                if (SPELL_FLAG
                                    (skill_table[sn], SF_SECRETSKILL))
                                        continue;

                                if (i == skill_table[sn]->min_level)
                                {
                                        if ((help =
                                             get_web_help(skill_table[sn]->
                                                          name)) != NULL)
                                        {
                                                if (skill_table[sn]->races
                                                    && skill_table[sn]->
                                                    races[0] != '\0')
                                                {
                                                        sprintf(buf3,
                                                                "/help/%s.htm ",
                                                                convert_sp
                                                                (strlower
                                                                 (help->
                                                                  keyword)));
                                                        sprintf(buf2,
                                                                "<a href="
                                                                "%s"
                                                                ">&w%-18.18s</a> ",
                                                                buf3,
                                                                skill_table
                                                                [sn]->name);
                                                        snprintf(buf, MSL,
                                                                 "&B(&w%3d&B)[&zR&B]&B[&z%s&B]",
                                                                 i,
                                                                 get_web_help
                                                                 (skill_table
                                                                  [sn]->
                                                                  name) ? "H"
                                                                 : " ");
                                                        strcat(buf, buf2);
                                                        send_buf(wdesc->fd,
                                                                 buf, 2);
                                                }
                                                else
                                                {
                                                        sprintf(buf3,
                                                                "/help/%s.htm ",
                                                                convert_sp
                                                                (strlower
                                                                 (help->
                                                                  keyword)));
                                                        sprintf(buf2,
                                                                "<a href="
                                                                "%s"
                                                                ">&w%-18.18s</a> ",
                                                                buf3,
                                                                skill_table
                                                                [sn]->name);
                                                        snprintf(buf, MSL,
                                                                 "&B(&w%3d&B)[&z &B]&B[&z%s&B]",
                                                                 i,
                                                                 get_web_help
                                                                 (skill_table
                                                                  [sn]->
                                                                  name) ? "H"
                                                                 : " ");
                                                        strcat(buf, buf2);
                                                        send_buf(wdesc->fd,
                                                                 buf, 2);
                                                }
                                        }
                                        else
                                        {
                                                if (skill_table[sn]->races
                                                    && skill_table[sn]->
                                                    races[0] != '\0')
                                                {
                                                        snprintf(buf, MSL,
                                                                 "&B(&w%3d&B)[&zR&B]&B[&z%s&B]&w%-18.18s ",
                                                                 i,
                                                                 get_web_help
                                                                 (skill_table
                                                                  [sn]->
                                                                  name) ? "H"
                                                                 : " ",
                                                                 skill_table
                                                                 [sn]->name);
                                                        send_buf(wdesc->fd,
                                                                 buf, 2);
                                                }
                                                else
                                                {
                                                        snprintf(buf, MSL,
                                                                 "&B(&w%3d&B)[&z &B]&B[&z%s&B]&w%-18.18s ",
                                                                 i,
                                                                 get_web_help
                                                                 (skill_table
                                                                  [sn]->
                                                                  name) ? "H"
                                                                 : " ",
                                                                 skill_table
                                                                 [sn]->name);
                                                        send_buf(wdesc->fd,
                                                                 buf, 2);
                                                }
                                        }
                                        if (++col == 3)
                                        {
                                                send_buf(wdesc->fd, "<br>",
                                                         2);
                                                col = 0;
                                        }

                                }
                        }
                }
        }
        send_buf(wdesc->fd, "<br><br>", 2);
        send_buf(wdesc->fd, "</pre></body>\n", FALSE);
        send_buf(wdesc->fd, "</html>\n", FALSE);
        return;
}

void handle_web_clan_request(WEB_DESCRIPTOR * wdesc)
{
        CLAN_DATA *clan;
        char buf[MAX_STRING_LENGTH];
        int count;


        send_buf(wdesc->fd, "<html>\n", FALSE);
        send_buf(wdesc->fd, "<head>\n", FALSE);
        send_buf(wdesc->fd, "<title>Dark Warriors - Clan Listing</title>\n",
                 FALSE);
        send_buf(wdesc->fd,
                 "<style> pre { display: inline; }  a:link { background-color: inherit; ",
                 FALSE);
        send_buf(wdesc->fd, "text-decoration: none; color: inherit; }\n\r",
                 FALSE);
        send_buf(wdesc->fd,
                 "a:visited { background-color: inherit; text-decoration: none; color: inherit; }",
                 FALSE);
        send_buf(wdesc->fd,
                 "\n\ra:hover { background-color: inherit; text-decoration: none; ",
                 FALSE);
        send_buf(wdesc->fd,
                 "color: inherit; }\n\ra:active { background-color: inherit; text-decoration: none; ",
                 FALSE);
        send_buf(wdesc->fd, "color: inherit; }</style></head>\n", FALSE);
        send_buf(wdesc->fd,
                 "<body bgcolor=black text=white topmargin=0 rightmargin=0 bottommargin=0 leftmargin=0>\n\r",
                 FALSE);
        send_buf(wdesc->fd,
                 "<h1><center>Summary of Dark Warrior Clan Listing</center></h1>\n",
                 FALSE);
        send_buf(wdesc->fd, "<b><font size=" "2" ">\n", FALSE);
        send_buf(wdesc->fd, "<br><hr color=" "#FFFFFF" "><br>\n", FALSE);

        send_buf(wdesc->fd, "<br><pre>", 2);
        send_buf(wdesc->fd,
                 "&B-----------------------------------------------------------------------------\n",
                 2);

        for (clan = first_clan; clan; clan = clan->next)
        {
                if (clan->clan_type == CLAN_SUBCLAN)
                        continue;


                send_buf(wdesc->fd, "&BO&zrganization: &W", 2);
                sprintf(buf, "<a href=/clan/%s.htm>",
                        convert_sp(strlower(clan->name)));
                send_buf(wdesc->fd, buf, 2);
                sprintf(buf, "%-37.37s", clan->name);
                send_buf(wdesc->fd, buf, 2);
                send_buf(wdesc->fd, "</a> ", 2);
                snprintf(buf, MSL, "&BT&zype&B: &W%s", clan_type(clan));
                send_buf(wdesc->fd, buf, 2);
                send_buf(wdesc->fd, "<br>", 0);
                snprintf(buf, MSL, "  &BE&znlisting?&B: &W%-3s&B         ",
                         clan->enliston == 1 ? "Yes" : clan->enliston ==
                         0 ? "No" : "Unknown");
                send_buf(wdesc->fd, buf, 2);
                snprintf(buf, MSL,
                         "  &BM&zembers&B: &W%-4d&B           &BM&zin. &BA&zlign&B: &W%-5d&B",
                         clan->members, clan->alignment);
                send_buf(wdesc->fd, buf, 2);
                send_buf(wdesc->fd, "<br>", 0);
                if (clan->first_subclan)
                {
                        CLAN_DATA *subclan;

                        snprintf(buf, MSL, "  &BS&zub clans&B:");
                        send_buf(wdesc->fd, buf, 2);
                        send_buf(wdesc->fd, "<br>", 0);

                        for (subclan = clan->first_subclan; subclan;
                             subclan = subclan->next_subclan)
                        {
                                send_buf(wdesc->fd,
                                         "    &BO&zrganization: &W", 2);
                                sprintf(buf, "<a href=/clan/%s.htm>",
                                        convert_sp(strlower(subclan->name)));
                                send_buf(wdesc->fd, buf, 2);
                                sprintf(buf, "%-33.33s", subclan->name);
                                send_buf(wdesc->fd, buf, 2);
                                send_buf(wdesc->fd, "</a> ", 2);
                                snprintf(buf, MSL, "&BM&zembers&B:&W %d",
                                         subclan->members);
                                send_buf(wdesc->fd, buf, 2);
                                send_buf(wdesc->fd, "<br>", 0);
                                count++;
                        }
                }

                if (clan->next)
                        send_buf(wdesc->fd,
                                 "                     &R-------------------------------                         \n",
                                 2);
                count++;
        }
        send_buf(wdesc->fd,
                 "&B-----------------------------------------------------------------------------",
                 2);
        send_buf(wdesc->fd, "<br><br>", 2);
        send_buf(wdesc->fd, "&YClick clan name&z for more information.\n\r",
                 2);
        send_buf(wdesc->fd, "</pre></body>\n", FALSE);
        send_buf(wdesc->fd, "</html>\n", FALSE);
        return;
}


void handle_web_race_request(WEB_DESCRIPTOR * wdesc)
{
        RACE_DATA *race = NULL;
        char buf[MAX_STRING_LENGTH];

        web_header(wdesc, "Race Listing");
        send_buf(wdesc->fd,
                 "&B-----------------------------------------------------------------------------\n<br>",
                 2);
        FOR_EACH_LIST(RACE_LIST, races, race)
        {
                send_buf(wdesc->fd, "&BN&zame: &W", 2);
                sprintf(buf, "<a href=/races/%s.htm>",
                        convert_sp(strlower(race->name())));
                send_buf(wdesc->fd, buf, 2);
                sprintf(buf, "%-37.37s", race->name());
                send_buf(wdesc->fd, buf, 2);
                send_buf(wdesc->fd, "</a>", 2);
                snprintf(buf, MSL, "&BR&zpp needed&B: &W%d\n<br>",
                         race->rpneeded());
                send_buf(wdesc->fd, buf, 2);
        }
        send_buf(wdesc->fd,
                 "&B-----------------------------------------------------------------------------<br>",
                 2);
        web_footer(wdesc);
        return;
}

bool check_race_net(WEB_DESCRIPTOR * wdesc)
{
        RACE_DATA *race = NULL;
        ROOM_INDEX_DATA *room;
        char buf[MAX_STRING_LENGTH];
        char buf2[MAX_STRING_LENGTH];
        char buf3[MAX_STRING_LENGTH];
        char buf4[MAX_STRING_LENGTH];
        char arg1[MAX_STRING_LENGTH];
        char     *argument = arg1;
        int iclass;
        HELP_DATA *help;

        web_header(wdesc, "Race Listing");
        FOR_EACH_LIST(RACE_LIST, races, race)
        {

                sprintf(buf3, "/races/%s.htm ",
                        convert_sp(strlower(race->name())));
                one_argument(race->name(), buf4);
                sprintf(buf2, "/races/%s.htm ", strlower(buf4));

                if (strstr(wdesc->request, buf3)
                    || strstr(wdesc->request, buf2))
                {
                        if ((help = get_web_help(race->name())) != NULL)
                        {
                                snprintf(buf, MSL,
                                         "&BR&zace name:          &W");
                                send_buf(wdesc->fd, buf, 2);
                                sprintf(buf3, "<a href=/help/%s.htm>",
                                        convert_sp(strlower(help->keyword)));
                                snprintf(buf, MSL, "%s%s</a>\n<br>", buf3,
                                         race->name());
                                send_buf(wdesc->fd, buf, 2);
                        }
                        else
                        {
                                snprintf(buf, MSL,
                                         "&BR&zace name:          &W%s\n<br>",
                                         race->name());
                                send_buf(wdesc->fd, buf, 2);
                        }
                        snprintf(buf, MSL,
                                 "&BL&zanguage spoken:    &W%s\n<br>",
                                 capitalize(race->language()->name));
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL, "&BR&zacial statistics<br>\n");
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "    &BS&ztrength:           &W%+d<br>\n",
                                 race->attr_modifier(ATTR_STRENGTH));
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "    &BW&zisdom:             &W%+d<br>\n",
                                 race->attr_modifier(ATTR_WISDOM));
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "    &BI&zntelligence:       &W%+d<br>\n",
                                 race->attr_modifier(ATTR_INTELLIGENCE));
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "    &BD&zexterity:          &W%+d<br>\n",
                                 race->attr_modifier(ATTR_WISDOM));
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "    &BC&zonstitution:       &W%+d<br>\n",
                                 race->attr_modifier(ATTR_CONSTITUTION));
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "    &BC&zharisma:           &W%+d<br>\n",
                                 race->attr_modifier(ATTR_CHARISMA));
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "    &BF&zorce:              &W%+d<br>\n",
                                 race->attr_modifier(ATTR_FORCE));
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "    &BL&zuck:               &W%+d<br>\n",
                                 race->attr_modifier(ATTR_LUCK));
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "    &BH&zit point modifier: &W%+d<br>\n",
                                 race->hit());
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "    &BE&zndurance modifier: &W%+d<br>\n",
                                 race->endurance());
                        send_buf(wdesc->fd, buf, 2);
                        if (race->home() != 0
                            && (room = get_room_index(race->home())) != NULL
                            && room->area && room->area->planet
                            && room->area->planet->name)
                        {
                                snprintf(buf, MSL,
                                         "    &BH&zome planet:        &W%s<br>\n",
                                         room->area->planet->name);
                                send_buf(wdesc->fd, buf, 2);
                        }
                        snprintf(buf, MSL,
                                 "    &BD&zeath age:          &W%+d<br>\n",
                                 race->death_age());
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "    &BS&ztart age:          &W%+d<br>\n",
                                 race->start_age());
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "    &BR&zPP needed:         &W%+d<br>\n",
                                 race->rpneeded());
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "    &BB&zonus languages:    &W%+d<br>\n",
                                 race->lang_bonus());
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "    &BR&zacial affects:     &W%s<br>\n",
                                 affect_bit_name(race->affected()));
                        send_buf(wdesc->fd, buf, 2);
                        if (race->class_restriction())
                        {
                                int iClass = 0;

                                snprintf(buf, MSL,
                                         "&BR&zestricted classes\n<br>");
                                send_buf(wdesc->fd, buf, 2);
                                for (iClass = 0; iClass < MAX_ABILITY;
                                     iClass++)
                                {
                                        if (IS_SET
                                            (race->class_restriction(),
                                             1 << iClass))
                                        {
                                                snprintf(buf, MSL,
                                                         "    &w%-10s\n<br>",
                                                         ability_name
                                                         [iClass]);
                                                send_buf(wdesc->fd, buf, 2);
                                        }
                                }

                        }

                        snprintf(buf, MSL, "&BC&zlass modifiers\n<br>");
                        send_buf(wdesc->fd, buf, 2);

                        for (iclass = 0; iclass < MAX_ABILITY; iclass++)
                        {
                                snprintf(buf, MSL,
                                         "    &B%c&z%-17s  &w%+2d\n<br>",
                                         toupper(ability_name[iclass][0]),
                                         ability_name[iclass] + 1,
                                         race->class_modifier(iclass));
                                send_buf(wdesc->fd, buf, 2);
                        }

                        snprintf(buf, MSL, "&BB&zody parts\n<br>");
                        send_buf(wdesc->fd, buf, 2);

                        if (xIS_EMPTY(race->body_parts()))
                        {
                                snprintf(buf, MSL, "    &BN&zone<br>\n");
                                send_buf(wdesc->fd, buf, 2);
                        }
                        else
                        {
                                mudstrlcpy(argument,
                                           ext_flag_string(&race->
                                                           body_parts(),
                                                           part_flags), MSL);

                                argument = one_argument(argument, buf2);
                                while (buf2 && buf2[0] != '\0')
                                {
                                        snprintf(buf, MSL,
                                                 "    &B%c&z%s<br>\n",
                                                 toupper(buf2[0]), buf2 + 1);
                                        send_buf(wdesc->fd, buf, 2);
                                        argument =
                                                one_argument(argument, buf2);
                                }
                        }
                        web_footer(wdesc);
                        return TRUE;
                }
        }
        web_footer(wdesc);
        return FALSE;
}

bool check_clan_net(WEB_DESCRIPTOR * wdesc)
{
        CLAN_DATA *clan;
        char buf[MAX_STRING_LENGTH];
        char buf2[MAX_STRING_LENGTH];
        char buf3[MAX_STRING_LENGTH];
        char buf4[MAX_STRING_LENGTH];


        web_header(wdesc, "Clan Listing");
        send_buf(wdesc->fd,
                 "&B-----------------------------------------------------------------------------\n",
                 2);

        for (clan = first_clan; clan; clan = clan->next)
        {

                sprintf(buf3, "/clan/%s.htm ",
                        convert_sp(strlower(clan->name)));
                one_argument(clan->name, buf4);
                sprintf(buf2, "/clan/%s.htm ", strlower(buf4));

                if (strstr(wdesc->request, buf3)
                    || strstr(wdesc->request, buf2))
                {
                        snprintf(buf, MSL, "&BO&zrganization: &W%s\n",
                                 clan->name);
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL, "  &BM&zotto: &W%s\n",
                                 clan->motto);
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL, "  &BD&zescription: &W%s\n",
                                 clan->description);
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL, "  &BE&znlisting?: &W%-3s&B\n",
                                 clan->enliston ==
                                 1 ? "Yes" : clan->enliston ==
                                 0 ? "No" : "Unknown");
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "  &BM&zembers: &W%-4d&B\n  &BM&zin. &BA&zlign: &W%-5d\n",
                                 clan->members, clan->alignment);
                        send_buf(wdesc->fd, buf, 2);
                        send_buf(wdesc->fd,
                                 "&B-----------------------------------------------------------------------------\n",
                                 2);
                        return TRUE;
                }
        }
        web_footer(wdesc);
        return FALSE;
}

void handle_web_planet_request(WEB_DESCRIPTOR * wdesc)
{
        PLANET_DATA *planet;
        char buf[MAX_STRING_LENGTH];


        send_buf(wdesc->fd, "<html>\n", FALSE);
        send_buf(wdesc->fd, "<head>\n", FALSE);
        send_buf(wdesc->fd, "<title>Dark Warriors - Planet Listing</title>\n",
                 FALSE);
        send_buf(wdesc->fd,
                 "<style> pre { display: inline; }  a:link { background-color: inherit; ",
                 FALSE);
        send_buf(wdesc->fd, "text-decoration: none; color: inherit; }\n\r",
                 FALSE);
        send_buf(wdesc->fd,
                 "a:visited { background-color: inherit; text-decoration: none; color: inherit; }",
                 FALSE);
        send_buf(wdesc->fd,
                 "\n\ra:hover { background-color: inherit; text-decoration: none; ",
                 FALSE);
        send_buf(wdesc->fd,
                 "color: inherit; }\n\ra:active { background-color: inherit; text-decoration: none; ",
                 FALSE);
        send_buf(wdesc->fd, "color: inherit; }</style></head>\n", FALSE);
        send_buf(wdesc->fd,
                 "<body bgcolor=black text=white topmargin=0 rightmargin=0 bottommargin=0 leftmargin=0>\n\r",
                 FALSE);
        send_buf(wdesc->fd,
                 "<h1><center>Summary of Dark Warrior Planet Listing</center></h1>\n",
                 FALSE);
        send_buf(wdesc->fd, "<b><font size=" "2" ">\n", FALSE);
        send_buf(wdesc->fd, "<br><hr color=" "#FFFFFF" "><br>\n", FALSE);

        send_buf(wdesc->fd, "<br><pre>", 2);
        if (!first_planet)
        {
                send_buf(wdesc->fd,
                         "&WThere are no planets currently formed.\n", 2);
                return;
        }

        send_buf(wdesc->fd,
                 "&B-----------------------------------------------------------------------------\n",
                 2);

        if (!first_planet)
        {
                send_buf(wdesc->fd,
                         "&WThere are no planets currently formed.\n", 2);
                send_buf(wdesc->fd, "</pre></body>\n", FALSE);
                send_buf(wdesc->fd, "</html>\n", FALSE);
                return;
        }

        for (planet = first_planet; planet; planet = planet->next)
        {
                send_buf(wdesc->fd, "&BP&zlanet: &w", 2);
                sprintf(buf, "<a href=/planet/%s.htm>",
                        convert_sp(strlower(planet->name)));
                send_buf(wdesc->fd, buf, 2);
                sprintf(buf, "%-15.15s        ", planet->name);
                send_buf(wdesc->fd, buf, 2);
                send_buf(wdesc->fd, "</a> ", 2);
                snprintf(buf, MSL, "&BG&zoverned &BB&zy: &w%s %s\n",
                         planet->governed_by ? planet->governed_by->name : "",
                         IS_SET(planet->flags,
                                PLANET_NOCAPTURE) ? "&B(&zpermanent&B)" : "");
                send_buf(wdesc->fd, buf, 2);
                snprintf(buf, MSL, "&BV&zalue: &w%-10ld&z/&w%-10ld   ",
                         get_taxes(planet), planet->base_value);
                send_buf(wdesc->fd, buf, 2);
                snprintf(buf, MSL,
                         "&BP&zopulation: &w%-5d    &BP&zop &BS&zupport: &w%.1d\n",
                         planet->population, planet->pop_support);
                send_buf(wdesc->fd, buf, 2);
                if (planet->next)
                        send_buf(wdesc->fd,
                                 "                     &R-------------------------------                         \n",
                                 2);
                else
                        send_buf(wdesc->fd,
                                 "&B-----------------------------------------------------------------------------\n",
                                 2);
        }

        send_buf(wdesc->fd, "<br><br>", 2);
        send_buf(wdesc->fd, "&YClick planet name&z for more information.\n\r",
                 2);
        send_buf(wdesc->fd, "</pre></body>\n", FALSE);
        send_buf(wdesc->fd, "</html>\n", FALSE);
        return;
}

void handle_edithelp_message_request(WEB_DESCRIPTOR * wdesc)
{
        HELP_DATA *help;
        char buf[MAX_STRING_LENGTH];
        char file[MSL];
        char     *filep;

        std::vector < std::string > arguments;

        web_header(wdesc, "Edit help file");

        mudstrlcpy(buf, strip_web_content(wdesc), MSL);

        arguments = explodestring(buf, "?");

        filep = convert_sp_reverse((char *) arguments[1].c_str());

        mudstrlcpy(file, filep, MSL);

        if (!(help = get_web_help(file)))
        {
                send_buf(wdesc->fd,
                         "Sorry, we can't find that help file!\n\r", 2);
                send_buf(wdesc->fd, file, 2);
                web_footer(wdesc);
                return;
        }

        send_buf(wdesc->fd,
                 "<form method=\"POST\" action=\"parsehelp.php\">\n", 2);
        send_buf(wdesc->fd,
                 "<table>\n<tr>\n<td>Keyword</td>\n<td><input type=\"text\" name=\"keyword\" value=\"",
                 2);
        send_buf(wdesc->fd, help->keyword, 2);
        send_buf(wdesc->fd, "\" /></td>\n</tr>\n", 2);
        send_buf(wdesc->fd,
                 "<tr>\n<td>Level</td>\n<td><input type=\"text\" name=\"level\" value=\"",
                 2);
        sprintf(buf, "%d", help->level);
        send_buf(wdesc->fd, buf, 2);
        send_buf(wdesc->fd, "\" /></td>\n</tr>", 2);
        send_buf(wdesc->fd,
                 "<tr>\n<td>Text</td\n><td><textarea cols=\"90\" rows=\"10\" name=\"level\">",
                 2);
        send_buf(wdesc->fd, help->text, 0);
        send_buf(wdesc->fd, "</textarea></td\n></tr>\n", 2);
        send_buf(wdesc->fd, "</table\n></form\n>", 2);
        web_footer(wdesc);
        return;
}

void handle_web_message_request(WEB_DESCRIPTOR * wdesc)
{
        char buf[MAX_STRING_LENGTH];
        char command[MSL];
        char argumentbuf[MAX_STRING_LENGTH];
        char     *argument = argumentbuf;
        bool incommand = FALSE;

        mudstrlcpy(command, "", MSL);
        mudstrlcpy(argument, convert_sp_reverse(wdesc->request), MSL);

        argument = one_argument(argument, buf);

        while (buf && buf[0] != '\0')
        {
                if (!str_cmp(buf, "get"))
                {
                        argument = one_argument(argument, buf);
                        incommand = TRUE;
                        continue;
                }

                if (incommand)
                {
                        argument = one_argument(argument, buf);
                        incommand = FALSE;
                        continue;
                }

                if (!str_prefix("http", strlower(buf)))
                        break;

                mudstrlcat(command, buf, MSL);
                mudstrlcat(command, " ", MSL);
                argument = one_argument(argument, buf);
        }

        web_broadcast(command);

        return;
}

bool check_planet_net(WEB_DESCRIPTOR * wdesc)
{
        PLANET_DATA *planet;
        char buf[MAX_STRING_LENGTH];
        char buf2[MAX_STRING_LENGTH];
        char buf3[MAX_STRING_LENGTH];
        char buf4[MAX_STRING_LENGTH];


        send_buf(wdesc->fd, "<html>\n", FALSE);
        send_buf(wdesc->fd, "<head>\n", FALSE);
        send_buf(wdesc->fd, "<title>Dark Warriors - Planet Listing</title>\n",
                 FALSE);
        send_buf(wdesc->fd,
                 "<style> pre { display: inline; }  a:link { background-color: inherit; ",
                 FALSE);
        send_buf(wdesc->fd, "text-decoration: none; color: inherit; }\n\r",
                 FALSE);
        send_buf(wdesc->fd,
                 "a:visited { background-color: inherit; text-decoration: none; color: inherit; }",
                 FALSE);
        send_buf(wdesc->fd,
                 "\n\ra:hover { background-color: inherit; text-decoration: none; ",
                 FALSE);
        send_buf(wdesc->fd,
                 "color: inherit; }\n\ra:active { background-color: inherit; text-decoration: none; ",
                 FALSE);
        send_buf(wdesc->fd, "color: inherit; }</style></head>\n", FALSE);
        send_buf(wdesc->fd,
                 "<body bgcolor=black text=white topmargin=0 rightmargin=0 bottommargin=0 leftmargin=0>\n\r",
                 FALSE);
        send_buf(wdesc->fd,
                 "<h1><center>Summary of Dark Warrior Planet Listing</center></h1>\n",
                 FALSE);
        send_buf(wdesc->fd, "<b><font size=" "2" ">\n", FALSE);
        send_buf(wdesc->fd, "<br><hr color=" "#FFFFFF" "><br>\n", FALSE);

        send_buf(wdesc->fd, "<br><pre>", 2);
        send_buf(wdesc->fd,
                 "&B-----------------------------------------------------------------------------\n",
                 2);

        for (planet = first_planet; planet; planet = planet->next)
        {

                sprintf(buf3, "/planet/%s.htm ",
                        convert_sp(strlower(planet->name)));
                one_argument(planet->name, buf4);
                sprintf(buf2, "/planet/%s.htm ", strlower(buf4));

                if (strstr(wdesc->request, buf3)
                    || strstr(wdesc->request, buf2))
                {
                        snprintf(buf, MSL,
                                 "&BP&zlanet: &w%-15s        &BG&zoverned &BB&zy: &w%s %s\n",
                                 planet->name,
                                 planet->governed_by ? planet->governed_by->
                                 name : "", IS_SET(planet->flags,
                                                   PLANET_NOCAPTURE) ?
                                 "&B(&zpermanent&B)" : "");
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "&BV&zalue: &w%-10ld&z/&w%-10ld   ",
                                 get_taxes(planet), planet->base_value);
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "&BP&zopulation: &w%-5d    &BP&zop &BS&zupport: &w%.1d\n",
                                 planet->population, planet->pop_support);
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "&BA&zttacking &BB&zatallions: &w%-5d    &BD&zefending &BB&zatallions: &w%.1d\n",
                                 planet->attbattalions,
                                 planet->defbattalions);
                        send_buf(wdesc->fd, buf, 2);
                        snprintf(buf, MSL,
                                 "&BP&zlanetary &BS&zhields: &w%-8d    &BT&zurbolasers: &w%.1d &BI&zon &BC&zannons: &w%.1d\n",
                                 planetary_installations(planet,
                                                         BATTERY_INSTALLATION),
                                 planetary_installations(planet,
                                                         TURBOLASER_INSTALLATION),
                                 planetary_installations(planet,
                                                         ION_INSTALLATION));
                        send_buf(wdesc->fd, buf, 2);
                        send_buf(wdesc->fd,
                                 "&B-----------------------------------------------------------------------------\n",
                                 2);
                        return TRUE;
                }
        }
        return FALSE;
}

CMDF do_webserver(CHAR_DATA * ch, char *argument)
{
//        WEB_DESCRIPTOR *current;
        char arg1[MAX_STRING_LENGTH];
        char arg2[MAX_STRING_LENGTH];

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                ch_printf(ch, "&BW&zebserver Information\n\r");
                ch_printf(ch, "&BS&ztatus: %s&z\n\r",
                          WEBSERVER_STATUS ? "&wOnline" : "&ROffline");
                ch_printf(ch, "&BS&zocket ID: &w%d  &B[&zPort: &w%d&B]\n\r",
                          sockfd ? sockfd : 0, portid ? portid : 0);
                ch_printf(ch, "\n\r");
                ch_printf(ch,
                          "&BS&zyntax:&w WEBSERVE [START] &z<&wport&z>\n\r");
                ch_printf(ch, "        &wWEBSERVE [STOP]\n\r");
                ch_printf(ch, "        &wWEBSERVE [SOCKETS]\n\r");
                return;
        }

        if (!str_cmp(arg1, "start"))
        {
                if (atoi(arg2) < 1 || atoi(arg2) > 9999)
                {
                        ch_printf(ch, "Invalid port number: %d\n\r",
                                  atoi(arg2));
                        return;
                }
                if (WEBSERVER_STATUS)
                {
                        ch_printf(ch, "Already Running the Web service.\n\r");
                        return;
                }
                log_string("Booting internal web service.");
                init_web(atoi(arg2));
        }
        else if (!str_cmp(arg1, "stop"))
        {
                if (!WEBSERVER_STATUS)
                {
                        ch_printf(ch,
                                  "The web service is not currently running.\n\r");
                        return;
                }
                log_string("Disabling internal web service.");
                shutdown_web();
        }
/*        else if (!str_cmp(arg1, "sockets"))
        {
                int       count = 0;

                if (!WEBSERVER_STATUS)
                {
                        send_to_char("&YThe webserver isn't running!\n\r",
                                     ch);
                        return;
                }

                for (current = first_webdesc; current;
                     current = current->next)
                {
                        count++;
                        ch_printf(ch,
                                  "Web descriptor: %d, Originating IP: %s\n\r",
                                  current->fd,
                                  inet_ntoa(current->their_addr.sin_addr));
                }

                ch_printf(ch, "Total Web descriptors: %d\n\r", count);
                return;
        }*/
        else
                do_webserver(ch, "");
}

void web_header(WEB_DESCRIPTOR * wdesc, char *title)
{
        char buf[MAX_STRING_LENGTH];

        sprintf(buf, "<title>Dark Warriors - %s</title>\n", title);
        send_buf(wdesc->fd, "<html>\n", FALSE);
        send_buf(wdesc->fd, "<head>\n", FALSE);
        send_buf(wdesc->fd, buf, FALSE);
        send_buf(wdesc->fd,
                 "<style> pre { display: inline; }  a:link { background-color: inherit; ",
                 FALSE);
        send_buf(wdesc->fd, "text-decoration: none; color: inherit; }\n\r",
                 FALSE);
        send_buf(wdesc->fd,
                 "a:visited { background-color: inherit; text-decoration: none; color: inherit; }",
                 FALSE);
        send_buf(wdesc->fd,
                 "\n\ra:hover { background-color: inherit; text-decoration: none; ",
                 FALSE);
        send_buf(wdesc->fd,
                 "color: inherit; }\n\ra:active { background-color: inherit; text-decoration: none; ",
                 FALSE);
        send_buf(wdesc->fd, "color: inherit; }</style></head>\n", FALSE);
        send_buf(wdesc->fd,
                 "<body bgcolor=black text=white topmargin=0 rightmargin=0 bottommargin=0 leftmargin=0>\n\r",
                 FALSE);
        sprintf(buf, "<h1><center>Dark Warrior %s</center></h1>\n", title);
        send_buf(wdesc->fd, buf, FALSE);
        send_buf(wdesc->fd, "<br><hr color=" "#FFFFFF" "><br>\n", FALSE);
}


void web_footer(WEB_DESCRIPTOR * wdesc)
{
        send_buf(wdesc->fd, "<br><br>", 2);
        send_buf(wdesc->fd, "</body>\n", FALSE);
        send_buf(wdesc->fd, "</html>\n", FALSE);
}


void print_ooc_history(WEB_DESCRIPTOR * wdesc)
{
        CHANNEL_DATA *channel;
        char buf[MAX_STRING_LENGTH];
        char buf1[MAX_STRING_LENGTH];
        int count = 0;
        int pos = 0;

        web_header(wdesc, "Index");

        channel = get_channel("ooc");
        if (channel == NULL)
        {
                send_buf(wdesc->fd, "No such channel", 2);
                return;
        }

        sprintf(buf, "&B%c&z%s History\n\rB-----------\n\r", channel->name[0],
                channel->name + 1);
        send_buf(wdesc->fd, buf, 2);
        while (1)
        {
                if (count++ >= sysdata.channellog)
                        break;
                if (pos > channel->logpos)
                        break;
                buf[0] = '\0';

                snprintf(buf1, MSL, "%s", channel->log[pos].message);
                snprintf(buf, MSL,
                         "&B[&W%2d&B][&W%.24s&B]&D %s %s&W: &W%s&w\n\r",
                         count,
                         ctime(&channel->log[pos].time),
                         channel->name, channel->log[pos].name, buf1);
                send_buf(wdesc->fd, buf, 2);
                send_buf(wdesc->fd, "<br>", 2);
                pos++;
        }

        web_footer(wdesc);
}


char     *strip_web_content(WEB_DESCRIPTOR * wdesc)
{
        static char buf[MSL];
        char     *getptr;
        char     *httpptr;
        char     *bufptr;

        strcpy(buf, "");
        if ((getptr = strstr(wdesc->request, "GET")) == NULL)
        {
                log_string("No get!");
        }
        else
        {
                getptr += 4;
        }
        if ((httpptr = strstr(wdesc->request, "HTTP")) == NULL)
        {
                log_string("No HTTP!");
        }
        else
        {
                for (bufptr = buf; getptr != httpptr; getptr++)
                        *bufptr++ = *getptr;
                bufptr++;
                bufptr = '\0';
        }
        return buf;
}


std::vector < std::string > explodestring(char *oldstring, char *delim)
{
        std::vector < std::string > ret;
        std::string str = oldstring;
        std::string del = delim;
        int iPos = str.find(delim, 0);
        int iPit = del.length();

        while (iPos > -1)
        {
                if (iPos != 0)
                        ret.push_back(str.substr(0, iPos));
                str.erase(0, iPos + iPit);
                iPos = str.find(delim, 0);
        }

        if (str != "")
                ret.push_back(str);
        return ret;
}

CMDF do_testexplode(CHAR_DATA * ch, char *argument)
{
        std::vector < std::string > stringarr;
        int i;

        stringarr = explodestring(argument, ";");

        for (i = 0; i < stringarr.size(); i++)
                ch_printf(ch, "%d) %s\n\r", i, stringarr[i].c_str());

}

void handle_web_changes_request(WEB_DESCRIPTOR * wdesc)
{
        CLAN_DATA *clan;
        char buf[MAX_STRING_LENGTH];
        int count;


        web_header(wdesc, "Clan Listing");
        send_buf(wdesc->fd, "<b><font size='2'>\n", FALSE);
        send_buf(wdesc->fd, "<br><hr color='#FFFFFF'><br>\n", FALSE);
        send_buf(wdesc->fd, "<br><pre>", 2);
        send_buf(wdesc->fd,"&B-----------------------------------------------------------------------------\n",2);

        send_buf(wdesc->fd, "</html>\n", FALSE);
        web_footer(wdesc);
        return;
}

