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
 *                              Mud Extension Protocol Module                            *
 *****************************************************************************************
 *                $Id: mxp.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
/*
 * Based on code from Nick Gammon - 
 * http://www.gammon.com.au/mushclient/addingservermxp.htm 
 */
#include <arpa/telnet.h>
#include <string.h>
#include "mud.h"
#include "mxp.h"

const unsigned char wont_mxp_str[] = { IAC, WONT, TELOPT_MXP, '\0' };
const unsigned char will_mxp_str[] = { IAC, WILL, TELOPT_MXP, '\0' };
const unsigned char start_mxp_str[] = { IAC, SB, TELOPT_MXP, IAC, SE, '\0' };
const unsigned char do_mxp_str[] = { IAC, DO, TELOPT_MXP, '\0' };
const unsigned char dont_mxp_str[] = { IAC, DONT, TELOPT_MXP, '\0' };

CMDF do_mxp(CHAR_DATA * ch, char *argument)
{
        if (IS_NPC(ch) || !ch->desc)
        {
                send_to_char("What descriptor?!\n", ch);
                return;
        }

        if (!str_cmp(argument, "all") && IS_IMMORTAL(ch))
        {
                char      buf[MSL];
                CHAR_DATA *vch;

                send_to_pager("MXP Info:\n", ch);
                for (vch = first_char; vch; vch = vch->next)
                {
                        if (vch->desc == NULL || !IS_PLAYING(vch->desc))
                                continue;
                        snprintf(buf, MSL, "%s: &B[&w%s&B]&w\n", vch->name,
                                 IS_MXP(vch) ? "ON " : "OFF");
                        send_to_pager(buf, ch);
                }
                return;
        }
        else if (!str_cmp(argument, "toggle"))
        {
                if (!ch->desc->out_compress)
                {
                        do_mxp(ch, "on");
                        return;
                }
                else
                {
                        do_mxp(ch, "off");
                        return;
                }
        }
        else if (!str_cmp(argument, "on"))
        {
                send_to_char("Initalizing MXP.\n\r", ch);
                SET_BIT(ch->act, PLR_MXP);
                send_mxp_stylesheet(ch->desc);
        }
        else if (!str_cmp(argument, "off"))
        {
                send_to_char("Terminating MXP.\n\r", ch);
                REMOVE_BIT(ch->act, PLR_MXP);
        }
        else if (!str_cmp(argument, "auto"))
        {
                send_to_char("Not Complete yet", ch);
        }
        else
        {
                send_to_char("MXP Info:\n\r", ch);
                send_to_char("MXP: &B[&w", ch);
                if (IS_MXP(ch))
                        send_to_char(MXPTAG("mxptoggle mxp"), ch);

                if (IS_MXP(ch))
                        send_to_char("ON ", ch);
                else
                        send_to_char("OFF", ch);

                if (IS_MXP(ch))
                        send_to_char(MXPTAG("/mxptoggle"), ch);
                send_to_char("&B]&W\n\r", ch);
                return;
        }
}

/*
 * Count number of mxp tags need converting
 *    ie. < becomes &lt;
 *        > becomes &gt;
 *        & becomes &amp;
 */

int count_mxp_tags(DESCRIPTOR_DATA * d, const char *txt, int length)
{
        char      c;
        const char *p;
        int       count;
        int       bInTag = FALSE;
        int       bInEntity = FALSE;
        bool      bMXP = d->mxp_detected;

/*	if (d->character)
		bMXP = IS_SET(d->character->act, PLR_MXP); */

        for (p = txt, count = 0; length > 0; p++, length--)
        {
                c = *p;

                if (bInTag) /* in a tag, eg. <send> */
                {
                        if (!bMXP)
                                count--;    /* not output if not MXP */
                        if (c == MXP_ENDc)
                                bInTag = FALSE;
                }   /* end of being inside a tag */
                else if (bInEntity) /* in a tag, eg. <send> */
                {
                        if (!bMXP)
                                count--;    /* not output if not MXP */
                        if (c == ';')
                                bInEntity = FALSE;
                }   /* end of being inside a tag */
                else
                        switch (c)
                        {
                        case MXP_BEGc:
                                bInTag = TRUE;
                                if (!bMXP)
                                        count--;    /* not output if not MXP */
                                break;

                        case MXP_ENDc: /* shouldn't get this case */
                                if (!bMXP)
                                        count--;    /* not output if not MXP */
                                break;

                        case MXP_AMPc:
                                bInEntity = TRUE;
                                if (!bMXP)
                                        count--;    /* not output if not MXP */
                                break;

                        default:
                                if (bMXP)
                                {
                                        switch (c)
                                        {
                                        case '<':  /* < becomes &lt; */
                                        case '>':  /* > becomes &gt; */
                                                count += 3;
                                                break;

                                        case '&':
                                                count += 4; /* & becomes &amp; */
                                                break;

                                        case '"':  /* " becomes &quot; */
                                                count += 5;
                                                break;

                                        }   /* end of inner switch */
                                }   /* end of MXP enabled */
                        }   /* end of switch on character */

        }   /* end of counting special characters */

        return count;
}   /* end of count_mxp_tags */

