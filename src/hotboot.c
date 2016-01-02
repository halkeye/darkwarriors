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
 *                          SWR Hotboot module                                           *
 ****************************************************************************************/

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <dlfcn.h>
#include "mud.h"
#include "changes.h"
#include "homes.h"
#include "olc-shuttle.h"
#include "account.h"
#include "channels.h"
#include "space2.h"

#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];
extern int port;    /* Port number to be used       */
extern int control, control2, conclient, conjava;
bool flush_buffer args((DESCRIPTOR_DATA * d, bool fPrompt));
void save_sysdata args((SYSTEM_DATA sys));
void      hotboot(bool debug, bool save);


#ifdef MCCP
bool      write_to_descriptor_old(int desc, char *txt, int length);
#endif
bool      write_to_descriptor(int desc, char *txt, int length);
void init_descriptor args((DESCRIPTOR_DATA * dnew, int desc));

extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
bool is_area_inprogress args((void));

/*
 * Save the world's ship files
 */
void write_ship(FILE * fp, SHIP_DATA * ship)
{
#ifndef HOTBOOT_SHIPS
        return;
#endif
        if (!fp)
                return;
        /*
         * What other conditions? 
         */
        /*
         * How about only ones that landed 
         */
        if (ship->shipstate == SHIP_DOCKED)
                return;
        if (ship->ship_class > SHIP_PLATFORM)
                return;

        fprintf(fp, "%s", "#SHIP\n");
        fprintf(fp, "Shipfname	%s~\n", ship->filename);
        if (ship->currspeed != 0)
                fprintf(fp, "Currspeed %d\n", ship->currspeed);
        if (ship->chaff != ship->maxchaff)
                fprintf(fp, "Chaff %d\n", ship->chaff);
        if (ship->shield != ship->maxshield)
                fprintf(fp, "Shield %d\n", ship->shield);
        if (ship->missiles != ship->maxmissiles)
                fprintf(fp, "Missiles %d\n", ship->missiles);
        if (ship->torpedos != ship->maxtorpedos)
                fprintf(fp, "Torpedos %d\n", ship->torpedos);
        if (ship->rockets != ship->maxrockets)
                fprintf(fp, "Rockets %d\n", ship->rockets);
        fprintf(fp, "Autorecharge %d\n", ship->autorecharge);
        fprintf(fp, "Autotrack %d\n", ship->autotrack);
        fprintf(fp, "Autospeed %d\n", ship->autospeed);
        fprintf(fp, "Autopilot %d\n", ship->autopilot);
        fprintf(fp, "Hatchopen %d\n", (int) ship->hatchopen);

        fprintf(fp, "VX %d\n", (int) ship->vx);
        fprintf(fp, "VY %d\n", (int) ship->vy);
        fprintf(fp, "VZ %d\n", (int) ship->vz);

        fprintf(fp, "HX %d\n", (int) ship->hx);
        fprintf(fp, "HY %d\n", (int) ship->hy);
        fprintf(fp, "HZ %d\n", (int) ship->hz);

        fprintf(fp, "JX %d\n", (int) ship->jx);
        fprintf(fp, "JY %d\n", (int) ship->jy);
        fprintf(fp, "JZ %d\n", (int) ship->jz);

        if (ship->target0 && ship->target0->name)
                fprintf(fp, "Target0 %s~\n", ship->target0->name);
        if (ship->target1 && ship->target1->name)
                fprintf(fp, "Target1 %s~\n", ship->target1->name);
        if (ship->target2 && ship->target2->name)
                fprintf(fp, "Target2 %s~\n", ship->target2->name);
        if (ship->dockedto && ship->dockedto->name)
                fprintf(fp, "Dockedto %s~\n", ship->dockedto->name);
        fprintf(fp, "Flags %d\n", ship->flags);

        fprintf(fp, "Shipstate %d\n", ship->shipstate);
        if (ship->dest != NULL && ship->dest[0] != '\0')
        {
                fprintf(fp, "LandDest %s~\n", ship->dest);
        }

        if (ship->shipstate == SHIP_HYPERSPACE)
        {
                fprintf(fp, "Hyperdistance %d\n", ship->hyperdistance);
                fprintf(fp, "Currjump %s~\n", ship->currjump->name);
        }
        if (ship->energy != ship->maxenergy)
                fprintf(fp, "Energy %d\n", ship->energy);
        if (ship->hull != ship->maxhull)
                fprintf(fp, "Hull %d\n", ship->hull);

        if (ship->starsystem)
                fprintf(fp, "Starsystem %s~\n", ship->starsystem->name);
        fprintf(fp, "%s", "EndShip\n\n");
        return;
}

void load_oochistory(void)
{
        FILE     *fp;
        int       i, ccount = 0, x;
        CHANNEL_DATA *channel;


        if (!(fp = fopen(OOCHISTORY_FILE, "r")))
        {
                bug("Could not open OOChistory File for reading.", 0);
                return;
        }
        ccount = fread_number(fp);
        for (x = 0; x < ccount; x++)
        {
                channel = get_channel(fread_string_noalloc(fp));
                channel->logpos = fread_number(fp);
                CREATE(channel->log, LOG_DATA, sysdata.channellog);

                for (i = 0; i < channel->logpos + 1; i++)
                {
                        channel->log[i].name = fread_string(fp);
                        channel->log[i].message = fread_string_nohash(fp);
                        channel->log[i].time = fread_number(fp);
                        channel->log[i].language =
                                get_language(fread_string_noalloc(fp));
                }
                continue;
        }
        FCLOSE(fp);
        unlink(OOCHISTORY_FILE);
        return;
}

