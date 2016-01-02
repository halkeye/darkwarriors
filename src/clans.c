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
 *             $Id: clans.c 1330 2005-12-05 03:23:24Z halkeye $                  *
 ****************************************************************************************/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #include <stdlib.h> */
#include <time.h>
#include <memory.h>
#include "mud.h"
#include "account.h"
#include "body.h"
#include "bounty.h"
#include "space2.h"
#include "installations.h"

#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];

CLAN_DATA *first_clan;
CLAN_DATA *last_clan;

PLANET_DATA *first_planet;
PLANET_DATA *last_planet;

/* local routines */
void fread_clan args((CLAN_DATA * clan, FILE * fp));
bool load_clan_file args((char *clanfile));
void write_clan_list args((void));
void free_clan args((CLAN_DATA * clan));
void free_planet args((PLANET_DATA * planet));
void free_bounty args((BOUNTY_DATA * bounty));

/*
 * Get pointer to clan structure from clan name.
 */
CLAN_DATA *get_clan(char *name)
{
        CLAN_DATA *clan;

        for (clan = first_clan; clan; clan = clan->next)
                if (!str_cmp(name, clan->name))
                        return clan;
        for (clan = first_clan; clan; clan = clan->next)
                if (nifty_is_name_prefix(name, clan->name))
                        return clan;
        return NULL;
}

PLANET_DATA *get_planet(char *name)
{
        PLANET_DATA *planet;

        for (planet = first_planet; planet; planet = planet->next)
                if (!str_prefix(name, planet->name))
                        return planet;
        return NULL;
}

void write_clan_list()
{
        CLAN_DATA *tclan;
        FILE     *fpout;
        char      filename[256];

        snprintf(filename, MSL, "%s%s", CLAN_DIR, CLAN_LIST);
        fpout = fopen(filename, "w");
        if (!fpout)
        {
                bug("FATAL: cannot open clan.lst for writing!\n\r", 0);
                return;
        }
        for (tclan = first_clan; tclan; tclan = tclan->next)
                fprintf(fpout, "%s\n", tclan->filename);
        fprintf(fpout, "$\n");
        FCLOSE(fpout);
}

void write_planet_list()
{
        PLANET_DATA *tplanet;
        FILE     *fpout;
        char      filename[256];

        snprintf(filename, MSL, "%s%s", PLANET_DIR, PLANET_LIST);
        fpout = fopen(filename, "w");
        if (!fpout)
        {
                bug("FATAL: cannot open planet.lst for writing!\n\r", 0);
                return;
        }
        for (tplanet = first_planet; tplanet; tplanet = tplanet->next)
                fprintf(fpout, "%s\n", tplanet->filename);
        fprintf(fpout, "$\n");
        FCLOSE(fpout);
}

/*
 * Save a clan's data to its data file
 */
void save_clan(CLAN_DATA * clan)
{
        FILE     *fp;
        char      filename[256];
        char      buf[MAX_STRING_LENGTH];

        if (!clan)
        {
                bug("save_clan: null clan pointer!", 0);
                return;
        }

        if (!clan->filename || clan->filename[0] == '\0')
        {
                snprintf(buf, MSL, "save_clan: %s has no filename",
                         clan->name);
                bug(buf, 0);
                return;
        }

        snprintf(filename, MSL, "%s%s", CLAN_DIR, clan->filename);

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_clan: fopen", 0);
                perror(filename);
        }
        else
        {
                int       count;

                fprintf(fp, "#CLAN\n");
                fprintf(fp, "Name         %s~\n", clan->name);
                fprintf(fp, "Description  %s~\n", clan->description);
                fprintf(fp, "Motto        %s~\n", clan->motto);
                if (clan->ally && clan->ally->name)
                        fprintf(fp, "AllyName         %s~\n",
                                clan->ally->name);
                if (clan->enemy && clan->enemy->name)
                        fprintf(fp, "EnemyName        %s~\n",
                                clan->enemy->name);
                fprintf(fp, "Leader       %s~\n", clan->leader);
                fprintf(fp, "NumberOne    %s~\n", clan->number1);
                fprintf(fp, "NumberTwo    %s~\n", clan->number2);
                fprintf(fp, "PKills       %d\n", clan->pkills);
                fprintf(fp, "PDeaths      %d\n", clan->pdeaths);
                fprintf(fp, "MKills       %d\n", clan->mkills);
                fprintf(fp, "MDeaths      %d\n", clan->mdeaths);
                fprintf(fp, "Type         %d\n", clan->clan_type);
                fprintf(fp, "Members      %d\n", clan->members);
                fprintf(fp, "Enlist       %d\n", clan->enlistroom);
                fprintf(fp, "Board        %d\n", clan->board);
                fprintf(fp, "Storeroom    %d\n", clan->storeroom);
                fprintf(fp, "Alignment    %d\n", clan->alignment);
                fprintf(fp, "Enliston     %d\n", clan->enliston);
                fprintf(fp, "Funds        %ld\n", clan->funds);
                fprintf(fp, "Jail         %d\n", clan->jail);
                for (count = 0; count < MAX_RANK; count++)
                        fprintf(fp, "Rank%d        %s~\n", count,
                                clan->rank[count]);
                for (count = 0; count < MAX_RANK; count++)
                        fprintf(fp, "Salary%d         %d\n", count,
                                clan->salary[count]);
                if (clan->mainclan)
                        fprintf(fp, "MainClan     %s~\n",
                                clan->mainclan->name);
                fprintf(fp, "Filename     %s~\n", clan->filename);
                fprintf(fp, "Roster     %s~\n", clan->roster);
                fprintf(fp, "End\n\n");
                fprintf(fp, "#END\n");
                FCLOSE(fp);
        }
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

void save_planet(PLANET_DATA * planet, bool copyover)
{
        FILE     *fp;
        char      filename[256];
        char      buf[MAX_STRING_LENGTH];
        int       i;

        if (!planet)
        {
                bug("save_planet: null planet pointer!", 0);
                return;
        }

        if (!planet->filename || planet->filename[0] == '\0')
        {
                snprintf(buf, MSL, "save_planet: %s has no filename",
                         planet->name);
                bug(buf, 0);
                return;
        }

        snprintf(filename, MSL, "%s%s", PLANET_DIR, planet->filename);

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_planet: fopen", 0);
                perror(filename);
                fpReserve = fopen(NULL_FILE, "r");
        }
        else
        {
                AREA_DATA *pArea;

                fprintf(fp, "#PLANET\n");
                fprintf(fp, "Name         %s~\n", planet->name);
                fprintf(fp, "Filename     %s~\n", planet->filename);
                fprintf(fp, "BaseValue    %ld\n", planet->base_value);
                fprintf(fp, "Flags        %d\n", planet->flags);
                /*
                 * Why were we saving these but not loading ? 
                 */
                //if (copyover)
                        fprintf(fp, "PopSupport   %d\n", planet->pop_support);
                //if (copyover)
                        fprintf(fp, "Population %d\n", planet->population);

                fprintf(fp, "Defbattalions   %d\n", planet->defbattalions);
                fprintf(fp, "Attbattalions   %d\n", planet->attbattalions);
                fprintf(fp, "Planet_Type   %d\n", planet->planet_type);
                fprintf(fp, "Ioncannons   %d\n", planet->ioncannons);
                fprintf(fp, "Jail   %d\n", planet->jail);
                fprintf(fp, "Turbolasers   %d\n", planet->turbolasers);
                fprintf(fp, "Shields   %d\n", planet->shields);
                if (planet->starsystem && planet->starsystem->name)
                        fprintf(fp, "Starsystem   %s~\n",
                                planet->starsystem->name);
                if (planet->attgovern && planet->attgovern->name)
                        fprintf(fp, "Attgovern   %s~\n",
                                planet->attgovern->name);
                if (planet->governed_by && planet->governed_by->name)
                        fprintf(fp, "GovernedBy   %s~\n",
                                planet->governed_by->name);
                for (pArea = planet->first_area; pArea;
                     pArea = pArea->next_on_planet)
                        if (pArea->filename)
                                fprintf(fp, "Area         %s~\n",
                                        pArea->filename);
                if (planet->body && planet->body->name())
                        fprintf(fp, "BodyName   %s~\n", planet->body->name());
                for (i = 1; i < CARGO_MAX; i++)
                        fprintf(fp, "Resource %d %d %d %d %d %d\n", i,
                                planet->cargoimport[i],
                                planet->cargoexport[i], planet->resource[i],
                                planet->consumes[i], planet->produces[i]);
                for (i = CONTRABAND_NONE + 1; i < CONTRABAND_MAX; i++)
                        fprintf(fp, "Contraband %d %d %d %d %d %d\n", i,
                                planet->cargoimport[i],
                                planet->cargoexport[i], planet->resource[i],
                                planet->consumes[i], planet->produces[i]);
                fprintf(fp, "End\n\n");
                fprintf(fp, "#END\n");
        }
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

/*
 * Read in actual clan data.
 */

void fread_clan(CLAN_DATA * clan, FILE * fp)
{
        char      buf[MAX_STRING_LENGTH];
        const char *word;
        bool      fMatch;

        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);
                fMatch = FALSE;

                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;

                case 'A':
                        KEY("Alignment", clan->alignment, fread_number(fp));
                        KEY("AllyName", clan->ally_name, fread_string(fp));
                        break;

                case 'B':
                        KEY("Board", clan->board, fread_number(fp));
                        break;

                case 'D':
                        KEY("Description", clan->description,
                            fread_string(fp));
                        break;

                case 'E':
                        KEY("EnemyName", clan->enemy_name, fread_string(fp));
                        KEY("Enlist", clan->enlistroom, fread_number(fp));
                        KEY("Enliston", clan->enliston, fread_number(fp));
                        if (!str_cmp(word, "End"))
                        {
                                if (!clan->name)
                                        clan->name = STRALLOC("");
                                if (!clan->leader)
                                        clan->leader = STRALLOC("");
                                if (!clan->description)
                                        clan->description = STRALLOC("");
                                if (!clan->number1)
                                        clan->number1 = STRALLOC("");
                                if (!clan->number2)
                                        clan->number2 = STRALLOC("");
                                if (!clan->tmpstr)
                                        clan->tmpstr = STRALLOC("");
								/* FIXME FIXME FIXME */
                                if (!clan->rank[0])
                                        clan->rank[0] = STRALLOC("Inductee");
                                if (!clan->rank[1])
                                        clan->rank[1] = STRALLOC("Cadet");
                                if (!clan->rank[2])
                                        clan->rank[2] = STRALLOC("Private");
                                if (!clan->rank[3])
                                        clan->rank[3] = STRALLOC("Seargent");
                                if (!clan->rank[4])
                                        clan->rank[4] = STRALLOC("Commander");
                                if (!clan->rank[5])
                                        clan->rank[5] =
                                                STRALLOC("Lt. Captain");
                                if (!clan->rank[6])
                                        clan->rank[6] = STRALLOC("Captain");
                                if (!clan->rank[7])
                                        clan->rank[7] = STRALLOC("Major");
                                if (!clan->rank[8])
                                        clan->rank[8] =
                                                STRALLOC("Lt. Colonel");
                                if (!clan->rank[9])
                                        clan->rank[9] = STRALLOC("Colonel");
                                if (!clan->rank[10])
                                        clan->rank[10] = STRALLOC("General");
                                if (!clan->rank[11])
                                        clan->rank[11] = STRALLOC("Second");
                                if (!clan->rank[12])
                                        clan->rank[12] = STRALLOC("First");
                                if (!clan->rank[13])
                                        clan->rank[13] = STRALLOC("Leader");

                                return;
                        }
                        break;

                case 'F':
                        KEY("Funds", clan->funds, fread_number(fp));
                        KEY("Filename", clan->filename,
                            fread_string_nohash(fp));
                        break;

                case 'J':
                        KEY("Jail", clan->jail, fread_number(fp));
                        break;

                case 'L':
                        KEY("Leader", clan->leader, fread_string(fp));
                        break;

                case 'M':
                        KEY("MDeaths", clan->mdeaths, fread_number(fp));
                        KEY("Motto", clan->motto, fread_string(fp));
                        KEY("Members", clan->members, fread_number(fp));
                        KEY("MKills", clan->mkills, fread_number(fp));
                        KEY("MainClan", clan->tmpstr, fread_string(fp));
                        break;

                case 'N':
                        KEY("Name", clan->name, fread_string(fp));
                        KEY("NumberOne", clan->number1, fread_string(fp));
                        KEY("NumberTwo", clan->number2, fread_string(fp));
                        break;

                case 'P':
                        KEY("PDeaths", clan->pdeaths, fread_number(fp));
                        KEY("PKills", clan->pkills, fread_number(fp));
                        break;

                case 'R':
						/* FIX ME FIX ME FIXME */
                        KEY("Rank0", clan->rank[0], fread_string(fp));
                        KEY("Rank1", clan->rank[1], fread_string(fp));
                        KEY("Rank2", clan->rank[2], fread_string(fp));
                        KEY("Rank3", clan->rank[3], fread_string(fp));
                        KEY("Rank4", clan->rank[4], fread_string(fp));
                        KEY("Rank5", clan->rank[5], fread_string(fp));
                        KEY("Rank6", clan->rank[6], fread_string(fp));
                        KEY("Rank7", clan->rank[7], fread_string(fp));
                        KEY("Rank8", clan->rank[8], fread_string(fp));
                        KEY("Rank9", clan->rank[9], fread_string(fp));
                        KEY("Rank10", clan->rank[10], fread_string(fp));
                        KEY("Rank11", clan->rank[11], fread_string(fp));
                        KEY("Rank12", clan->rank[12], fread_string(fp));
                        KEY("Rank13", clan->rank[13], fread_string(fp));
                        KEY("Roster", clan->roster, fread_string(fp));
                        break;

                case 'S':
                        KEY("Storeroom", clan->storeroom, fread_number(fp));
						/* FIX ME FIX ME FIXME */
                        KEY("Salary0", clan->salary[0], fread_number(fp));
                        KEY("Salary1", clan->salary[1], fread_number(fp));
                        KEY("Salary2", clan->salary[2], fread_number(fp));
                        KEY("Salary3", clan->salary[3], fread_number(fp));
                        KEY("Salary4", clan->salary[4], fread_number(fp));
                        KEY("Salary5", clan->salary[5], fread_number(fp));
                        KEY("Salary6", clan->salary[6], fread_number(fp));
                        KEY("Salary7", clan->salary[7], fread_number(fp));
                        KEY("Salary8", clan->salary[8], fread_number(fp));
                        KEY("Salary9", clan->salary[9], fread_number(fp));
                        KEY("Salary10", clan->salary[10], fread_number(fp));
                        KEY("Salary11", clan->salary[11], fread_number(fp));
                        KEY("Salary12", clan->salary[12], fread_number(fp));
                        KEY("Salary13", clan->salary[13], fread_number(fp));
                        break;

                case 'T':
                        KEY("Type", clan->clan_type, fread_number(fp));
                        break;
                default:
                        /*
                         * Just in case - Gavin 
                         */
                        fMatch = FALSE;
                }

                if (!fMatch)
                {
                        snprintf(buf, MSL, "Fread_clan: no match: %s", word);
                        bug(buf, 0);
                }

        }
}

