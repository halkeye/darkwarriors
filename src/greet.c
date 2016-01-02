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

#include "mud.h"
#include "greet.h"
#include "races.h"

GREET_INFO * get_greet(CHAR_DATA * ch, CHAR_DATA * victim);
bool isavowel(char letter);
char     *aoran(const char *str);

CMDF do_remember(CHAR_DATA * ch, char * argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA * victim = NULL;
        
        argument = one_argument(argument, arg);
        if ((victim = get_char_room(ch, arg)) == NULL)
        {
                send_to_char("Who?\n\r", ch);
                return;
        }
        if (argument[0] == '\0') {
                send_to_char("Remember as what?\n", ch);
                return;
        }
        add_greet_to_char(ch, victim, argument);
        ch_printf(ch, "You now remember them as %s", argument);
}

/*
 * This is how you get players in the room too
 *
 * so take into account remembered info
 *
 * then to a str_prefix or nifty is name or something on get_char_here
 * */
char * get_char_desc(CHAR_DATA * ch, CHAR_DATA * looker)
{
        static char desc[MSL];
        char temp_desc[MSL];
        char cha_desc[25];
        char str_desc[25];
        int cha =  get_curr_cha(ch);
        int str =  get_curr_str(ch);

        // DISABLE IT
        if (!sysdata.GREET || IS_IMMORTAL(looker) || IS_NPC(ch))
        {
                strcpy(desc, IS_NPC(ch) ? ch->name : ch->pcdata->full_name);
                return desc;
        }
        /* Hash name + Hash toplayer of clothing (ordering?) 
         * If hash exists, you know them quite well
         * if hashname exists, you are familiar but not quite sure (depends on int)
         * */
                

        temp_desc[0] = desc[0] = '\0';
        cha_desc[0] = str_desc[0] = '\0';
        if (looker && !IS_NPC(looker)) {
                GREET_INFO * info = get_greet(looker, ch);
                if (info)
                {
                        strcpy(desc, info->remembered_name);
                        return desc;
                }
        }

        if (looker && looker->in_room == ch->in_room) 
        {
                if (cha >= 3 && cha <= 9) {
                        strcpy(cha_desc, "ugly");
                }
                else if (cha >= 19 && cha <= 22) {
                        strcpy(cha_desc, "handsome");
                }
                else if (cha >= 23 && cha <= 27) {
                        strcpy(cha_desc, "stunning");
                }
                else if (cha >= 28 && cha <= 40) {
                        strcpy(cha_desc, "amazing");
                }

                if (str >= 3 && str <= 9) {
                        strcpy(str_desc, "weak");
                }
                else if (str >= 19 && str <= 22) {
                        strcpy(str_desc, "strong");
                }
                else if (str >= 23 && str <= 27) {
                        strcpy(str_desc, "brave");
                }
                else if (str >= 28 && str <= 40) {
                        strcpy(str_desc, "heroic");
                }
        }

        if (cha_desc[0] != '\0')
                strcat(temp_desc,cha_desc);

        if (str_desc[0] != '\0') 
        {
                if (temp_desc[0] != '\0') 
                        strcat(temp_desc, ", ");
                strcat(temp_desc, str_desc);
        }

        if (temp_desc[0] != '\0') 
                strcat(temp_desc, " ");

        if (ch->sex >= 0 && ch->sex < SEX_MAX)
                strcat(temp_desc, strlower(npc_sex[ch->sex]));
        strcat(temp_desc, " ");
        strcat(temp_desc, strlower(ch->race->name()));
        
        strcpy(desc, aoran(temp_desc));
        return desc;
}

GREET_INFO * get_greet(CHAR_DATA * ch, CHAR_DATA * victim)
{
        if (!ch->pcdata->greet_info)
                ch->pcdata->greet_info = new temp_greet_ptr;
        else {
                GREET_MAP::iterator i = ch->pcdata->greet_info->greet_info.find(victim->name);
                if ( i != ch->pcdata->greet_info->greet_info.end())
                        return (i->second);
        }

        return NULL;
}
bool has_greet(CHAR_DATA * ch, CHAR_DATA * victim)
{
        if (get_greet(ch, victim))
                return TRUE;
        return FALSE;
}

/* should be CHAR_DATA->add_greet */
void add_greet_to_char(CHAR_DATA * ch, CHAR_DATA * victim, char * name) 
{
        GREET_INFO * greetinfo;
        if (!ch->pcdata->greet_info)
                ch->pcdata->greet_info = new temp_greet_ptr;

        if (!name || name[0] == '\0') 
                return;


        // key is char->name or new identity
        if (has_greet(ch, victim))
        {
                GREET_MAP::iterator i = ch->pcdata->greet_info->greet_info.find(victim->name);

                return;
        }
        greetinfo = new GREET_INFO;
        greetinfo->remembered_name = STRALLOC(name);
        greetinfo->key = STRALLOC(victim->name);
        greetinfo->char_name = STRALLOC(victim->name);
        ch->pcdata->greet_info->greet_info.insert(std::make_pair(victim->name, greetinfo));
        return;
}

void fwrite_greet(CHAR_DATA * ch, FILE * fp) 
{
        if (!ch->pcdata->greet_info)
                return;

        GREET_MAP::iterator i;
        GREET_INFO * info;

        for( i = ch->pcdata->greet_info->greet_info.begin(); i != ch->pcdata->greet_info->greet_info.end(); ++i)
        {
                fprintf(fp, "#GREET\n");
                char * key = i->first;
                info = i->second;
                fprintf(fp, "LastSeen   %d\n", info->last_seen);
                fprintf(fp, "LastHeard  %d\n", info->last_heard);
                fprintf(fp, "Key        %s~\n", key);
                fprintf(fp, "CharName   %s~\n", info->char_name);
                fprintf(fp, "RememberedName   %s~\n", info->remembered_name);
                fprintf(fp, "End\n\n");
        }
        return;
}

void fread_greet(CHAR_DATA * ch, FILE * fp)
{

        char     *charname, *key, *rememberedname;
        int      lastseen, lastheard;
        char     *line;
        const char *word;
        bool      fMatch;

        if (!ch->pcdata->greet_info)
                ch->pcdata->greet_info = new temp_greet_ptr;

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
                
                case 'C':
                        KEY("CharName",  charname,  fread_string(fp));
                        break;

                case 'L':
                        KEY("LastSeen",  lastseen,  fread_number(fp));
                        KEY("LastHeard", lastheard, fread_number(fp));
                        break;

                case 'K':
                        KEY("Key",       key,       fread_string(fp));
                        break;

                case 'E':
                        if (!str_cmp(word, "End")) {
                                // create and add greet info
                                GREET_INFO * greetinfo = new GREET_INFO;
                                greetinfo->last_seen = lastseen;
                                greetinfo->last_heard = lastheard;
                                greetinfo->remembered_name = rememberedname;
                                greetinfo->key = key;
                                greetinfo->char_name = charname;
                                ch->pcdata->greet_info->greet_info.insert(std::make_pair(key, greetinfo));
                                return;
                                // Free if not used
                        }
                        break;

                case 'R':
                        KEY("RememberedName",  rememberedname,  fread_string(fp));
                        break;
                }
                
                if (!fMatch)
                        bug("Fread_char: no match: %s", word);
        }
        return;
}