void save_oochistory(void)
{
        FILE     *fp;
        int       i, ccount = 0;
        CHANNEL_DATA *channel;

        if (!(fp = fopen(OOCHISTORY_FILE, "w")))
        {
                perror(OOCHISTORY_FILE);
                return;
        }

        for (channel = first_channel; channel; channel = channel->next)
                if (channel->history && channel->log)
                        ccount++;
        fprintf(fp, "%d\n", ccount);
        for (channel = first_channel; channel; channel = channel->next)
        {
                if (!channel->history || !channel->log)
                        continue;

                fprintf(fp, "%s~\n", channel->name);
                fprintf(fp, "%d\n", channel->logpos);
                for (i = 0; i <= channel->logpos; i++)
                {
                        if (!channel->log[i].name ||
                            channel->log[i].name[0] == '\0' ||
                            !channel->log[i].message ||
                            channel->log[i].message[0] == '\0')
                                continue;
                        fprintf(fp, "%s~\n", channel->log[i].name);
                        fprintf(fp, "%s~\n", channel->log[i].message);
                        fprintf(fp, "%ld\n", channel->log[i].time);
                        fprintf(fp, "%s~\n", channel->log[i].language->name);
                }

        }
        fprintf(fp, "\n");
        FCLOSE(fp);
        return;
}

SHIP_DATA *load_ship(FILE * fp)
{
        SHIP_DATA *ship = NULL;
        const char *word;
        bool      fMatch;

#ifndef HOTBOOT_SHIPS
        return NULL;
#endif
        word = feof(fp) ? "EndShip" : fread_word(fp);
        if (!str_cmp(word, "EndShip"))
                return NULL;

        if (!str_cmp(word, "SHIPFNAME"))
        {
                char     *name = fread_string_nohash(fp);   /* is this right? - Gavin */
                SHIP_DATA *temp_ship = NULL;

                for (temp_ship = first_ship; temp_ship;
                     temp_ship = temp_ship->next)
                {
                        if (!str_cmp(name, temp_ship->filename))
                        {
                                ship = temp_ship;
                                break;
                        }
                }

                if (ship == NULL)
                {
                        bug("load_ship: No ship data for filename %s", name);
                        DISPOSE(name);
                        return NULL;

                        for (;;)
                        {
                                word = feof(fp) ? "EndShip" : fread_word(fp);
                                if (!str_cmp(word, "EndShip"))
                                        break;
                        }

                }

                DISPOSE(name);
        }
        else
        {
                for (;;)
                {
                        word = feof(fp) ? "EndShip" : fread_word(fp);
                        if (!str_cmp(word, "EndShip"))
                                break;
                }
                bug("load_ship: shipfname not found");
                return NULL;
        }
        for (;;)
        {
                word = feof(fp) ? "EndShip" : fread_word(fp);
                fMatch = FALSE;
                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;
                        /*
                         * case '#':
                         * if ( !str_cmp( word, "#TORPEDO" ) )
                         * {
                         * mob->tempnum = -9999; 
                         * fread_obj( mob, fp, OS_CARRY );
                         * }
                         */
                case 'A':
                        KEY("Autorecharge", ship->autorecharge,
                            (bool) fread_number(fp));
                        KEY("Autotrack", ship->autotrack,
                            (bool) fread_number(fp));
                        KEY("Autospeed", ship->autospeed,
                            (bool) fread_number(fp));
                        KEY("Autopilot", ship->autopilot,
                            (bool) fread_number(fp));
                        break;
                case 'C':
                        KEY("Currspeed", ship->currspeed, fread_number(fp));
                        if (!str_cmp(word, "Currjump"))
                        {
                                char     *temp = fread_string_nohash(fp);

                                extract_ship(ship);
                                ship->location = 0;
                                ship->currjump = starsystem_from_name(temp);
                                DISPOSE(temp);
                                fMatch = TRUE;
                                break;
                        }
                        KEY("Chaff", ship->chaff, fread_number(fp));
                        break;

                case 'D':
                        KEY("Dockedto", ship->dockedto,
                            get_ship(fread_string_noalloc(fp)));
                        break;

                case 'E':
                        KEY("Energy", ship->energy, fread_number(fp));
                        if (!str_cmp(word, "EndShip"))
                        {
                                return ship;
                        }
                        if (!str_cmp(word, "End"))  /* End of object, need to ignore this. sometimes they creep in there somehow -- Scion */
                                fMatch = TRUE;  /* Trick the system into thinking it matched something */
                        break;

                case 'F':
                        KEY("Flags", ship->flags, fread_number(fp));
                        break;

                case 'H':
                        KEY("Hull", ship->hull, fread_number(fp));
                        KEY("HX", ship->hx, (float) fread_number(fp));
                        KEY("HY", ship->hy, (float) fread_number(fp));
                        KEY("HZ", ship->hz, (float) fread_number(fp));
                        KEY("Hyperdistance", ship->hyperdistance,
                            fread_number(fp));
                        KEY("Hatchopen", ship->hatchopen,
                            (bool) fread_number(fp));
                        break;

                case 'J':
                        KEY("JX", ship->jx, (float) fread_number(fp));
                        KEY("JY", ship->jy, (float) fread_number(fp));
                        KEY("JZ", ship->jz, (float) fread_number(fp));
                        break;
                case 'L':
                        if (!str_cmp(word, "Landdest"))
                        {
                                if (ship->dest != NULL)
                                        STRFREE(ship->dest);
                                ship->dest = fread_string(fp);
                                fMatch = TRUE;
                                break;
                        }
                        break;

                case 'M':
                        KEY("Missiles", ship->missiles, fread_number(fp));
                        break;
                case 'R':
                        KEY("Rockets", ship->rockets, fread_number(fp));
                        break;
                case 'S':
                        KEY("Shield", ship->shield, fread_number(fp));
                        KEY("Shipstate", ship->shipstate, fread_number(fp));

                        if (!str_cmp(word, "Starsystem"))
                        {
                                char     *star_name = fread_string_nohash(fp);
                                SPACE_DATA *starsystem =
                                        starsystem_from_name(star_name);
                                DISPOSE(star_name);
                                fMatch = TRUE;

                                extract_ship(ship);
                                ship->location = 0;
                                if (ship->starsystem)
                                        ship_from_starsystem(ship,
                                                             starsystem);
                                ship_to_starsystem(ship, starsystem);
                                break;
                        }
                        break;
                case 'T':
                        KEY("Torpedos", ship->torpedos, fread_number(fp));
                        KEY("Target0", ship->target0,
                            get_ship(fread_string_noalloc(fp)));
                        KEY("Target1", ship->target1,
                            get_ship(fread_string_noalloc(fp)));
                        KEY("Target2", ship->target2,
                            get_ship(fread_string_noalloc(fp)));
                        break;

                case 'V':
                        KEY("VX", ship->vx, (float) fread_number(fp));
                        KEY("VY", ship->vy, (float) fread_number(fp));
                        KEY("VZ", ship->vz, (float) fread_number(fp));
                        break;

                }
                if (!fMatch && str_cmp(word, "End"))
                        bug("load_ship: no match: %s", word);
        }
        return NULL;
}