void fread_planet(PLANET_DATA * planet, FILE * fp)
{
        char      buf[MAX_STRING_LENGTH];
        const char *word;
        bool      fMatch;
        char     *line;
        int       x0, x1, x2, x3, x4, x5;



        for (;;)
        {
                word = feof(fp) ? "End" : fread_word(fp);
                fMatch = FALSE;

                switch (UPPER(word[0]))
                {
                case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;

                case 'A':
                        KEY("Attbattalions", planet->attbattalions,
                            fread_number(fp));
                        if (!str_cmp(word, "Attgovern"))
                        {
                                char     *temp = fread_string(fp);

                                planet->attgovern = get_clan(temp);
                                STRFREE(temp);
                                fMatch = TRUE;
                                break;
                        }

                        if (!str_cmp(word, "Area"))
                        {
                                char     *temp = fread_string(fp);
                                AREA_DATA *pArea;

                                for (pArea = first_area; pArea;
                                     pArea = pArea->next)
                                {
                                        if (pArea->filename
                                            && !str_cmp(pArea->filename,
                                                        temp))
                                        {
                                                pArea->planet = planet;
                                                LINK(pArea,
                                                     planet->first_area,
                                                     planet->last_area,
                                                     next_on_planet,
                                                     prev_on_planet);
                                        }
                                }
                                STRFREE(temp);
                                fMatch = TRUE;
                                break;
                        }
                        break;

                case 'B':
                        KEY("BaseValue", planet->base_value,
                            fread_number(fp));
                        KEY("BodyName", planet->bodyname,
                            fread_string_nohash(fp));
                        break;

                case 'C':
                        /*
                         * FIXED this, it had a ; at the end of the str_cmp 
                         */
                        if (!str_cmp(word, "Contraband"))
                        {
                                line = fread_line(fp);
                                x0 = x1 = x2 = x3 = x4 = x5 = 0;
                                sscanf(line, "%d %d %d %d %d %d\n",
                                       &x0, &x1, &x2, &x3, &x4, &x5);
                                planet->cargoimport[x0] = x1;
                                planet->cargoexport[x0] = x2;
                                planet->resource[x0] = x3;
                                planet->consumes[x0] = x4;
                                planet->produces[x0] = x5;
                                fMatch = TRUE;
                                break;
                        }
                        break;

                case 'D':
                        KEY("Defbattalions", planet->defbattalions,
                            fread_number(fp));


                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!planet->name)
                                        planet->name = STRALLOC("");
                                return;
                        }
                        break;

                case 'F':
                        KEY("Filename", planet->filename,
                            fread_string_nohash(fp));
                        KEY("Flags", planet->flags, fread_number(fp));
                        break;
                case 'J':
                        KEY("Jail", planet->jail, fread_number(fp));
                        break;

                case 'R':
                        /*
                         * FIXED this, it had a ; at the end of the str_cmp 
                         */
                        if (!str_cmp(word, "Resource"))
                        {
                                line = fread_line(fp);
                                x0 = x1 = x2 = x3 = x4 = x5 = 0;
                                sscanf(line, "%d %d %d %d %d %d\n",
                                       &x0, &x1, &x2, &x3, &x4, &x5);
                                planet->cargoimport[x0] = x1;
                                planet->cargoexport[x0] = x2;
                                planet->resource[x0] = x3;
                                planet->consumes[x0] = x4;
                                planet->produces[x0] = x5;
                                fMatch = TRUE;
                                break;
                        }
                        break;

                case 'G':
                        KEY("GovernedBy", planet->governed_by,
                            get_clan(fread_string_noalloc(fp)));
                        break;

                case 'I':
                        KEY("Ioncannons", planet->ioncannons,
                            fread_number(fp));
                        break;

                case 'N':
                        KEY("Name", planet->name, fread_string(fp));
                        break;

                case 'P':
                        KEY("PopSupport", planet->pop_support,
                            fread_number(fp));
                        KEY("Population", planet->population,
                            fread_number(fp));
                        KEY("Planet_Type", planet->planet_type,
                            fread_number(fp));
                        break;

                case 'S':
                        if (!str_cmp(word, "Starsystem"))
                        {
                                char     *temp = fread_string(fp);

                                planet->starsystem =
                                        starsystem_from_name(temp);
                                if (planet->starsystem)
                                {
                                        SPACE_DATA *starsystem =
                                                planet->starsystem;

                                        LINK(planet, starsystem->first_planet,
                                             starsystem->last_planet,
                                             next_in_system, prev_in_system);
                                }
                                fMatch = TRUE;
                                STRFREE(temp);
                                break;
                        }
                        KEY("Shields", planet->shields, fread_number(fp));
                        break;

                case 'T':
                        KEY("Taxes", planet->base_value, fread_number(fp));
                        KEY("Turbolasers", planet->turbolasers,
                            fread_number(fp));
                        break;
                default:
                        /*
                         * Just in case - Gavin 
                         */
                        fMatch = FALSE;

                }

                if (!fMatch)
                {
                        snprintf(buf, MSL, "Fread_planet: no match: %s",
                                 word);
                        bug(buf, 0);
                }

        }
}


/*
 * Load a clan file
 */

bool load_clan_file(char *clanfile)
{
        char      filename[256];
        CLAN_DATA *clan;
        FILE     *fp;
        bool      found;

        CREATE(clan, CLAN_DATA, 1);
        clan->next_subclan = NULL;
        clan->prev_subclan = NULL;
        clan->last_subclan = NULL;
        clan->first_subclan = NULL;
        clan->mainclan = NULL;
        memset(clan->salary, 0, sizeof(clan->salary));
        found = FALSE;
        snprintf(filename, MSL, "%s%s", CLAN_DIR, clanfile);

        if ((fp = fopen(filename, "r")) != NULL)
        {

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
                                bug("Load_clan_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "CLAN"))
                        {
                                fread_clan(clan, fp);
                                break;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                char      buf[MAX_STRING_LENGTH];

                                snprintf(buf, MSL,
                                         "Load_clan_file: bad section: %s.",
                                         word);
                                bug(buf, 0);
                                break;
                        }
                }
                FCLOSE(fp);
        }

        if (found)
        {
                ROOM_INDEX_DATA *storeroom;

                LINK(clan, first_clan, last_clan, next, prev);

                if (clan->storeroom == 0
                    || (storeroom = get_room_index(clan->storeroom)) == NULL)
                {
                        boot_log(" ERROR: Storeroom not found");
                        return found;
                }

                snprintf(filename, MSL, "%s%s.vault", CLAN_DIR,
                         clan->filename);
                if ((fp = fopen(filename, "r")) != NULL)
                {
                        int       iNest;
                        OBJ_DATA *tobj, *tobj_next;

                        boot_log("Loading clan storage room");
                        rset_supermob(storeroom);
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
                                        bug("Load_clan_vault: # not found.",
                                            0);
                                        bug(clan->name, 0);
                                        break;
                                }

                                word = fread_word(fp);
                                if (!str_cmp(word, "OBJECT"))   /* Objects  */
                                        fread_obj(supermob, fp, OS_CARRY);
                                else if (!str_cmp(word, "END")) /* Done     */
                                        break;
                                else
                                {
                                        bug("Load_clan_vault: bad section.",
                                            0);
                                        bug(clan->name, 0);
                                        break;
                                }
                        }
                        FCLOSE(fp);
                        for (tobj = supermob->first_carrying; tobj;
                             tobj = tobj_next)
                        {
                                tobj_next = tobj->next_content;
                                obj_from_char(tobj);
                                obj_to_room(tobj, storeroom);
                        }
                        release_supermob();
                }
                else
                {
                        char      buf[MSL];

                        snprintf(buf, MSL,
                                 "ERROR: CANNOT OPEN CLAN VAULT: VAULT:%s CLAN: %s",
                                 filename, clan->name);
                        boot_log(buf);
                }
        }
        else
                DISPOSE(clan);

        return found;
}

bool load_planet_file(char *planetfile)
{
        char      filename[256];
        PLANET_DATA *planet;
        FILE     *fp;
        bool      found;

        CREATE(planet, PLANET_DATA, 1);

        planet->governed_by = NULL;
        planet->next_in_system = NULL;
        planet->prev_in_system = NULL;
        planet->starsystem = NULL;
        planet->first_area = NULL;
        planet->last_area = NULL;
        planet->attgovern = NULL;
        planet->body = NULL;
        found = FALSE;
        snprintf(filename, MSL, "%s%s", PLANET_DIR, planetfile);

        if ((fp = fopen(filename, "r")) != NULL)
        {

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
                                bug("Load_planet_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "PLANET"))
                        {
                                fread_planet(planet, fp);
                                break;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                char      buf[MAX_STRING_LENGTH];

                                snprintf(buf, MSL,
                                         "Load_planet_file: bad section: %s.",
                                         word);
                                bug(buf, 0);
                                break;
                        }
                }
                FCLOSE(fp);
        }

        if (!found)
                DISPOSE(planet);
        else
                LINK(planet, first_planet, last_planet, next, prev);


        return found;
}


/*
 * Load in all the clan files.
 */
void load_clans()
{
        FILE     *fpList;
        const char *filename;
        char      clanlist[256];
        char      buf[MAX_STRING_LENGTH];
        CLAN_DATA *clan;
        CLAN_DATA *bosclan;

        first_clan = NULL;
        last_clan = NULL;

        snprintf(clanlist, MSL, "%s%s", CLAN_DIR, CLAN_LIST);
        FCLOSE(fpReserve);
        if ((fpList = fopen(clanlist, "r")) == NULL)
        {
                perror(clanlist);
                exit(1);
        }

        for (;;)
        {
                filename = feof(fpList) ? "$" : fread_word(fpList);
                if (filename[0] == '$')
                        break;

                if (!load_clan_file((char *) filename))
                {
                        snprintf(buf, MSL, "Cannot load clan file: %s",
                                 filename);
                        bug(buf, 0);
                }
        }
        FCLOSE(fpList);
        boot_log(" Done clans");
        boot_log("Sorting clans....");
        fpReserve = fopen(NULL_FILE, "r");

        for (clan = first_clan; clan; clan = clan->next)
        {
                if (!clan->tmpstr || clan->tmpstr[0] == '\0')
                        continue;

                bosclan = get_clan(clan->tmpstr);
                if (!bosclan)
                        continue;

                LINK(clan, bosclan->first_subclan, bosclan->last_subclan,
                     next_subclan, prev_subclan);
                clan->mainclan = bosclan;
        }

        boot_log(" Done sorting");
        for (clan = first_clan; clan; clan = clan->next)
        {
                if (clan->ally_name)
                {
                        clan->ally = get_clan(clan->ally_name);
                        STRFREE(clan->ally_name);   /* Freeing up unneeded mem now */
                }
                if (clan->enemy_name)
                {
                        clan->enemy = get_clan(clan->enemy_name);
                        STRFREE(clan->enemy_name);  /* uneeeded now */
                }
        }
        return;
}

