#ifdef MCCP
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
 *                $Id: mccp.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
/*
 * Ported to SMAUG by Garil of DOTDII Mud
 * aka Jesse DeFer <dotd@dotd.com>  http://www.dotd.com
 *
 * revision 1: MCCP v1 support
 * revision 2: MCCP v2 support
 * revision 3: Correct MMCP v2 support
 * revision 4: clean up of write_to_descriptor() suggested by Noplex@CB
 *
 * See the web site below for more info.
 */

/*
 * mccp.c - support functions for mccp (the Mud Client Compression Protocol)
 *
 * see http://homepages.ihug.co.nz/~icecube/compress/ and README.Rom24-mccp
 *
 * Copyright (c) 1999, Oliver Jowett <icecube@ihug.co.nz>.
 *
 * This code may be freely distributed and used if this copyright notice is
 * retained intact.
 */

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
#include <zlib.h>
#include <errno.h>

/* For memmove */
#include <string.h>

#include "mud.h"
#ifdef MXP
#include "mxp.h"
#endif

bool write_to_descriptor args((int desc, char *txt, int length));

void     *zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
        opaque = NULL;
        return calloc(items, size);
}

void zlib_free(void *opaque, void *address)
{
        opaque = NULL;
        DISPOSE(address);
}


bool process_compressed(DESCRIPTOR_DATA * d)
{
        int       iStart = 0, nBlock, nWrite, len;

        if (!d->out_compress)
                return TRUE;

        len = d->out_compress->next_out - d->out_compress_buf;

        if (len > 0)
        {
                for (iStart = 0; iStart < len; iStart += nWrite)
                {
                        nBlock = UMIN(len - iStart, 4096);
                        if ((nWrite =
                             write(d->descriptor,
                                   d->out_compress_buf + iStart, nBlock)) < 0)
                        {
                                if (errno == EAGAIN || errno == ENOSR)
                                        break;

                                return FALSE;
                        }

                        if (!nWrite)
                                break;
                }

                if (iStart)
                {
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

/* equiv to start_mxp */
unsigned char enable_compress[] =
        { IAC, SB, TELOPT_COMPRESS, WILL, SE, '\0' };
unsigned char enable_compress2[] =
        { IAC, SB, TELOPT_COMPRESS2, IAC, SE, '\0' };

bool compressStart(DESCRIPTOR_DATA * d, unsigned char telopt)
{
        z_stream *s;

        if (d->out_compress)
                return TRUE;

/*    bug("Starting compression for descriptor %d", d->descriptor); */

        CREATE(s, z_stream, 1);
        CREATE(d->out_compress_buf, unsigned char, COMPRESS_BUF_SIZE);

        s->next_in = NULL;
        s->avail_in = 0;

        s->next_out = d->out_compress_buf;
        s->avail_out = COMPRESS_BUF_SIZE;

        s->zalloc = Z_NULL;
        s->zfree = Z_NULL;
        s->opaque = Z_NULL;

        if (deflateInit(s, 9) != Z_OK)
        {
                DISPOSE(d->out_compress_buf);
                DISPOSE(s);
                return FALSE;
        }

        if (telopt == TELOPT_COMPRESS)
                write_to_descriptor(d->descriptor, (char *) enable_compress,
                                    0);
        else if (telopt == TELOPT_COMPRESS2)
                write_to_descriptor(d->descriptor, (char *) enable_compress2,
                                    0);
        else
                bug("compressStart: bad TELOPT passed");

        d->compressing = telopt;
        d->out_compress = s;
        d->shellcompressing = 0;

        return TRUE;
}

bool compressEnd(DESCRIPTOR_DATA * d)
{
        unsigned char dummy[1];

        if (!d->out_compress)
                return TRUE;

/*    bug("Stopping compression for descriptor %d", d->descriptor); */

        d->out_compress->avail_in = 0;
        d->out_compress->next_in = dummy;

        if (deflate(d->out_compress, Z_FINISH) != Z_STREAM_END)
                return FALSE;

        if (!process_compressed(d))
                return FALSE;

        deflateEnd(d->out_compress);
        DISPOSE(d->out_compress_buf);
        DISPOSE(d->out_compress);
        d->shellcompressing = d->compressing;
        d->compressing = 0;

        return TRUE;
}

CMDF do_compress(CHAR_DATA * ch, char *argument)
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

                send_to_pager("Compression Info:\n", ch);
                for (vch = first_char; vch; vch = vch->next)
                {
                        if (vch->desc == NULL || !IS_PLAYING(vch->desc))
                                continue;
                        snprintf(buf, MSL, "%s: &B[&w%s&B]&w\n", vch->name,
                                 vch->desc->compressing ? "ON " : "OFF");
                        send_to_pager(buf, ch);
                }
                return;
        }
        else if (!str_cmp(argument, "toggle"))
        {
                if (!ch->desc->out_compress)
                {
                        do_compress(ch, "on");
                        return;
                }
                else
                {
                        do_compress(ch, "off");
                        return;
                }
        }
        else if (!str_cmp(argument, "on"))
        {
                send_to_char("Initiating compression.\n\r", ch);
                write_to_buffer(ch->desc, (const char *) will_compress2_str,
                                0);
                write_to_buffer(ch->desc, (const char *) will_compress_str,
                                0);
        }
        else if (!str_cmp(argument, "off"))
        {
                send_to_char("Terminating compression.\n\r", ch);
                compressEnd(ch->desc);
        }
        else if (!str_cmp(argument, "auto"))
        {
                send_to_char("Not Complete yet", ch);
        }
        else
        {
                set_char_color(AT_GREEN, ch);
                send_to_char("Compression Info:\n\r", ch);
                send_to_char("Compression: &B[&w", ch);
                if (IS_MXP(ch))
                        send_to_char(MXPTAG("mxptoggle compress"), ch);
                if (ch->desc->compressing)
                        send_to_char("ON ", ch);
                else
                {
                        send_to_char("OFF", ch);
                }
                if (IS_MXP(ch))
                        send_to_char(MXPTAG("/mxptoggle"), ch);
                send_to_char("&B]&D\n\r", ch);
                if (ch->desc->out_compress
                    && ch->desc->out_compress->total_in)
                        ch_printf(ch,
                                  "Total size of input compressed:  &B[&w%d&B]&D\n\r",
                                  ch->desc->out_compress->total_in);
                if (ch->desc->out_compress
                    && ch->desc->out_compress->total_out)
                        ch_printf(ch,
                                  "Total size of output compressed: &B[&w%d&B]&D\n\r",
                                  ch->desc->out_compress->total_out);
                if (ch->desc->out_compress && ch->desc->out_compress->total_in
                    && ch->desc->out_compress->total_out)
                        ch_printf(ch,
                                  "Current compression ratio:       &B[&w%.2f%&B]&D&D\n\r",
                                  100.0 -
                                  (float) ((float) ch->desc->out_compress->
                                           total_out /
                                           (float) ch->desc->out_compress->
                                           total_in * 100));
                return;
        }


}

CMDF do_mccpstats(CHAR_DATA * ch, char *argument)
{
        DESCRIPTOR_DATA *d;
        int       count = 0, total = 0;
        float     in = 0, out = 0;

        argument = NULL;

        for (d = first_descriptor; d; d = d->next)
        {
                total++;
                if (d->compressing && d->out_compress
                    && d->out_compress->total_in
                    && d->out_compress->total_out)
                {
                        count++;
                        in += d->out_compress->total_in;
                        out += d->out_compress->total_out;
                }
        }

        send_to_char("&zCompression Info for Dark Warriors:\n\r", ch);
        ch_printf(ch,
                  "Total size of data before compression: &C[&z%.0fKB&C]&z\n\r",
                  in / 1024);
        ch_printf(ch,
                  "Total size of data after compression: &C[&z%.0fKB&C]&z\n\r",
                  out / 1024);
        ch_printf(ch, "Total bandwidth saved: &C[&z%.0fKB&C]&z\n\r",
                  (in - out) / 1024);
        ch_printf(ch, "Compression Ratio: &C[&z%.2f%&C]&z\n\r",
                  (100.0 - (float) (out / in * 100)));
        ch_printf(ch, "MCCP Usage: &C[&z%d/%d players&C]&z\n\r", count,
                  total);
}
#endif
