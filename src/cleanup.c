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
 *                      Color Module -- Allow user customizable Colors.                  *
 *                                         --Matthew                                     *
 *                             Enhanced ANSI parser by Samson                            *
 ****************************************************************************************/
#include <dlfcn.h>
#include <list>
#include "mud.h"
#include "homes.h"
#include "restore.h"
#include "boards.h"
#include "bounty.h"
#include "ban.h"
#include "olc-shuttle.h"
#include "channels.h"
#include "body.h"
#include "installations.h"
#include "space2.h"
#include "races.h"
#include "utils.h"
#include "body.h"

void free_desc args((DESCRIPTOR_DATA * d));
void free_object args((OBJ_DATA * obj));
void free_clan args((CLAN_DATA * clan));
void free_planet args((PLANET_DATA * planet));
void free_ship args((SHIP_DATA * ship));
void free_protoship args((PROTOSHIP_DATA * ship));
void free_system args((SPACE_DATA * system));
void free_skill args((SKILLTYPE * skill));
void free_home args((HOME_DATA * home));
void free_dns args((DNS_DATA * cache));
void free_board args((BOARD_DATA * board));
void free_ban args((BAN_DATA * ban));
void free_shuttle args((SHUTTLE_DATA * shuttle));
void free_illness args((ILLNESS_DATA * illness));
void free_channel args((CHANNEL_DATA * channel));
void free_restore args((RESTORE_DATA * restore));
void free_changes args((void));
void free_bounty args((BOUNTY_DATA * bounty));
void free_baccount(BANK_ACCOUNT * account);

extern RACE_LIST races;
extern BODY_LIST bodies;

void free_helps(void)
{
        HELP_DATA *help, *next_help;

        for (help = first_help; help; help = next_help)
        {
                next_help = help->next;
                UNLINK(help, first_help, last_help, next, prev);
                delete_help(help);
        }
}

void free_shops(void)
{
        SHOP_DATA *shop, *next_shop;
        REPAIR_DATA *repair, *next_repair;

        for (shop = first_shop; shop; shop = next_shop)
        {
                next_shop = shop->next;
                UNLINK(shop, first_shop, last_shop, next, prev);
                DISPOSE(shop);
        }

        for (repair = first_repair; repair; repair = next_repair)
        {
                next_repair = repair->next;
                UNLINK(repair, first_repair, last_repair, next, prev);
                DISPOSE(repair);
        }
}

void free_characters(void)
{
        CHAR_DATA *ch;

        if (supermob)
        {
                char_from_room(supermob);
                UNLINK(supermob, first_char, last_char, next, prev);
                free_char(supermob);
        }

        clean_char_queue();
        while ((ch = last_char) != NULL)
                extract_char(ch, TRUE);
        clean_char_queue();
}

void free_descriptors(void)
{
        DESCRIPTOR_DATA *d, *next_d;

        for (d = first_descriptor; d; d = next_d)
        {
                next_d = d->next;
                UNLINK(d, first_descriptor, last_descriptor, next, prev);
                free_desc(d);
        }
}

void free_objects(void)
{
        OBJ_DATA *object;

        clean_obj_queue();
        while ((object = last_object) != NULL)
                extract_obj(object);
        clean_obj_queue();

        DISPOSE(auction);
}

void free_clans(void)
{
        CLAN_DATA *clan, *next_clan;

        for (clan = first_clan; clan; clan = next_clan)
        {
                next_clan = clan->next;
                UNLINK(clan, first_clan, last_clan, next, prev);
                free_clan(clan);
        }
}

void free_planets(void)
{
        PLANET_DATA *planet, *next_planet;

        for (planet = first_planet; planet; planet = next_planet)
        {
                next_planet = planet->next;
                UNLINK(planet, first_planet, last_planet, next, prev);
                free_planet(planet);
        }
}

void free_ships(void)
{
        SHIP_DATA *ship, *next_ship;
        PROTOSHIP_DATA *protoship, *next_protoship;

        for (ship = first_ship; ship; ship = next_ship)
        {
                next_ship = ship->next;
                UNLINK(ship, first_ship, last_ship, next, prev);
                free_ship(ship);
                DISPOSE(ship);
        }

        for (protoship = first_protoship; protoship;
             protoship = next_protoship)
        {
                next_protoship = protoship->next;
                UNLINK(protoship, first_protoship, last_protoship, next,
                       prev);
                free_protoship(protoship);
        }
}

void free_starsystems(void)
{
        SPACE_DATA *system, *next_system;

        for (system = first_starsystem; system; system = next_system)
        {
                next_system = system->next;
                UNLINK(system, first_starsystem, last_starsystem, next, prev);
                free_system(system);
        }
}

void free_areas(void)
{
        AREA_DATA *area, *next_area;

        for (area = first_area; area; area = next_area)
        {
                next_area = area->next;
				UNLINK(area, first_area, last_area, next, prev);
                free_area(area);
        }

        for (area = first_build; area; area = next_area)
        {
                next_area = area->next;
				UNLINK(area, first_build, last_build, next, prev);
                free_area(area);
        }
}