void load_planets()
{
        FILE     *fpList;
        const char *filename;
        char      planetlist[256];
        char      buf[MAX_STRING_LENGTH];

        first_planet = NULL;
        last_planet = NULL;


        snprintf(planetlist, MSL, "%s%s", PLANET_DIR, PLANET_LIST);
        FCLOSE(fpReserve);
        if ((fpList = fopen(planetlist, "r")) == NULL)
        {
                perror(planetlist);
                exit(1);
        }

        for (;;)
        {
                filename = feof(fpList) ? "$" : fread_word(fpList);
                if (filename[0] == '$')
                        break;

                if (!load_planet_file((char *) filename))
                {
                        snprintf(buf, MSL, "Cannot load planet file: %s",
                                 filename);
                        bug(buf, 0);
                }
        }
        FCLOSE(fpList);
        boot_log(" Done planets ");
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

CMDF do_make(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        send_to_char("Huh?\n\r", ch);
        return;
}


CMDF do_induct(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        CLAN_DATA *clan;

        if (!IS_CLANNED(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        clan = ch->pcdata->clan;

        if (!HAS_CLAN_PERM(ch, clan, "induct"))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Induct whom?\n\r", ch);
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

        if (victim->pcdata->clan)
        {
                if (victim->pcdata->clan->clan_type == CLAN_CRIME)
                {
                        if (victim->pcdata->clan == clan)
                                send_to_char
                                        ("This player already belongs to your crime family!\n\r",
                                         ch);
                        else
                                send_to_char
                                        ("This player already belongs to an organization!\n\r",
                                         ch);
                        return;
                }
                else if (victim->pcdata->clan->clan_type == CLAN_GUILD)
                {
                        if (victim->pcdata->clan == clan)
                                send_to_char
                                        ("This player already belongs to your guild!\n\r",
                                         ch);
                        else
                                send_to_char
                                        ("This player already belongs to an organization!\n\r",
                                         ch);
                        return;
                }
                else
                {
                        if (victim->pcdata->clan == clan)
                                send_to_char
                                        ("This player already belongs to your organization!\n\r",
                                         ch);
                        else
                                send_to_char
                                        ("This player already belongs to an organization!\n\r",
                                         ch);
                        return;
                }

        }
#ifndef ACCOUNT
        if (victim->pcdata->rp < 2)
#else
        if (victim->pcdata->account && victim->pcdata->account->rppoints < 2)
#endif
        {
                ch_printf(ch,
                          "That person need more than four role playing points to joing your clan.\n\r");
                ch_printf(victim,
                          "%s is trying to induct you into %s, but you do not have enough role playing points.\n\r",
                          ch->name, clan->name);
                return;
        }

        clan->members++;

        victim->pcdata->clan = clan;
        if (clan->roster)
        {
                if (!hasname(clan->roster, victim->name))
                        addname(&clan->roster, victim->name);
        }
        else
                clan->roster = STRALLOC(victim->name);
        act(AT_MAGIC, "You induct $N into $t", ch, clan->name, victim,
            TO_CHAR);
        act(AT_MAGIC, "$n inducts $N into $t", ch, clan->name, victim,
            TO_NOTVICT);
        act(AT_MAGIC, "$n inducts you into $t", ch, clan->name, victim,
            TO_VICT);
        save_char_obj(victim);
        return;
}

CMDF do_outcast(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        CLAN_DATA *clan;
        char      buf[MAX_STRING_LENGTH];

        if (IS_NPC(ch) || !ch->pcdata->clan)
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        clan = ch->pcdata->clan;
        if (!HAS_CLAN_PERM(ch, clan, "outcast"))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }


        argument = one_argument(argument, arg);

        if (arg[0] == '\0')
        {
                send_to_char("Outcast whom?\n\r", ch);
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

        if (victim == ch)
        {
                send_to_char("Kick yourself out of your own clan?\n\r", ch);
                return;
        }

        if (victim->pcdata->clan != ch->pcdata->clan)
        {
                send_to_char("This player does not belong to your clan!\n\r",
                             ch);
                return;
        }


        --clan->members;
        if (!str_cmp(victim->name, ch->pcdata->clan->number1))
        {
                STRFREE(ch->pcdata->clan->number1);
                ch->pcdata->clan->number1 = STRALLOC("");
        }
        if (!str_cmp(victim->name, ch->pcdata->clan->number2))
        {
                STRFREE(ch->pcdata->clan->number2);
                ch->pcdata->clan->number2 = STRALLOC("");
        }

        if (ch->pcdata->clan->roster)
                if (hasname(ch->pcdata->clan->roster, ch->name))
                        removename(&ch->pcdata->clan->roster, ch->name);

        victim->pcdata->clan = NULL;
        act(AT_MAGIC, "You outcast $N from $t", ch, clan->name, victim,
            TO_CHAR);
        act(AT_MAGIC, "$n outcasts $N from $t", ch, clan->name, victim,
            TO_ROOM);
        act(AT_MAGIC, "$n outcasts you from $t", ch, clan->name, victim,
            TO_VICT);
        snprintf(buf, MSL, "%s has been outcast from %s!", victim->name,
                 clan->name);
        echo_to_all(AT_MAGIC, buf, ECHOTAR_ALL);

        STRFREE(victim->pcdata->bestowments);
        victim->pcdata->bestowments = STRALLOC("");

        save_char_obj(victim);  /* clan gets saved when pfile is saved */
        return;
}

CMDF do_setclan(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        CLAN_DATA *clan;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char
                        ("Usage: setclan <clan> <field> <leader|number1|number2> <player>\n\r",
                         ch);
                send_to_char("\n\rField being one of:\n\r", ch);
                send_to_char(" leader number1 number2 subclan enlistroom\n\r",
                             ch);
                send_to_char(" members board recall storage\n\r", ch);
                send_to_char(" funds jail alignment enlist motto\n\r", ch);
                send_to_char(" enemy ally delete", ch);
                if (get_trust(ch) >= LEVEL_SUB_IMPLEM)
                {
                        send_to_char(" name filename desc\n\r", ch);
                }
                return;
        }

        clan = get_clan(arg1);
        if (!clan)
        {
                send_to_char("No such clan.\n\r", ch);
                return;
        }

        if (!strcmp(arg2, "leader"))
        {
                STRFREE(clan->leader);
                clan->leader = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg2, "enlistroom"))
        {
                clan->enlistroom = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg2, "subclan"))
        {
                CLAN_DATA *subclan;

                subclan = get_clan(argument);
                if (!subclan)
                {
                        send_to_char("Subclan is not a clan.\n\r", ch);
                        return;
                }
                if (subclan->clan_type == CLAN_SUBCLAN || subclan->mainclan)
                {
                        send_to_char
                                ("Subclan is already part of another organization.\n\r",
                                 ch);
                        return;
                }
                if (subclan->first_subclan)
                {
                        send_to_char
                                ("Subclan has subclans of its own that need removing first.\n\r",
                                 ch);
                        return;
                }
                subclan->clan_type = CLAN_SUBCLAN;
                subclan->mainclan = clan;
                LINK(subclan, clan->first_subclan, clan->last_subclan,
                     next_subclan, prev_subclan);
                save_clan(clan);
                save_clan(subclan);
                return;
        }

        if (!strcmp(arg2, "number1"))
        {
                STRFREE(clan->number1);
                clan->number1 = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg2, "number2"))
        {
                STRFREE(clan->number2);
                clan->number2 = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg2, "enemy"))
        {
                clan->enemy = get_clan(argument);
                if (clan->enemy == NULL)
                        send_to_char("Enemy Unset.\n\r", ch);
                else
                        send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg2, "ally"))
        {
                clan->ally = get_clan(argument);
                if (clan->ally == NULL)
                        send_to_char("Ally Unset.\n\r", ch);
                else
                        send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }


        if (!strcmp(arg2, "board"))
        {
                clan->board = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg2, "members"))
        {
                clan->members = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg2, "funds"))
        {
                clan->funds = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg2, "storage"))
        {
                clan->storeroom = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg2, "jail"))
        {
                clan->jail = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (get_trust(ch) < LEVEL_SUB_IMPLEM)
        {
                do_setclan(ch, "");
                return;
        }

        if (!strcmp(arg2, "type"))
        {
                if (clan->mainclan)
                {
                        UNLINK(clan, clan->mainclan->first_subclan,
                               clan->mainclan->last_subclan, next_subclan,
                               prev_subclan);
                        clan->mainclan = NULL;
                }
                if (!str_cmp(argument, "crime"))
                        clan->clan_type = CLAN_CRIME;
                else if (!str_cmp(argument, "crime family"))
                        clan->clan_type = CLAN_CRIME;
                else if (!str_cmp(argument, "guild"))
                        clan->clan_type = CLAN_GUILD;
                else
                        clan->clan_type = 0;
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg2, "name"))
        {
                STRFREE(clan->name);
                clan->name = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg2, "motto"))
        {
                STRFREE(clan->motto);
                clan->motto = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg2, "enlist"))
        {
                if (str_cmp(argument, "On") && str_cmp(argument, "Off"))
                {
                        send_to_char("&CChoices&R:&W On and Off.\n\r", ch);
                        return;
                }
                if (!str_cmp(argument, "On") || !str_cmp(argument, "on"))
                {
                        clan->enliston = 1;
                }
                if (!str_cmp(argument, "Off") || !str_cmp(argument, "off"))
                {
                        clan->enliston = 0;
                }
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }


        if (!strcmp(arg2, "alignment"))
        {
                clan->alignment = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg2, "filename"))
        {
                DISPOSE(clan->filename);
                clan->filename = str_dup(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                write_clan_list();
                return;
        }

        if (!strcmp(arg2, "desc"))
        {
                STRFREE(clan->description);
                clan->description = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg2, "delete"))
        {
                UNLINK(clan, first_clan, last_clan, next, prev);
                free_clan(clan);
                write_clan_list();
                return;
        }

        do_setclan(ch, "");
        return;
}


CMDF do_clanset(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        CLAN_DATA *clan;

        clan = ch->pcdata->clan;
        if (!clan)
        {
                send_to_char("You are not in a clan.\n\r", ch);
                return;
        }

        if (IS_NPC(ch) || str_cmp(ch->name, clan->leader))
        {
                send_to_char("You are not the leader of your clan.\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);

        if (arg1[0] == '\0')
        {
                send_to_char("Usage: clanset <field> <value>\n\r", ch);
                send_to_char("\n\rField being one of:\n\r", ch);
                send_to_char(" Motto Description Enlist Ally Enemy\n\r", ch);
                send_to_char
                        (" rank (1-13) eg. rank 1 <title>, rank 12 <title>\n\r",
                         ch);
                send_to_char
                        (" salary (1-13) eg. salary 1 <amount>, salary 12 <amount>\n\r",
                         ch);
                return;
        }


        if (!strcmp(arg1, "description"))
        {
                STRFREE(clan->description);
                clan->description = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg1, "motto"))
        {
                STRFREE(clan->description);
                clan->motto = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg1, "enlist"))
        {
                if (str_cmp(argument, "On") && str_cmp(argument, "Off"))
                {
                        send_to_char("&CChoices&R:&W On and Off.\n\r", ch);
                        return;
                }
                if (!str_cmp(argument, "On") || !str_cmp(argument, "on"))
                        clan->enliston = 1;
                if (!str_cmp(argument, "Off") || !str_cmp(argument, "off"))
                        clan->enliston = 0;

                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg1, "alignment"))
        {
                clan->alignment = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }


        if (!strcmp(arg1, "enemy"))
        {
                clan->enemy = get_clan(argument);
                if (clan->enemy == NULL)
                        send_to_char("Enemy Unset.\n\r", ch);
                else
                        send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg1, "ally"))
        {
                clan->ally = get_clan(argument);
                if (clan->ally == NULL)
                        send_to_char("Ally Unset.\n\r", ch);
                else
                        send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }


        if (!strcmp(arg1, "rank"))
        {
                char      arg2[MSL];
                int       rank;

                argument = one_argument(argument, arg2);
                rank = atoi(arg2);
                if (rank > MAX_RANK || rank < 0)
                {
                        send_to_char("Range is 0 - 13", ch);
                        return;
                }
                if (clan->rank[rank])
                        STRFREE(clan->rank[rank]);
                clan->rank[rank] = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        if (!strcmp(arg1, "salary"))
        {
                char      arg2[MSL];
                int       rank;

                argument = one_argument(argument, arg2);
                rank = atoi(arg2);
                if (rank > MAX_RANK || rank < 0)
                {
                        send_to_char("Range is 0 - 13", ch);
                        return;
                }
                clan->salary[rank] = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_clan(clan);
                return;
        }

        do_clanset(ch, "");
        return;
}

