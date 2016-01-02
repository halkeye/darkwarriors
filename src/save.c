/* vim: ts=8 et ft=cpp sw=8
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
 *                $Id: save.c 1339 2005-12-07 06:09:32Z halkeye $                *
 ****************************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#if defined(__CYGWIN__)
#include <dirent.h>
#else
#include <sys/dir.h>
#endif
#include "mud.h"
#include "bounty.h"
#include "account.h"
#include "channels.h"
#include "races.h"
#include "space2.h"
#include "greet.h"

/*
 * Increment with every major format change.
 */
#define SAVEVERSION	5

/*
 * Array to keep track of equipment temporarily.		-Thoric
 */
OBJ_DATA *save_equipment[MAX_WEAR][MAX_LAYERS];
CHAR_DATA *quitting_char, *loading_char, *saving_char;

int       file_ver;

/*
 * Externals
 */
sh_int    get_age_old(CHAR_DATA * ch);
bool      char_exists(char *player);

/*
 * Array of containers read for proper re-nesting of objects.
 */
static OBJ_DATA *rgObjNest[MAX_NEST];

/*
 * Local functions.
 */
void fwrite_char args((CHAR_DATA * ch, FILE * fp));
void      fread_char
args((CHAR_DATA * ch, FILE * fp, bool preload, bool copyover));
void write_corpses args((CHAR_DATA * ch, char *name));
void      fread_comment(ACCOUNT_DATA * account, FILE * fp);

void save_home(CHAR_DATA * ch)
{
        if (ch->plr_home)
        {
                FILE     *fp;
                char      filename[256];
                sh_int    templvl;
                OBJ_DATA *contents;


                snprintf(filename, MSL, "%s%c/%s.home", PLAYER_DIR,
                         tolower(ch->name[0]), capitalize(ch->name));
                if ((fp = fopen(filename, "w")) == NULL)
                {
                }
                else
                {
                        templvl = ch->top_level;
                        ch->top_level = LEVEL_HERO; /* make sure EQ doesn't get lost */
                        contents = ch->plr_home->last_content;
                        if (contents)
                                fwrite_obj(ch, contents, fp, 0, OS_CARRY,
                                           FALSE);
                        fprintf(fp, "#END\n");
                        ch->top_level = templvl;
                        FCLOSE(fp);
                }
        }
}

void load_home(CHAR_DATA * ch)
{
        char      filename[256];
        FILE     *fph;
        ROOM_INDEX_DATA *storeroom = ch->plr_home;
        OBJ_DATA *obj, *obj_next;


        if (!storeroom)
        {
                return;
        }

        for (obj = storeroom->first_content; obj; obj = obj_next)
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
                        if (!str_cmp(word, "OBJECT"))   /* Objects  */
                                fread_obj(supermob, fph, OS_CARRY);
                        else if (!str_cmp(word, "END")) /* Done     */
                                break;
                        else
                        {
                                bug("Load_plr_home: bad section.", 0);
                                bug(ch->name, 0);
                                break;
                        }
                }

                FCLOSE(fph);

                for (tobj = supermob->first_carrying; tobj; tobj = tobj_next)
                {
                        tobj_next = tobj->next_content;
                        obj_from_char(tobj);
                        obj_to_room(tobj, storeroom);
                }

                release_supermob();

        }
        else
                bug("%s:: filename \"%s\" does not exist", __FUNCTION__,
                    filename);
}


/*
 * Un-equip character before saving to ensure proper	-Thoric
 * stats are saved in case of changes to or removal of EQ
 */
void de_equip_char(CHAR_DATA * ch)
{
        char      buf[MAX_STRING_LENGTH];
        OBJ_DATA *obj;
        int       x, y;

        for (x = 0; x < MAX_WEAR; x++)
                for (y = 0; y < MAX_LAYERS; y++)
                        save_equipment[x][y] = NULL;
        for (obj = ch->first_carrying; obj; obj = obj->next_content)
                if (obj->wear_loc > -1 && obj->wear_loc < MAX_WEAR)
                {

                        for (x = 0; x < MAX_LAYERS; x++)
                                if (!save_equipment[obj->wear_loc][x])
                                {
                                        save_equipment[obj->wear_loc][x] =
                                                obj;
                                        break;
                                }
                        if (x == MAX_LAYERS)
                        {
                                snprintf(buf, MSL,
                                         "%s had on more than %d layers of clothing in one location (%d): %s",
                                         ch->name, MAX_LAYERS, obj->wear_loc,
                                         obj->name);
                                bug(buf, 0);
                        }

                        unequip_char(ch, obj);
                }
}

/*
 * Re-equip character					-Thoric
 */
void re_equip_char(CHAR_DATA * ch)
{
        int       x, y;

        for (x = 0; x < MAX_WEAR; x++)
                for (y = 0; y < MAX_LAYERS; y++)
                        if (save_equipment[x][y] != NULL)
                        {
                                if (quitting_char != ch)
                                        equip_char(ch, save_equipment[x][y],
                                                   x);
                                save_equipment[x][y] = NULL;
                        }
                        else
                                break;
}


/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj(CHAR_DATA * ch)
{
        char      strsave[MAX_INPUT_LENGTH];
        char      strback[MAX_INPUT_LENGTH];
        FILE     *fp;

        if (!ch)
        {
                bug("Save_char_obj: null ch!", 0);
                return;
        }

        if (IS_NPC(ch) || NOT_AUTHED(ch))
                return;

#ifdef ACCOUNT
        if (ch->pcdata && ch->pcdata->account)
                save_account(ch->pcdata->account);
#endif

        saving_char = ch;
        /*
         * save pc's clan's data while we're at it to keep the data in sync 
         */
        if (!IS_NPC(ch) && ch->pcdata->clan)
                save_clan(ch->pcdata->clan);

        if (ch->desc && ch->desc->original)
                ch = ch->desc->original;

        de_equip_char(ch);

        ch->save_time = current_time;
        if (!ch->name || ch->name[0] == '\0')
        {
                bug("save_char_obj: ch with no name!", 0);
                return;
        }
        snprintf(strsave, MSL, "%s%c/%s", PLAYER_DIR, tolower(ch->name[0]),
                 capitalize(ch->name));

        /*
         * Auto-backup pfile (can cause lag with high disk access situtations
         */
        if (IS_SET(sysdata.save_flags, SV_BACKUP))
        {
                snprintf(strback, MSL, "%s%c/%s", BACKUP_DIR,
                         tolower(ch->name[0]), capitalize(ch->name));
                rename(strsave, strback);
        }

        /*
         * Save immortal stats, level & vnums for wizlist       -Thoric
         * and do_vnums command
         *
         * Also save the player flags so we the wizlist builder can see
         * who is a guest and who is retired.
         */
        if (get_trust(ch) > LEVEL_HERO)
        {
                snprintf(strback, MSL, "%s%s", GOD_DIR, capitalize(ch->name));

                if ((fp = fopen(strback, "w")) == NULL)
                {
                        bug("Save_god_level: fopen", 0);
                        perror(strsave);
                }
                else
                {
                        fprintf(fp, "Level        %d\n", ch->top_level);
                        fprintf(fp, "Pcflags      %d\n",
                                ch->pcdata->godflags);
                        if (ch->pcdata->r_range_lo && ch->pcdata->r_range_hi
                            && (ch->top_level > MAX_LEVEL - 4))
                                fprintf(fp, "RoomRange    %d %d\n",
                                        ch->pcdata->r_range_lo,
                                        ch->pcdata->r_range_hi);
                        if (ch->pcdata->o_range_lo && ch->pcdata->o_range_hi)
                                fprintf(fp, "ObjRange     %d %d\n",
                                        ch->pcdata->o_range_lo,
                                        ch->pcdata->o_range_hi);
                        if (ch->pcdata->m_range_lo && ch->pcdata->m_range_hi)
                                fprintf(fp, "MobRange     %d %d\n",
                                        ch->pcdata->m_range_lo,
                                        ch->pcdata->m_range_hi);
                        FCLOSE(fp);
                }
        }

        if ((fp = fopen(strsave, "w")) == NULL)
        {
                bug("Save_char_obj: fopen", 0);
                perror(strsave);
        }
        else
        {
                fwrite_char(ch, fp);
                fwrite_wanted(ch, fp);
                if (ch->first_carrying)
                        fwrite_obj(ch, ch->last_carrying, fp, 0, OS_CARRY,
                                   ch->pcdata->hotboot);
                fwrite_greet(ch, fp);
                fprintf(fp, "#END\n");
                FCLOSE(fp);
        }

        re_equip_char(ch);

        write_corpses(ch, NULL);
        quitting_char = NULL;
        saving_char = NULL;
        return;
}

void save_clone(CHAR_DATA * ch)
{
        char      strsave[MAX_INPUT_LENGTH];
        char      strback[MAX_INPUT_LENGTH];
        FILE     *fp;

        if (!ch)
        {
                bug("Save_char_obj: null ch!", 0);
                return;
        }

        if (IS_NPC(ch) || NOT_AUTHED(ch))
                return;

        if (ch->desc && ch->desc->original)
                ch = ch->desc->original;

        de_equip_char(ch);

        ch->save_time = current_time;
        snprintf(strsave, MSL, "%s%c/%s.clone", PLAYER_DIR,
                 tolower(ch->name[0]), capitalize(ch->name));

        /*
         * Auto-backup pfile (can cause lag with high disk access situtations
         */
        if (IS_SET(sysdata.save_flags, SV_BACKUP))
        {
                snprintf(strback, MSL, "%s%c/%s", BACKUP_DIR,
                         tolower(ch->name[0]), capitalize(ch->name));
                rename(strsave, strback);
        }

        if ((fp = fopen(strsave, "w")) == NULL)
        {
                bug("Save_char_obj: fopen", 0);
                perror(strsave);
        }
        else
        {
                fwrite_char(ch, fp);
//                if (ch->pcdata->account->comments)   /* comments */
                //                       fwrite_comments(ch, fp);    /* comments */
                fprintf(fp, "#END\n");
                FCLOSE(fp);
        }

        re_equip_char(ch);

        write_corpses(ch, NULL);
        quitting_char = NULL;
        saving_char = NULL;
        return;
}



/*
 * Write the char.
 */