void save_mobile(FILE * fp, CHAR_DATA * mob)
{
        AFFECT_DATA *paf;
        SKILLTYPE *skill = NULL;

        if (!IS_NPC(mob) || !fp)
                return;
        fprintf(fp, "%s", "#MOBILE\n");
        fprintf(fp, "Vnum	%d\n", mob->pIndexData->vnum);
        fprintf(fp, "Level   %d\n", mob->top_level);
        fprintf(fp, "Gold	%ld\n", mob->gold);
        if (mob->in_room)
        {
                if (IS_SET(mob->act, ACT_SENTINEL))
                {
                        /*
                         * Sentinel mobs get stamped with a "home room" when they are created
                         * by create_mobile(), so we need to save them in their home room regardless
                         * of where they are right now, so they will go to their home room when they
                         * enter the game from a reboot or copyover -- Scion 
                         */
                        fprintf(fp, "Room	%d\n", mob->home_vnum);
                }
                else
                        fprintf(fp, "Room	%d\n", mob->in_room->vnum);
        }
        else
                fprintf(fp, "Room	%d\n", ROOM_VNUM_LIMBO);
#ifdef OVERLANDCODE
        fprintf(fp, "Coordinates  %d %d %d\n", mob->x, mob->y, mob->map);
#endif
        if (QUICKMATCH(mob->name, mob->pIndexData->player_name) == 0)
                fprintf(fp, "Name     %s~\n", mob->name);
        if (QUICKMATCH(mob->short_descr, mob->pIndexData->short_descr) == 0)
                fprintf(fp, "Short	%s~\n", mob->short_descr);
        if (QUICKMATCH(mob->long_descr, mob->pIndexData->long_descr) == 0)
                fprintf(fp, "Long	%s~\n", mob->long_descr);
        if (QUICKMATCH(mob->description, mob->pIndexData->description) == 0)
                fprintf(fp, "Description %s~\n", mob->description);
        if (QUICKMATCH(mob->mob_clan, mob->pIndexData->mob_clan) == 0)
                fprintf(fp, "Mobclan %s~\n", mob->mob_clan);
        fprintf(fp, "HpEndurance   %d %d %d %d\n",
                mob->hit, mob->max_hit, mob->endurance, mob->max_endurance);
        fprintf(fp, "Position %d\n", mob->position);
        if (mob->master && mob->master->name)
                fprintf(fp, "Master %s~\n", mob->master->name);
        if (mob->leader && mob->leader->name)
                fprintf(fp, "Leader %s~\n", mob->leader->name);
        fprintf(fp, "Flags %d\n", mob->act);
        if (mob->affected_by)
                fprintf(fp, "AffectedBy   %d\n", mob->affected_by);

        for (paf = mob->first_affect; paf; paf = paf->next)
        {
                if (paf->type >= 0
                    && (skill = get_skilltype(paf->type)) == NULL)
                        continue;

                if (paf->type >= 0 && paf->type < TYPE_PERSONAL)
                        fprintf(fp, "AffectData   '%s' %3d %3d %3d %d\n",
                                skill->name, paf->duration, paf->modifier,
                                paf->location, paf->bitvector);
                else
                        fprintf(fp, "Affect       %3d %3d %3d %3d %d\n",
                                paf->type, paf->duration, paf->modifier,
                                paf->location, paf->bitvector);
        }

        de_equip_char(mob);

        if (mob->first_carrying)
                fwrite_obj(mob, mob->last_carrying, fp, 0, OS_CARRY, TRUE);

        re_equip_char(mob);

        fprintf(fp, "%s", "EndMobile\n\n");
        return;
}