void free_specfuns(void)
{
        SPEC_LIST *spec, *next_spec;

        for (spec = first_specfun; spec; spec = next_spec)
        {
                next_spec = spec->next;
                UNLINK(spec, first_specfun, last_specfun, next, prev);
                if (spec->name)
                        DISPOSE(spec->name);
                DISPOSE(spec);
        }

}

void free_sysdata(void)
{

        if (sysdata.mud_name)
                STRFREE(sysdata.mud_name);
        if (sysdata.mud_url)
                STRFREE(sysdata.mud_url);
        if (sysdata.mail_path)
                STRFREE(sysdata.mail_path);
        if (sysdata.mud_email)
                STRFREE(sysdata.mud_email);
        if (sysdata.time_of_max)
                DISPOSE(sysdata.time_of_max);
}

void free_commands(void)
{
        CMDTYPE  *command, *cmd_next;
        int       hash;

        for (hash = 0; hash < 126; hash++)
        {
                for (command = command_hash[hash]; command;
                     command = cmd_next)
                {
                        cmd_next = command->next;
                        command->next = NULL;
                        command->do_fun = NULL;
                        free_command(command);
                }
        }
        return;
}

void free_skills(void)
{
        int       x;

        for (x = 0; x < MAX_SKILL; x++)
                free_skill(skill_table[x]);
}

void free_leftovers(void)
{
        int       hash;

        for (hash = 0; hash < 1024; hash++)
                hash_dump(hash);
        dlclose(sysdata.dlHandle);
        if (fpReserve)
                FCLOSE(fpReserve);
        if (fpLOG)
                FCLOSE(fpLOG);
}

void free_dnses(void)
{
        DNS_DATA *dns, *dns_next;

        for (dns = first_cache; dns; dns = dns_next)
        {
                dns_next = dns->next;
                UNLINK(dns, first_cache, last_cache, next, prev);
                free_dns(dns);
        }
}

void free_socials(void)
{
        SOCIALTYPE *social, *social_next;
        int       hash;

        for (hash = 0; hash < 27; hash++)
        {
                for (social = social_index[hash]; social;
                     social = social_next)
                {
                        social_next = social->next;
                        free_social(social);
                }
        }
}

void free_boards(void)
{
        BOARD_DATA *board, *next_board;

        for (board = first_board; board; board = next_board)
        {
                next_board = board->next;
                free_board(board);
                DISPOSE(board);
        }
}

void free_races(void)
{
        std::list < RACE_DATA * >::iterator iter;
        RACE_DATA * race;

        for (iter = races.begin(); iter != races.end(); iter++)
        {
                race = (*iter);
                DELETE(race);

				races.erase(iter);
                iter = races.begin();
        }
}

void free_languages(void)
{
        LANGUAGE_DATA *language, *next_language;

        for (language = first_language; language; language = next_language)
        {
                next_language = language->next;
                UNLINK(language, first_language, last_language, next, prev);
                if (language->name)
                        STRFREE(language->name);
                DISPOSE(language);
        }
}

void free_bounties(void)
{
        BOUNTY_DATA *bounty, *next_bounty;

        for (bounty = first_disintigration; bounty; bounty = next_bounty)
        {
                next_bounty = bounty->next;
                UNLINK(bounty, first_bounty, last_bounty, next, prev);
                free_bounty(bounty);
        }
}

void free_bans(void)
{
        RESERVE_DATA *reserve, *next_reserve;
        BAN_DATA *ban, *next_ban;

        for (reserve = first_reserved; reserve; reserve = next_reserve)
        {
                next_reserve = reserve->next;
                UNLINK(reserve, first_reserved, last_reserved, next, prev);
                if (reserve->name)
                        DISPOSE(reserve->name);
                DISPOSE(reserve);
        }

        for (ban = first_ban_class; ban; ban = next_ban)
        {
                next_ban = ban->next;
                UNLINK(ban, first_ban, last_ban, next, prev);
                free_ban(ban);
        }
        for (ban = first_ban_race; ban; ban = next_ban)
        {
                next_ban = ban->next;
                UNLINK(ban, first_ban, last_ban, next, prev);
                free_ban(ban);
        }
        for (ban = first_ban; ban; ban = next_ban)
        {
                next_ban = ban->next;
                UNLINK(ban, first_ban, last_ban, next, prev);
                free_ban(ban);
        }
}

/* free_changes() */
void free_installations(void)
{
        INSTALLATION_DATA *install, *next_install;

        for (install = first_installation; install; install = next_install)
        {
                next_install = install->next;
                UNLINK(install, first_installation, last_installation, next,
                       prev);
                if (install->filename)
                        STRFREE(install->filename);
                DISPOSE(install);
        }
}