void fwrite_char(CHAR_DATA * ch, FILE * fp)
{
        AFFECT_DATA *paf;
        int       sn, track, drug;
        SKILLTYPE *skill = NULL;
        int       pos;

        fprintf(fp, "#%s\n", IS_NPC(ch) ? "MOB" : "PLAYER");

        fprintf(fp, "Version      %d\n", SAVEVERSION);
        fprintf(fp, "Name         %s~\n", ch->name);
#ifdef ACCOUNT
        if (ch->pcdata && ch->pcdata->account && ch->pcdata->account->name)
                fprintf(fp, "Account         %s~\n",
                        ch->pcdata->account->name);
#endif
        if (ch->short_descr && ch->short_descr[0] != '\0')
                fprintf(fp, "ShortDescr   %s~\n", ch->short_descr);
        if (ch->long_descr && ch->long_descr[0] != '\0')
                fprintf(fp, "LongDescr    %s~\n", ch->long_descr);
        if (ch->description && ch->description[0] != '\0')
                fprintf(fp, "Description  %s~\n", ch->description);
        fprintf(fp, "Sex          %d\n", ch->sex);
        fprintf(fp, "Race         %s~\n", ch->race->name());
        fprintf(fp, "MainAbility  %d\n", ch->main_ability);
        fprintf(fp, "Age          %d\n", ch->pcdata->age);
        if (ch->pcdata->birthday.year > -1 && ch->pcdata->birthday.day > -1
            && ch->pcdata->birthday.month > -1
            && ch->pcdata->birthday.hour > -1)
                fprintf(fp, "BDay %d %d %d %d\n", ch->pcdata->birthday.hour,
                        ch->pcdata->birthday.day, ch->pcdata->birthday.month,
                        ch->pcdata->birthday.year);
        if (ch->speaking)
                fprintf(fp, "Languages    %s~\n", ch->speaking->name);
        fprintf(fp, "Toplevel     %d\n", ch->top_level);
        if (ch->bodyparts)
                fprintf(fp, "Bodyparts          %d\n", ch->bodyparts);
        if (ch->trust)
                fprintf(fp, "Trust        %d\n", ch->trust);
        fprintf(fp, "Played       %d\n",
                ch->played + (int) (current_time - ch->logon));
        fprintf(fp, "Room         %d\n",
                (ch->in_room == get_room_index(ROOM_VNUM_LIMBO)
                 && ch->was_in_room)
                ? ch->was_in_room->vnum : ch->in_room->vnum);
        if (ch->plr_home != NULL)
                fprintf(fp, "PlrHome      %d\n", ch->plr_home->vnum);
        if (ch->nextquest != 0)
                fprintf(fp, "QuestNext %d\n", ch->nextquest);
        else if (ch->countdown != 0)
                fprintf(fp, "Countdown %d\n", ch->countdown);
        if (ch->questobj != 0)
                fprintf(fp, "Questobj %d\n", ch->questobj);
        if (ch->questmob != 0)
                fprintf(fp, "Questmob %d\n", ch->questmob);
        if (ch->questgiver && ch->questgiver->name
            && ch->questgiver->name[0] != '\0')
                fprintf(fp, "Questgiver %s~\n", ch->questgiver->name);
        if (ch->questhp > 0)
                fprintf(fp, "Questhp %d\n", ch->questhp);
        fprintf(fp, "HpManaMove   %d %d 0 0 %d %d\n",
                ch->hit, ch->max_hit, ch->endurance, ch->max_endurance);
        fprintf(fp, "Force        %d %d 0 0\n", ch->perm_frc, ch->mod_frc);
        fprintf(fp, "Gold         %ld\n", ch->gold);
        fprintf(fp, "Bank         %ld\n", ch->pcdata->bank);
        fprintf(fp, "Speed        %d\n", ch->speed);

        {
                int       ability;

                for (ability = 0; ability < MAX_ABILITY; ability++)
                        fprintf(fp, "Ability        %d %d %ld\n",
                                ability, ch->skill_level[ability],
                                ch->experience[ability]);
        }
        if (ch->act)
                fprintf(fp, "Act          %d\n", ch->act);
        if (ch->affected_by)
                fprintf(fp, "AffectedBy   %d\n", ch->affected_by);
        fprintf(fp, "Position     %d\n",
                ch->position ==
                (int) POS_FIGHTING ? (int) POS_STANDING : ch->position);

        fprintf(fp, "SavingThrows %d %d %d %d %d\n",
                ch->saving_poison_death,
                ch->saving_wand,
                ch->saving_para_petri,
                ch->saving_breath, ch->saving_spell_staff);
        fprintf(fp, "Alignment    %d\n", ch->alignment);
        fprintf(fp, "Glory        %d\n", ch->pcdata->quest_curr);
        fprintf(fp, "MGlory       %d\n", ch->pcdata->quest_accum);
        fprintf(fp, "Hitroll      %d\n", ch->hitroll);
        fprintf(fp, "Damroll      %d\n", ch->damroll);
        fprintf(fp, "Armor        %d\n", ch->armor);
        if (ch->wimpy)
                fprintf(fp, "Wimpy        %d\n", ch->wimpy);
        if (!xIS_EMPTY(ch->deaf) && IS_IMMORTAL(ch))
                fprintf(fp, "Deaf         %s\n", print_bitvector(&ch->deaf));
        fprintf(fp, "Listening %s~\n", ch->pcdata->listening);
        if (ch->pcdata->recall != 0)
                fprintf(fp, "Recall    %d\n", ch->pcdata->recall);
        if (ch->resistant)
                fprintf(fp, "Resistant    %d\n", ch->resistant);
        if (ch->immune)
                fprintf(fp, "Immune       %d\n", ch->immune);
        if (ch->susceptible)
                fprintf(fp, "Susceptible  %d\n", ch->susceptible);
        if (ch->pcdata && ch->pcdata->restore_time)
                fprintf(fp, "Restore_time %ld\n", ch->pcdata->restore_time);
        if (ch->mental_state != -10)
                fprintf(fp, "Mentalstate  %d\n", ch->mental_state);
        if (ch->master && ch->master->name)
                fprintf(fp, "Master  %s~\n", ch->master->name);
        if (ch->leader && ch->leader->name)
                fprintf(fp, "Leader  %s~\n", ch->leader->name);

        if (IS_NPC(ch))
        {
                fprintf(fp, "Vnum         %d\n", ch->pIndexData->vnum);
                fprintf(fp, "Mobinvis     %d\n", ch->mobinvis);
        }
        else
        {
                fprintf(fp, "Password     %s~\n", ch->pcdata->pwd);
                fprintf(fp, "Lastplayed   %d\n", (int) current_time);
                if (ch->pcdata->bamfin && ch->pcdata->bamfin[0] != '\0')
                        fprintf(fp, "Bamfin       %s~\n", ch->pcdata->bamfin);
                if (ch->pcdata->spouse && ch->pcdata->spouse[0] != '\0')
                        fprintf(fp, "Spouse       %s~\n", ch->pcdata->spouse);

                if (ch->pcdata->bamfout && ch->pcdata->bamfout[0] != '\0')
                        fprintf(fp, "Bamfout      %s~\n",
                                ch->pcdata->bamfout);
                if (ch->pcdata->rank && ch->pcdata->rank[0] != '\0')
                        fprintf(fp, "Rank         %s~\n", ch->pcdata->rank);
                if (ch->pcdata->clanrank)
                        fprintf(fp, "Clanrank     %d\n",
                                ch->pcdata->clanrank);
                if (ch->pcdata->bestowments
                    && ch->pcdata->bestowments[0] != '\0')
                        fprintf(fp, "Bestowments  %s~\n",
                                ch->pcdata->bestowments);
                fprintf(fp, "Title        %s~\n", ch->pcdata->title);
                if (ch->pcdata->homepage && ch->pcdata->homepage[0] != '\0')
                        fprintf(fp, "Homepage     %s~\n",
                                ch->pcdata->homepage);
                if (ch->pcdata->aolim && ch->pcdata->aolim[0] != '\0')
                        fprintf(fp, "Aim          %s~\n", ch->pcdata->aolim);
                if (ch->pcdata->icq && ch->pcdata->icq[0] != '\0')
                        fprintf(fp, "Icq          %s~\n", ch->pcdata->icq);
                if (ch->pcdata->yahoo && ch->pcdata->yahoo[0] != '\0')
                        fprintf(fp, "Yahoo        %s~\n", ch->pcdata->yahoo);
                if (ch->pcdata->msn && ch->pcdata->msn[0] != '\0')
                        fprintf(fp, "Msn          %s~\n", ch->pcdata->msn);
                if (ch->pcdata->realname && ch->pcdata->realname[0] != '\0')
                        fprintf(fp, "Realname     %s~\n",
                                ch->pcdata->realname);
                if (ch->pcdata->bio && ch->pcdata->bio[0] != '\0')
                        fprintf(fp, "Bio          %s~\n", ch->pcdata->bio);
                if (ch->pcdata->authed_by && ch->pcdata->authed_by[0] != '\0')
                        fprintf(fp, "AuthedBy     %s~\n",
                                ch->pcdata->authed_by);
                if (ch->pcdata->min_snoop)
                        fprintf(fp, "Minsnoop     %d\n",
                                ch->pcdata->min_snoop);
                if (ch->pcdata->prompt && *ch->pcdata->prompt)
                        fprintf(fp, "Prompt       %s~\n", ch->pcdata->prompt);
                if (ch->pcdata->fprompt && *ch->pcdata->fprompt)
                        fprintf(fp, "Fprompt       %s~\n",
                                ch->pcdata->fprompt);
                if (ch->pcdata->pagerlen != 24)
                        fprintf(fp, "Pagerlen     %d\n",
                                ch->pcdata->pagerlen);
                if (ch->pcdata->rp != 0)
                        fprintf(fp, "RP          %d\n", ch->pcdata->rp);
                if (ch->pcdata->comchan != 0)
                        fprintf(fp, "ComChan     %d\n", ch->pcdata->comchan);
                if (ch->pcdata->illness)
                        fprintf(fp, "Illness          %d\n",
                                ch->pcdata->illness);
                if (ch->pcdata->full_name[0] != '\0'
                    && str_cmp(ch->pcdata->full_name, ch->pcdata->title))
                        fprintf(fp, "FullName         %s~\n",
                                ch->pcdata->full_name);
                fprintf(fp, "Email       %s~\n", ch->pcdata->email);
                fprintf(fp, "Height          %d\n", ch->pcdata->height);
                fprintf(fp, "Weight          %d\n", ch->pcdata->weight);
                fprintf(fp, "Hair          %d\n", ch->pcdata->hair);
                fprintf(fp, "Eyes          %d\n", ch->pcdata->eye);
                fprintf(fp, "Build          %d\n", ch->pcdata->build);
                fprintf(fp, "Complextion          %d\n",
                        ch->pcdata->complextion);

                if (ch->pcdata->realage)
                        fprintf(fp, "Realage          %d\n",
                                ch->pcdata->realage);
                fprintf(fp, "Addiction   ");
                for (drug = 0; drug <= 9; drug++)
                        fprintf(fp, " %d", ch->pcdata->addiction[drug]);
                fprintf(fp, "\n");
                fprintf(fp, "Druglevel   ");
                for (drug = 0; drug <= 9; drug++)
                        fprintf(fp, " %d", ch->pcdata->drug_level[drug]);
                fprintf(fp, "\n");
                for (pos = 0; pos < MAX_IGNORE; pos++)
                {
                        if (ch->pcdata->ignore[pos] == NULL)
                                continue;

                        fprintf(fp, "Ignore %s~\n", ch->pcdata->ignore[pos]);
                }
                for (pos = 0; pos < MAX_IMPLANT_TYPES; pos++)
                        if (ch->pcdata->implants[pos] >= 0
                            && ch->pcdata->implants[pos] <= 3)
                                fprintf(fp, "Implant %d %d\n", pos,
                                        ch->pcdata->implants[pos]);

                if (IS_IMMORTAL(ch) || ch->pcdata->area)
                {
                        fprintf(fp, "WizInvis     %d\n",
                                ch->pcdata->wizinvis);
                        if (ch->pcdata->r_range_lo && ch->pcdata->r_range_hi)
                                fprintf(fp, "RoomRange    %d %d\n",
                                        ch->pcdata->r_range_lo,
                                        ch->pcdata->r_range_hi);
                        if (ch->pcdata->o_range_lo && ch->pcdata->o_range_hi)
                                fprintf(fp, "ObjRange     %d %d\n",
                                        ch->pcdata->o_range_lo,
                                        ch->pcdata->o_range_hi);
                        if (ch->pcdata->m_range_lo && ch->pcdata->m_range_hi)
                                fprintf(fp, "MobRange     %d %d\n",
                                        ch->pcdata->m_range_lo,
                                        ch->pcdata->m_range_hi);
                }
                if (ch->pcdata->clan)
                        fprintf(fp, "Clan         %s~\n",
                                ch->pcdata->clan->name);
                fprintf(fp, "Flags        %d\n", ch->pcdata->flags);
                fprintf(fp, "GodFlags        %d\n", ch->pcdata->godflags);
                if (ch->pcdata->release_date > current_time)
                        fprintf(fp, "Helled       %d %s~\n",
                                (int) ch->pcdata->release_date,
                                ch->pcdata->helled_by);
                if (ch->pcdata->pkills)
                        fprintf(fp, "PKills       %d\n", ch->pcdata->pkills);
                if (ch->pcdata->pdeaths)
                        fprintf(fp, "PDeaths      %d\n", ch->pcdata->pdeaths);
                if (get_timer(ch, TIMER_PKILLED)
                    && (get_timer(ch, TIMER_PKILLED) > 0))
                        fprintf(fp, "PTimer       %d\n",
                                get_timer(ch, TIMER_PKILLED));
                fprintf(fp, "MKills       %d\n", ch->pcdata->mkills);
                fprintf(fp, "MDeaths      %d\n", ch->pcdata->mdeaths);
                if (ch->pcdata->illegal_pk)
                        fprintf(fp, "IllegalPK    %d\n",
                                ch->pcdata->illegal_pk);
                fprintf(fp, "AttrPerm     %d %d %d %d %d %d %d\n",
                        ch->perm_str, ch->perm_int, ch->perm_wis,
                        ch->perm_dex, ch->perm_con, ch->perm_cha,
                        ch->perm_lck);

                fprintf(fp, "AttrMod      %d %d %d %d %d %d %d\n",
                        ch->mod_str,
                        ch->mod_int,
                        ch->mod_wis,
                        ch->mod_dex, ch->mod_con, ch->mod_cha, ch->mod_lck);

                fprintf(fp, "AttrBonus      %d %d %d %d %d %d %d %d\n",
                        ch->bonus_str,
                        ch->bonus_int,
                        ch->bonus_wis,
                        ch->bonus_dex,
                        ch->bonus_con,
                        ch->bonus_cha, ch->bonus_lck, ch->bonus_frc);


                fprintf(fp, "Condition    %d %d %d %d\n",
                        ch->pcdata->condition[0],
                        ch->pcdata->condition[1],
                        ch->pcdata->condition[2], ch->pcdata->condition[3]);
                if (ch->desc && ch->desc->host)
                        fprintf(fp, "Site         %s\n", ch->desc->host);
                else
                        fprintf(fp, "Site         (Link-Dead)\n");

                for (sn = 1; sn < top_sn; sn++)
                {
                        if (skill_table[sn]->name
                            && ch->pcdata->learned[sn] > 0)
                                switch (skill_table[sn]->type)
                                {
                                default:
                                        fprintf(fp, "Skill        %d '%s'\n",
                                                ch->pcdata->learned[sn],
                                                skill_table[sn]->name);
                                        break;
                                case SKILL_SPELL:
                                        fprintf(fp, "Spell        %d '%s'\n",
                                                ch->pcdata->learned[sn],
                                                skill_table[sn]->name);
                                        break;
                                case SKILL_WEAPON:
                                        fprintf(fp, "Weapon       %d '%s'\n",
                                                ch->pcdata->learned[sn],
                                                skill_table[sn]->name);
                                        break;
                                case SKILL_TONGUE:
                                        fprintf(fp, "Tongue       %d '%s'\n",
                                                ch->pcdata->learned[sn],
                                                skill_table[sn]->name);
                                        break;
                                }
                }
        }

        for (paf = ch->first_affect; paf; paf = paf->next)
        {
                if (paf->type >= 0
                    && (skill = get_skilltype(paf->type)) == NULL)
                        continue;

                if (paf->type >= 0 && paf->type < TYPE_PERSONAL)
                        fprintf(fp, "AffectData   '%s' %3d %3d %3d %10d\n",
                                skill->name,
                                paf->duration,
                                paf->modifier, paf->location, paf->bitvector);
                else
                        fprintf(fp, "Affect       %3d %3d %3d %3d %10d\n",
                                paf->type,
                                paf->duration,
                                paf->modifier, paf->location, paf->bitvector);
        }

        track = URANGE(2,
                       ((ch->top_level + 3) * MAX_KILLTRACK) / LEVEL_AVATAR,
                       MAX_KILLTRACK);
        for (sn = 0; sn < track; sn++)
        {
                if (ch->pcdata->killed[sn].vnum == 0)
                        break;
                fprintf(fp, "Killed       %d %d\n",
                        ch->pcdata->killed[sn].vnum,
                        ch->pcdata->killed[sn].count);
        }

        /*
         * Save color values - Samson 9-29-98 
         */
        {
                int       x;

                fprintf(fp, "MaxColors    %d\n", MAX_COLORS);
                fprintf(fp, "Colors       ");
                for (x = 0; x < MAX_COLORS; x++)
                        fprintf(fp, "%d ", ch->colors[x]);
                fprintf(fp, "\n");
        }
#ifdef IMC
        imc_savechar(ch, fp);
#endif
        fprintf(fp, "End\n\n");
        return;
}