void save_world(CHAR_DATA * ch)
{
        FILE     *mobfp;
        FILE     *shipfp;
        FILE     *objfp;
        int       mobfile = 0;
        int       shipfile = 0;
        char      filename[256];
        CHAR_DATA *rch;
        ROOM_INDEX_DATA *pRoomIndex;
        int       iHash;

        ch = NULL;
        log_string("Preserving world state....");

        snprintf(filename, 256, "%s%s", SYSTEM_DIR, MOB_FILE);
        if ((mobfp = fopen(filename, "w")) == NULL)
        {
                bug("%s", "save_world: fopen mob file");
                perror(filename);
        }
        else
                mobfile++;

        snprintf(filename, 256, "%s%s", SYSTEM_DIR, SHIP_FILE);
        if ((shipfp = fopen(filename, "w")) == NULL)
        {
                bug("%s", "save_world: fopen ship file");
                perror(filename);
        }
        else
                shipfile++;

        for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
        {
                for (pRoomIndex = room_index_hash[iHash]; pRoomIndex;
                     pRoomIndex = pRoomIndex->next)
                {
                        if (pRoomIndex)
                        {
                                if (!pRoomIndex->first_content  /* Skip room if nothing in it */
#ifdef OLC_HOMES
                                    || pRoomIndex->home
#endif
                                    || xIS_SET(pRoomIndex->room_flags, ROOM_CLANSTOREROOM)  /* These rooms save on their own */
                                    || xIS_SET(pRoomIndex->room_flags, ROOM_PLR_HOME))
                                        continue;

                                snprintf(filename, 256, "%s%d", HOTBOOT_DIR,
                                         pRoomIndex->vnum);
                                if ((objfp = fopen(filename, "w")) == NULL)
                                {
                                        bug("save_world: fopen %d",
                                            pRoomIndex->vnum);
                                        perror(filename);
                                        continue;
                                }
                                fwrite_obj(NULL, pRoomIndex->last_content,
                                           objfp, 0, OS_CARRY, TRUE);
                                fprintf(objfp, "%s", "#END\n");
                                FCLOSE(objfp);
                        }
                }
        }

        if (mobfile)
        {
                for (rch = first_char; rch; rch = rch->next)
                {
                        if (!IS_NPC(rch) || rch == supermob
                            || IS_SET(rch->act, ACT_PROTOTYPE)
                            || IS_SET(rch->act, ACT_PET)
                            || rch->owner != NULL)
                                continue;
                        else
                                save_mobile(mobfp, rch);
                }
                fprintf(mobfp, "%s", "#END\n");
                FCLOSE(mobfp);
        }

        /*
         * FIXME - Loop Through ships here.
         * * Problem would be to make sure they are uniquely identified, so you don't set 2 ships that are set exactly the same way here.
         * * If its in space. Store the system its in, and its coords.. Current energy too?
         */
#ifdef HOTBOOT_SHIPS
        if (shipfile)
        {
                SHIP_DATA *ship = NULL;

                for (ship = first_ship; ship; ship = ship->next)
                {
                        write_ship(shipfp, ship);
                }
                fprintf(shipfp, "%s", "#END\n");
                FCLOSE(shipfp); /* This should be here instead of below the brace */
        }

#endif

        /*
         * Save sysdata, so that we have the proper number when we read in oochistory log 
         */
        save_sysdata(sysdata);
        save_oochistory();

        return;
}