CMDF do_setplanet(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        PLANET_DATA *planet;
        int       i;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0')
        {
                send_to_char("Usage: setplanet <planet> <field> [value]\n\r",
                             ch);
                send_to_char("\n\rField being one of:\n\r", ch);
                send_to_char(" base_value flags planet_type\n\r", ch);
                send_to_char
                        (" name filename starsystem governed_by pop_support delete\n\r",
                         ch);
                send_to_char
                        (" import export resource produces consumes jail\n\r",
                         ch);
                send_to_char
                        (" shields turbolasers ioncannons defbattalions attbattalions\n\r",
                         ch);
                return;
        }

        planet = get_planet(arg1);
        if (!planet)
        {
                send_to_char("No such planet.\n\r", ch);
                return;
        }


        if (!strcmp(arg2, "name"))
        {
                argument = one_argument(argument, arg3);
                STRFREE(planet->name);
                planet->name = STRALLOC(arg3);
        }
        else if (!strcmp(arg2, "governed_by"))
        {
                CLAN_DATA *clan;

                clan = get_clan(argument);
                if (clan)
                {
                        planet->governed_by = clan;
                }
                else
                {
                        send_to_char("No such clan.\n\r", ch);
                        return;
                }
        }
        else if (!strcmp(arg2, "starsystem"))
        {
                SPACE_DATA *starsystem;

                argument = one_argument(argument, arg3);

                if ((starsystem = planet->starsystem) != NULL)
                        UNLINK(planet, starsystem->first_planet,
                               starsystem->last_planet, next_in_system,
                               prev_in_system);
                if ((planet->starsystem = starsystem_from_name(arg3)))
                {
                        starsystem = planet->starsystem;
                        LINK(planet, starsystem->first_planet,
                             starsystem->last_planet, next_in_system,
                             prev_in_system);
                        send_to_char("Done.\n\r", ch);
                }
                else
                {
                        send_to_char("No such starsystem.\n\r", ch);
                        return;
                }
        }
        else if (!strcmp(arg2, "filename"))
        {
                DISPOSE(planet->filename);
                planet->filename = str_dup(argument);
                send_to_char("Done.\n\r", ch);
                save_planet(planet, FALSE);
                write_planet_list();
                return;
        }
        else if (!strcmp(arg2, "initialize"))
        {
                for (i = 0; i < CARGO_MAX; i++)
                {
                        planet->resource[i] = 50000;
                        planet->produces[i] = 275;
                        planet->consumes[i] = 100;
                }
                send_to_char("Planet Initialized\r\n", ch);
        }
        else if (!strcmp(arg2, "contraband"))
        {
                for (i = CARGO_MAX + 1; i < CONTRABAND_MAX; i++)
                {
                        planet->resource[i] = 5000;
                        planet->produces[i] = 10;
                        planet->consumes[i] = 7;
                }
                send_to_char("Planet Contraband Initialized\r\n", ch);
        }
        else if (!strcmp(arg2, "pop_support"))
        {
                planet->pop_support = atoi(argument);
        }
        else if (!strcmp(arg2, "base_value"))
        {
                planet->base_value = atoi(argument);
        }
        else if (!strcmp(arg2, "planet_type"))
        {
                planet->planet_type = atoi(argument);
        }
        else if (!strcmp(arg2, "shields"))
        {
                planet->shields = atoi(argument);
        }
        else if (!strcmp(arg2, "turbolasers"))
        {
                planet->turbolasers = atoi(argument);
        }
        else if (!strcmp(arg2, "ioncannons"))
        {
                planet->ioncannons = atoi(argument);
        }
        else if (!strcmp(arg2, "jail"))
        {
                planet->jail = atoi(argument);
        }
        else if (!str_prefix(arg2, "defbattalions"))
        {
                planet->defbattalions = atoi(argument);
        }
        else if (!str_prefix(arg2, "attbattalions"))
        {
                planet->attbattalions = atoi(argument);
        }
        else if (!strcmp(arg2, "flags"))
        {
                char      farg[MAX_INPUT_LENGTH];

                if (argument[0] == '\0')
                {
                        send_to_char(argument, ch);
                        send_to_char("Possible flags: nocapture\n\r", ch);
                        return;
                }

                while (argument[0] != '\0')
                {
                        argument = one_argument(argument, farg);

                        if (farg[0] == '\0')
                                break;

                        if (!str_cmp(farg, "nocapture"))
                                TOGGLE_BIT(planet->flags, PLANET_NOCAPTURE);
                        else if (!str_cmp(farg, "shield"))
                                TOGGLE_BIT(planet->flags, PLANET_SHIELD);
                        else
                                ch_printf(ch, "No such flag: %s\n\r", farg);
                }
        }
        else if (!strcmp(arg2, "import"))
        {
                argument = one_argument(argument, arg3);
                for (i = 0; i < CONTRABAND_MAX; i++)
                {
                        if (!str_cmp(arg3, cargo_names[i]))
                        {
                                planet->cargoimport[i] = atoi(argument);
                                planet->cargoexport[i] = 0;
                                break;
                        }
                }
                if (i == CONTRABAND_MAX)
                {
                        send_to_char("No such resource type\r\n", ch);
                        return;
                }
        }
        else if (!strcmp(arg2, "export"))
        {
                argument = one_argument(argument, arg3);
                for (i = 0; i < CONTRABAND_MAX; i++)
                {
                        if (!str_cmp(arg3, cargo_names[i]))
                        {
                                planet->cargoexport[i] = atoi(argument);
                                planet->cargoimport[i] = 0;
                                break;
                        }
                }
                if (i == CONTRABAND_MAX)
                {
                        send_to_char("No such resource type\r\n", ch);
                        return;
                }
        }
        else if (!strcmp(arg2, "resource"))
        {
                argument = one_argument(argument, arg3);
                for (i = 0; i < CONTRABAND_MAX; i++)
                {
                        if (!str_cmp(arg3, cargo_names[i]))
                        {
                                planet->resource[i] = atoi(argument);
                                break;
                        }
                }
                if (i == CONTRABAND_MAX)
                {
                        send_to_char("No such resource type\r\n", ch);
                        return;
                }
        }
        else if (!strcmp(arg2, "produces"))
        {
                argument = one_argument(argument, arg3);
                for (i = 0; i < CONTRABAND_MAX; i++)
                {
                        if (!str_cmp(arg3, cargo_names[i]))
                        {
                                planet->produces[i] = atoi(argument);
                                break;
                        }
                }
                if (i == CONTRABAND_MAX)
                {
                        send_to_char("No such resource type\r\n", ch);
                        return;
                }
        }
        else if (!strcmp(arg2, "consumes"))
        {
                argument = one_argument(argument, arg3);
                for (i = 0; i < CONTRABAND_MAX; i++)
                {
                        if (!str_cmp(arg3, cargo_names[i]))
                        {
                                planet->consumes[i] = atoi(argument);
                                break;
                        }
                }
                if (i == CONTRABAND_MAX)
                {
                        send_to_char("No such resource type\r\n", ch);
                        return;
                }
        }
        else if (!strcmp(arg2, "body"))
        {
                argument = one_argument(argument, arg3);
                planet->body = get_body(arg3);
                if (!planet->body)
                {
                        planet->body = NULL;
                        send_to_char("No such celestial body.\n\r", ch);
                        return;
                }
        }
        else if (!strcmp(arg2, "delete"))
        {
                UNLINK(planet, first_planet, last_planet, next, prev);
                free_planet(planet);
                write_planet_list();
        }
        else
        {
                do_setplanet(ch, "");
                return;
        }
        send_to_char("Done.\n\r", ch);
        save_planet(planet, FALSE);
        return;
}

CMDF do_showclan(CHAR_DATA * ch, char *argument)
{
        CLAN_DATA *clan;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Usage: showclan <clan>\n\r", ch);
                return;
        }

        clan = get_clan(argument);
        if (!clan)
        {
                send_to_char("No such clan.\n\r", ch);
                return;
        }

        ch_printf(ch, "%s      : %s\n\rFilename: %s\n\r",
                  clan->clan_type == CLAN_CRIME ? "Crime Family " :
                  clan->clan_type == CLAN_GUILD ? "Guild " : "Organization ",
                  clan->name, clan->filename);
        ch_printf(ch, "Description: %s\n\rLeader: %s\n\r",
                  clan->description, clan->leader);
        ch_printf(ch,
                  "Number1: %s\n\rNumber2: %s\n\rPKills: %6d    PDeaths: %6d\n\r",
                  clan->number1, clan->number2, clan->pkills, clan->pdeaths);
        ch_printf(ch, "MKills: %6d    MDeaths: %6d\n\r", clan->mkills,
                  clan->mdeaths);
        ch_printf(ch, "Type: %d\n\r", clan->clan_type);
        ch_printf(ch, "Members: %3d\n\r", clan->members);
        ch_printf(ch, "Enlist Room: %ld\n\r", clan->enlistroom);
        ch_printf(ch, "Board: %5d   Jail: %5d\n\r", clan->board, clan->jail);
        ch_printf(ch, "Funds: %ld\n\r", clan->funds);
        ch_printf(ch, "Alignment: %ld\n\r", clan->alignment);
        ch_printf(ch, "Motto: %s\n\r", clan->motto);
        ch_printf(ch, "Enlist: %d\n\r", clan->enliston);
        ch_printf(ch, "Ally: %s   Enemy: %s   \n\r",
                  clan->ally ? clan->ally->name : "none",
                  clan->enemy ? clan->enemy->name : "none");
        ch_printf(ch, "Roster: %s\n\r",
                  clan->roster ? wordwrap(clan->roster, 80) : "(None)");
        return;
}

CMDF do_showplanet(CHAR_DATA * ch, char *argument)
{
        PLANET_DATA *planet = NULL;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Usage: showplanet <planet>\n\r", ch);
                return;
        }

        planet = get_planet(argument);
        if (!planet)
        {
                send_to_char("No such planet.\n\r", ch);
                return;
        }

        ch_printf(ch, "&W%s\n\r", planet->name);
        if (IS_IMMORTAL(ch))
        {
                AREA_DATA *area = NULL;

                ch_printf(ch, "&WFilename: &G%s\n\r", planet->filename);
                send_to_char("&WAreas: &G", ch);
                if (planet->first_area)
                {
                        for (area = planet->first_area; area;
                             area = area->next_on_planet)
                        {
                                send_to_char(area->filename, ch);
                                send_to_char(", ", ch);
                        }
                        send_to_char("\n\r", ch);
                }
                else
                {
                        send_to_char("None\n\r", ch);
                }
        }

        if (planet->starsystem && planet->starsystem->name)
                ch_printf(ch, "&WStar system:&G%s\n\r",
                          planet->starsystem->name);
        if (planet->body && planet->body->name())
                ch_printf(ch, "&WBody:&G%s\n\r", planet->body->name());
        ch_printf(ch, "&WGoverned by: &G%s\n\r",
                  planet->governed_by ? planet->governed_by->name : "");
        ch_printf(ch, "&WPopulation: &G%d&W\n\r", planet->population);
        ch_printf(ch, "&WPopular Support: &G%.2f\n\r", planet->pop_support);
        ch_printf(ch, "&WCurrent Monthly Revenue: &G%ld\n\r",
                  get_taxes(planet));
        ch_printf(ch, "&WJail: &G%d\n\r", planet->jail);
        return;
}


CMDF do_makeclan(CHAR_DATA * ch, char *argument)
{
        char      filename[256];
        CLAN_DATA *clan;
        bool      found;
		short     i;

        if (!argument || argument[0] == '\0')
        {
                send_to_char("Usage: makeclan <clan name>\n\r", ch);
                return;
        }

        found = FALSE;
        snprintf(filename, 256, "%s.clan", smash_space(argument));

        CREATE(clan, CLAN_DATA, 1);
        LINK(clan, first_clan, last_clan, next, prev);
        clan->next_subclan = NULL;
        clan->prev_subclan = NULL;
        clan->last_subclan = NULL;
        clan->first_subclan = NULL;
        clan->mainclan = NULL;
		clan->filename = STRALLOC(filename);
        clan->name = STRALLOC(argument);
        clan->description = STRALLOC("");
        clan->motto = STRALLOC("");
        clan->leader = STRALLOC("");
        clan->number1 = STRALLOC("");
        clan->number2 = STRALLOC("");
        clan->tmpstr = STRALLOC("");
        clan->enliston = 1;
        clan->alignment = 0;
        clan->roster = NULL;
		for (i=0;i<MAX_RANK ;i++ )
			clan->rank[i] = STRALLOC("None");
		save_clan(clan);
		write_clan_list();
}

CMDF do_makeplanet(CHAR_DATA * ch, char *argument)
{
        char      filename[256];
        PLANET_DATA *planet;
        bool      found;

        if (!argument || argument[0] == '\0')
        {
                send_to_char("Usage: makeplanet <planet name>\n\r", ch);
                return;
        }

        found = FALSE;
        snprintf(filename, MSL, "%s%s", PLANET_DIR, strlower(argument));

        CREATE(planet, PLANET_DATA, 1);
        LINK(planet, first_planet, last_planet, next, prev);
        planet->governed_by = NULL;
        planet->next_in_system = NULL;
        planet->prev_in_system = NULL;
        planet->starsystem = NULL;
        planet->first_area = NULL;
        planet->last_area = NULL;
        planet->name = STRALLOC(argument);
        planet->flags = 0;
}

char     *clan_type(CLAN_DATA * clan)
{
        static char type[250];

        switch (clan->clan_type)
        {
        case CLAN_PLAIN:
                mudstrlcpy(type, "Government", 250);
                break;
        case CLAN_CRIME:
                mudstrlcpy(type, "Crime", 250);
                break;
        case CLAN_GUILD:
                mudstrlcpy(type, "Guild", 250);
                break;
        case CLAN_SUBCLAN:
                mudstrlcpy(type, "Subclan", 250);
                break;
        default:
                mudstrlcpy(type, "(Error)", 250);
                break;
        }
        return type;
}