/*
 * Write an object and its contents.
 */
void fwrite_obj(CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest,
                sh_int os_type, bool hotboot)
{
        EXTRA_DESCR_DATA *ed;
        AFFECT_DATA *paf;
        sh_int    wear, wear_loc, x;

        if (iNest >= MAX_NEST)
        {
                bug("fwrite_obj: iNest hit MAX_NEST %d", iNest);
                return;
        }

        /*
         * Slick recursion to write lists backwards,
         *   so loading them will load in forwards order.
         */
        if (obj->prev_content && os_type != OS_CORPSE)
                fwrite_obj(ch, obj->prev_content, fp, iNest, OS_CARRY, FALSE);

        /*
         * Catch deleted objects                    -Thoric
         */
        if (obj_extracted(obj))
                return;

        /*
         * Do NOT save prototype items!             -Thoric
         */
        if (IS_OBJ_STAT(obj, ITEM_PROTOTYPE))
                return;

        /*
         * DO NOT save corpses lying on the ground as a hotboot item, they already saved elsewhere! - Samson 
         */
        if (hotboot && obj->item_type == ITEM_CORPSE_PC)
                return;

        /*
         * Corpse saving. -- Altrag 
         */
        fprintf(fp, (os_type == OS_CORPSE ? "#CORPSE\n" : "#OBJECT\n"));

        if (iNest)
                fprintf(fp, "Nest         %d\n", iNest);
        if (obj->count > 1)
                fprintf(fp, "Count        %d\n", obj->count);
        if (QUICKMATCH(obj->name, obj->pIndexData->name) == 0)
                fprintf(fp, "Name         %s~\n", obj->name);
        if (QUICKMATCH(obj->short_descr, obj->pIndexData->short_descr) == 0)
                fprintf(fp, "ShortDescr   %s~\n", obj->short_descr);
        if (QUICKMATCH(obj->description, obj->pIndexData->description) == 0)
                fprintf(fp, "Description  %s~\n", obj->description);
        if (QUICKMATCH(obj->action_desc, obj->pIndexData->action_desc) == 0)
                fprintf(fp, "ActionDesc   %s~\n", obj->action_desc);
        if (obj->armed_by && obj->armed_by[0] != '\0')
                fprintf(fp, "Armedby      %s~\n", obj->armed_by);
        fprintf(fp, "Vnum         %d\n", obj->pIndexData->vnum);
        if ((os_type == OS_CORPSE || hotboot) && obj->in_room)
        {
                fprintf(fp, "Room         %d\n", obj->in_room->vnum);
                fprintf(fp, "Rvnum	   %d\n", obj->room_vnum);
        }
        if (obj->extra_flags != obj->pIndexData->extra_flags)
                fprintf(fp, "ExtraFlags   %d\n", obj->extra_flags);
        if (obj->wear_flags != obj->pIndexData->wear_flags)
                fprintf(fp, "WearFlags    %d\n", obj->wear_flags);
        wear_loc = -1;
        for (wear = 0; wear < MAX_WEAR; wear++)
                for (x = 0; x < MAX_LAYERS; x++)
                        if (obj == save_equipment[wear][x])
                        {
                                wear_loc = wear;
                                break;
                        }
                        else if (!save_equipment[wear][x])
                                break;
        if (wear_loc != -1)
                fprintf(fp, "WearLoc      %d\n", wear_loc);
        if (obj->item_type != obj->pIndexData->item_type)
                fprintf(fp, "ItemType     %d\n", obj->item_type);
        if (obj->weight != obj->pIndexData->weight)
                fprintf(fp, "Weight       %d\n", obj->weight);
        if (obj->level)
                fprintf(fp, "Level        %d\n", obj->level);
        if (obj->timer)
                fprintf(fp, "Timer        %d\n", obj->timer);
        if (obj->cost != obj->pIndexData->cost)
                fprintf(fp, "Cost         %d\n", obj->cost);
        if (obj->value[0] || obj->value[1] || obj->value[2]
            || obj->value[3] || obj->value[4] || obj->value[5])
                fprintf(fp, "Values       %d %d %d %d %d %d\n",
                        obj->value[0], obj->value[1], obj->value[2],
                        obj->value[3], obj->value[4], obj->value[5]);

        switch (obj->item_type)
        {
        case ITEM_PILL:    /* was down there with staff and wand, wrongly - Scryn */
        case ITEM_POTION:
                if (IS_VALID_SN(obj->value[1]))
                        fprintf(fp, "Spell 1      '%s'\n",
                                skill_table[obj->value[1]]->name);

                if (IS_VALID_SN(obj->value[2]))
                        fprintf(fp, "Spell 2      '%s'\n",
                                skill_table[obj->value[2]]->name);

                if (IS_VALID_SN(obj->value[3]))
                        fprintf(fp, "Spell 3      '%s'\n",
                                skill_table[obj->value[3]]->name);

                break;

        case ITEM_DEVICE:
                if (IS_VALID_SN(obj->value[3]))
                        fprintf(fp, "Spell 3      '%s'\n",
                                skill_table[obj->value[3]]->name);

                break;
        case ITEM_SALVE:
                if (IS_VALID_SN(obj->value[4]))
                        fprintf(fp, "Spell 4      '%s'\n",
                                skill_table[obj->value[4]]->name);

                if (IS_VALID_SN(obj->value[5]))
                        fprintf(fp, "Spell 5      '%s'\n",
                                skill_table[obj->value[5]]->name);
                break;
        }

        for (paf = obj->first_affect; paf; paf = paf->next)
        {
                /*
                 * Save extra object affects                -Thoric
                 */
                if (paf->type < 0 || paf->type >= top_sn)
                {
                        fprintf(fp, "Affect       %d %d %d %d %d\n",
                                paf->type,
                                paf->duration,
                                ((paf->location == APPLY_WEAPONSPELL
                                  || paf->location == APPLY_WEARSPELL
                                  || paf->location == APPLY_REMOVESPELL
                                  || paf->location == APPLY_STRIPSN)
                                 && IS_VALID_SN(paf->modifier))
                                ? skill_table[paf->modifier]->slot : paf->
                                modifier, paf->location, paf->bitvector);
                }
                else
                        fprintf(fp, "AffectData   '%s' %d %d %d %d\n",
                                skill_table[paf->type]->name,
                                paf->duration,
                                ((paf->location == APPLY_WEAPONSPELL
                                  || paf->location == APPLY_WEARSPELL
                                  || paf->location == APPLY_REMOVESPELL
                                  || paf->location == APPLY_STRIPSN)
                                 && IS_VALID_SN(paf->modifier))
                                ? skill_table[paf->modifier]->slot : paf->
                                modifier, paf->location, paf->bitvector);
        }

        for (ed = obj->first_extradesc; ed; ed = ed->next)
                fprintf(fp, "ExtraDescr   %s~ %s~\n",
                        ed->keyword, ed->description);

        fprintf(fp, "End\n\n");

        if (obj->first_content)
                fwrite_obj(ch, obj->last_content, fp, iNest + 1, OS_CARRY,
                           hotboot);

        return;
}



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj(DESCRIPTOR_DATA * d, char *name, bool preload,
                   bool copyover)
{
        char      strsave[MAX_INPUT_LENGTH];
        CHAR_DATA *ch;
        FILE     *fp;
        bool      found;
        struct stat fst;
        int       i, x;
        extern FILE *fpArea;
        extern char strArea[MAX_INPUT_LENGTH];
        char      buf[MAX_INPUT_LENGTH];

        CREATE(ch, CHAR_DATA, 1);
        for (x = 0; x < MAX_WEAR; x++)
                for (i = 0; i < MAX_LAYERS; i++)
                        save_equipment[x][i] = NULL;
        clear_char(ch);
        loading_char = ch;

        CREATE(ch->pcdata, PC_DATA, 1);
        d->character = ch;
        ch->desc = d;
        ch->name = STRALLOC(name);
        ch->act = PLR_BLANK | PLR_COMBINE | PLR_PROMPT;
        ch->perm_str = 10;
        ch->perm_int = 10;
        ch->perm_wis = 10;
        ch->perm_dex = 10;
        ch->perm_con = 10;
        ch->perm_cha = 10;
        ch->perm_lck = 10;
        ch->max_endurance = 0;
        ch->endurance = 0;
        ch->pcdata->condition[COND_THIRST] = 48;
        ch->pcdata->condition[COND_FULL] = 48;
        ch->pcdata->condition[COND_BLOODTHIRST] = 10;
        ch->pcdata->statpoints = 0;
        ch->pcdata->statedit = 0;
        ch->pcdata->wizinvis = 0;
        ch->mental_state = -10;
        ch->mobinvis = 0;
        for (i = 0; i < MAX_SKILL; i++)
                ch->pcdata->learned[i] = 0;
        ch->pcdata->release_date = 0;
        ch->pcdata->helled_by = NULL;
        ch->pcdata->recall = 0;
        ch->saving_poison_death = 0;
        ch->saving_wand = 0;
        ch->questhp = 0;
        ch->saving_para_petri = 0;
        ch->saving_breath = 0;
        ch->saving_spell_staff = 0;
        ch->pcdata->pagerlen = 24;
        ch->mob_clan = STRALLOC("");
        ch->was_sentinel = NULL;
        ch->plr_home = NULL;
        ch->pcdata->hotboot = FALSE;    /* Never changed except when PC is saved during hotboot save */
        for (i = 0; i < MAX_IMPLANT_TYPES; i++)
                ch->pcdata->implants[i] = -2;
        found = FALSE;
#ifdef IMC
        imc_initchar(ch);
#endif
        snprintf(strsave, MSL, "%s%c/%s", PLAYER_DIR, tolower(name[0]),
                 capitalize(name));
        if (stat(strsave, &fst) != -1)
        {
                if (fst.st_size == 0)
                {
                        snprintf(buf, MSL, "%s%c/%s", BACKUP_DIR,
                                 tolower(name[0]), capitalize(name));
                        send_to_char("Restoring your backup player file...",
                                     ch);
                        if (rename(buf, strsave) == -1)
                        {
                                remove(strsave);
                                send_to_char
                                        (" Error restoring backup file.\n\r",
                                         ch);
                                return FALSE;
                        }
                        return load_char_obj(d, name, preload, copyover);
                }
                else
                {
                        snprintf(buf, MSL, "%s player data for: %s (%dK)",
                                 preload ? "Preloading" : "Loading", ch->name,
                                 (int) fst.st_size / 1024);
                        log_string_plus(buf, LOG_COMM, LEVEL_GREATER);
                }
        }
        /*
         * else no player file 
         */

        if ((fp = fopen(strsave, "r")) != NULL)
        {
                int       iNest;

                for (iNest = 0; iNest < MAX_NEST; iNest++)
                        rgObjNest[iNest] = NULL;

                found = TRUE;
                /*
                 * Cheat so that bug will show line #'s -- Altrag 
                 */
                fpArea = fp;
                mudstrlcpy(strArea, strsave, MIL);
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
                                bug("Load_char_obj: # not found.", 0);
                                bug(name, 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "PLAYER"))
                        {
                                fread_char(ch, fp, preload, copyover);
                                if (preload)
                                        break;
                        }
                        else if (!str_cmp(word, "WANTED"))  /* Wanted */
                                fread_wanted(ch, fp);
                        else if (!str_cmp(word, "OBJECT"))  /* Objects  */
                                fread_obj(ch, fp, OS_CARRY);
                        else if (!str_cmp(word, "COMMENT"))
                        {
                                ACCOUNT_DATA *account = ch->pcdata->account;

                                fread_comment(account, fp); /* Comments */
                        }
                        else if (!str_cmp(word, "GREET"))  /* Greet */
                        {
                                fread_greet(ch, fp);
                        }
                        else if (!str_cmp(word, "END")) /* Done     */
                                break;
                        else
                        {
                                bug("Load_char_obj: bad section.", 0);
                                bug(name, 0);
                                break;
                        }
                }
                FCLOSE(fp);
                fpArea = NULL;
                mudstrlcpy(strArea, "$", MIL);
        }


        if (!found)
        {
                ch->short_descr = STRALLOC("");
                ch->long_descr = STRALLOC("");
                ch->description = STRALLOC("");
                ch->editor = NULL;
                ch->questhp = 0;
                ch->pcdata->clan = NULL;
                ch->pcdata->pwd = str_dup("");
                ch->pcdata->email = STRALLOC("");
                ch->pcdata->spouse = STRALLOC("");
                ch->pcdata->bamfin = STRALLOC("");
                ch->pcdata->bamfout = STRALLOC("");
                ch->pcdata->rank = STRALLOC("");
                ch->pcdata->bestowments = STRALLOC("");
                ch->pcdata->title = STRALLOC("");
                ch->pcdata->homepage = STRALLOC("");
                ch->pcdata->bio = STRALLOC("");
                ch->pcdata->authed_by = STRALLOC("");
                ch->pcdata->prompt = STRALLOC("");
                ch->pcdata->icq = STRALLOC("");
                ch->pcdata->msn = STRALLOC("");
                ch->pcdata->aolim = STRALLOC("");
                ch->pcdata->yahoo = STRALLOC("");
                ch->pcdata->realname = STRALLOC("");
                ch->pcdata->listening = STRALLOC("");
                ch->pcdata->r_range_lo = 0;
                ch->pcdata->r_range_hi = 0;
                ch->pcdata->m_range_lo = 0;
                ch->pcdata->m_range_hi = 0;
                ch->pcdata->o_range_lo = 0;
                ch->pcdata->o_range_hi = 0;
                ch->pcdata->wizinvis = 0;
                ch->pcdata->recall = 0;
        }
        else
        {
                if (!ch->pcdata->bio)
                        ch->pcdata->bio = STRALLOC("");

                if (!ch->pcdata->authed_by)
                        ch->pcdata->authed_by = STRALLOC("");

                if (!IS_NPC(ch) && get_trust(ch) > LEVEL_AVATAR)
                {
                        if (ch->pcdata->wizinvis < 2)
                                ch->pcdata->wizinvis = ch->top_level;
                        assign_area(ch);
                }
                if (file_ver > 1)
                {
                        for (i = 0; i < MAX_WEAR; i++)
                        {
                                for (x = 0; x < MAX_LAYERS; x++)
                                {
                                        if (save_equipment[i][x])
                                        {
                                                equip_char(ch,
                                                           save_equipment[i]
                                                           [x], i);
                                                save_equipment[i][x] = NULL;
                                        }
                                        else
                                                break;
                                }
                        }
                }

        }
		if (ch->pcdata->clanrank >= MAX_RANK) 
			ch->pcdata->clanrank = MAX_RANK - 1;


		if (ch->pcdata->spouse && ch->pcdata->spouse[0] != '\0' && !char_exists(ch->pcdata->spouse))
		{
			REMOVE_BIT(ch->pcdata->flags, PCFLAG_MARRIED);
			if (ch->pcdata->spouse)
                STRFREE(ch->pcdata->spouse);
		}


        loading_char = NULL;
        return found;
}