void free_bodies(void)
{
        std::list < BODY_DATA * >::iterator iter;
        BODY_DATA * body;

        for (iter = bodies.begin(); iter != bodies.end(); iter++)
        {
                body = (*iter);
                DELETE(body);

				bodies.erase(iter);
                iter = bodies.begin();
        }
}

void free_docks(void)
{
        DOCK_DATA *dock, *next_dock;

        for (dock = first_dock; dock; dock = next_dock)
        {
                next_dock = dock->next;
                UNLINK(dock, first_dock, last_dock, next, prev);
                if (dock->name)
                        STRFREE(dock->name);
                DISPOSE(dock);
        }
}

void free_shuttles(void)
{
        SHUTTLE_DATA *shuttle, *next_shuttle;

        for (shuttle = first_shuttle; shuttle; shuttle = next_shuttle)
        {
                next_shuttle = shuttle->next;
                UNLINK(shuttle, first_shuttle, last_shuttle, next, prev);
                free_shuttle(shuttle);
        }
}

void free_illnesses(void)
{
        ILLNESS_DATA *illness, *next_illness;

        for (illness = first_illness; illness; illness = next_illness)
        {
                next_illness = illness->next;
                UNLINK(illness, first_illness, last_illness, next, prev);
                free_illness(illness);
        }
}

void free_channels(void)
{
        CHANNEL_DATA *channel, *next_channel;

        for (channel = first_channel; channel; channel = next_channel)
        {
                next_channel = channel->next;
                free_channel(channel);
        }
}

void free_homes(void)
{
        HOME_DATA *home, *next_home;

        for (home = first_home; home; home = next_home)
        {
                next_home = home->next;
                DELETE(home);
        }
}

void free_restores(void)
{
        RESTORE_DATA *restore, *next_restore;

        for (restore = first_restore; restore; restore = next_restore)
        {
                next_restore = restore->next;
                free_restore(restore);
        }
}

void free_bank_accounts(void)
{
	BANK_ACCOUNT *baccount, *next_baccount;

	for(baccount = first_baccount; baccount; baccount = next_baccount)
	{
		next_baccount = baccount->next;
        free_baccount(baccount);
	}
}

void memory_cleanup(void)
{

        fprintf(stdout, "%s", "Freeing Helps.\n");
        free_helps();

        fprintf(stdout, "%s", "Freeing Characters.\n");
        free_characters();

        fprintf(stdout, "%s", "Freeing Descriptors.\n");
        free_descriptors();

        fprintf(stdout, "%s", "Freeing Objects.\n");
        free_objects();

        fprintf(stdout, "%s", "Freeing Clans.\n");
        free_clans();

        fprintf(stdout, "%s", "Freeing Planets.\n");
        free_planets();

        fprintf(stdout, "%s", "Freeing Ships.\n");
        free_ships();

        fprintf(stdout, "%s", "Freeing Areas.\n");
        free_areas();

        fprintf(stdout, "%s", "Freeing Shops.\n");
        free_shops();

        fprintf(stdout, "%s", "Freeing Specfuns.\n");
        free_specfuns();

        fprintf(stdout, "%s", "Freeing Bank Accounts.\n");
        free_bank_accounts();

        fprintf(stdout, "%s", "Freeing Commands.\n");
        free_commands();

        fprintf(stdout, "%s", "Freeing Skills.\n");
        free_skills();

        fprintf(stdout, "%s", "Freeing DNS.\n");
        free_dnses();

        fprintf(stdout, "%s", "Freeing Socials.\n");
        free_socials();

        fprintf(stdout, "%s", "Freeing Boards.\n");
        free_boards();

        fprintf(stdout, "%s", "Freeing Races.\n");
        free_races();

        fprintf(stdout, "%s", "Freeing Languages.\n");
        free_languages();

        fprintf(stdout, "%s", "Freeing Bounties.\n");
        free_bounties();

        fprintf(stdout, "%s", "Freeing Bans.\n");
        free_bans();

        fprintf(stdout, "%s", "Freeing Installations.\n");
        free_installations();

        fprintf(stdout, "%s", "Freeing Bodies.\n");
        free_bodies();

        fprintf(stdout, "%s", "Freeing Starsystems.\n");
        free_starsystems();

        fprintf(stdout, "%s", "Freeing Docks.\n");
        free_docks();

        fprintf(stdout, "%s", "Freeing Shuttles.\n");
        free_shuttles();

        fprintf(stdout, "%s", "Freeing Illneses.\n");
        free_illnesses();

        fprintf(stdout, "%s", "Freeing Channels.\n");
        free_channels();

        fprintf(stdout, "%s", "Freeing Homes.\n");
        free_homes();

        fprintf(stdout, "%s", "Freeing Restores.\n");
        free_restores();

        fprintf(stdout, "%s", "Freeing Changes.\n");
        free_changes();

        fprintf(stdout, "%s", "Freeing System Data.\n");
        free_sysdata();

        fprintf(stdout, "%s", "Freeing Leftovers.\n");
        free_leftovers();

}