void convert_mxp_tags(DESCRIPTOR_DATA * d, char *dest, const char *src,
                      int length)
{
        char      c;
        const char *ps;
        char     *pd;
        int       bInTag = FALSE;
        int       bInEntity = FALSE;
        bool      bMXP = d->mxp_detected;

/*	if (d->character)
		bMXP = IS_SET(d->character->act, PLR_MXP);*/

        for (ps = src, pd = dest; length > 0; ps++, length--)
        {
                c = *ps;
                if (bInTag) /* in a tag, eg. <send> */
                {
                        if (c == MXP_ENDc)
                        {
                                bInTag = FALSE;
                                if (bMXP)
                                        *pd++ = '>';
                        }
                        else if (bMXP)
                                *pd++ = c;  /* copy tag only in MXP mode */
                }   /* end of being inside a tag */
                else if (bInEntity) /* in a tag, eg. <send> */
                {
                        if (bMXP)
                                *pd++ = c;  /* copy tag only in MXP mode */
                        if (c == ';')
                                bInEntity = FALSE;
                }   /* end of being inside a tag */
                else
                        switch (c)
                        {
                        case MXP_BEGc:
                                bInTag = TRUE;
                                if (bMXP)
                                        *pd++ = '<';
                                break;

                        case MXP_ENDc: /* shouldn't get this case */
                                if (bMXP)
                                        *pd++ = '>';
                                break;

                        case MXP_AMPc:
                                bInEntity = TRUE;
                                if (bMXP)
                                        *pd++ = '&';
                                break;

                        default:
                                if (bMXP)
                                {
                                        switch (c)
                                        {
                                        case '<':
                                                memcpy(pd, "&lt;", 4);
                                                pd += 4;
                                                break;

                                        case '>':
                                                memcpy(pd, "&gt;", 4);
                                                pd += 4;
                                                break;

                                        case '&':
                                                memcpy(pd, "&amp;", 5);
                                                pd += 5;
                                                break;

                                        case '"':
                                                memcpy(pd, "&quot;", 6);
                                                pd += 6;
                                                break;

                                        default:
                                                *pd++ = c;
                                                break;  /* end of default */

                                        }   /* end of inner switch */
                                }
                                else
                                        *pd++ = c;  /* not MXP - just copy character */
                                break;

                        }   /* end of switch on character */

        }   /* end of converting special characters */
}   /* end of convert_mxp_tags */