/*
 * Read in a char.
 */

void fread_char(CHAR_DATA * ch, FILE * fp, bool preload, bool copyover)
{
        char      buf[MAX_STRING_LENGTH];
        char     *line;
        const char *word;
        int       x1, x2, x3, x4, x5, x6, x7, x8, x9, x0;
        sh_int    killcnt;
        bool      fMatch;
        int       max_colors = 0;   /* Color code */
        time_t    lastplayed = 0;
        int       skill_number, extra, count = 0;
        CHANNEL_DATA *channel;

        copyover = 0;


        file_ver = 0;
        killcnt = 0;
        /*
         * Setup color values in case player has none set - Samson 
         */
        memcpy(&ch->colors, &default_set, sizeof(default_set));
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
                        KEY("Act", ch->act, fread_number(fp));
                        KEY("AffectedBy", ch->affected_by, fread_number(fp));
                        KEY("Alignment", ch->alignment, fread_number(fp));
                        KEY("Armor", ch->armor, fread_number(fp));
                        KEY("Age", ch->pcdata->age, fread_number(fp));
                        KEY("Aim", ch->pcdata->aolim, fread_string(fp));
#ifdef ACCOUNT
                        if (!str_cmp(word, "Account"))
                        {
                                char     *name = fread_string_nohash(fp);

                                if (ch->desc && ch->desc->account)
                                        ch->pcdata->account =
                                                ch->desc->account;
                                else
                                {
                                        ACCOUNT_DATA *account =
                                                load_account(name);
                                        if (account)
                                                ch->pcdata->account = account;
                                        else
                                                bug("Account %s not found.",
                                                    name);
                                }
                                DISPOSE(name);
                                fMatch = TRUE;
                                break;
                        }
#endif
                        if (!str_cmp(word, "Addiction"))
                        {
                                line = fread_line(fp);
                                x0 = x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 =
                                        x9 = 0;
                                sscanf(line, "%d %d %d %d %d %d %d %d %d %d",
                                       &x0, &x1, &x2, &x3, &x4, &x5, &x6, &x7,
                                       &x8, &x9);
                                ch->pcdata->addiction[0] = x0;
                                ch->pcdata->addiction[1] = x1;
                                ch->pcdata->addiction[2] = x2;
                                ch->pcdata->addiction[3] = x3;
                                ch->pcdata->addiction[4] = x4;
                                ch->pcdata->addiction[5] = x5;
                                ch->pcdata->addiction[6] = x6;
                                ch->pcdata->addiction[7] = x7;
                                ch->pcdata->addiction[8] = x8;
                                ch->pcdata->addiction[9] = x9;
                                fMatch = TRUE;
                                break;
                        }

                        if (!str_cmp(word, "Ability"))
                        {
                                line = fread_line(fp);
                                x0 = x1 = x2 = 0;
                                sscanf(line, "%d %d %d", &x0, &x1, &x2);
                                if (x0 >= 0 && x0 < MAX_ABILITY)
                                {
                                        ch->skill_level[x0] = x1;
                                        ch->experience[x0] = x2;
                                }
                                fMatch = TRUE;
                                break;
                        }


                        if (!str_cmp(word, "Affect")
                            || !str_cmp(word, "AffectData"))
                        {
                                AFFECT_DATA *paf;

                                if (preload)
                                {
                                        fMatch = TRUE;
                                        fread_to_eol(fp);
                                        break;
                                }
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
                                                bug("Fread_char: unknown skill.", 0);
                                        else
                                                paf->type = sn;
                                }

                                paf->duration = fread_number(fp);
                                paf->modifier = fread_number(fp);
                                paf->location = fread_number(fp);
                                paf->bitvector = fread_number(fp);
                                LINK(paf, ch->first_affect, ch->last_affect,
                                     next, prev);
                                fMatch = TRUE;
                                break;
                        }

                        if (!str_cmp(word, "AttrMod"))
                        {
                                line = fread_line(fp);
                                x1 = x2 = x3 = x4 = x5 = x6 = x7 = 13;
                                sscanf(line, "%d %d %d %d %d %d %d",
                                       &x1, &x2, &x3, &x4, &x5, &x6, &x7);
                                ch->mod_str = x1;
                                ch->mod_int = x2;
                                ch->mod_wis = x3;
                                ch->mod_dex = x4;
                                ch->mod_con = x5;
                                ch->mod_cha = x6;
                                ch->mod_lck = x7;
                                if (!x7)
                                        ch->mod_lck = 0;
                                fMatch = TRUE;
                                break;
                        }

                        if (!str_cmp(word, "AttrPerm"))
                        {
                                line = fread_line(fp);
                                x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;
                                sscanf(line, "%d %d %d %d %d %d %d",
                                       &x1, &x2, &x3, &x4, &x5, &x6, &x7);
                                ch->perm_str = x1;
                                ch->perm_int = x2;
                                ch->perm_wis = x3;
                                ch->perm_dex = x4;
                                ch->perm_con = x5;
                                ch->perm_cha = x6;
                                ch->perm_lck = x7;
                                if (!x7 || x7 == 0)
                                        ch->perm_lck = 13;
                                fMatch = TRUE;
                                break;
                        }

                        if (!str_cmp(word, "AttrBonus"))
                        {
                                line = fread_line(fp);
                                x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;
                                sscanf(line, "%d %d %d %d %d %d %d %d",
                                       &x1, &x2, &x3, &x4, &x5, &x6, &x7,
                                       &x8);
                                ch->bonus_str = x1;
                                ch->bonus_int = x2;
                                ch->bonus_wis = x3;
                                ch->bonus_dex = x4;
                                ch->bonus_con = x5;
                                ch->bonus_cha = x6;
                                ch->bonus_lck = x7;
                                ch->bonus_frc = x8;
                                fMatch = TRUE;
                                break;
                        }
                        KEY("AuthedBy", ch->pcdata->authed_by,
                            fread_string(fp));
                        break;

                case 'B':
                        KEY("Bamfin", ch->pcdata->bamfin, fread_string(fp));
                        KEY("Build", ch->pcdata->build, fread_number(fp));
                        KEY("Bamfout", ch->pcdata->bamfout, fread_string(fp));
                        KEY("Bestowments", ch->pcdata->bestowments,
                            fread_string(fp));
                        KEY("Bio", ch->pcdata->bio, fread_string(fp));
                        KEY("Bank", ch->pcdata->bank, fread_number(fp));
                        KEY("Bodyparts", ch->bodyparts, fread_number(fp));
                        if (!strcmp(word, "BDay"))  /* Voltecs player birthdays! */
                        {
                                ch->pcdata->birthday.hour = fread_number(fp);
                                ch->pcdata->birthday.day = fread_number(fp);
                                ch->pcdata->birthday.month = fread_number(fp);
                                ch->pcdata->birthday.year = fread_number(fp);
                                fMatch = TRUE;
                                break;
                        }
                        break;

                case 'C':
                        if (!str_cmp(word, "Clan"))
                        {
                                char     *temp = fread_string(fp);

                                if (!preload && temp[0] != '\0' &&
                                    (ch->pcdata->clan =
                                     get_clan(temp)) == NULL)
                                {
                                        snprintf(buf, MSL,
                                                 "Warning: the organization %s no longer exists, and therefore you no longer\n\rbelong to that organization.\n\r",
                                                 temp);
                                        send_to_char(buf, ch);
                                }
                                STRFREE(temp);
                                if (ch->pcdata->clan && ch->name)
                                {
                                        if (ch->pcdata->clan->roster)
                                        {
                                                if (!hasname
                                                    (ch->pcdata->clan->roster,
                                                     ch->name))
                                                        addname(&ch->pcdata->
                                                                clan->roster,
                                                                ch->name);
                                        }
                                        else
                                                ch->pcdata->clan->roster =
                                                        STRALLOC(ch->name);
                                }
                                fMatch = TRUE;
                                break;
                        }

                        /*
                         * Load color values - Samson 9-29-98 
                         */
                        {
                                int       x;

                                if (!str_cmp(word, "Colors"))
                                {
                                        for (x = 0; x < max_colors; x++)
                                                ch->colors[x] =
                                                        fread_number(fp);
                                        fMatch = TRUE;
                                        break;
                                }
                        }

                        KEY("Clanrank", ch->pcdata->clanrank,
                            fread_number(fp));
                        KEY("Comchan", ch->pcdata->comchan, fread_number(fp));
                        KEY("Countdown", ch->countdown, fread_number(fp));
                        KEY("Complextion", ch->pcdata->complextion,
                            fread_number(fp));

                        if (!str_cmp(word, "Condition"))
                        {
                                line = fread_line(fp);
                                sscanf(line, "%d %d %d %d",
                                       &x1, &x2, &x3, &x4);
                                ch->pcdata->condition[0] = x1;
                                ch->pcdata->condition[1] = x2;
                                ch->pcdata->condition[2] = x3;
                                ch->pcdata->condition[3] = x4;
                                fMatch = TRUE;
                                break;
                        }
                        break;

                case 'D':
                        KEY("Damroll", ch->damroll, fread_number(fp));
                        KEY("Deaf", ch->deaf, fread_bitvector(fp));
                        KEY("Description", ch->description, fread_string(fp));
                        if (!str_cmp(word, "Druglevel"))
                        {
                                line = fread_line(fp);
                                x0 = x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 =
                                        x9 = 0;
                                sscanf(line, "%d %d %d %d %d %d %d %d %d %d",
                                       &x0, &x1, &x2, &x3, &x4, &x5, &x6, &x7,
                                       &x8, &x9);
                                ch->pcdata->drug_level[0] = x0;
                                ch->pcdata->drug_level[1] = x1;
                                ch->pcdata->drug_level[2] = x2;
                                ch->pcdata->drug_level[3] = x3;
                                ch->pcdata->drug_level[4] = x4;
                                ch->pcdata->drug_level[5] = x5;
                                ch->pcdata->drug_level[6] = x6;
                                ch->pcdata->drug_level[7] = x7;
                                ch->pcdata->drug_level[8] = x8;
                                ch->pcdata->drug_level[9] = x9;
                                fMatch = TRUE;
                                break;
                        }
                        break;

                        /*
                         * 'E' was moved to after 'S' 
                         */
                case 'F':
                        KEY("Flags", ch->pcdata->flags, fread_number(fp));
                        KEY("FPrompt", ch->pcdata->fprompt, fread_string(fp));
                        KEY("FullName", ch->pcdata->full_name,
                            fread_string(fp));
                        if (!str_cmp(word, "Force"))
                        {
                                line = fread_line(fp);
                                x1 = x2 = x3 = x4 = x5 = x6 = 0;
                                sscanf(line, "%d %d %d %d",
                                       &x1, &x2, &x3, &x4);
                                ch->perm_frc = x1;
                                ch->mod_frc = x2;
                                ch->endurance += x3;
                                ch->max_endurance += x4;
                                fMatch = TRUE;
                                break;
                        }
                        break;

                case 'G':
                        KEY("Glory", ch->pcdata->quest_curr,
                            fread_number(fp));
                        KEY("Gold", ch->gold, fread_number(fp));
                        KEY("GodFlags", ch->pcdata->godflags,
                            fread_number(fp));
                        break;

                case 'H':
                        if (!str_cmp(word, "Helled"))
                        {
                                ch->pcdata->release_date = fread_number(fp);
                                ch->pcdata->helled_by = fread_string(fp);
                                if (ch->pcdata->release_date < current_time)
                                {
                                        STRFREE(ch->pcdata->helled_by);
                                        ch->pcdata->helled_by = NULL;
                                        ch->pcdata->release_date = 0;
                                }
                                fMatch = TRUE;
                                break;
                        }

                        KEY("Hitroll", ch->hitroll, fread_number(fp));
                        KEY("Homepage", ch->pcdata->homepage,
                            fread_string(fp));
                        KEY("Hair", ch->pcdata->hair, fread_number(fp));
                        KEY("Height", ch->pcdata->height, fread_number(fp));

                        if (!str_cmp(word, "HpManaMove"))
                        {
                                line = fread_line(fp);
                                x1 = x2 = x3 = x4 = x5 = x6 = 0;
                                sscanf(line, "%d %d %d %d %d %d",
                                       &x1, &x2, &x3, &x4, &x5, &x6);
                                ch->hit = x1;
                                ch->max_hit = x2;
                                ch->endurance += x5;
                                ch->max_endurance += x6;
                                if (x4 >= 100)
                                {
                                        ch->perm_frc = number_range(1, 20);
                                        ch->max_endurance += x4;
                                        ch->endurance += x4;
                                }
                                else if (x4 >= 10)
                                {
                                        ch->perm_frc = 1;
                                        ch->max_endurance += x4;
                                }
                                fMatch = TRUE;
                                break;
                        }

                        break;

                case 'I':
                        KEY("IllegalPK", ch->pcdata->illegal_pk,
                            fread_number(fp));
                        KEY("Illness", ch->pcdata->illness, fread_number(fp));
                        KEY("Immune", ch->immune, fread_number(fp));
                        KEY("Icq", ch->pcdata->icq, fread_string(fp));
                        if (!str_cmp(word, "Ignore"))
                        {
                                char     *temp = fread_string(fp);

                                fMatch = TRUE;
                                if (char_exists(temp))
                                {
                                        ch->pcdata->ignore[count] = temp;
                                        count++;
                                }
                                else
                                        STRFREE(temp);
                                break;
                        }
                        if (!str_cmp(word, "Implant"))
                        {
                                int       number = 0;

                                number = fread_number(fp);
                                ch->pcdata->implants[number] =
                                        fread_number(fp);
                                if (ch->pcdata->implants[number] > 3
                                    || ch->pcdata->implants[number] < 0)
                                        ch->pcdata->implants[number] = -2;
                                fMatch = TRUE;
                                break;
                        }