CHAR_DATA *load_mobile(FILE * fp)
{
        CHAR_DATA *mob = NULL;
        const char *word;
        bool      fMatch;
        int       inroom = 0;
        ROOM_INDEX_DATA *pRoomIndex = NULL;

        word = feof(fp) ? "EndMobile" : fread_word(fp);
        if (!str_cmp(word, "Vnum"))
        {
                int       vnum;

                vnum = fread_number(fp);
                if (get_mob_index(vnum) == NULL)
                {
                        bug("load_mobile: No index data for vnum %d", vnum);
                        return NULL;
                }
                mob = create_mobile(get_mob_index(vnum));
                if (!mob)
                {
                        for (;;)
                        {
                                word = feof(fp) ? "EndMobile" :
                                        fread_word(fp);
                                /*
                                 * So we don't get so many bug messages when something messes up
                                 * * --Shaddai 
                                 */
                                if (!str_cmp(word, "EndMobile"))
                                        break;
                        }
                        bug("load_mobile: Unable to create mobile for vnum %d", vnum);
                        return NULL;
                }
        }
        else
        {
                for (;;)
                {
                        word = feof(fp) ? "EndMobile" : fread_word(fp);
                        /*
                         * So we don't get so many bug messages when something messes up
                         * * --Shaddai 
                         */
                        if (!str_cmp(word, "EndMobile"))
                                break;
                }
                extract_char(mob, TRUE);
                bug("%s", "load_mobile: Vnum not found");
                return NULL;
        }
        for (;;)
        {
                word = feof(fp) ? "EndMobile" : fread_word(fp);
                fMatch = FALSE;
                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;
                case '#':
                        if (!str_cmp(word, "#OBJECT"))
                        {
                                mob->tempnum = -9999;   /* Hackish, yes. Works though doesn't it? */
                                fread_obj(mob, fp, OS_CARRY);
                        }
                case 'A':
                        if (!str_cmp(word, "Affect")
                            || !str_cmp(word, "AffectData"))
                        {
                                AFFECT_DATA *paf;

                                CREATE(paf, AFFECT_DATA, 1);
                                if (!str_cmp(word, "Affect"))
                                {
                                        paf->type = fread_number(fp);
                                }
                                else
                                {
                                        int       sn;
                                        char     *sname = fread_word(fp);

                                        if ((sn = skill_lookup(sname)) < 0)
                                                bug("%s",
                                                    "load_mobile: unknown skill.");
                                        else
                                                paf->type = sn;
                                }

                                paf->duration = fread_number(fp);
                                paf->modifier = fread_number(fp);
                                paf->location = fread_number(fp);
                                if (paf->location == APPLY_WEAPONSPELL
                                    || paf->location == APPLY_WEARSPELL
                                    || paf->location == APPLY_REMOVESPELL
                                    || paf->location == APPLY_STRIPSN)
                                        paf->modifier =
                                                slot_lookup(paf->modifier);
                                paf->bitvector = fread_number(fp);
                                LINK(paf, mob->first_affect, mob->last_affect,
                                     next, prev);
                                fMatch = TRUE;
                                break;
                        }
                        KEY("AffectedBy", mob->affected_by, fread_number(fp));
                        break;
#ifdef OVERLANDCODE
                case 'C':
                        if (!str_cmp(word, "Coordinates"))
                        {
                                mob->x = fread_number(fp);
                                mob->y = fread_number(fp);
                                mob->map = fread_number(fp);

                                fMatch = TRUE;
                                break;
                        }
                        break;
#endif
                case 'D':
                        KEY("Description", mob->description,
                            fread_string(fp));
                        break;
                case 'E':
                        if (!str_cmp(word, "EndMobile"))
                        {
                                if (inroom == 0)
                                        inroom = ROOM_VNUM_LIMBO;
                                pRoomIndex = get_room_index(inroom);
                                if (!pRoomIndex)
                                        pRoomIndex =
                                                get_room_index
                                                (ROOM_VNUM_LIMBO);
                                char_to_room(mob, pRoomIndex);
                                mob->tempnum = -9998;   /* Yet another hackish fix! */
                                return mob;
                        }
                        if (!str_cmp(word, "End"))  /* End of object, need to ignore this. sometimes they creep in there somehow -- Scion */
                                fMatch = TRUE;  /* Trick the system into thinking it matched something */
                        break;
                case 'F':
                        KEY("Flags", mob->act, fread_number(fp));
                case 'G':
                        KEY("Gold", mob->gold, fread_number(fp));
                        break;
                case 'H':
                        if (!str_cmp(word, "HpEndurance"))
                        {
                                mob->hit = fread_number(fp);
                                mob->max_hit = fread_number(fp);
                                mob->endurance = fread_number(fp);
                                mob->max_endurance = fread_number(fp);

                                if (mob->max_endurance <= 0)
                                        mob->max_endurance = 150;

                                fMatch = TRUE;
                                break;
                        }
                        break;
                case 'L':
                        KEY("Long", mob->long_descr, fread_string(fp));
                        KEY("Level", mob->top_level, fread_number(fp));
                        KEY("Leader", mob->groupleader, fread_string(fp));
                        break;
                case 'M':
                        KEY("Mobclan", mob->mob_clan, fread_string(fp));
                        KEY("Master", mob->following, fread_string(fp));
                        break;
                case 'N':
                        KEY("Name", mob->name, fread_string(fp));
                        break;
                case 'P':
                        KEY("Position", mob->position, fread_number(fp));
                        break;
                case 'R':
                        KEY("Room", inroom, fread_number(fp));
                        break;
                case 'S':
                        KEY("Short", mob->short_descr, fread_string(fp));
                        break;
                }
                if (!fMatch && str_cmp(word, "End"))
                        bug("load_mobile: no match: %s", word);
        }
        return NULL;
}

void read_obj_file(char *dirname, char *filename)
{
        ROOM_INDEX_DATA *room;
        FILE     *fp;
        char      fname[256];
        int       vnum;

        vnum = atoi(filename);
        snprintf(fname, 256, "%s%s", dirname, filename);
        if ((room = get_room_index(vnum)) == NULL)
        {
                bug("read_obj_file: ARGH! Missing room index for %d!", vnum);
                unlink(fname);
                return;
        }


        if ((fp = fopen(fname, "r")) != NULL)
        {
                sh_int    iNest;
                bool      found;
                OBJ_DATA *tobj, *tobj_next;

                rset_supermob(room);
                for (iNest = 0; iNest < MAX_NEST; iNest++)
                        rgObjNest[iNest] = NULL;

                found = TRUE;
                for (;;)
                {
                        char      letter;
                        char     *word;

                        letter = fread_letter(fp);
                        if (letter == '*')
                        {
                                fread_to_eol(fp);
                                continue;
                        }

                        if (letter != '#')
                        {
                                bug("%s", "read_obj_file: # not found.");
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "OBJECT"))   /* Objects  */
                                fread_obj(supermob, fp, OS_CARRY);
                        else if (!str_cmp(word, "END")) /* Done     */
                                break;
                        else
                        {
                                bug("read_obj_file: bad section: %s", word);
                                break;
                        }
                }
                FCLOSE(fp);
                unlink(fname);
                for (tobj = supermob->first_carrying; tobj; tobj = tobj_next)
                {
                        tobj_next = tobj->next_content;
#ifdef OVERLANDCODE
                        if (IS_OBJ_STAT(tobj, ITEM_ONMAP))
                        {
                                SET_ACT_FLAG(supermob, ACT_ONMAP);
                                supermob->map = tobj->map;
                                supermob->x = tobj->x;
                                supermob->y = tobj->y;
                        }
#endif
                        obj_from_char(tobj);
#ifndef OVERLANDCODE
                        obj_to_room(tobj, room);
#else
                        obj_to_room(tobj, room, supermob);
                        REMOVE_ACT_FLAG(supermob, ACT_ONMAP);
                        supermob->map = -1;
                        supermob->x = -1;
                        supermob->y = -1;
#endif
                }
                release_supermob();
        }
        else
                log_string("Cannot open obj file");

        return;
}