CMDF do_clans(CHAR_DATA * ch, char *argument)
{
        CLAN_DATA *clan;
        int       count = 0;

        if (!first_clan)
        {
                set_pager_color(AT_BLOOD, ch);
                send_to_pager("There are no clans currently formed.\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                set_pager_color(AT_WHITE, ch);


                send_to_pager
                        ("&B-----------------------------------------------------------------------------\n\r",
                         ch);
                for (clan = first_clan; clan; clan = clan->next)
                {
                        if (    /*clan->clan_type == CLAN_CRIME || 
                                 * clan->clan_type == CLAN_GUILD ||  */
                                   clan->clan_type == CLAN_SUBCLAN)
                                continue;

                        pager_printf(ch,
                                     "&BO&zrganization: &W%-37.37s &BT&zype&B: &W%s\n\r",
                                     clan->name, clan_type(clan));
                        pager_printf(ch,
                                     "  &BE&znlisting?&B: &W%-3s&B           &BM&zembers&B: &W%-4d&B           &BM&zin. &BA&zlign&B: &W%-5d&B\n\r",
                                     clan->enliston ==
                                     1 ? "Yes" : clan->enliston ==
                                     0 ? "No" : "Unknown", clan->members,
                                     clan->alignment);

                        if (clan->first_subclan)
                        {
                                CLAN_DATA *subclan;

                                pager_printf(ch, "  &BS&zub clans&B:\n\r");
                                for (subclan = clan->first_subclan; subclan;
                                     subclan = subclan->next_subclan)
                                {
                                        pager_printf(ch,
                                                     "    &BO&zrganization: &W%-33.33s &BM&zembers&B:&W %d\n\r",
                                                     subclan->name,
                                                     subclan->members);
                                        count++;
                                }
                        }
                        /*
                         * send_to_pager("\n\r", ch); 
                         */
                        if (clan->next)
                                send_to_pager
                                        ("                     &R-------------------------------                         \n\r",
                                         ch);
                        count++;
                }

                /*
                 * for ( clan = first_clan; clan; clan = clan->next )
                 * {
                 * if ( !str_cmp ( clan->name, "none" ) ) continue;
                 * if ( clan->clan_type != CLAN_CRIME && clan->clan_type != CLAN_GUILD ) continue;
                 * pager_printf( ch, "&BO&zrganization: &W%s\n\r", clan->name);
                 * pager_printf( ch, "  &BE&znlisting?: &W%-3s&B           &BM&zembers: &W%-4d&B           &BM&zin. &BA&zlign: &W%-5d&B\n\r",
                 * clan->enliston == 1 ? "Yes" : 
                 * clan->enliston == 0 ? "No"  : "Unknown",
                 * clan->members, clan->alignment);
                 * 
                 * send_to_pager("\n\r", ch);
                 * count++;
                 * }
                 */

                send_to_pager
                        ("&B-----------------------------------------------------------------------------\n\r",
                         ch);
                pager_printf(ch,
                             "\n\r&BT&zotal: &W%d\n\r&z\"&Yclans [clan name]&z\" for more information.\n\r",
                             count);
        }
        else
        {
                clan = get_clan(argument);
                if (!clan)
                {
                        send_to_pager("&YNo such clan.\n\r", ch);
                        do_clans(ch, "");
                        return;
                }
                send_to_pager
                        ("&B-----------------------------------------------------------------------------\n\r",
                         ch);
                pager_printf(ch, "&BO&zrganization: &W%s\n\r", clan->name);
                pager_printf(ch, "  &BM&zotto: &W%s\n\r", clan->motto);
                pager_printf(ch, "  &BD&zescription: &W%s\n\r",
                             clan->description);
                pager_printf(ch,
                             "  &BE&znlisting?: &W%-3s&B\n\r  &BM&zembers: &W%-4d&B\n\r  &BM&zin. &BA&zlign: &W%-5d\n\r",
                             clan->enliston == 1 ? "Yes" : clan->enliston ==
                             0 ? "No" : "Unknown", clan->members,
                             clan->alignment);
                send_to_pager
                        ("&B-----------------------------------------------------------------------------\n\r",
                         ch);
        }
}

CMDF do_planets(CHAR_DATA * ch, char *argument)
{
        PLANET_DATA *planet;
        AREA_DATA *area;

        if (argument[0] == '\0')
        {
                set_pager_color(AT_WHITE, ch);
                if (!first_planet)
                {
                        set_pager_color(AT_BLOOD, ch);
                        send_to_pager
                                ("There are no planets currently formed.\n\r",
                                 ch);
                        return;
                }

                send_to_pager
                        ("&B-----------------------------------------------------------------------------\n\r",
                         ch);
                for (planet = first_planet; planet; planet = planet->next)
                {
                        pager_printf(ch,
                                     "&BP&zlanet: &w%-15s        &BG&zoverned &BB&zy: &w%s %s\n\r",
                                     planet->name,
                                     planet->governed_by ? planet->
                                     governed_by->name : "",
                                     IS_SET(planet->flags,
                                            PLANET_NOCAPTURE) ?
                                     "&B(&zpermanent&B)" : "");
                        pager_printf(ch, "&BV&zalue: &w%-10ld&z/&w%-10d   ",
                                     get_taxes(planet), planet->base_value);
                        pager_printf(ch,
                                     "&BP&zopulation: &w%-5d    &BP&zop &BS&zupport: &w%.1d\n\r",
                                     planet->population, planet->pop_support);
                        if (planet->next)
                                send_to_pager
                                        ("                     &R-------------------------------                         \n\r",
                                         ch);
                        else
                                send_to_pager
                                        ("&B-----------------------------------------------------------------------------\n\r",
                                         ch);
                }
                send_to_pager
                        ("\n\r&z\"&Yplanet [planetname]&z\" for more information.\n\r",
                         ch);
        }
        else
        {
                planet = get_planet(argument);
                if (!planet)
                {
                        do_planets(ch, "");
                        send_to_char("No such planet.\n\r", ch);
                        return;
                }
                send_to_pager
                        ("&B-----------------------------------------------------------------------------\n\r",
                         ch);
                pager_printf(ch,
                             "&BP&zlanet: &w%-15s        &BG&zoverned &BB&zy: &w%s %s\n\r",
                             planet->name,
                             planet->governed_by ? planet->governed_by->
                             name : "", IS_SET(planet->flags,
                                               PLANET_NOCAPTURE) ?
                             "&B(&zpermanent&B)" : "");
                pager_printf(ch, "&BV&zalue: &w%-10ld&z/&w%-10d   ",
                             get_taxes(planet), planet->base_value);
                pager_printf(ch,
                             "&BP&zopulation: &w%-5d    &BP&zop &BS&zupport: &w%.1d\n\r",
                             planet->population, planet->pop_support);
                pager_printf(ch,
                             "&BA&zttacking &BB&zatallions: &w%-5d    &BD&zefending &BB&zatallions: &w%.1d\n\r",
                             planet->attbattalions, planet->defbattalions);
                pager_printf(ch,
                             "&BP&zlanetary &BS&zhields: &w%-8d    &BT&zurbolasers: &w%.1d &BI&zon &BC&zannons: &w%.1d\n\r",
                             planetary_installations(planet,
                                                     BATTERY_INSTALLATION),
                             planetary_installations(planet,
                                                     TURBOLASER_INSTALLATION),
                             planetary_installations(planet,
                                                     ION_INSTALLATION));
                if (IS_IMMORTAL(ch))
                {
                        pager_printf(ch, "&BA&zreas: &w");
                        for (area = planet->first_area; area;
                             area = area->next_on_planet)
                                pager_printf(ch, "%s, ", area->filename);
                        pager_printf(ch, "\n\r");
                        pager_printf(ch, "&BT&zype: &w%d",
                                     planet->planet_type);
                        if (planet->attgovern)
                                pager_printf(ch, "\n\r&BA&zttacker:&G %s",
                                             planet->attgovern->name);
                }
                send_to_pager
                        ("\n\r&B-----------------------------------------------------------------------------\n\r",
                         ch);
        }


}

CMDF do_shove(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        int       exit_dir;
        EXIT_DATA *pexit;
        CHAR_DATA *victim;
        bool      nogo;
        ROOM_INDEX_DATA *to_room;
        int       percent_chance;

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);


        if (arg[0] == '\0')
        {
                send_to_char("Shove whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char("You shove yourself around, to no avail.\n\r",
                             ch);
                return;
        }

        if ((victim->position) != POS_STANDING)
        {
                act(AT_PLAIN, "$N isn't standing up.", ch, NULL, victim,
                    TO_CHAR);
                return;
        }

        if (arg2[0] == '\0')
        {
                send_to_char("Shove them in which direction?\n\r", ch);
                return;
        }

        exit_dir = get_dir(arg2);
        if (xIS_SET(victim->in_room->room_flags, ROOM_SAFE)
            && get_timer(victim, TIMER_SHOVEDRAG) <= 0)
        {
                send_to_char("That character cannot be shoved right now.\n\r",
                             ch);
                return;
        }
        victim->position = POS_SHOVE;
        nogo = FALSE;
        if ((pexit = get_exit(ch->in_room, exit_dir)) == NULL)
                nogo = TRUE;
        else if (IS_SET(pexit->exit_info, EX_CLOSED)
                 && (!IS_AFFECTED(victim, AFF_PASS_DOOR)
                     || IS_SET(pexit->exit_info, EX_NOPASSDOOR)))
                nogo = TRUE;
        if (nogo)
        {
                send_to_char("There's no exit in that direction.\n\r", ch);
                victim->position = POS_STANDING;
                return;
        }
        to_room = pexit->to_room;


        if (ch->in_room->area != to_room->area
            && !in_hard_range(victim, to_room->area))
        {
                send_to_char("That character cannot enter that area.\n\r",
                             ch);
                victim->position = POS_STANDING;
                return;
        }

        percent_chance = 50;

/* Add 3 points to percent_chance for every str point above 15, subtract for 
below 15 */

        percent_chance += ((get_curr_str(ch) - 15) * 3);

        percent_chance += (ch->top_level - victim->top_level);

/* Debugging purposes - show percentage for testing */

/* snprintf(buf, MSL, "Shove percentage of %s = %d", ch->name, percent_chance);
act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
*/

        if (percent_chance < number_percent())
        {
                send_to_char("You failed.\n\r", ch);
                victim->position = POS_STANDING;
                return;
        }
        act(AT_ACTION, "You shove $M.", ch, NULL, victim, TO_CHAR);
        act(AT_ACTION, "$n shoves you.", ch, NULL, victim, TO_VICT);
        move_char(victim, get_exit(ch->in_room, exit_dir), 0, FALSE);
        if (!char_died(victim))
                victim->position = POS_STANDING;
        WAIT_STATE(ch, 12);
        /*
         * Remove protection from shove/drag if char shoves -- Blodkai 
         */
        if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE)
            && get_timer(ch, TIMER_SHOVEDRAG) <= 0)
                add_timer(ch, TIMER_SHOVEDRAG, 10, NULL, 0);
}

CMDF do_drag(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        int       exit_dir;
        CHAR_DATA *victim;
        EXIT_DATA *pexit;
        ROOM_INDEX_DATA *to_room;
        bool      nogo;
        int       percent_chance;

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);

        if (arg[0] == '\0')
        {
                send_to_char("Drag whom?\n\r", ch);
                return;
        }

        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("They aren't here.\n\r", ch);
                return;
        }

        if (victim == ch)
        {
                send_to_char
                        ("You take yourself by the scruff of your neck, but go nowhere.\n\r",
                         ch);
                return;
        }



        if (victim->fighting)
        {
                send_to_char("You try, but can't get close enough.\n\r", ch);
                return;
        }

        if (arg2[0] == '\0')
        {
                send_to_char("Drag them in which direction?\n\r", ch);
                return;
        }

        exit_dir = get_dir(arg2);

        if (xIS_SET(victim->in_room->room_flags, ROOM_SAFE)
            && get_timer(victim, TIMER_SHOVEDRAG) <= 0)
        {
                send_to_char
                        ("That character cannot be dragged right now.\n\r",
                         ch);
                return;
        }

        nogo = FALSE;
        if ((pexit = get_exit(ch->in_room, exit_dir)) == NULL)
                nogo = TRUE;
        else if (IS_SET(pexit->exit_info, EX_CLOSED)
                 && (!IS_AFFECTED(victim, AFF_PASS_DOOR)
                     || IS_SET(pexit->exit_info, EX_NOPASSDOOR)))
                nogo = TRUE;
        if (nogo)
        {
                send_to_char("There's no exit in that direction.\n\r", ch);
                return;
        }

        to_room = pexit->to_room;

        if (ch->in_room->area != to_room->area
            && !in_hard_range(victim, to_room->area))
        {
                send_to_char("That character cannot enter that area.\n\r",
                             ch);
                victim->position = POS_STANDING;
                return;
        }

        percent_chance = 50;


/*
snprintf(buf,MSL,  "Drag percentage of %s = %d", ch->name, percent_chance);
act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
*/
        if (percent_chance < number_percent())
        {
                send_to_char("You failed.\n\r", ch);
                victim->position = POS_STANDING;
                return;
        }
        if (victim->position < POS_STANDING)
        {
                sh_int    temp;

                temp = victim->position;
                victim->position = POS_DRAG;
                act(AT_ACTION, "You drag $M into the next room.", ch, NULL,
                    victim, TO_CHAR);
                act(AT_ACTION, "$n grabs your hair and drags you.", ch, NULL,
                    victim, TO_VICT);
                move_char(victim, get_exit(ch->in_room, exit_dir), 0, FALSE);
                if (!char_died(victim))
                        victim->position = temp;
/* Move ch to the room too.. they are doing dragging - Scryn */
                move_char(ch, get_exit(ch->in_room, exit_dir), 0, FALSE);
                WAIT_STATE(ch, 12);
                return;
        }
        send_to_char("You cannot do that to someone who is standing.\n\r",
                     ch);
        return;
}

CMDF do_enlist(CHAR_DATA * ch, char *argument)
{

        CLAN_DATA *clan;
        INSTALLATION_DATA *installation;

        argument = NULL;

        if (IS_NPC(ch) || !ch->pcdata)
        {
                send_to_char("You can't do that.\n\r", ch);
                return;
        }

        if (ch->pcdata->clan)
        {
                ch_printf(ch,
                          "You will have to resign from %s before you can join a new organization.\n\r",
                          ch->pcdata->clan->name);
                return;
        }

#ifndef ACCOUNT
        if (ch->pcdata->rp < 2)
#else
        if (ch->pcdata->account && ch->pcdata->account->rppoints < 2)
#endif
        {
                ch_printf(ch,
                          "You will have to gain more role playing points before you can join a clan.\n\r");
                return;
        }

        if (!xIS_SET(ch->in_room->room_flags, ROOM_RECRUIT))
        {
                send_to_char
                        ("You don't seem to be in a recruitment office.\n\r",
                         ch);
                return;
        }

        installation = installation_from_room(ch->in_room->vnum);

        if (!installation)
                for (clan = first_clan; clan; clan = clan->next)
                {
                        if (ch->in_room->vnum == clan->enlistroom)
                        {
                                if (clan->alignment > 0)
                                {
                                        if (ch->alignment < clan->alignment)
                                        {
                                                send_to_char
                                                        ("Your alignment is not within the ideals of this clan",
                                                         ch);
                                                return;
                                        }

                                }
                                else if (clan->alignment < 0)
                                {
                                        if (ch->alignment > clan->alignment)
                                        {
                                                send_to_char
                                                        ("Your alignment is not within the ideals of this clan",
                                                         ch);
                                                return;
                                        }
                                }

                                if (clan->enliston == 1)
                                {
                                        ++clan->members;
                                        ch->pcdata->clan = clan;
   									    ch->pcdata->clanrank = 0;
                                        if (clan->roster)
                                        {
                                                if (!hasname
                                                    (clan->roster, ch->name))
                                                        addname(&clan->roster,
                                                                ch->name);
                                        }
                                        else
                                                clan->roster =
                                                        STRALLOC(ch->name);
                                        ch_printf(ch, "Welcome to %s.\n\r",
                                                  clan->name);
                                        save_clan(clan);
                                        return;
                                }
                                else
                                        send_to_char
                                                ("They don't seem to be recruiting right now.\n\r",
                                                 ch);
                                return;
                        }
                }

        else
                for (clan = first_clan; clan; clan = clan->next)
                {
                        if (installation->clan)
                        {
                                if (clan->enliston == 1)
                                {
                                        ++clan->members;
                                        ch->pcdata->clan = installation->clan;
                                        ch_printf(ch, "Welcome to %s.\n\r",
                                                  clan->name);
                                        save_clan(clan);
                                        return;
                                }
                                else
                                        send_to_char
                                                ("They don't seem to be recruiting right now.\n\r",
                                                 ch);
                                return;
                        }
                }

        send_to_char("They don't seem to be recruiting right now.\n\r", ch);
        return;

}