#ifdef IMC
                        if ((fMatch = imc_loadchar(ch, fp, word)))
                                break;
#endif
                        break;

                case 'Y':
                        KEY("Yahoo", ch->pcdata->yahoo, fread_string(fp));

                case 'K':
                        if (!str_cmp(word, "Killed"))
                        {
                                fMatch = TRUE;
                                if (killcnt >= MAX_KILLTRACK)
                                        bug("fread_char: killcnt (%d) >= MAX_KILLTRACK", killcnt);
                                else
                                {
                                        ch->pcdata->killed[killcnt].vnum =
                                                fread_number(fp);
                                        ch->pcdata->killed[killcnt++].count =
                                                fread_number(fp);
                                }
                        }
                        break;

                case 'L':
                        if (!str_cmp(word, "Lastplayed"))
                        {
                                lastplayed = fread_number(fp);
                                fMatch = TRUE;
                                break;
                        }
                        KEY("LongDescr", ch->long_descr, fread_string(fp));
                        KEY("Leader", ch->groupleader, fread_string(fp));
                        KEY("Listening", ch->pcdata->listening,
                            fread_string(fp));
                        if (!str_cmp(word, "Languages"))
                        {
                                if (file_ver < 5)
                                        fread_number(fp);
                                if (file_ver < 5)
                                {
                                        ch->speaking = ch->race->language();
                                        fread_number(fp);
                                }
                                else
                                {
                                        char     *language;

                                        language = fread_string(fp);
                                        ch->speaking = get_language(language);
                                        STRFREE(language);
                                }
                                fMatch = TRUE;
                        }
                        break;

                case 'M':
                        KEY("MainAbility", ch->main_ability,
                            fread_number(fp));
                        KEY("MDeaths", ch->pcdata->mdeaths, fread_number(fp));
                        KEY("MaxColors", max_colors, fread_number(fp));
                        KEY("Mentalstate", ch->mental_state,
                            fread_number(fp));
                        KEY("MGlory", ch->pcdata->quest_accum,
                            fread_number(fp));
                        KEY("Minsnoop", ch->pcdata->min_snoop,
                            fread_number(fp));
                        KEY("MKills", ch->pcdata->mkills, fread_number(fp));
                        KEY("Mobinvis", ch->mobinvis, fread_number(fp));
                        KEY("Msn", ch->pcdata->msn, fread_string(fp));
                        KEY("Master", ch->following, fread_string(fp));
                        if (!str_cmp(word, "MobRange"))
                        {
                                ch->pcdata->m_range_lo = fread_number(fp);
                                ch->pcdata->m_range_hi = fread_number(fp);
                                fMatch = TRUE;
                        }
                        break;

                case 'N':
                        if (!str_cmp(word, "Name"))
                        {
                                /*
                                 * Name already set externally.
                                 */
                                fread_to_eol(fp);
                                fMatch = TRUE;
                                break;
                        }
                        break;

                case 'O':
                        if (!str_cmp(word, "ObjRange"))
                        {
                                ch->pcdata->o_range_lo = fread_number(fp);
                                ch->pcdata->o_range_hi = fread_number(fp);
                                fMatch = TRUE;
                        }
                        break;

                case 'P':
                        KEY("Pagerlen", ch->pcdata->pagerlen,
                            fread_number(fp));
                        KEY("Password", ch->pcdata->pwd,
                            fread_string_nohash(fp));
                        KEY("PDeaths", ch->pcdata->pdeaths, fread_number(fp));
                        KEY("PKills", ch->pcdata->pkills, fread_number(fp));
                        KEY("Played", ch->played, fread_number(fp));
                        KEY("Position", ch->position, fread_number(fp));
                        KEY("Practice", extra, fread_number(fp));
                        KEY("Prompt", ch->pcdata->prompt, fread_string(fp));
                        if (!str_cmp(word, "PTimer"))
                        {
                                add_timer(ch, TIMER_PKILLED, fread_number(fp),
                                          NULL, 0);
                                fMatch = TRUE;
                                break;
                        }
                        if (!str_cmp(word, "PlrHome"))
                        {
                                int       home = fread_number(fp);

                                ch->plr_home = get_room_index(home);
                                if (!ch->plr_home)
                                {
                                        ch->plr_home = NULL;
                                        bug("%s has a home but index was not found:: %d", ch->name, home);
                                }
                                fMatch = TRUE;
                                break;
                        }
                        break;

                case 'Q':
                        KEY("QuestNext", ch->nextquest, fread_number(fp));
                        KEY("Questobj", ch->questobj, fread_number(fp));
                        KEY("Questmob", ch->questmob, fread_number(fp));
                        KEY("Questhp", ch->questhp, fread_number(fp));
                        if (!str_cmp(word, "questgiver"))
                        {
                                char     *temp;

                                temp = fread_string_nohash(fp);
                                ch->questgiver = get_char_world_nocheck(temp);
                                DISPOSE(temp);
                                fMatch = TRUE;
                        }
                        break;

                case 'R':
                        if (!str_cmp(word, "RP"))
                        {
#ifndef ACCOUNT
                                ch->pcdata->rp = fread_number(fp);
#else
                                if (ch->pcdata->account)
                                {
                                        ch->pcdata->account->rppoints +=
                                                fread_number(fp);
                                        ch->pcdata->rp = 0; /* Should this still even be here? - Gavin */
                                }
                                else
                                        ch->pcdata->rp = fread_number(fp);
#endif
                                fMatch = TRUE;
                                break;
                        }
                        KEY("Realage", ch->pcdata->realage, fread_number(fp));
                        KEY("Rank", ch->pcdata->rank, fread_string(fp));
                        KEY("Realname", ch->pcdata->realname,
                            fread_string(fp));
                        KEY("Resistant", ch->resistant, fread_number(fp));
                        KEY("Recall", ch->pcdata->recall, fread_number(fp));
                        KEY("Restore_time", ch->pcdata->restore_time,
                            fread_number(fp));

                        if (!str_cmp(word, "Room"))
                        {
                                ch->in_room =
                                        get_room_index(fread_number(fp));
                                if (!ch->in_room)
                                        ch->in_room =
                                                get_room_index
                                                (ROOM_VNUM_LIMBO);
                                fMatch = TRUE;
                                break;
                        }
                        if (!str_cmp(word, "RoomRange"))
                        {
                                ch->pcdata->r_range_lo = fread_number(fp);
                                ch->pcdata->r_range_hi = fread_number(fp);
                                fMatch = TRUE;
                        }
                        if (!str_cmp(word, "Race"))
                        {
                                if (file_ver >= 4)
                                {
                                        char     *race;

                                        race = fread_string(fp);
                                        ch->race = get_race(race);
                                        STRFREE(race);
                                }
                                else
                                        ch->race =
                                                get_race_number(fread_number
                                                                (fp));
                                fMatch = TRUE;
                        }
                        break;

                case 'S':
                        KEY("Sex", ch->sex, fread_number(fp));
                        KEY("ShortDescr", ch->short_descr, fread_string(fp));
                        KEY("Spouse", ch->pcdata->spouse, fread_string(fp));
                        KEY("Susceptible", ch->susceptible, fread_number(fp));
                        if (!str_cmp(word, "SavingThrow"))
                        {
                                ch->saving_wand = fread_number(fp);
                                ch->saving_poison_death = ch->saving_wand;
                                ch->saving_para_petri = ch->saving_wand;
                                ch->saving_breath = ch->saving_wand;
                                ch->saving_spell_staff = ch->saving_wand;
                                fMatch = TRUE;
                                break;
                        }

                        if (!strcmp(word, "Speed"))
                        {
                                sh_int    speed;

                                fMatch = TRUE;
                                speed = fread_number(fp);
                                if (ch->desc)
                                        ch->desc->speed = speed;
                                ch->speed = speed;
                                break;
                        }

                        if (!str_cmp(word, "SavingThrows"))
                        {
                                ch->saving_poison_death = fread_number(fp);
                                ch->saving_wand = fread_number(fp);
                                ch->saving_para_petri = fread_number(fp);
                                ch->saving_breath = fread_number(fp);
                                ch->saving_spell_staff = fread_number(fp);
                                fMatch = TRUE;
                                break;
                        }

                        if (!str_cmp(word, "Site"))
                        {
                                if (!preload && 1 == 2)
                                {
                                        snprintf(buf, MSL,
                                                 "Last connected from: %s\n\r",
                                                 fread_word(fp));
                                        send_to_char(buf, ch);
                                }
                                else
                                        fread_to_eol(fp);
                                fMatch = TRUE;
                                if (preload)
                                        word = "End";
                                else
                                        break;
                        }

                        if (!str_cmp(word, "Skill"))
                        {
                                int       value;

                                if (preload)
                                        word = "End";
                                else
                                {
                                        value = fread_number(fp);
                                        if (file_ver < 3)
                                                skill_number =
                                                        skill_lookup
                                                        (fread_word(fp));
                                        else
                                                skill_number =
                                                        bsearch_skill_exact
                                                        (fread_word(fp),
                                                         gsn_first_skill,
                                                         gsn_first_weapon -
                                                         1);
                                        if (skill_number < 0)
                                                bug("Fread_char: unknown skill.", 0);
                                        else
                                        {
                                                ch->pcdata->
                                                        learned[skill_number]
                                                        = value;

                                        }
                                        fMatch = TRUE;
                                        break;
                                }
                        }

                        if (!str_cmp(word, "Spell"))
                        {
                                int       sn;
                                int       value;

                                if (preload)
                                        word = "End";
                                else
                                {
                                        value = fread_number(fp);

                                        sn = bsearch_skill_exact(fread_word
                                                                 (fp),
                                                                 gsn_first_spell,
                                                                 gsn_first_skill
                                                                 - 1);
                                        if (sn < 0)
                                                bug("Fread_char: unknown spell.", 0);
                                        else
                                        {
                                                ch->pcdata->learned[sn] =
                                                        value;

                                        }
                                        fMatch = TRUE;
                                        break;
                                }
                        }
                        if (str_cmp(word, "End"))
                                break;

                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!ch->short_descr)
                                        ch->short_descr = STRALLOC("");
                                if (!ch->long_descr)
                                        ch->long_descr = STRALLOC("");
                                if (!ch->description)
                                        ch->description = STRALLOC("");
                                if (!ch->pcdata->pwd)
                                        ch->pcdata->pwd = str_dup("");
                                if (!ch->pcdata->email)
                                        ch->pcdata->email = STRALLOC("");
                                if (!ch->pcdata->spouse)
                                        ch->pcdata->spouse = STRALLOC("");
                                if (!ch->pcdata->bamfin)
                                        ch->pcdata->bamfin = STRALLOC("");
                                if (!ch->pcdata->bamfout)
                                        ch->pcdata->bamfout = STRALLOC("");
                                if (!ch->pcdata->bio)
                                        ch->pcdata->bio = STRALLOC("");
                                if (!ch->pcdata->rank)
                                        ch->pcdata->rank = STRALLOC("");
                                if (!ch->pcdata->bestowments)
                                        ch->pcdata->bestowments =
                                                STRALLOC("");
                                if (!ch->pcdata->title)
                                        ch->pcdata->title = STRALLOC("");
                                if (!ch->pcdata->homepage)
                                        ch->pcdata->homepage = STRALLOC("");
                                if (!ch->pcdata->authed_by)
                                        ch->pcdata->authed_by = STRALLOC("");
                                if (!ch->pcdata->prompt)
                                        ch->pcdata->prompt = STRALLOC("");
                                if (!ch->pcdata->full_name)
                                        ch->pcdata->full_name =
                                                STRALLOC(ch->name);
                                if (!ch->pcdata->birthday.hour)
                                        ch->pcdata->birthday.hour =
                                                time_info.hour;
                                if (!ch->pcdata->birthday.day)
                                        ch->pcdata->birthday.day =
                                                time_info.day;
                                if (!ch->pcdata->birthday.month)
                                        ch->pcdata->birthday.month =
                                                time_info.month;
                                if (!ch->pcdata->birthday.year)
                                        ch->pcdata->birthday.year =
                                                time_info.year;
                                if (!ch->pcdata->age)
                                        ch->pcdata->age = get_age_old(ch);
                                if (!ch->pcdata->listening)
                                        ch->pcdata->listening = STRALLOC("");
                                if (ch->pcdata->listening[0] == '\0') {
                                        for (channel = first_channel; channel;channel = channel->next)
                                        {
                                                if (ch->top_level >= channel->level && !hasname(ch->pcdata->listening,channel->name))
                                                        addname(&ch->pcdata->listening,channel->name);
                                        }
                                }
                                else {
                                        for (channel = first_channel; channel;channel = channel->next)
                                        {
                                                // Time to sanatize data
                                                if (channel->level > ch->top_level && hasname(ch->pcdata->listening, channel->name))
                                                {
                                                        removename(&ch->pcdata->listening, channel->name);
                                                }
                                        }
                                }