void load_obj_files(void)
{
        DIR      *dp;
        struct dirent *dentry;
        char      directory_name[100];

        boot_log("World state: loading objs");
        snprintf(directory_name, 100, "%s", HOTBOOT_DIR);
        dp = opendir(directory_name);
        dentry = readdir(dp);
        while (dentry)
        {
                /*
                 * Added by Tarl 3 Dec 02 because we are now using CVS 
                 */
                if (!str_cmp(dentry->d_name, "CVS"))
                {
                        dentry = readdir(dp);
                        continue;
                }
                if (dentry->d_name[0] != '.')
                        read_obj_file(directory_name, dentry->d_name);
                dentry = readdir(dp);
        }
        closedir(dp);
        return;
}

void load_world(CHAR_DATA * ch)
{
        FILE     *mobfp;
        FILE     *shipfp;
        char      file1[256];
        char      file2[256];
        char     *word;
        int       done = 0;
        bool      mobfile = FALSE;
        bool      shipfile = FALSE;

        ch = NULL;

        snprintf(file1, 256, "%s%s", SYSTEM_DIR, MOB_FILE);
        if ((mobfp = fopen(file1, "r")) == NULL)
        {
                bug("%s", "load_world: fopen mob file");
                perror(file1);
        }
        else
                mobfile = TRUE;

        snprintf(file2, 256, "%s%s", SYSTEM_DIR, SHIP_FILE);
        if ((shipfp = fopen(file2, "r")) == NULL)
        {
                bug("%s", "load_world: fopen ship file");
                perror(file1);
        }
        else
                shipfile = TRUE;

        if (mobfile)
        {
                boot_log("World state: loading mobs");
                while (done == 0)
                {
                        if (feof(mobfp))
                                done++;
                        else
                        {
                                word = fread_word(mobfp);
                                if (str_cmp(word, "#END"))
                                        load_mobile(mobfp);
                                else
                                        done++;
                        }
                }
                FCLOSE(mobfp);
        }

        load_obj_files();

#ifdef HOTBOOT_SHIPS
        if (shipfile)
        {
                done = 0;
                boot_log("World state: loading ships");
                while (done == 0)
                {
                        if (feof(shipfp))
                                done++;
                        else
                        {
                                word = fread_word(shipfp);
                                if (str_cmp(word, "#END"))
                                        load_ship(shipfp);
                                else
                                        done++;
                        }
                }
                FCLOSE(shipfp);
                boot_log("World_state:  Done");
        }
#endif

        /*
         * Once loaded, the data needs to be purged in the event it causes a crash so that it won't try to reload 
         */
        unlink(file1);
        unlink(file2);

        boot_log("World_state: Loading Channel History");
        load_oochistory();
        boot_log("World_state:  Done");
        return;
}

/*  Warm reboot stuff, gotta make sure to thank Erwin for this :) */
CMDF do_hotboot(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim = NULL;
        DESCRIPTOR_DATA *d;
        int       count = 0;
        bool      found = FALSE;

        if (compilelock)
        {
                set_char_color(AT_RED, ch);
                send_to_char
                        ("Sorry, the mud cannot be shutdown during a compiler operation.\n\rPlease wait for the compiler to finish.\n\r",
                         ch);
                return;
        }

        if (argument[0] != '\0' && str_cmp(argument, "now")
            && str_cmp(argument, "warn")
            && str_cmp(argument, "poscrash")
            && str_cmp(argument, "nosave") && str_cmp(argument, "debug"))
        {
                send_to_char
                        ("Syntax: hotboot (warn/now/nosave/poscrash/debug)\n\r",
                         ch);
                return;
        }

        if (is_area_inprogress())
        {
                send_to_char("Arena in progress", ch);
                return;
        }

        for (d = first_descriptor; d; d = d->next)
        {
                if ((d->connected == CON_PLAYING
                     || d->connected == CON_EDITING)
                    && (victim = d->character) != NULL && !IS_NPC(victim)
                    && victim->in_room && victim->fighting
                    && victim->top_level >= 1
                    && victim->top_level <= MAX_LEVEL)
                {
                        found = TRUE;
                        count++;
                }
        }

        if (found)
        {
                ch_printf(ch,
                          "Cannot hotboot at this time. There are %d combats in progress.\n\r",
                          count);
                return;
        }

        found = FALSE;
        for (d = first_descriptor; d; d = d->next)
        {
                if (d->connected == CON_EDITING && d->character)
                {
                        found = TRUE;
                        break;
                }
                if (d->character && NOT_AUTHED(d->character))
                {
                        found = TRUE;
                        break;
                }
        }

        if (found)
        {
                send_to_char
                        ("Cannot hotboot at this time. Someone is using the line editor, or someone isn't authed.\n\r",
                         ch);
                return;
        }

        snprintf(log_buf, MSL, "Hotboot initiated by %s.", ch->name);
        log_string(log_buf);

        if (!str_cmp(argument, "warn"))
        {
                echo_to_all(AT_WHITE, "Hotboot Warning, commencing soon.",
                            ECHOTAR_ALL);
                return;
        }

        if (!str_cmp(argument, "poscrash"))
        {
                echo_to_all(AT_WHITE,
                            "Possible Crash. Please prepare accordingly.",
                            ECHOTAR_ALL);
                return;
        }

        hotboot(!str_cmp(argument, "debug"), !!str_cmp(argument, "nosave"));
        return;
}