CMDF do_resign(CHAR_DATA * ch, char *argument)
{

        CLAN_DATA *clan;
        long      lose_exp;
        char      buf[MAX_STRING_LENGTH];

        argument = NULL;

        if (IS_NPC(ch) || !ch->pcdata)
        {
                send_to_char("You can't do that.\n\r", ch);
                return;
        }

        clan = ch->pcdata->clan;

        if (clan == NULL)
        {
                send_to_char
                        ("You have to join an organization before you can quit it.\n\r",
                         ch);
                return;
        }

        if (!str_cmp(ch->name, ch->pcdata->clan->leader))
        {
                ch_printf(ch,
                          "You can't resign from %s ... you are the leader!\n\r",
                          clan->name);
                return;
        }

        --clan->members;
        if (!str_cmp(ch->name, ch->pcdata->clan->number1))
        {
                STRFREE(ch->pcdata->clan->number1);
                ch->pcdata->clan->number1 = STRALLOC("");
        }
        if (!str_cmp(ch->name, ch->pcdata->clan->number2))
        {
                STRFREE(ch->pcdata->clan->number2);
                ch->pcdata->clan->number2 = STRALLOC("");
        }
        if (ch->pcdata->clan->roster)
                if (hasname(ch->pcdata->clan->roster, ch->name))
                        removename(&ch->pcdata->clan->roster, ch->name);
        ch->pcdata->clan = NULL;
        act(AT_MAGIC, "You resign your position in $t", ch, clan->name, NULL,
            TO_CHAR);
        snprintf(buf, MSL, "%s has quit %s!", ch->name, clan->name);
        echo_to_all(AT_MAGIC, buf, ECHOTAR_ALL);

        lose_exp =
                UMAX(ch->experience[DIPLOMACY_ABILITY] -
                     exp_level(ch->skill_level[DIPLOMACY_ABILITY]), 0);
        ch_printf(ch, "You lose %ld diplomacy experience.\n\r", lose_exp);
        ch->experience[DIPLOMACY_ABILITY] -= lose_exp;

        STRFREE(ch->pcdata->bestowments);
        ch->pcdata->bestowments = STRALLOC("");
		ch->pcdata->clanrank = 0;

        save_char_obj(ch);  /* clan gets saved when pfile is saved */

        return;

}

CMDF do_clan_withdraw(CHAR_DATA * ch, char *argument)
{
        CLAN_DATA *clan;
        long      amount;

        if (IS_NPC(ch) || !ch->pcdata->clan)
        {
                send_to_char
                        ("You don't seem to belong to an organization to withdraw funds from...\n\r",
                         ch);
                return;
        }

        if (!ch->in_room || !xIS_SET(ch->in_room->room_flags, ROOM_BANK))
        {
                send_to_char("You must be in a bank to do that!\n\r", ch);
                return;
        }

        if (!HAS_CLAN_PERM(ch, ch->pcdata->clan, "withdraw"))
        {
                send_to_char
                        ("&RYour organization hasn't seen fit to bestow you with that ability.",
                         ch);
                return;
        }

        clan = ch->pcdata->clan;

        amount = atoi(argument);

        if (!amount)
        {
                send_to_char("How much would you like to withdraw?\n\r", ch);
                return;
        }

        if (amount > clan->funds)
        {
                ch_printf(ch, "%s doesn't have that much!\n\r", clan->name);
                return;
        }

        if (amount < 0)
        {
                ch_printf(ch, "Nice try...\n\r");
                return;
        }

        ch_printf(ch, "You withdraw %ld credits from %s's funds.\n\r", amount,
                  clan->name);

        clan->funds -= amount;
        ch->gold += amount;
        save_clan(clan);

}


CMDF do_clan_donate(CHAR_DATA * ch, char *argument)
{
        CLAN_DATA *clan;
        long      amount;

        if (IS_NPC(ch) || !ch->pcdata->clan)
        {
                send_to_char
                        ("You don't seem to belong to an organization to donate to...\n\r",
                         ch);
                return;
        }

        if (!ch->in_room || !xIS_SET(ch->in_room->room_flags, ROOM_BANK))
        {
                send_to_char("You must be in a bank to do that!\n\r", ch);
                return;
        }

        clan = ch->pcdata->clan;

        amount = atoi(argument);

        if (!amount)
        {
                send_to_char("How much would you like to donate?\n\r", ch);
                return;
        }

        if (amount < 0)
        {
                ch_printf(ch, "Nice try...\n\r");
                return;
        }

        if (amount > ch->gold)
        {
                send_to_char("You don't have that much!\n\r", ch);
                return;
        }

        ch_printf(ch, "You donate %ld credits to %s's funds.\n\r", amount,
                  clan->name);

        clan->funds += amount;
        ch->gold -= amount;
        save_clan(clan);

}

CMDF do_newclan(CHAR_DATA * ch, char *argument)
{
        argument = NULL;
        send_to_char
                ("This command is being recycled to conserve thought.\n\r",
                 ch);
        return;
}

CMDF do_appoint(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_STRING_LENGTH];

        argument = one_argument(argument, arg);

        if (IS_NPC(ch) || !ch->pcdata)
                return;

        if (!ch->pcdata->clan)
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (str_cmp(ch->name, ch->pcdata->clan->leader))
        {
                send_to_char("Only your leader can do that!\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Useage: appoint <name> < first | second >\n\r",
                             ch);
                return;
        }

        if (!str_cmp(argument, "first"))
        {
                if (ch->pcdata->clan->number1
                    && str_cmp(ch->pcdata->clan->number1, ""))
                {
                        send_to_char
                                ("You already have someone in that position ... demote them first.\n\r",
                                 ch);
                        return;
                }

                STRFREE(ch->pcdata->clan->number1);
                ch->pcdata->clan->number1 = STRALLOC(arg);
        }
        else if (!str_cmp(argument, "second"))
        {
                if (ch->pcdata->clan->number2
                    && str_cmp(ch->pcdata->clan->number2, ""))
                {
                        send_to_char
                                ("You already have someone in that position ... demote them first.\n\r",
                                 ch);
                        return;
                }

                STRFREE(ch->pcdata->clan->number2);
                ch->pcdata->clan->number2 = STRALLOC(arg);
        }
        else
                do_appoint(ch, "");
        save_clan(ch->pcdata->clan);

}

CMDF do_demote(CHAR_DATA * ch, char *argument)
{

        if (IS_NPC(ch) || !ch->pcdata)
                return;

        if (!ch->pcdata->clan)
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (str_cmp(ch->name, ch->pcdata->clan->leader))
        {
                send_to_char("Only your leader can do that!\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Demote who?\n\r", ch);
                return;
        }

        if (!str_cmp(argument, ch->pcdata->clan->number1))
        {
                send_to_char("Player Demoted!", ch);

                STRFREE(ch->pcdata->clan->number1);
                ch->pcdata->clan->number1 = STRALLOC("");
        }
        else if (!str_cmp(argument, ch->pcdata->clan->number2))
        {
                send_to_char("Player Demoted!", ch);

                STRFREE(ch->pcdata->clan->number2);
                ch->pcdata->clan->number2 = STRALLOC("");
        }
        else
        {
                send_to_char("They seem to have been demoted already.\n\r",
                             ch);
                return;
        }
        save_clan(ch->pcdata->clan);

}

CMDF do_capture(CHAR_DATA * ch, char *argument)
{
        CLAN_DATA *clan;
        INSTALLATION_DATA *install;
        PLANET_DATA *planet;

/*	float support = 0.0;
	int pCount = 0; */
        char      buf[MAX_STRING_LENGTH];

        argument = NULL;

        if (!ch->in_room || !ch->in_room->area)
                return;

        if (IS_NPC(ch) || !ch->pcdata)
        {
                send_to_char("huh?\n\r", ch);
                return;
        }
        if (!ch->pcdata->clan)
        {
                send_to_char
                        ("You need to be a member of an organization to do that!\n\r",
                         ch);
                return;
        }

        if (ch->pcdata->clan->mainclan)
                clan = ch->pcdata->clan->mainclan;
        else
                clan = ch->pcdata->clan;

        if (clan->clan_type == CLAN_CRIME)
        {
                send_to_char
                        ("Crime fimilies aren't in the business of controlling worlds.\n\r",
                         ch);
                return;
        }

        if (clan->clan_type == CLAN_GUILD)
        {
                send_to_char
                        ("Your organization serves a much greater purpose.\n\r",
                         ch);
                return;
        }

        if ((planet = ch->in_room->area->planet) == NULL)
        {
                send_to_char("You must be on a planet to capture it.\n\r",
                             ch);
                return;
        }

        /*
         * Always do simple checks first, before the loops 
         */
        if (IS_SET(planet->flags, PLANET_NOCAPTURE))
        {
                send_to_char("This planet cannot be captured.\n\r", ch);
                return;
        }


        if (clan == planet->governed_by)
        {
                send_to_char
                        ("Your organization already controls this planet.\n\r",
                         ch);
                return;
        }

        if (planet->pop_support > 0)
        {
                send_to_char
                        ("The population is not in favour of changing leaders right now.\n\r",
                         ch);
                return;
        }

        if (planet->defbattalions > 0)
        {
                send_to_char
                        ("There are still defensive battalions left on this planet!.\n\r",
                         ch);
                return;
        }

/*	if ( planet->attbattalions < 1 )
	{
		send_to_char ( "There are no attack battalions left on this planet!.\n\r" , ch );
		return;
	}*/

        if (planet->starsystem)
        {
                SHIP_DATA *ship;
                CLAN_DATA *sClan;

                for (ship = planet->starsystem->first_ship; ship;
                     ship = ship->next_in_starsystem)
                {
                        sClan = get_clan(ship->owner);
                        if (!sClan)
                                continue;
                        if (sClan->mainclan)
                                sClan = sClan->mainclan;
                        if (sClan == planet->governed_by)
                        {
                                send_to_char
                                        ("A planet cannot be captured while protected by orbiting spacecraft.\n\r",
                                         ch);
                                return;
                        }
                }
        }


/*	if ( planet->attgovern != clan )
	{
		send_to_char ( "You clan does not have battalions on this planet!.\n\r" , ch );
		return;
	}*/

        planet->attgovern = NULL;
        planet->governed_by = clan;
        planet->pop_support = 50;
        for (install = planet->first_install; install;
             install = install->next_on_planet)
                install->clan = clan;

        snprintf(buf, MSL, "%s has been captured by %s!", planet->name,
                 clan->name);
        echo_to_all(AT_RED, buf, 0);

        save_planet(planet, FALSE);

        return;
}