#ifdef ACCOUNT
                                if (ch->pcdata->account)
                                {
                                        ch->pcdata->account->
                                                rppoints +=
                                                ch->pcdata->rp;
                                        ch->pcdata->rp = 0; /* Should this still even be here? - Gavin */
                                }
                                else if (ch->desc->account)
                                {
                                        ch->desc->account->rppoints +=
                                                ch->pcdata->rp;
                                        ch->pcdata->rp = 0; /* Should this still even be here? - Gavin */
                                }

#endif
                                if (ch->pcdata)
                                        ch->pcdata->arousal = 0;
                                ch->editor = NULL;
                                killcnt =
                                        URANGE(2,
                                               ((ch->top_level +
                                                 3) * MAX_KILLTRACK) /
                                               LEVEL_AVATAR, MAX_KILLTRACK);
                                if (killcnt < MAX_KILLTRACK)
                                        ch->pcdata->killed[killcnt].vnum = 0;
                                {
                                        int       ability;

                                        for (ability = 0;
                                             ability < MAX_ABILITY; ability++)
                                        {
                                                if (ch->
                                                    skill_level[ability] == 0)
                                                        ch->skill_level
                                                                [ability] = 1;
                                        }
                                }
                                if (!IS_IMMORTAL(ch) && !ch->speaking)
                                        ch->speaking = ch->race->language();
                                if (IS_IMMORTAL(ch))
                                        ch->speaking = ch->race->language();

                                if (!ch->speaking
                                    || ch->speaking->name[0] == '0')
                                        ch->speaking = ch->race->language();
                                if (!ch->pcdata->prompt)
                                        ch->pcdata->prompt = STRALLOC("");

                                if (lastplayed != 0)
                                {
                                        int       hitgain;

                                        hitgain =
                                                ((int)
                                                 (current_time -
                                                  lastplayed) / 60);
                                        ch->hit =
                                                URANGE(1, ch->hit + hitgain,
                                                       ch->max_hit);
                                        ch->endurance =
                                                URANGE(1,
                                                       ch->endurance +
                                                       hitgain,
                                                       ch->max_endurance);

                                        better_mental_state(ch, hitgain);
                                }
                                for (skill_number = 0; skill_number < top_sn;
                                     skill_number++)
                                {
                                        if (!skill_table[skill_number]->name)
                                                break;

                                        if (skill_table[skill_number]->guild <
                                            0
                                            || skill_table[skill_number]->
                                            guild >= MAX_ABILITY)
                                                continue;

                                        if (ch->pcdata->
                                            learned[skill_number] > 0
                                            && ch->
                                            skill_level[skill_table
                                                        [skill_number]->
                                                        guild] <
                                            skill_table[skill_number]->
                                            min_level)
                                                ch->pcdata->
                                                        learned[skill_number]
                                                        = 0;

                                }
                                return;
                        }
                        KEY("Email", ch->pcdata->email, fread_string(fp));
                        KEY("Eyes", ch->pcdata->eye, fread_number(fp));
                        break;

                case 'T':
                        KEY("Toplevel", ch->top_level, fread_number(fp));
                        if (!str_cmp(word, "Tongue"))
                        {
                                int       sn;
                                int       value;

                                if (preload)
                                        word = "End";
                                else
                                {
                                        value = fread_number(fp);

                                        sn = bsearch_skill_exact(fread_word
                                                                 (fp),
                                                                 gsn_first_tongue,
                                                                 gsn_top_sn -
                                                                 1);
                                        if (sn < 0)
                                                bug("Fread_char: unknown tongue.", 0);
                                        else
                                        {
                                                ch->pcdata->learned[sn] =
                                                        value;

                                        }
                                        fMatch = TRUE;
                                }
                                break;
                        }
                        KEY("Trust", ch->trust, fread_number(fp));
                        /*
                         * Let no character be trusted higher than one below maxlevel -- Narn 
                         */
                        ch->trust = UMIN(ch->trust, MAX_LEVEL - 1);

                        if (!str_cmp(word, "Title"))
                        {
                                char     *temp = fread_string_noalloc(fp);

                                set_title(ch, temp);
                                fMatch = TRUE;
                                break;
                        }

                        break;

                case 'V':
                        if (!str_cmp(word, "Vnum"))
                        {
                                ch->pIndexData =
                                        get_mob_index(fread_number(fp));
                                fMatch = TRUE;
                                break;
                        }
                        KEY("Version", file_ver, fread_number(fp));
                        break;

                case 'W':
                        if (!str_cmp(word, "Weapon"))
                        {
                                int       sn;
                                int       value;

                                if (preload)
                                        word = "End";
                                else
                                {
                                        value = fread_number(fp);

                                        sn = bsearch_skill_exact(fread_word
                                                                 (fp),
                                                                 gsn_first_weapon,
                                                                 gsn_first_tongue
                                                                 - 1);
                                        if (sn < 0)
                                                bug("Fread_char: unknown weapon.", 0);
                                        else
                                        {
                                                ch->pcdata->learned[sn] =
                                                        value;

                                        }
                                        fMatch = TRUE;
                                }
                                break;
                        }
                        KEY("Wimpy", ch->wimpy, fread_number(fp));
                        KEY("Weight", ch->pcdata->weight, fread_number(fp));
                        KEY("WizInvis", ch->pcdata->wizinvis,
                            fread_number(fp));
                        if (!str_cmp(word, "Wanted"))
                        {
                                int       temp = 0;

                                temp = fread_number(fp);
                                if (!preload)
                                {
                                        if (IS_SET(temp, WANTED_MON_CALAMARI))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Calamari"));
                                        if (IS_SET(temp, WANTED_CORUSCANT))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Coruscant"));
                                        if (IS_SET(temp, WANTED_ADARI))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Adari"));
                                        if (IS_SET(temp, WANTED_RODIA))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Rodia"));
                                        if (IS_SET(temp, WANTED_RYLOTH))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Ryloth"));
                                        if (IS_SET(temp, WANTED_GAMORR))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Gamorr"));
                                        if (IS_SET(temp, WANTED_TATOOINE))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Tatooine"));
                                        if (IS_SET(temp, WANTED_BYSS))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Byss"));
                                        if (IS_SET(temp, WANTED_NAL_HUTTA))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Nal Hutta"));
                                        if (IS_SET(temp, WANTED_KASHYYYK))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Kashyyyk"));
                                        if (IS_SET(temp, WANTED_HONOGHR))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Honoghr"));
                                        if (IS_SET(temp, WANTED_ENDOR))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Endor"));
                                        if (IS_SET(temp, WANTED_ROCHE))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Roche"));
                                        if (IS_SET(temp, WANTED_AF_EL))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Af El"));
                                        if (IS_SET(temp, WANTED_TRANDOSHA))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Trandosha"));
                                        if (IS_SET(temp, WANTED_CHAD))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Chad"));
                                        if (IS_SET(temp, WANTED_HOTH))
                                                add_wanted(ch,
                                                           get_planet
                                                           ("Hoth"));
                                }
                                fMatch = TRUE;
                        }
                        break;
                }

                if (!fMatch)
                {
                        snprintf(buf, MSL, "Fread_char: no match: %s", word);
                        bug(buf, 0);
                }
        }
}