void crash_hotboot(void)
{

        echo_to_all(AT_RED,
                    "\n\rReality swirls and changes around you, and things are not quite as they were...\n\r",
                    0);
        snprintf(log_buf, MSL, "%s", "Hotboot initiated by crash.");
        log_string(log_buf);
        hotboot(FALSE, TRUE);
}

void hotboot(bool debug, bool save)
{
        FILE     *fp;
        DESCRIPTOR_DATA *d, *de_next;
        char      buf[100], buf2[100], buf3[100];
        AREA_DATA *tarea;
        SHIP_DATA *ship;
        PLANET_DATA *planet;
        TIMER    *timer, *timer_next;

#ifdef OLC_SHUTTLE
        SHUTTLE_DATA *tshuttle;
#endif
        fp = fopen(HOTBOOT_FILE, "w");

        if (!fp)
        {
                bug("Could not write to hotboot file: %s. Hotboot aborted.",
                    HOTBOOT_FILE);
                perror("do_hotboot:fopen");
                return;
        }

        /*
         * And this one here will save the status of all objects and mobs in the game.
         * * This really should ONLY ever be used here. The less we do stuff like this the better.
         */
        save_world(supermob);


        if (save)
        {
                log_string("Saving player files and connection states....");
                /*
                 * load_changes(); - Gavin 
                 */
                save_changes();
                for (tarea = first_area; tarea; tarea = tarea->next)
                        fold_area(tarea, tarea->filename, FALSE, FALSE);
                for (tarea = first_bsort; tarea; tarea = tarea->next_sort)
                        if (IS_SET(tarea->status, AREA_LOADED))
                                fold_area(tarea, tarea->filename, FALSE,
                                          TRUE);
#ifdef OLC_SHUTTLE
                for (tshuttle = first_shuttle; tshuttle;
                     tshuttle = tshuttle->next)
                        save_shuttle(tshuttle);
#endif
#ifdef OLC_HOMES
                {
                        HOME_DATA *home = NULL;

                        for (home = first_home; home; home = home->next)
                               home->save(); 
                }
#endif
                for (ship = first_ship; ship; ship = ship->next)
                        save_ship(ship);

                for (planet = first_planet; planet; planet = planet->next)
                        save_planet(planet, TRUE);

                save_changes();
                save_world(NULL);
        }

        CHECK_LINKS(first_descriptor, last_descriptor, next, prev,
                    DESCRIPTOR_DATA);
        /*
         * Write out all the pulses and times and such. To make sure the copyover is seamless
         * * pulse_area
         * * pulse_taxes
         * * pulse_mobile
         * * pulse_space
         * * pulse_recharge
         * * pulse_ship
         * * pulse_point
         * * pulse_second - Maybe
         * * 
         */
        fprintf(fp, "%d %d %d %d %d %d %d %d\n",
                sysdata.pulse_area,
                sysdata.pulse_taxes,
                sysdata.pulse_mobile,
                sysdata.pulse_space,
                sysdata.pulse_recharge,
                sysdata.pulse_space,
                sysdata.pulse_point, sysdata.pulse_second);

        /*
         * For each playing descriptor, save its state 
         */
        for (d = first_descriptor; d; d = de_next)
        {
                CHAR_DATA *och = CH(d);

                de_next = d->next;  /* We delete from the list , so need to save this */
                if (!d->character || d->connected < CON_PLAYING)    /* drop those logging on */
                {
                        write_to_descriptor(d->descriptor,
                                            "\n\rSorry, we are rebooting. Come back in a few minutes.\n\r",
                                            0);
                        close_socket(d, FALSE); /* throw'em out */
                }
                else
                {

                        if (IS_IMMORTAL(och) && save)
                                do_savearea(och, "");

                        for (timer = och->first_timer; timer;
                             timer = timer_next)
                        {
                                timer_next = timer->next;
                                if (timer->type == TIMER_DO_FUN)
                                {
                                        int       tempsub;

                                        tempsub = och->substate;
                                        och->substate = timer->value;
                                        (timer->do_fun) (och, "");
                                        if (char_died(och))
                                                break;
                                        och->substate = tempsub;
                                }
                                extract_timer(och, timer);
                        }

                        fprintf(fp, "%d %d %d %d %d %s %s %s\n",
                                d->descriptor, och->in_room->vnum,
#ifdef MCCP
                                d->compressing, /* Do we want MCCP compression? */
#else
                                0,
#endif
                                (int) d->msp_detected,
                                (int) d->mxp_detected,
                                och->name, d->host, d->client);
                        /*
                         * One of two places this gets changed 
                         */
                        och->pcdata->hotboot = TRUE;
                        save_char_obj(och);
                        save_account(och->pcdata->account);
                        save_home(och);
#ifdef MCCP
                        compressEnd(d);
#endif
                }
        }
        fprintf(fp, "%s", "-1\n");
        FCLOSE(fp);
        log_string("Executing hotboot....");

        /*
         * added this in case there's a need to debug the contents of the various files 
         */
        if (debug)
        {
                log_string("Hotboot debug - Aborting before execl");
                return;
        }

#ifdef WEB
        if (sysdata.web)
        {

                printf(log_buf, "Shutting down webserver on port %d.",
                       port + 2);
                log_string(log_buf);
                shutdown_web();
        }
#endif
        /*
         * Close reserve and other always-open files and release other resources 
         */
        FCLOSE(fpReserve);
        FCLOSE(fpLOG);

        /*
         * exec - descriptors are inherited 
         */
#ifdef IMC
        imc_hotboot();
#endif
        snprintf(buf, 100, "%d", port);
        snprintf(buf2, 100, "%d", control);
#ifdef IMC
        if (this_imcmud)
                snprintf(buf3, 100, "%d", this_imcmud->desc);
        else
                strncpy(buf3, "-1", 100);
#else
        strncpy(buf3, "-1", 100);
#endif

        /*
         * Uncomment this bfd_close line if you've installed the dlsym snippet, you'll need it. 
         */
        dlclose(sysdata.dlHandle);
        execl(EXE_FILE, "swr", buf, "hotboot", buf2, buf3, (char *) NULL);

        /*
         * Failed - sucessful exec will not return 
         */
        perror("do_hotboot: execl");

        /*
         * Here you might want to reopen fpReserve 
         */
        /*
         * Since I'm a neophyte type guy, I'll assume this is a good idea and cut and past from main()  
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
        bug("%s", "Hotboot execution failed!!");
}

/* Recover from a hotboot - load players*/
void hotboot_recover()
{
        DESCRIPTOR_DATA *d;
        FILE     *fp;
        char      name[100];
        char      client[100];
        char      host[MAX_STRING_LENGTH];
        int       desc, room;
        bool      fOld;
        int       bCompress;
        int       mxp, msp;

        log_string("Hotboot recovery initiated");

        fp = fopen(HOTBOOT_FILE, "r");

        if (!fp)    /* there are some descriptors open which will hang forever then ? */
        {
                perror("hotboot_recover: fopen");
                bug("%s", "Hotboot file not found. Exitting.");
                exit(1);
        }

        unlink(HOTBOOT_FILE);   /* In case something crashes - doesn't prevent reading */
        fscanf(fp, "%d %d %d %d %d %d %d %d\n",
               &sysdata.pulse_area, &sysdata.pulse_taxes,
               &sysdata.pulse_mobile, &sysdata.pulse_space,
               &sysdata.pulse_recharge, &sysdata.pulse_space,
               &sysdata.pulse_point, &sysdata.pulse_second);
        for (;;)
        {
                fscanf(fp, "%d %d %d %d %d %s %s %s \n", &desc, &room,
                       &bCompress, &msp, &mxp, name, host, client);
                if (desc == -1)
                        break;

                /*
                 * Write something, and check if it goes error-free 
                 */
#ifdef MCCP
                if (!write_to_descriptor_old(desc, " ", 0))
#else
                if (!write_to_descriptor(desc, " ", 0))
#endif
                {
                        close(desc);    /* nope */
                        continue;
                }

                /*
                 * I hate code duplication 
                 */
                if (++num_descriptors > sysdata.maxplayers)
                        sysdata.maxplayers = num_descriptors;
                CREATE(d, DESCRIPTOR_DATA, 1);
                init_descriptor(d, desc);   /* set up various stuff */
                if (d->host)
                        STRFREE(d->host);
                d->host = STRALLOC(host);
                if (d->client)
                        STRFREE(d->client);
                d->client = STRALLOC(client);
                d->mxp_detected = (bool) mxp;
                d->msp_detected = (bool) msp;
                d->host = STRALLOC(host);
                d->ifd = -1;
                d->ipid = -1;

                LINK(d, first_descriptor, last_descriptor, next, prev);
                d->connected = CON_COPYOVER_RECOVER;    /* negative so close_socket
                                                         * will cut them off */
                /*
                 * telet negotiation asking what thier client is 
                 */
                write_to_buffer(d, (const char *) do_termtype_str, 0);
#ifdef MCCP
                if (bCompress)
                        compressStart(d, bCompress);
#endif

                /*
                 * Now, find the pfile 
                 */

                fOld = load_char_obj(d, name, FALSE, FALSE);

                if (!fOld)  /* Player file not found?! */
                {
                        write_to_descriptor(desc,
                                            "\n\rSomehow, your character was lost in the copyover sorry.\n\r",
                                            0);
                        close_socket(d, FALSE);
                }
                else    /* ok! */
                {
                        /*
                         * Just In Case,  Someone said this isn't necassary, but _why_
                         * do we want to dump someone in limbo? 
                         */
                        if (!d->character->in_room)
                                d->character->in_room =
                                        get_room_index(ROOM_VNUM_TEMPLE);

                        /*
                         * Insert in the char_list 
                         */
                        LINK(d->character, first_char, last_char, next, prev);

                        char_to_room(d->character, d->character->in_room);
                        load_home(d->character);
                        d->connected = CON_PLAYING;
#ifdef ACCOUNT
                        d->account = d->character->pcdata->account;
#endif

                        if (d->character->pcdata->area)
                                do_loadarea(d->character, "");
                }
        }

        FCLOSE(fp);
        {
                CHAR_DATA *ch;

                for (ch = first_char; ch; ch = ch->next)
                {
                        if (ch->following && ch->following[0] != '\0')
                        {
                                ch->master =
                                        get_char_world_nocheck(ch->following);
                                STRFREE(ch->following);
                        }

                        if (ch->groupleader && ch->groupleader[0] != '\0')
                        {
                                ch->leader =
                                        get_char_world_nocheck(ch->
                                                               groupleader);
                                STRFREE(ch->groupleader);
                        }
                }
        }

}