CMDF do_empower(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        char      arg3[MAX_INPUT_LENGTH];
        CHAR_DATA *victim;
        CLAN_DATA *clan;
        char      buf[MAX_STRING_LENGTH];
        int       ranknum;

        if (IS_NPC(ch) || !ch->pcdata->clan)
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        clan = ch->pcdata->clan;
		if (!HAS_CLAN_PERM(ch, clan, "empower"))
        {
                send_to_char
                        ("You clan hasn't seen fit to bestow that ability to you!\n\r",
                         ch);
                return;
        }

        argument = one_argument(argument, arg);
        argument = one_argument(argument, arg2);
        argument = one_argument(argument, arg3);

        if (arg[0] == '\0')
        {
                send_to_char("Empower whom to do what?\n\r", ch);
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

        if (victim == ch && str_cmp(arg2, "Rank"))
        {
                send_to_char("Nice try.\n\r", ch);
                return;
        }

        if (victim->pcdata->clan != ch->pcdata->clan)
        {
                send_to_char("This player does not belong to your clan!\n\r",
                             ch);
                return;
        }

        if (!victim->pcdata->bestowments)
                victim->pcdata->bestowments = STRALLOC("");

        if (arg2[0] == '\0' || !str_cmp(arg2, "list"))
        {
                ch_printf(ch, "Current bestowed commands on %s: %s.\n\r",
                          victim->name, victim->pcdata->bestowments);
                return;
        }

        if (!str_cmp(arg2, "none"))
        {
                STRFREE(victim->pcdata->bestowments);
                victim->pcdata->bestowments = STRALLOC("");
                ch_printf(ch, "Bestowments removed from %s.\n\r",
                          victim->name);
                ch_printf(victim,
                          "%s has removed your bestowed clan abilities.\n\r",
                          ch->name);
                return;
        }
        else if (!str_cmp(arg2, "pilot"))
        {
                snprintf(buf, MSL, "%s %s", victim->pcdata->bestowments,
                         arg2);
                STRFREE(victim->pcdata->bestowments);
                victim->pcdata->bestowments = STRALLOC(buf);
                ch_printf(victim,
                          "%s has given you permission to fly clan ships.\n\r",
                          ch->name);
                send_to_char
                        ("Ok, they now have the ability to fly clan ships.\n\r",
                         ch);
        }
        else if (!str_cmp(arg2, "installations"))
        {
                snprintf(buf, MSL, "%s %s", victim->pcdata->bestowments,
                         arg2);
                STRFREE(victim->pcdata->bestowments);
                victim->pcdata->bestowments = STRALLOC(buf);
                ch_printf(victim,
                          "%s has given you permission to construct clan installations.\n\r",
                          ch->name);
                send_to_char
                        ("Ok, they now have the ability to construct clan installations.\n\r",
                         ch);
        }
        else if (!str_cmp(arg2, "battalions"))
        {
                snprintf(buf, MSL, "%s %s", victim->pcdata->bestowments,
                         arg2);
                STRFREE(victim->pcdata->bestowments);
                victim->pcdata->bestowments = STRALLOC(buf);
                ch_printf(victim,
                          "%s has given you permission to control clan battalions.\n\r",
                          ch->name);
                send_to_char
                        ("Ok, they now have the ability to control battalions.\n\r",
                         ch);
        }
        else if (!str_cmp(arg2, "withdraw"))
        {
                snprintf(buf, MSL, "%s %s", victim->pcdata->bestowments,
                         arg2);
                STRFREE(victim->pcdata->bestowments);
                victim->pcdata->bestowments = STRALLOC(buf);
                ch_printf(victim,
                          "%s has given you permission to withdraw clan funds.\n\r",
                          ch->name);
                send_to_char
                        ("Ok, they now have the ablitity to withdraw clan funds.\n\r",
                         ch);
        }
        else if (!str_cmp(arg2, "clanbuyship"))
        {
                snprintf(buf, MSL, "%s %s", victim->pcdata->bestowments,
                         arg2);
                STRFREE(victim->pcdata->bestowments);
                victim->pcdata->bestowments = STRALLOC(buf);
                ch_printf(victim,
                          "%s has given you permission to buy clan ships.\n\r",
                          ch->name);
                send_to_char
                        ("Ok, they now have the ablitity to use clanbuyship.\n\r",
                         ch);
        }
        else if (!str_cmp(arg2, "induct"))
        {
                snprintf(buf, MSL, "%s %s", victim->pcdata->bestowments,
                         arg2);
                STRFREE(victim->pcdata->bestowments);
                victim->pcdata->bestowments = STRALLOC(buf);
                ch_printf(victim,
                          "%s has given you permission to induct new members.\n\r",
                          ch->name);
                send_to_char
                        ("Ok, they now have the ablitity to induct new members.\n\r",
                         ch);
        }
        else if (!str_cmp(arg2, "empower"))
        {
                snprintf(buf, MSL, "%s %s", victim->pcdata->bestowments,
                         arg2);
                STRFREE(victim->pcdata->bestowments);
                victim->pcdata->bestowments = STRALLOC(buf);
                act(AT_PLAIN,
                    "$n has given you permission to empower members.", ch,
                    NULL, victim, TO_VICT);
                act(AT_PLAIN,
                    "Okay, $n now has the ability to empower members.", ch,
                    NULL, victim, TO_CHAR);
        }
        else if (!str_cmp(arg2, "clansellship"))
        {
                snprintf(buf, MSL, "%s %s", victim->pcdata->bestowments,
                         arg2);
                STRFREE(victim->pcdata->bestowments);
                victim->pcdata->bestowments = STRALLOC(buf);
                act(AT_PLAIN,
                    "$n has given you permission to sell clan ships.", ch,
                    NULL, victim, TO_VICT);
                act(AT_PLAIN,
                    "Okay, $n now has the ability to sell clan ships.", ch,
                    NULL, victim, TO_CHAR);
        }
        else if (!str_cmp(arg2, "rank"))
        {
				int newrank;
                ranknum = victim->pcdata->clanrank;
                if (!is_number(arg3))
                {
                        send_to_char("Rank must be set as a number.\n\r", ch);
                        return;
                }
				newrank = atoi(arg3);

				if (newrank >= MAX_RANK) {
						send_to_char("There are not that many ranks available.\n\r", ch);
						return;
				}

                if (newrank < 0)
                {
                        send_to_char("Ranks cannot be negative\n\r", ch);
                        return;
                }

                if (newrank == (MAX_RANK - 1) && str_cmp(victim->name, clan->leader))
                {
                        ch_printf(ch, "Only the leaders rank %d.\n\r", (MAX_RANK - 1));
                        return;
                }
                if (newrank == (MAX_RANK - 2) && str_cmp(victim->name, clan->number1))
                {
                        ch_printf(ch, "Only the second in commands get rank %d.\n\r", MAX_RANK - 2);
                        return;
                }
                if (newrank == (MAX_RANK - 3) && str_cmp(victim->name, clan->number2))
                {
                        ch_printf(ch, "Only the third in commands get rank %d.\n\r", MAX_RANK - 3);
                        return;
                }

                if (ranknum < newrank)
                {
                        victim->pcdata->clanrank = newrank;
                        ch_printf(victim, "%s has promoted you.\n\r",ch->name);
                }
                if (ranknum == newrank)
                {
                        send_to_char("They are already at that rank.\n\r",ch);
                        return;
                }
                if (ranknum > newrank)
                {
                        victim->pcdata->clanrank = newrank;
                        ch_printf(victim, "%s has demoted you.\n\r", ch->name);
                }
                send_to_char("New rank set.\n\r", ch);
        }
        else
        {
                send_to_char
                        ("Currently you may empower members with only the following:\n\r",
                         ch);
                send_to_char("\n\rpilot:       ability to fly clan ships\n\r",
                             ch);
                send_to_char
                        ("withdraw:    ability to withdraw clan funds\n\r",
                         ch);
                send_to_char("clanbuyship: ability to buy clan ships\n\r",
                             ch);
                send_to_char("induct:      ability to induct new members\n\r",
                             ch);
                send_to_char
                        ("battalions:  ability to control clan battalions\n\r",
                         ch);
                send_to_char("none:        removes bestowed abilities\n\r",
                             ch);
        }

        save_char_obj(victim);  /* clan gets saved when pfile is saved */
        return;


}

long get_taxes(PLANET_DATA * planet)
{
        long      gain;

        gain = planet->base_value;
        gain += planet->base_value * planet->pop_support;
        gain += UMAX(0, planet->pop_support / 10 * planet->population);
        if (gain < 0)
                return 0;
        else
                return gain;
}

CMDF do_imports(CHAR_DATA * ch, char *argument)
{
        PLANET_DATA *planet;
        int       i;

        if (argument[0] == '\0')
        {
                send_to_char("Usage: imports <planet>\r\n", ch);
                return;
        }

        planet = get_planet(argument);

        if (!planet)
        {
                send_to_char("&RNo such planet\r\n", ch);
                return;
        }
        ch_printf(ch, "&BImport and Export data for %s:\r\n", planet->name);
        ch_printf(ch,
                  "&GResource       &CImport     &YExport    &PProduces    &RConsumes         &GAmount\r\n");
        ch_printf(ch,
                  "&G-------------    -------      ------     --------     --------          ------\r\n");
        for (i = 1; i < CARGO_MAX; i++)
                ch_printf(ch,
                          "&G%-14.14s    &C%5d/ton  &Y%5d/ton &P%6d tons  &R%6d tons  &G%9d\r\n",
                          cargo_names[i], planet->cargoimport[i],
                          planet->cargoexport[i], planet->produces[i],
                          planet->consumes[i], planet->resource[i]);
        if (IS_IMMORTAL(ch) || ch->pcdata->learned[gsn_contraband] > 0)
                for (i = CONTRABAND_NONE + 1; i < CONTRABAND_MAX; i++)
                        ch_printf(ch,
                                  "&G%-14.14s    &C%5d/ton  &Y%5d/ton &P%6d tons  &R%6d tons  &G%9d\r\n",
                                  cargo_names[i], planet->cargoimport[i],
                                  planet->cargoexport[i], planet->produces[i],
                                  planet->consumes[i], planet->resource[i]);
        return;
}


CMDF do_recruit(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        CLAN_DATA *clan;
        PLANET_DATA *planet;
        SHIP_DATA *ship;
        int       cost, batamount, percent_chance, xp;

        clan = ch->pcdata->clan;
        if (!clan)
        {
                send_to_char("You are not in a clan.\n\r", ch);
                return;
        }

        if (!HAS_CLAN_PERM(ch, clan, "battalions"))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);

        if (arg1[0] == '\0')
        {
                send_to_char("Usage: recruit <number>\n\r", ch);
                return;
        }

        planet = ch->in_room->area->planet;
        ship = ship_from_hanger(ch->in_room->vnum);

        batamount = atoi(arg1);

        if (ship)
        {

                if ((ship->maxbattalions - ship->battalions) < batamount)
                {
                        send_to_char
                                ("You cannot fit that many battalions on your ship.\n\r",
                                 ch);
                        return;
                }

                if (ship->ship_class != CAPITAL_SHIP)
                {
                        send_to_char
                                ("You can only recruit on capital ships.\n\r",
                                 ch);
                        return;
                }

        }

        if (ship)
                cost = batamount * 175000;
        else
                cost = batamount * 50000;

        if (clan->funds < cost)
        {
                send_to_char("You cannot afford that many battalions.\n\r",
                             ch);
                return;
        }

        if (!planet && !ship)
        {
                send_to_char
                        ("You must be on a planet or in a starship hanger to recruit.\n\r",
                         ch);
                return;
        }


        if (planet)
        {
                if (clan != planet->governed_by)
                {
                        send_to_char
                                ("Your organization does not controls this planet.\n\r",
                                 ch);
                        return;
                }
        }

        percent_chance = IS_NPC(ch) ? ch->top_level
                : (int) (ch->pcdata->learned[gsn_recruit]);

        if (number_percent() > percent_chance)
        {
                send_to_char
                        ("&RYou try to recruit some battalions, but you fail.\n\r",
                         ch);
                learn_from_failure(ch, gsn_recruit);
                return;
        }

        clan->funds -= cost;
        if (planet)
                planet->defbattalions += batamount;

        else if (ship)
                ship->battalions += batamount;

        else
        {
                send_to_char("Severe error!", ch);
                return;
        }
        if (planet)
                send_to_char("You recruit your planetary battalions!\n\r",
                             ch);
        if (ship)
                send_to_char("You recruit your battalions!", ch);
        xp = UMIN(cost,
                  exp_level(ch->skill_level[LEADERSHIP_ABILITY] + 1) * 100);
        gain_exp(ch, xp, LEADERSHIP_ABILITY);
        ch_printf(ch, "&WYou gain %ld leadership experience!\n\r", xp);

        if (planet)
                save_planet(planet, FALSE);
        else if (ship)
                save_ship(ship);
        WAIT_STATE(ch, skill_table[gsn_recruit]->beats);
        learn_from_success(ch, gsn_recruit);
        return;

}

CMDF do_load_battalions(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        CLAN_DATA *clan;
        PLANET_DATA *planet;
        int       amount;
        SHIP_DATA *ship;
        SHIP_DATA *target;


        if (argument[0] == '\0')
        {
                send_to_char
                        ("&RSyntax: &Gload_battalions &C<&cship&C> &C<&camount&C>&w\n\r",
                         ch);
                return;
        }
        clan = ch->pcdata->clan;
        if (!clan)
        {
                send_to_char("You are not in a clan.\n\r", ch);
                return;
        }

        if (!HAS_CLAN_PERM(ch, clan, "battalions"))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }



        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0')
        {
                do_load_battalions(ch, "");
                return;
        }

        ship = ship_in_room(ch->in_room, arg1);
        target = ship_from_hanger(ch->in_room->vnum);
        planet = ch->in_room->area->planet;

        if (!ship)
        {
                act(AT_PLAIN, "I see no $T here.", ch, ship, argument,
                    TO_CHAR);
                return;
        }


        amount = atoi(arg2);

        if ((ship->maxbattalions == 0)
            || (amount > (ship->maxbattalions - ship->battalions)))
        {
                send_to_char("There is no more room on that ship.\n\r", ch);
                return;
        }

        if (!planet && !target)
        {
                send_to_char
                        ("You must be on a planet or a capital ship to load troops to a ship.\n\r",
                         ch);
                return;
        }

        if (!target)
                if (clan != planet->governed_by)
                {
                        send_to_char
                                ("Your organization does not controls this planet.\n\r",
                                 ch);
                        return;
                }

        if (!planet)
                if (!check_pilot(ch, target))
                {
                        send_to_char("You are not a pilot on that ship.\n\r",
                                     ch);
                        return;
                }

        if (planet)
                if (amount > planet->defbattalions)
                {
                        send_to_char("You do not have that many to load.\n\r",
                                     ch);
                        return;
                }
        if (target)
                if (amount > target->battalions)
                {
                        send_to_char("You do not have that many to load.\n\r",
                                     ch);
                        return;
                }

        if (planet)
                planet->defbattalions -= amount;
        if (target)
                target->battalions -= amount;
        ship->battalions += amount;
        send_to_char("You load your planetary battalions onto the ship.", ch);

        if (planet)
                save_planet(planet, FALSE);
        if (target)
                save_ship(target);



        return;
}

CMDF do_deploy_battalions(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        CLAN_DATA *clan;
        PLANET_DATA *planet;
        int       amount;
        SHIP_DATA *ship;
        char      buf[MAX_STRING_LENGTH];

        if (argument[0] == '\0')
        {
                send_to_char
                        ("&RSyntax: &Gdeploy_battalions &C<&cship&C> &C<&camount&C>&w\n\r",
                         ch);
                return;
        }

        clan = ch->pcdata->clan;
        if (!clan)
        {
                send_to_char("You are not in a clan.\n\r", ch);
                return;
        }


        if (!HAS_CLAN_PERM(ch, clan, "battalions"))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }


        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);


        if (arg1[0] == '\0' || arg2[0] == '\0')
        {
                do_deploy_battalions(ch, "");
                return;
        }

        ship = ship_in_room(ch->in_room, arg1);

        if (!ship)
        {
                act(AT_PLAIN, "I see no $T here.", ch, NULL, arg2, TO_CHAR);
                return;
        }

        amount = atoi(arg2);

        if ((ship->maxbattalions == 0) || (amount > ship->battalions))
        {
                send_to_char
                        ("There is not that many battalions to deploy from that ship.\n\r",
                         ch);
                return;
        }

        if ((planet = ch->in_room->area->planet) == NULL)
        {
                send_to_char
                        ("You must be on a planet to deply troops to a ship.\n\r",
                         ch);
                return;
        }

        if (planet->attgovern && (planet->attgovern != clan))
        {
                send_to_char
                        ("You cannot attack a planet already engaged in planetary warfare!.\n\r",
                         ch);
                return;
        }

        if (clan == planet->governed_by)
        {
                planet->defbattalions += amount;
                ship->battalions = ship->battalions - amount;
                send_to_char
                        ("You deploy your ships battalions onto the planet.",
                         ch);
        }

        if ((clan != planet->governed_by))
        {
                planet->attbattalions += amount;
                ship->battalions = ship->battalions - amount;
                if (planet->attgovern != clan)
                {
                        planet->attgovern = clan;
                        snprintf(buf, MSL, "%s is being attacked by %s!",
                                 planet->name, clan->name);
                        echo_to_all(AT_RED, buf, 0);
                }
                send_to_char
                        ("You deploy your ships battalions onto the planet.",
                         ch);
        }

        save_planet(planet, FALSE);
        save_ship(ship);

        return;
}