void send_mxp_stylesheet(DESCRIPTOR_DATA * d)
{
#if 0
        FILE     *stylesheet_file;
        int       num = 0;
        char      buf[MSL * 2];

        d->mxp = TRUE;
        write_to_buffer(d, start_mxp_str, 0);
        write_to_buffer(d, MXPMODE(6), 0);  /* permanent secure mode */
        write_to_buffer(d, MXPTAG("!-- Set up MXP elements --"), 0);

        if ((stylesheet_file = fopen(MXP_STYLESHEET_FILE, "r")) != NULL)
        {
                while ((buf[num] = fgetc(stylesheet_file)) != EOF)
                        num++;
                FCLOSE(stylesheet_file);
                stylesheet_file = NULL;
                buf[num] = 0;
                write_to_buffer(d, buf, num);
        }
#else
        write_to_buffer(d, (const char *) start_mxp_str, 0);
        write_to_buffer(d, MXPMODE(6), 0);  /* permanent secure mode */
        write_to_buffer(d, MXPTAG("!-- Set up MXP elements --"), 0);

        write_to_buffer(d,
                        MXPTAG("!-- Elements to support the Automapper --"),
                        0);
        write_to_buffer(d, MXPTAG("!ELEMENT RName '<B>' FLAG=RoomName"), 0);
        write_to_buffer(d, MXPTAG("!ELEMENT RNum FLAG=RoomNum"), 0);
        /*
         * Room description tag 
         */
        write_to_buffer(d, MXPTAG("!ELEMENT RDesc FLAG=RoomDesc"), 0);
        write_to_buffer(d, MXPTAG("!ELEMENT RExits FLAG=RoomExit"), 0);
        /*
         * Exit tag 
         */
        write_to_buffer(d, MXPTAG("!ELEMENT Ex '<send>' "), 0);
        /*
         * Prompt Stuff 
         */
        write_to_buffer(d, MXPTAG("!-- Elements to support the Prompt --"),
                        0);
        write_to_buffer(d, MXPTAG("!ELEMENT Prompt FLAG=Prompt"), 0);
        write_to_buffer(d, MXPTAG("!ELEMENT Hp FLAG='Set hp'"), 0);
        write_to_buffer(d, MXPTAG("!ELEMENT MaxHp FLAG='Set maxhp'"), 0);
        write_to_buffer(d, MXPTAG("!ELEMENT Mana FLAG='Set mana'"), 0);
        write_to_buffer(d, MXPTAG("!ELEMENT Maxmana FLAG='Set maxmana'"), 0);
        write_to_buffer(d, MXPTAG("!ELEMENT Move FLAG='Set move'"), 0);
        write_to_buffer(d, MXPTAG("!ELEMENT Maxmove FLAG='Set maxmove'"), 0);
        /*
         * Option Toggle 
         */
        write_to_buffer(d,
                        MXPTAG
                        ("!ELEMENT mxptoggle \"<send href='&cmd; toggle'>\" ATT='cmd'"),
                        0);
        /*
         * Help 
         */
        write_to_buffer(d,
                        MXPTAG
                        ("!ELEMENT help \"<send href='help &keyword;'>\" ATT='keyword'"),
                        0);
        /*
         * Practice
         */
        //write_to_buffer(d,MXPTAG("!ELEMENT practice \"<send href='practice &cmd;'>\" ATT='cmd'"),0);
        write_to_buffer(d, MXPTAG
                        ("!ELEMENT practice \"<send href='"
                         "practice &cmd;|"
                         "&cmd;"
                         "'"
                         "hint='RH mouse click to use this object|"
                         "practice &cmd;|" "do &cmd;" "'>\" ATT='cmd'"), 0);
        /*
         * Account for voting too 
         */
        write_to_buffer(d, MXPTAG
                        ("!ELEMENT note \"<send href='"
                         "&cmd; read &number;|"
                         "&cmd; take &number;|"
                         "&cmd; remove &number;"
                         "'"
                         "hint='RH mouse click to use this object|"
                         "Read &number;|"
                         "Take &number;|"
                         "Delete &number;" "'>\" ATT='cmd number'"), 0);
        /*
         * Channel stuff 
         */
        /*
         * Auction = Tag 20 
         */
        write_to_buffer(d, MXPTAG("!ELEMENT auction \"<bold>\" TAG=20 OPEN"),
                        0);
        write_to_buffer(d, MXPTAG("!ELEMENT chat \"<bold>\" TAG=21 OPEN"), 0);
        write_to_buffer(d, MXPTAG
                        ("!ELEMENT shuttle \"<send href='"
                         "board &name;|"
                         "' "
                         "hint='RH mouse click to use this object|"
                         "Board &name;|" "'>\" ATT='name'"), 0);
        write_to_buffer(d, MXPTAG
                        ("!ELEMENT ship \"<send href='"
                         "board &name;|"
                         "open &quot;&name;&quot;|"
                         "close &quot;&name;&quot;"
                         "' "
                         "hint='RH mouse click to use this object|"
                         "Board &name;|"
                         "Open &name;&#39;s Hatch|"
                         "Close &name;&#39;s Hatch" "'>\" ATT='name'"), 0);
        /*
         * Get an item tag (for things on the ground) 
         */
        write_to_buffer(d, MXPTAG
                        ("!ELEMENT Get \"<send href='"
                         "get &#39;&name;&#39;|"
                         "examine &#39;&name;&#39;|"
                         "drink &#39;&name;&#39;"
                         "' "
                         "hint='RH mouse click to use this object|"
                         "Get &desc;|"
                         "Examine &desc;|"
                         "Drink from &desc;" "'>\" ATT='name desc'"), 0);
        /*
         * Drop an item tag (for things in the inventory) 
         */
        write_to_buffer(d, MXPTAG
                        ("!ELEMENT Drop \"<send href='"
                         "drop &#39;&name;&#39;|"
                         "examine &#39;&name;&#39;|"
                         "look in &#39;&name;&#39;|"
                         "wear &#39;&name;&#39;|"
                         "eat &#39;&name;&#39;|"
                         "drink &#39;&name;&#39;"
                         "' "
                         "hint='RH mouse click to use this object|"
                         "Drop &desc;|"
                         "Examine &desc;|"
                         "Look inside &desc;|"
                         "Wear &desc;|"
                         "Eat &desc;|"
                         "Drink &desc;" "'>\" ATT='name desc'"), 0);
        /*
         * Bid an item tag (for things in the auction) 
         */
        write_to_buffer(d, MXPTAG
                        ("!ELEMENT Bid \"<send href='bid &#39;&name;&#39;' "
                         "hint='Bid for &desc;'>\" " "ATT='name desc'"), 0);
        /*
         * List an item tag (for things in a shop) 
         */
        write_to_buffer(d, MXPTAG
                        ("!ELEMENT List \"<send href='buy &#39;&name;&#39;' "
                         "hint='Buy &desc;'>\" " "ATT='name desc'"), 0);
        /*
         * Player tag (for who lists, tells etc.) 
         */
        write_to_buffer(d, MXPTAG
                        ("!ELEMENT Player \"<send href='"
                         "tell &#39;&name;&#39; |"
                         "otell &#39;&name;&#39; |"
                         "beep &#39;&name;&#39; |"
                         "whois &#39;&name;&#39;"
                         "' "
                         "hint='Send a message to &name;|"
                         "Send a OOC message to &name;|"
                         "Send a beep message to &name;|"
                         "whois &name;' prompt>\" " "ATT='name'"), 0);
/*	write_to_buffer( d, MXPTAG("version"), 0);
	write_to_buffer( d, MXPTAG("support"), 0); */
#endif
}