void fread_obj(CHAR_DATA * ch, FILE * fp, sh_int os_type)
{
        OBJ_DATA *obj;
        const char *word;
        int       iNest;
        bool      fMatch;
        bool      fNest;
        bool      fVnum;
        ROOM_INDEX_DATA *room = NULL;

        if (ch)
        {
                room = ch->in_room;
                if (ch->tempnum == -9999)
                        file_ver = 0;
        }

        CREATE(obj, OBJ_DATA, 1);
        obj->count = 1;
        obj->wear_loc = -1;
        obj->weight = 1;

        fNest = TRUE;   /* Requiring a Nest 0 is a waste */
        fVnum = TRUE;
        iNest = 0;

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
                        KEY("ArmedBy", obj->armed_by, fread_string(fp));
                        if (!str_cmp(word, "Affect")
                            || !str_cmp(word, "AffectData"))
                        {
                                AFFECT_DATA *paf;
                                int       pafmod;

                                CREATE(paf, AFFECT_DATA, 1);
                                if (!str_cmp(word, "Affect"))
                                {
                                        paf->type = fread_number(fp);
                                }
                                else
                                {
                                        int       sn;

                                        sn = skill_lookup(fread_word(fp));
                                        if (sn < 0)
                                                bug("Fread_obj: unknown skill.", 0);
                                        else
                                                paf->type = sn;
                                }
                                paf->duration = fread_number(fp);
                                pafmod = fread_number(fp);
                                paf->location = fread_number(fp);
                                paf->bitvector = fread_number(fp);
                                if (paf->location == APPLY_WEAPONSPELL
                                    || paf->location == APPLY_WEARSPELL
                                    || paf->location == APPLY_REMOVESPELL)
                                        paf->modifier = slot_lookup(pafmod);
                                else
                                        paf->modifier = pafmod;
                                LINK(paf, obj->first_affect, obj->last_affect,
                                     next, prev);
                                fMatch = TRUE;
                                break;
                        }
                        KEY("Actiondesc", obj->action_desc, fread_string(fp));
                        break;

                case 'C':
                        KEY("Cost", obj->cost, fread_number(fp));
                        KEY("Count", obj->count, fread_number(fp));
                        break;

                case 'D':
                        KEY("Description", obj->description,
                            fread_string(fp));
                        break;

                case 'E':
                        KEY("ExtraFlags", obj->extra_flags, fread_number(fp));

                        if (!str_cmp(word, "ExtraDescr"))
                        {
                                EXTRA_DESCR_DATA *ed;

                                CREATE(ed, EXTRA_DESCR_DATA, 1);
                                ed->keyword = fread_string(fp);
                                ed->description = fread_string(fp);
                                LINK(ed, obj->first_extradesc,
                                     obj->last_extradesc, next, prev);
                                fMatch = TRUE;
                        }

                        if (!str_cmp(word, "End"))
                        {

                                if (!fNest || !fVnum)
                                {
                                        bug("Fread_obj: incomplete object.",
                                            0);
                                        if (obj->name)
                                                STRFREE(obj->name);
                                        if (obj->description)
                                                STRFREE(obj->description);
                                        if (obj->short_descr)
                                                STRFREE(obj->short_descr);
                                        DISPOSE(obj);
                                        return;
                                }
                                else
                                {
                                        sh_int    wear_loc = obj->wear_loc;

                                        if (!obj->name)
                                                obj->name =
                                                        QUICKLINK(obj->
                                                                  pIndexData->
                                                                  name);
                                        if (!obj->description)
                                                obj->description =
                                                        QUICKLINK(obj->
                                                                  pIndexData->
                                                                  description);
                                        if (!obj->short_descr)
                                                obj->short_descr =
                                                        QUICKLINK(obj->
                                                                  pIndexData->
                                                                  short_descr);
                                        if (!obj->action_desc)
                                                obj->action_desc =
                                                        QUICKLINK(obj->
                                                                  pIndexData->
                                                                  action_desc);
                                        if (!obj->armed_by)
                                                obj->armed_by = STRALLOC("");
                                        LINK(obj, first_object, last_object,
                                             next, prev);
                                        obj->pIndexData->count += obj->count;
                                        if (fNest)
                                                rgObjNest[iNest] = obj;
                                        numobjsloaded += obj->count;
                                        physicalobjects++;
                                        if (file_ver > 1 || obj->wear_loc < -1
                                            || obj->wear_loc >= MAX_WEAR)
                                                obj->wear_loc = -1;
                                        /*
                                         * Corpse saving. -- Altrag 
                                         */
                                        if (os_type == OS_CORPSE)
                                        {
                                                if (!room)
                                                {
                                                        bug("Fread_obj: Corpse without room", 0);
                                                        room = get_room_index
                                                                (ROOM_VNUM_LIMBO);
                                                }
                                                obj = obj_to_room(obj, room);
                                        }
                                        else if (iNest == 0
                                                 || rgObjNest[iNest] == NULL)
                                        {
                                                int       slot = 0;
                                                bool      reslot = FALSE;

                                                if (file_ver > 1
                                                    && wear_loc > -1
                                                    && wear_loc < MAX_WEAR)
                                                {
                                                        int       x;

                                                        for (x = 0;
                                                             x < MAX_LAYERS;
                                                             x++)
                                                                if (!save_equipment[wear_loc][x])
                                                                {
                                                                        save_equipment
                                                                                [wear_loc]
                                                                                [x]
                                                                                =
                                                                                obj;
                                                                        slot = x;
                                                                        reslot = TRUE;
                                                                        break;
                                                                }
                                                        if (x == MAX_LAYERS)
                                                                bug("Fread_obj: too many layers %d", wear_loc);
                                                }
                                                obj = obj_to_char(obj, ch);
                                                if (reslot)
                                                        save_equipment
                                                                [wear_loc]
                                                                [slot] = obj;
                                        }
                                        else
                                        {
                                                if (rgObjNest[iNest - 1])
                                                {
                                                        separate_obj(rgObjNest
                                                                     [iNest -
                                                                      1]);
                                                        obj = obj_to_obj(obj,
                                                                         rgObjNest
                                                                         [iNest
                                                                          -
                                                                          1]);
                                                }
                                                else
                                                        bug("Fread_obj: nest layer missing %d", iNest - 1);
                                        }
                                        if (fNest)
                                                rgObjNest[iNest] = obj;
                                        return;
                                }
                        }
                        break;

                case 'I':
                        KEY("ItemType", obj->item_type, fread_number(fp));
                        break;

                case 'L':
                        KEY("Level", obj->level, fread_number(fp));
                        break;

                case 'N':
                        KEY("Name", obj->name, fread_string(fp));

                        if (!str_cmp(word, "Nest"))
                        {
                                iNest = fread_number(fp);
                                if (iNest < 0 || iNest >= MAX_NEST)
                                {
                                        bug("Fread_obj: bad nest %d.", iNest);
                                        iNest = 0;
                                        fNest = FALSE;
                                }
                                fMatch = TRUE;
                        }
                        break;

                case 'R':
                        KEY("Room", room, get_room_index(fread_number(fp)));
                        KEY("Rvnum", obj->room_vnum, fread_number(fp));

                case 'S':
                        KEY("ShortDescr", obj->short_descr, fread_string(fp));

                        if (!str_cmp(word, "Spell"))
                        {
                                int       iValue;
                                int       sn;

                                iValue = fread_number(fp);
                                sn = skill_lookup(fread_word(fp));
                                if (iValue < 0 || iValue > 5)
                                        bug("Fread_obj: bad iValue %d.",
                                            iValue);
                                else if (sn < 0)
                                        bug("Fread_obj: unknown skill.", 0);
                                else
                                        obj->value[iValue] = sn;
                                fMatch = TRUE;
                                break;
                        }

                        break;

                case 'T':
                        KEY("Timer", obj->timer, fread_number(fp));
                        break;

                case 'V':
                        if (!str_cmp(word, "Values"))
                        {
                                int       x1, x2, x3, x4, x5, x6;
                                char     *ln = fread_line(fp);

                                x1 = x2 = x3 = x4 = x5 = x6 = 0;
                                sscanf(ln, "%d %d %d %d %d %d", &x1, &x2, &x3,
                                       &x4, &x5, &x6);
                                /*
                                 * clean up some garbage 
                                 */

                                obj->value[0] = x1;
                                obj->value[1] = x2;
                                obj->value[2] = x3;
                                obj->value[3] = x4;
                                obj->value[4] = x5;
                                obj->value[5] = x6;
                                fMatch = TRUE;
                                break;
                        }

                        if (!str_cmp(word, "Vnum"))
                        {
                                int       vnum;

                                vnum = fread_number(fp);
                                if ((obj->pIndexData =
                                     get_obj_index(vnum)) == NULL)
                                {
                                        fVnum = FALSE;
                                        bug("Fread_obj: bad vnum %d.", vnum);
                                }
                                else
                                {
                                        fVnum = TRUE;
                                        obj->cost = obj->pIndexData->cost;
                                        obj->weight = obj->pIndexData->weight;
                                        obj->item_type =
                                                obj->pIndexData->item_type;
                                        obj->wear_flags =
                                                obj->pIndexData->wear_flags;
                                        obj->extra_flags =
                                                obj->pIndexData->extra_flags;
                                }
                                fMatch = TRUE;
                                break;
                        }
                        break;

                case 'W':
                        KEY("WearFlags", obj->wear_flags, fread_number(fp));
                        KEY("WearLoc", obj->wear_loc, fread_number(fp));
                        KEY("Weight", obj->weight, fread_number(fp));
                        break;

                }

                if (!fMatch)
                {
                        EXTRA_DESCR_DATA *ed;
                        AFFECT_DATA *paf;

                        bug("Fread_obj: no match.", 0);
                        bug(word, 0);
                        fread_to_eol(fp);
                        if (obj->name)
                                STRFREE(obj->name);
                        if (obj->description)
                                STRFREE(obj->description);
                        if (obj->short_descr)
                                STRFREE(obj->short_descr);
                        while ((ed = obj->first_extradesc) != NULL)
                        {
                                STRFREE(ed->keyword);
                                STRFREE(ed->description);
                                UNLINK(ed, obj->first_extradesc,
                                       obj->last_extradesc, next, prev);
                                DISPOSE(ed);
                        }
                        while ((paf = obj->first_affect) != NULL)
                        {
                                UNLINK(paf, obj->first_affect,
                                       obj->last_affect, next, prev);
                                DISPOSE(paf);
                        }
                        DISPOSE(obj);
                        return;
                }
        }
}