CMDF do_clanstat(CHAR_DATA * ch, char *argument)
{
        CLAN_DATA *clan;
        CLAN_DATA *wclan;
        PLANET_DATA *planet;
        int       count = 0;
        int       pCount = 0;
        int       support;
        long      revenue;
        SHIP_DATA *ship;
        CHAR_DATA *wch;

        argument = NULL;

        clan = ch->pcdata->clan;
        if (!clan)
        {
                send_to_char("You are not in a clan.\n\r", ch);
                return;
        }

        pCount = 0;
        support = 0;
        revenue = 0;

        for (planet = first_planet; planet; planet = planet->next)
                if (clan == planet->governed_by)
                {
                        support += planet->pop_support;
                        pCount++;
                        revenue += get_taxes(planet);
                }




        if (pCount > 1)
                support /= pCount;
        ch_printf(ch, "&B%s\n\r", clan->name);
        ch_printf(ch,
                  "&c-----------------------------------------------------------------------------&R&W\n\r");
        ch_printf(ch, "&BClan Statistics\n\r");
        ch_printf(ch,
                  "&c-----------------------------------------------------------------------------&R&W\n\r");
        ch_printf(ch, "&BEnlisting: &c%s\n\r",
                  clan->enliston == 1 ? "Yes" : clan->enliston ==
                  0 ? "No" : "Unknown");
        ch_printf(ch, "&BPlanets: &c%d\n\r", pCount);
        ch_printf(ch, "&BSupport: &c%d\n\r", support);
        ch_printf(ch, "&BRevenue: &c%d\n\r", revenue);
        ch_printf(ch, "&BFunds: &c%ld\n\r", clan->funds);
        ch_printf(ch, "&BLeader: &c%-15s\n\r", clan->leader);
        ch_printf(ch, "&BFirst:  &c%-15s\n\r", clan->number1);
        ch_printf(ch, "&BSecond: &c%-15s\n\r", clan->number2);
        ch_printf(ch,
                  "&c-----------------------------------------------------------------------------&R&W\n\r");
        ch_printf(ch, "&BShip List\n\r");
        ch_printf(ch,
                  "&c-----------------------------------------------------------------------------&R&W\n\r");
        count = 0;
        for (ship = first_ship; ship; ship = ship->next)
        {
                if (ship->type == MOB_SHIP)
                        continue;
                if (str_cmp(ship->owner, ch->pcdata->clan->name)
                    || ship->ship_class > SHIP_PLATFORM)
                        continue;


                if (ship->in_room)
                        ch_printf(ch, "&c%s (%s)\n\r", ship->name,
                                  ship->in_room->name);
                else
                        ch_printf(ch, "&c%s\n\r", ship->name);

                count++;
        }

        if (!count)
        {
                send_to_char
                        ("&BThere are no ships owned by your organization.\n\r",
                         ch);
        }
        ch_printf(ch,
                  "&c-----------------------------------------------------------------------------&R&W\n\r");
        ch_printf(ch, "&BRank Titles\n\r");
        ch_printf(ch,
                  "&c-----------------------------------------------------------------------------&R&W\n\r");
        for (count = 0; count < MAX_RANK; count++)
        {
                ch_printf(ch, "&BRank %-2d: &c%-18s ", count,clan->rank[count]);
                if ((count+1) % 3 == 0 && count != 0)
                        send_to_char("\n\r", ch);
        }
        ch_printf(ch,
                  "\n\r&c-----------------------------------------------------------------------------&R&W\n\r");
        ch_printf(ch, "&BRank Salaries\n\r");
        ch_printf(ch,
                  "&c-----------------------------------------------------------------------------&R&W\n\r");
        for (count = 0; count < MAX_RANK; count++)
        {
                ch_printf(ch, "&BRank %-2d: &c%-18d ", count, clan->salary[count]);
                if ((count+1) % 3 == 0 && count != 0)
                        send_to_char("\n\r", ch);
        }
        ch_printf(ch,
                  "\n\r&c-----------------------------------------------------------------------------&R&W\n\r");
        ch_printf(ch, "&BAvailable Members\n\r");
        ch_printf(ch,
                  "&c-----------------------------------------------------------------------------&R&W\n\r");
        for (wch = first_char; wch; wch = wch->next)
        {
                if (wch->pcdata && wch->pcdata->clan)
                {
                        wclan = wch->pcdata->clan;
                        if (clan == wclan)
                        {
                                ch_printf(ch, "&B%s", wch->name);
                                ch_printf(ch, " &C(&c%s&C) \n\r",
                                          clan->rank[wch->pcdata->clanrank]);
                        }
                }
        }
        ch_printf(ch,
                  "&c-----------------------------------------------------------------------------&R&W\n\r");
        if (!str_cmp(ch->name, clan->leader)
            || !str_cmp(ch->name, clan->number1)
            || !str_cmp(ch->name, clan->number2))
        {
                ch_printf(ch, "&BComplete Clan Roster\n\r");
                ch_printf(ch,
                          "&c-----------------------------------------------------------------------------&R&W\n\r");
                ch_printf(ch, "&c%s\n\r", wordwrap(clan->roster, 73));
        }
        set_pager_color(AT_WHITE, ch);
}

CMDF do_overthrow(CHAR_DATA * ch, char *argument)
{
        argument = NULL;    /* Not Needed - Gavin */
        CLAN_DATA *clan;

        if (IS_NPC(ch))
                return;

        if (!ch->pcdata || !ch->pcdata->clan)
        {
                send_to_char
                        ("You have to be part of an organization before you can claim leadership.\n\r",
                         ch);
                return;
        }

        clan = ch->pcdata->clan;
        if (!clan)
        {
                send_to_char("No such clan.\n\r", ch);
                return;
        }

        if (clan->leader[0] != '\0')
        {
                send_to_char
                        ("Your organization already has strong leadership...\n\r",
                         ch);
                return;
        }

        ch_printf(ch, "OK. You are now a leader of %s.\n\r",
                  ch->pcdata->clan->name);

        STRFREE(ch->pcdata->clan->leader);
        if (!strcmp(ch->name, ch->pcdata->clan->number1))
        {
                ch->pcdata->clan->leader =
                        STRALLOC(ch->pcdata->clan->number1);
                STRFREE(ch->pcdata->clan->number1);
                ch->pcdata->clan->number1 = STRALLOC("");
        }
        else if (!strcmp(ch->name, ch->pcdata->clan->number2))
        {
                ch->pcdata->clan->leader =
                        STRALLOC(ch->pcdata->clan->number2);
                STRFREE(ch->pcdata->clan->number2);
                ch->pcdata->clan->number2 = STRALLOC("");
        }
        else
                ch->pcdata->clan->leader = STRALLOC(ch->name);

        ch->pcdata->clanrank = MAX_RANK;
        save_char_obj(ch);  /* clan gets saved when pfile is saved */
}

bool is_clan_enemy(CHAR_DATA * ch, CHAR_DATA * victim)
{
        CLAN_DATA *ch_clan = NULL;
        CLAN_DATA *vict_clan = NULL;

        if (IS_NPC(victim))
                vict_clan = get_clan(victim->mob_clan);
        else
                vict_clan = victim->pcdata->clan;

        if (IS_NPC(ch))
                ch_clan = get_clan(ch->mob_clan);
        else
                ch_clan = ch->pcdata->clan;

        if (!ch_clan || !vict_clan)
                return FALSE;

        if (ch_clan->mainclan)
                ch_clan = ch_clan->mainclan;

        if (vict_clan->mainclan)
                vict_clan = vict_clan->mainclan;

        if (ch_clan == vict_clan)
                return FALSE;

        if (vict_clan == ch_clan->ally)
                return TRUE;

        return FALSE;
}

bool is_clan_ally(CHAR_DATA * ch, CHAR_DATA * victim)
{
        CLAN_DATA *ch_clan = NULL;
        CLAN_DATA *vict_clan = NULL;

        if (IS_NPC(victim))
                vict_clan = get_clan(victim->mob_clan);
        else
                vict_clan = victim->pcdata->clan;

        if (IS_NPC(ch))
                ch_clan = get_clan(ch->mob_clan);
        else
                ch_clan = ch->pcdata->clan;

        if (!ch_clan || !vict_clan)
                return FALSE;

        if (ch_clan->mainclan)
                ch_clan = ch_clan->mainclan;

        if (vict_clan->mainclan)
                vict_clan = vict_clan->mainclan;

        if (ch_clan == vict_clan)
                return TRUE;

        if (vict_clan == ch_clan->ally)
                return TRUE;

        return FALSE;
}

bool is_same_clan(CHAR_DATA * ch, CHAR_DATA * victim)
{
        CLAN_DATA *ch_clan = NULL;
        CLAN_DATA *vict_clan = NULL;

        if (IS_NPC(victim))
                vict_clan = get_clan(victim->mob_clan);
        else
                vict_clan = victim->pcdata->clan;

        if (IS_NPC(ch))
                ch_clan = get_clan(ch->mob_clan);
        else
                ch_clan = ch->pcdata->clan;

        if (!ch_clan || !vict_clan)
                return FALSE;

        if (ch_clan->mainclan)
                ch_clan = ch_clan->mainclan;

        if (vict_clan->mainclan)
                vict_clan = vict_clan->mainclan;

        if (ch_clan == vict_clan)
                return TRUE;
        return FALSE;
}

void free_clan(CLAN_DATA * clan)
{
        int       x;
        PLANET_DATA *planet;
        CLAN_DATA *tclan;
        SHIP_DATA *ship;
        CHAR_DATA *ch;
        BOUNTY_DATA *bounty, *next_bounty = NULL;
        DOCK_DATA *dock;

        for (planet = first_planet; planet; planet = planet->next)
        {
                if (planet->governed_by == clan)
                        planet->governed_by = NULL;
                if (planet->attgovern == clan)
                        planet->attgovern = NULL;
        }

        if (clan->mainclan)
                UNLINK(clan, clan->mainclan->first_subclan,
                       clan->mainclan->last_subclan, next_subclan,
                       prev_subclan);

        for (tclan = clan->first_subclan; tclan; tclan = tclan->next)
                UNLINK(tclan, clan->first_subclan, clan->last_subclan,
                       next_subclan, prev_subclan);

        for (tclan = first_clan; tclan; tclan = tclan->next)
        {
                if (tclan->enemy == clan)
                        tclan->enemy = NULL;
                if (tclan->ally == clan)
                        tclan->ally = NULL;
        }

        for (ship = first_ship; ship; ship = ship->next)
                if (ship->clan == clan)
                        ship->clan = NULL;
        for (ch = first_char; ch; ch = ch->next)
        {
                if (!IS_NPC(ch) && ch->pcdata && ch->pcdata->clan)
                        ch->pcdata->clan = NULL;
                else if (IS_NPC(ch) && ch->mob_clan
                         && ch->mob_clan[0] != '\0')
                {
                        STRFREE(ch->mob_clan);
                }
        }

        for (bounty = first_disintigration; bounty; bounty = next_bounty)
        {
                next_bounty = bounty->next;
                if (bounty->source && !str_cmp(bounty->source, clan->name))
                {
                        UNLINK(bounty, first_disintigration,
                               last_disintigration, next, prev);
                        free_bounty(bounty);
                }
        }

        for (dock = first_dock; dock; dock = dock->next)
                if (dock->clan == clan)
                        dock->clan = NULL;

        if (clan->ally_name)
                STRFREE(clan->ally_name);
        if (clan->description)
                STRFREE(clan->description);
        if (clan->enemy_name)
                STRFREE(clan->enemy_name);
        if (clan->filename)
                DISPOSE(clan->filename);
        if (clan->leader)
                STRFREE(clan->leader);
        if (clan->number1)
                STRFREE(clan->number1);
        if (clan->number2)
                STRFREE(clan->number2);
        if (clan->motto)
                STRFREE(clan->motto);
        if (clan->tmpstr)
                STRFREE(clan->tmpstr);
        if (clan->name)
                STRFREE(clan->name);
        if (clan->ally_name)
                STRFREE(clan->ally_name);
        if (clan->roster)
                STRFREE(clan->roster);
        for (x = 0; x < MAX_RANK; x++)
                if (clan->rank[x])
                        STRFREE(clan->rank[x]);
        DISPOSE(clan);
}

void free_planet(PLANET_DATA * planet)
{
        if (planet->bodyname)
                DISPOSE(planet->bodyname);
        if (planet->filename)
                DISPOSE(planet->filename);
        if (planet->name)
                STRFREE(planet->name);
        if (planet->bodyname)
                DISPOSE(planet->bodyname);
        DISPOSE(planet);
}

CMDF do_stepdown(CHAR_DATA * ch, char *argument)
{
        argument = NULL;    /* Not Needed - Gavin */
        CLAN_DATA *clan;

        if (IS_NPC(ch))
                return;

        if (!ch->pcdata || !ch->pcdata->clan)
        {
                send_to_char
                        ("You have to be part of an organization before you can claim leadership.\n\r",
                         ch);
                return;
        }

        clan = ch->pcdata->clan;
        if (!clan)
        {
                send_to_char("No such clan.\n\r", ch);
                return;
        }

        if (!str_cmp(clan->leader, ch->name))
		{
				STRFREE(clan->leader);
                clan->leader = STRALLOC(clan->number1);
				STRFREE(clan->number1);
                clan->number1 = STRALLOC(clan->number2);
                STRFREE(clan->number2);
                clan->number2 = STRALLOC("");
		}
		else if (!str_cmp(clan->number1, ch->name))
		{
				STRFREE(clan->number1);
                clan->number1 = STRALLOC(clan->number2);
                STRFREE(clan->number2);
                clan->number2 = STRALLOC("");
		}
		else if (!str_cmp(clan->number2, ch->name))
		{
				STRFREE(clan->number2);
                clan->number2 = STRALLOC("");
		}
		else 
        {
                send_to_char("Your organization already has strong leadership...\n\r",ch);
                return;
        }

        ch_printf(ch, "OK. You are no longer in the leadership positions of %s.\n\r",clan->name);


        ch->pcdata->clanrank = 0;
        save_char_obj(ch);  /* clan gets saved when pfile is saved */
}