void set_alarm(long seconds)
{
        alarm(seconds);
}

/*
 * Based on last time modified, show when a player was last on	-Thoric
 */
CMDF do_last(CHAR_DATA * ch, char *argument)
{
        char      buf[MAX_STRING_LENGTH];
        char      arg[MAX_INPUT_LENGTH];
        char      name[MAX_INPUT_LENGTH];
        struct stat fst;

        one_argument(argument, arg);
        if (arg[0] == '\0')
        {
                send_to_char("Usage: last <playername>\n\r", ch);
                return;
        }
        mudstrlcpy(name, capitalize(arg), MIL);
        snprintf(buf, MSL, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), name);
        if (stat(buf, &fst) != -1)
                snprintf(buf, MSL, "%s was last on: %s\r", name,
                         ctime(&fst.st_mtime));
        else
                snprintf(buf, MSL, "%s was not found.\n\r", name);
        send_to_char(buf, ch);
}

void write_corpses(CHAR_DATA * ch, char *name)
{
        OBJ_DATA *corpse;
        FILE     *fp = NULL;

        /*
         * Name and ch support so that we dont have to have a char to save their
         * corpses.. (ie: decayed corpses while offline) 
         */
        if (ch && IS_NPC(ch))
        {
                bug("Write_corpses: writing NPC corpse.", 0);
                return;
        }
        if (ch)
                name = ch->name;
        /*
         * Go by vnum, less chance of screwups. -- Altrag 
         */
        for (corpse = first_object; corpse; corpse = corpse->next)
                if (corpse->pIndexData->vnum == OBJ_VNUM_CORPSE_PC &&
                    corpse->in_room != NULL &&
                    !str_cmp(corpse->short_descr + 14, name))
                {
                        if (!fp)
                        {
                                char      buf[127];

                                snprintf(buf, 127, "%s%s", CORPSE_DIR,
                                         capitalize(name));
                                if (!(fp = fopen(buf, "w")))
                                {
                                        bug("Write_corpses: Cannot open file.", 0);
                                        perror(buf);
                                        return;
                                }
                        }
                        fwrite_obj(ch, corpse, fp, 0, OS_CORPSE, FALSE);
                }
        if (fp)
        {
                fprintf(fp, "#END\n\n");
                FCLOSE(fp);
        }
        else
        {
                char      buf[127];

                snprintf(buf, MSL, "%s%s", CORPSE_DIR, capitalize(name));
                remove(buf);
        }
        return;
}

void load_corpses(void)
{
        DIR      *dp;
        struct dirent *de;
        extern FILE *fpArea;
        extern char strArea[MAX_INPUT_LENGTH];
        extern int falling;

        if (!(dp = opendir(CORPSE_DIR)))
        {
                bug("Load_corpses: can't open CORPSE_DIR", 0);
                perror(CORPSE_DIR);
                return;
        }

        falling = 1;    /* Arbitrary, must be >0 though. */
        while ((de = readdir(dp)) != NULL)
        {
                if (de->d_name[0] != '.')
                {
                        snprintf(strArea, MSL, "%s%s", CORPSE_DIR,
                                 de->d_name);
                        fprintf(stderr, "Corpse -> %s\n", strArea);
                        if (!(fpArea = fopen(strArea, "r")))
                        {
                                perror(strArea);
                                continue;
                        }
                        for (;;)
                        {
                                char      letter;
                                char     *word;

                                letter = fread_letter(fpArea);
                                if (letter == '*')
                                {
                                        fread_to_eol(fpArea);
                                        continue;
                                }
                                if (letter != '#')
                                {
                                        bug("Load_corpses: # not found.", 0);
                                        break;
                                }
                                word = fread_word(fpArea);
                                if (!str_cmp(word, "CORPSE"))
                                        fread_obj(NULL, fpArea, OS_CORPSE);
                                else if (!str_cmp(word, "OBJECT"))
                                        fread_obj(NULL, fpArea, OS_CARRY);
                                else if (!str_cmp(word, "END"))
                                        break;
                                else
                                {
                                        bug("Load_corpses: bad section.", 0);
                                        break;
                                }
                        }
                        FCLOSE(fpArea);
                        fpArea = NULL;
                }
        }
        mudstrlcpy(strArea, "$", MIL);
        closedir(dp);
        falling = 0;
        boot_log(" Done corpses ");
        return;
}

void load_vendors(void)
{
        DIR      *dp;
        CHAR_DATA *mob = NULL;
        struct dirent *de;
        extern FILE *fpArea;
        extern char strArea[MAX_INPUT_LENGTH];
        extern int falling;

        if (!(dp = opendir(VENDOR_DIR)))
        {
                bug("Load_vendors: can't open VENDOR_DIR", 0);
                perror(VENDOR_DIR);
                return;
        }

        falling = 1;
        while ((de = readdir(dp)) != NULL)
        {
                if (de->d_name[0] != '.')
                {
                        snprintf(strArea, MSL, "%s%s", VENDOR_DIR,
                                 de->d_name);
                        if (!(fpArea = fopen(strArea, "r")))
                        {
                                perror(strArea);
                                continue;
                        }
                        for (;;)
                        {
                                char      letter;
                                char     *word;

                                letter = fread_letter(fpArea);
                                if (letter == '*')
                                {
                                        fread_to_eol(fpArea);
                                        continue;
                                }
                                if (letter != '#')
                                {
                                        bug("Load_vendor: # not found.", 0);
                                        break;
                                }
                                word = fread_word(fpArea);
                                if (!strcmp(word, "VENDOR"))
                                        mob = fread_vendor(fpArea);
                                else if (!strcmp(word, "OBJECT"))
                                        fread_obj(mob, fpArea, OS_CARRY);
                                else if (!strcmp(word, "END"))
                                        break;
                        }
                        FCLOSE(fpArea);
                        fpArea = NULL;
                }
        }
        mudstrlcpy(strArea, "$", MIL);
        closedir(dp);
        falling = 0;
        boot_log(" Done vendors ");
        return;
}
