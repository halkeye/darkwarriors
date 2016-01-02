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
 *                $Id: olc.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#include <stdio.h>
#include "mud.h"
#include "races.h"

PROTOSHIP_DATA *first_protoship;
PROTOSHIP_DATA *last_protoship;
LANGUAGE_DATA *first_language;
LANGUAGE_DATA *last_language;
CLASS_DATA *first_class;
CLASS_DATA *last_class;
PROTOSHIP_DATA *first_list;
PROTOSHIP_DATA *last_list;
ILLNESS_DATA *first_illness;
ILLNESS_DATA *last_illness;

/* Local Routines */
void write_protoship_list args((void));
int get_partflag args((char *flag));

#ifdef IMAGES
bool      shipimage_exist(char *name);
#endif

#define  NULLSTR( str )  ( str == NULL || str[0] == '\0' )



/*
 * LANGUAGES
 */

void fwrite_language_list(void)
{
        FILE     *fp;
        char      filename[256];
        LANGUAGE_DATA *language;

        snprintf(filename, 256, "%s%s", RACES_DIR, LANGUAGE_LIST);
        fp = fopen(filename, "w");
        if (!fp)
        {
                bug("FATAL: cannot open language.lst for writing!\n\r", 0);
                return;
        }
        for (language = first_language; language; language = language->next)
                fprintf(fp, "%s.language\n", smash_space(language->name));
        fprintf(fp, "$\n");
        FCLOSE(fp);
}

void fread_language(LANGUAGE_DATA * language, FILE * fp)
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
                case 'E':
                        if (!str_cmp(word, "End"))
                                return;
                        break;
                case 'M':
                        KEY("Min_Intelligence", language->min_intelligence,
                            fread_number(fp));
                        break;
                case 'N':
                        KEY("Name", language->name, fread_string(fp));
                        break;
                }
                if (!fMatch)
                {
                        snprintf(buf, MSL, "Fread_language: no match: %s",
                                 word);
                        bug(buf, 0);
                }
        }
}


void fwrite_language(LANGUAGE_DATA * language)
{
        FILE     *fp;
        char      filename[256];
        char      buf[MAX_STRING_LENGTH];

        if (!language)
        {
                bug("fwrite_language: null language pointer!", 0);
                return;
        }

        if (!language->name || language->name[0] == '\0')
        {
                snprintf(buf, MSL, "%s",
                         "fwrite_language: language has no name");
                bug(buf, 0);
                return;
        }

        snprintf(filename, 256, "%s%s.language", RACES_DIR,
                 smash_space(language->name));

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("fwrite_language: fopen", 0);
                perror(filename);
        }
        else
        {
                fprintf(fp, "#LANGUAGE\n");
                fprintf(fp, "Name              %s~\n", language->name);
                fprintf(fp, "Min_Intelligence  %d\n",
                        language->min_intelligence);
                fprintf(fp, "End\n\n");
                fprintf(fp, "#END\n");
        }
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        return;
}


bool load_language_file(char *languagefile)
{
        char      filename[256];
        LANGUAGE_DATA *language;
        FILE     *fp;
        bool      found;

        CREATE(language, LANGUAGE_DATA, 1);
        language->next = NULL;
        language->prev = NULL;

        found = FALSE;
        snprintf(filename, 256, "%s%s", RACES_DIR, languagefile);

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
                                bug("Load_language_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "LANGUAGE"))
                        {
                                fread_language(language, fp);
                                break;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                char      buf[MAX_STRING_LENGTH];

                                snprintf(buf, MSL,
                                         "Load_language_file: bad section: %s.",
                                         word);
                                bug(buf, 0);
                                break;
                        }
                }
                FCLOSE(fp);
        }

        if (found)
                LINK(language, first_language, last_language, next, prev);
        else
                DISPOSE(language);

        return found;
}


void load_languages(void)
{
        FILE     *fpList;
        const char *filename;
        char      languagelist[256];
        char      buf[MAX_STRING_LENGTH];

        first_language = NULL;
        last_language = NULL;

        snprintf(languagelist, 256, "%s%s", RACES_DIR, LANGUAGE_LIST);
        FCLOSE(fpReserve);
        if ((fpList = fopen(languagelist, "r")) == NULL)
        {
                perror(languagelist);
                fpReserve = fopen(NULL_FILE, "r");
                return;
        }

        for (;;)
        {
                filename = feof(fpList) ? "$" : fread_word(fpList);
                if (filename[0] == '$')
                        break;

                if (!load_language_file((char *) filename))
                {
                        snprintf(buf, MSL, "Cannot load language file: %s",
                                 filename);
                        bug(buf, 0);
                }
        }
        FCLOSE(fpList);
        boot_log(" Done languages");
        fpReserve = fopen(NULL_FILE, "r");
        return;
}


LANGUAGE_DATA *get_language(char *string)
{
        LANGUAGE_DATA *language;

        for (language = first_language; language; language = language->next)
                if (!str_cmp(string, language->name))
                        return language;
        return NULL;
}


PROTOSHIP_DATA *get_protoship(char *name)
{
        PROTOSHIP_DATA *ship;

        for (ship = first_protoship; ship; ship = ship->next)
                if (!str_cmp(name, ship->name))
                        return ship;

        for (ship = first_protoship; ship; ship = ship->next)
                if (nifty_is_name_prefix(name, ship->name))
                        return ship;

        return NULL;
}


CMDF do_setlanguage(CHAR_DATA * ch, char *argument)
{
        LANGUAGE_DATA *language;
        char      arg1[MSL];
        char      arg2[MSL];

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0')
        {
                send_to_char
                        ("&cInvalid syntax, usage: setlanguage <language>  <field> <value>\n\r&YValid fields are:\n\r",
                         ch);
                send_to_char("&RName, min_intelligence\n\r", ch);
                return;
        }
        if (!str_cmp(arg1, "save"))
        {
                if (!str_cmp(arg2, "all"))
                {
                        for (language = first_language; language;
                             language = language->next)
                                fwrite_language(language);
                        return;
                }
        }
        if ((language = get_language(arg1)) == NULL)
        {
                send_to_char
                        ("&RThat is not a valid language, choose one of the following:\n\r",
                         ch);
                for (language = first_language; language;
                     language = language->next)
                        ch_printf(ch, "&W\t%s\n\r", language->name);
                return;
        }

        if (!str_cmp(arg2, "name"))
        {
                stralloc_printf(&language->name, "%s", argument);
        }
        fwrite_language(language);
        send_to_char("Language set.", ch);
}




CMDF do_showlanguages(CHAR_DATA * ch, char *argument)
{
        LANGUAGE_DATA *language;

        argument = NULL;

        for (language = first_language; language; language = language->next)
                ch_printf(ch, "Name: %-20s Minimum Intelligence: %d\n\r",
                          language->name, language->min_intelligence);
}

CMDF do_makelanguage(CHAR_DATA * ch, char *argument)
{
        LANGUAGE_DATA *language;
        int       sn;

        for (language = first_language; language; language = language->next)
                if (!str_cmp(language->name, argument))
                {
                        send_to_char("That language already exists!", ch);
                        return;
                }

        CREATE(language, LANGUAGE_DATA, 1);
        language->next = NULL;
        language->prev = NULL;
        LINK(language, first_language, last_language, next, prev);
        language->min_intelligence = 0;
        language->name = STRALLOC(argument);
        fwrite_language(language);
        fwrite_language_list();
        ch_printf(ch, "Language \"%s\" created and saved.\n\r",
                  language->name);
        if ((sn = skill_lookup(language->name)) < 0)
        {
                struct skill_type *skill;
                sh_int    type = SKILL_TONGUE;

                ch_printf(ch,
                          "Language \"%s\" does not exist, attempting to create...",
                          language->name);
                CREATE(skill, struct skill_type, 1);

                skill_table[top_sn++] = skill;
                skill->name = str_dup(language->name);
                skill->noun_damage = str_dup("");
                stralloc_printf(&skill->msg_off, "!%s!", language->name);
                skill->spell_fun = spell_function("spell_null");
                skill->skill_fun = NULL;
                skill->spell_fun_name = str_dup("spell_null");
                skill->type = type;
                skill->guild = -1;
                ch_printf(ch, "Language \"%s\" create successfully.\n\r",
                          language->name);
        }
        else
                ch_printf(ch,
                          "Language \"%s\" already exists, continuing...\n\r",
                          language->name);
}


PROTOSHIP_DATA *create_protoship(void)
{
        PROTOSHIP_DATA *ship;

        CREATE(ship, PROTOSHIP_DATA, 1);
        ship->name = STRALLOC("");
        ship->description = STRALLOC("");
        ship->mingroundspeed = 0;
        ship->maxgroundspeed = 0;
        ship->minrooms = 0;
        ship->maxrooms = 0;
        ship->mincomm = 0;
        ship->maxcomm = 0;
        ship->minsensor = 0;
        ship->maxsensor = 0;
        ship->minastro_array = 0;
        ship->maxastro_array = 0;
        ship->minhyperspeed = 0;
        ship->maxhyperspeed = 0;
        ship->minspeed = 0;
        ship->maxspeed = 0;
        ship->minmissiles = 0;
        ship->maxmissiles = 0;
        ship->mintorpedos = 0;
        ship->maxtorpedos = 0;
        ship->minrockets = 0;
        ship->maxrockets = 0;
        ship->minlasers = 0;
        ship->maxlasers = 0;
        ship->mintractorbeam = 0;
        ship->maxtractorbeam = 0;
        ship->minions = 0;
        ship->maxions = 0;
        ship->minmanuever = 0;
        ship->maxmanuever = 0;
        ship->maxcargo = 0;
        ship->mincargo = 0;
        ship->maxenergy = 0;
        ship->minenergy = 0;
        ship->minshield = 0;
        ship->maxshield = 0;
        ship->minhull = 0;
        ship->maxhull = 0;
        ship->minchaff = 0;
        ship->maxchaff = 0;
        ship->minbattalions = 0;
        ship->maxbattalions = 0;
        ship->next = NULL;
        ship->prev = NULL;
        return ship;
}

void write_protoship_list()
{
        PROTOSHIP_DATA *tprotoship;
        FILE     *fpout;
        char      filename[256];

        snprintf(filename, MSL, "%s%s", PROTOSHIP_DIR, PROTOSHIP_LIST);
        fpout = fopen(filename, "w");
        if (!fpout)
        {
                bug("FATAL: cannot open protoship.lst for writing!\n\r", 0);
                return;
        }
        for (tprotoship = first_protoship; tprotoship;
             tprotoship = tprotoship->next)
                if (str_cmp(tprotoship->name, "generic"))
                        fprintf(fpout, "%s.proto\n",
                                smash_space(tprotoship->name));
        fprintf(fpout, "$\n");
        FCLOSE(fpout);
}

void save_protoship(PROTOSHIP_DATA * ship)
{
        FILE     *fp;
        char      filename[256];
        char      buf[MAX_STRING_LENGTH];

        if (!ship)
        {
                bug("save_protoship: null proto ship pointer!", 0);
                return;
        }

        if (!ship->name || ship->name[0] == '\0')
        {
                snprintf(buf, MSL, "%s", "save_protoship: ship has no name");
                bug(buf, 0);
                return;
        }

        snprintf(filename, 256, "%s%s.proto", PROTOSHIP_DIR,
                 smash_space(ship->name));

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("save_ship: fopen", 0);
                perror(filename);
        }
        else
        {
                fprintf(fp, "#PROTOSHIP\n");
                fprintf(fp, "Name					%s~\n", ship->name);
                fprintf(fp, "Description			%s~\n",
                        ship->description);
                fprintf(fp, "Shipclass					%s~\n",
                        ship->shipclass);
                fprintf(fp, "Mingroundspeed		%d\n", ship->mingroundspeed);
                fprintf(fp, "Maxgroundspeed		%d\n", ship->maxgroundspeed);
                fprintf(fp, "Minrooms			%d\n", ship->minrooms);
                fprintf(fp, "Maxrooms			%d\n", ship->maxrooms);
                fprintf(fp, "Mincomm			%d\n", ship->mincomm);
                fprintf(fp, "Maxcomm			%d\n", ship->maxcomm);
                fprintf(fp, "Minsensor		%d\n", ship->minsensor);
                fprintf(fp, "Maxsensor		%d\n", ship->maxsensor);
                fprintf(fp, "Minastro_array		%d\n", ship->minastro_array);
                fprintf(fp, "Maxastro_array		%d\n", ship->maxastro_array);
                fprintf(fp, "Minhyperspeed		%d\n", ship->minhyperspeed);
                fprintf(fp, "Maxhyperspeed		%d\n", ship->maxhyperspeed);
                fprintf(fp, "Minspeed			%d\n", ship->minspeed);
                fprintf(fp, "Maxspeed			%d\n", ship->maxspeed);
                fprintf(fp, "Minmissiles		%d\n", ship->minmissiles);
                fprintf(fp, "Maxmissiles		%d\n", ship->maxmissiles);
                fprintf(fp, "Mintorpedos		%d\n", ship->mintorpedos);
                fprintf(fp, "Maxtorpedos		%d\n", ship->maxtorpedos);
                fprintf(fp, "Minrockets		%d\n", ship->minrockets);
                fprintf(fp, "Maxrockets		%d\n", ship->maxrockets);
                fprintf(fp, "Minlasers			%d\n", ship->minlasers);
                fprintf(fp, "Maxlasers			%d\n", ship->maxlasers);
                fprintf(fp, "Mintractorbeams		%d\n",
                        ship->mintractorbeam);
                fprintf(fp, "Maxtractorbeams		%d\n",
                        ship->maxtractorbeam);
                fprintf(fp, "Minions			%d\n", ship->minions);
                fprintf(fp, "Maxions			%d\n", ship->maxions);
                fprintf(fp, "Minmanuever		%d\n", ship->minmanuever);
                fprintf(fp, "Maxmanuever		%d\n", ship->maxmanuever);
                fprintf(fp, "Mincargo			%d\n", ship->mincargo);
                fprintf(fp, "Maxcargo			%d\n", ship->maxcargo);
                fprintf(fp, "Minenergy			%d\n", ship->minenergy);
                fprintf(fp, "Maxenergy			%d\n", ship->maxenergy);
                fprintf(fp, "Minshield			%d\n", ship->minshield);
                fprintf(fp, "Maxshield			%d\n", ship->maxshield);
                fprintf(fp, "Minhull			%d\n", ship->minhull);
                fprintf(fp, "Maxhull			%d\n", ship->maxhull);
                fprintf(fp, "Minchaff			%d\n", ship->minchaff);
                fprintf(fp, "Maxchaff			%d\n", ship->maxchaff);
                fprintf(fp, "Minbattalions		%d\n", ship->minbattalions);
                fprintf(fp, "Maxbattalions		%d\n", ship->maxbattalions);
                fprintf(fp, "End\n\n");
                fprintf(fp, "#END\n");
        }
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

void fread_protoship(PROTOSHIP_DATA * ship, FILE * fp)
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

                case 'D':
                        KEY("Description", ship->description,
                            fread_string(fp));
                        break;

                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (!ship->name)
                                        ship->name = STRALLOC("");
                                if (!ship->description)
                                        ship->description = STRALLOC("");
                                if (!ship->shipclass)
                                        ship->shipclass =
                                                STRALLOC("Undefined");
#ifdef IMAGES
                                else if (!shipimage_exist(ship->shipclass)
                                         && str_cmp(ship->shipclass,
                                                    "Undefined"))
                                {
                                        bug("%s:: image file not found- %s",
                                            __FUNCTION__, ship->shipclass);
                                        ship->shipclass =
                                                STRALLOC("Undefined");
                                }
#endif
                                return;
                        }
                        break;

                case 'M':
                        KEY("Mingroudspeed", ship->mingroundspeed,
                            fread_number(fp));
                        KEY("Maxgroundspeed", ship->maxgroundspeed,
                            fread_number(fp));
                        KEY("Minrooms", ship->minrooms, fread_number(fp));
                        KEY("Maxrooms", ship->maxrooms, fread_number(fp));
                        KEY("Mincomm", ship->mincomm, fread_number(fp));
                        KEY("Maxcomm", ship->maxcomm, fread_number(fp));
                        KEY("Minsensor", ship->minsensor, fread_number(fp));
                        KEY("Maxsensor", ship->maxsensor, fread_number(fp));
                        KEY("Minastro_array", ship->minastro_array,
                            fread_number(fp));
                        KEY("Maxastro_array", ship->maxastro_array,
                            fread_number(fp));
                        KEY("Minhyperspeed", ship->minhyperspeed,
                            fread_number(fp));
                        KEY("Maxhyperspeed", ship->maxhyperspeed,
                            fread_number(fp));
                        KEY("Minspeed", ship->minspeed, fread_number(fp));
                        KEY("Maxspeed", ship->maxspeed, fread_number(fp));
                        KEY("Minmissiles", ship->minmissiles,
                            fread_number(fp));
                        KEY("Maxmissiles", ship->maxmissiles,
                            fread_number(fp));
                        KEY("Mintorpedos", ship->mintorpedos,
                            fread_number(fp));
                        KEY("Maxtorpedos", ship->maxtorpedos,
                            fread_number(fp));
                        KEY("Minrockets", ship->minrockets, fread_number(fp));
                        KEY("Maxrockets", ship->maxrockets, fread_number(fp));
                        KEY("Minlasers", ship->minlasers, fread_number(fp));
                        KEY("Maxlasers", ship->maxlasers, fread_number(fp));
                        KEY("Mintractorbeam", ship->mintractorbeam,
                            fread_number(fp));
                        KEY("Maxtractorbeam", ship->maxtractorbeam,
                            fread_number(fp));
                        KEY("Minions", ship->minions, fread_number(fp));
                        KEY("Maxions", ship->maxions, fread_number(fp));
                        KEY("Minmanuever", ship->minmanuever,
                            fread_number(fp));
                        KEY("Maxmanuever", ship->maxmanuever,
                            fread_number(fp));
                        KEY("MinCargo", ship->mincargo, fread_number(fp));
                        KEY("MaxCargo", ship->maxcargo, fread_number(fp));
                        KEY("Minenergy", ship->minenergy, fread_number(fp));
                        KEY("Maxenergy", ship->maxenergy, fread_number(fp));
                        KEY("Minshield", ship->minshield, fread_number(fp));
                        KEY("Maxshield", ship->maxshield, fread_number(fp));
                        KEY("Minhull", ship->minhull, fread_number(fp));
                        KEY("Maxhull", ship->maxhull, fread_number(fp));
                        KEY("Minchaff", ship->minchaff, fread_number(fp));
                        KEY("Maxchaff", ship->maxchaff, fread_number(fp));
                        KEY("Minbattalions", ship->minbattalions,
                            fread_number(fp));
                        KEY("Maxbattalions", ship->maxbattalions,
                            fread_number(fp));
                        break;

                case 'N':
                        KEY("Name", ship->name, fread_string(fp));
                        break;
                case 'S':
                        KEY("Shipclass", ship->shipclass, fread_string(fp));
                        break;

                        if (!fMatch)
                        {
                                snprintf(buf, MSL, "Fread_ship: no match: %s",
                                         word);
                                bug(buf, 0);
                        }
                }
        }
}

bool load_protoship_file(char *shipfile)
{
        char      filename[256];
        PROTOSHIP_DATA *ship;
        FILE     *fp;
        bool      found;

        if ((ship = create_protoship()) == NULL)
        {
                bug("Cannot create protoship_data", 0);
                return FALSE;
        }

        found = FALSE;
        snprintf(filename, 256, "%s%s", PROTOSHIP_DIR, shipfile);

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
                                bug("Load_protoship_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "PROTOSHIP"))
                        {
                                fread_protoship(ship, fp);
                                break;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                char      buf[MAX_STRING_LENGTH];

                                snprintf(buf, MSL,
                                         "Load_protoship_file: bad section: %s.",
                                         word);
                                bug(buf, 0);
                                break;
                        }
                }
                FCLOSE(fp);
        }
        if (!(found))
                DISPOSE(ship);
        else
                LINK(ship, first_protoship, last_protoship, next, prev);
        return found;
}

void load_protoships(void)
{
        FILE     *fpList;
        const char *filename;
        char      shiplist[256];
        char      buf[MAX_STRING_LENGTH];
        PROTOSHIP_DATA *ship;


        first_protoship = NULL;
        last_protoship = NULL;


        CREATE(ship, PROTOSHIP_DATA, 1);
        LINK(ship, first_protoship, last_protoship, next, prev);
        ship->name = STRALLOC("generic");
        ship->description = STRALLOC("");

        snprintf(shiplist, 256, "%s%s", PROTOSHIP_DIR, PROTOSHIP_LIST);
        FCLOSE(fpReserve);
        if ((fpList = fopen(shiplist, "r")) == NULL)
        {
                perror(shiplist);
                fpReserve = fopen(NULL_FILE, "r");
                return;
        }

        for (;;)
        {

                filename = feof(fpList) ? "$" : fread_word(fpList);

                if (filename[0] == '$')
                        break;

                if (!load_protoship_file((char *) filename))
                {
                        snprintf(buf, MSL,
                                 "Cannot load prototype ship file: %s",
                                 filename);
                        bug(buf, 0);
                }

        }
        FCLOSE(fpList);
        boot_log(" Done Ship Prototypes ");
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

CMDF do_setprotoship(CHAR_DATA * ch, char *argument)
{
        char      arg1[MAX_INPUT_LENGTH];
        char      arg2[MAX_INPUT_LENGTH];
        PROTOSHIP_DATA *ship;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0' || arg1[0] == '\0')
        {
                send_to_char
                        ("Usage: setprotoship <protoship> <field> <values>\n\r",
                         ch);
                send_to_char("\n\rField being one of:\n\r", ch);
                send_to_char("name desc create\n\r", ch);
                send_to_char
                        ("mingroundspeed maxgroundspeed minrooms maxrooms mincomm maxcomm\n\r",
                         ch);
                send_to_char
                        ("minsensor maxsensor minastro_array maxastro_array minhyperspeed maxhyperspeed \n\r",
                         ch);
                send_to_char
                        ("minspeed maxspeed minmissiles maxmissiles morpedos maxtorpedos\n\r",
                         ch);
                send_to_char
                        ("minrockets maxrockets minlasers maxlasers mractorbeam maxtractorbeam\n\r",
                         ch);
                send_to_char
                        ("minions maxions minmanuever maxmanuever maxcargo mincargo maxenergy \n\r",
                         ch);
                send_to_char
                        ("minenergy minshield maxshield minhull maxhull\n\r",
                         ch);
                send_to_char
                        ("minchaff maxchaff minbattalions maxbattalions \n\r",
                         ch);
                return;
        }

        ship = get_protoship(arg1);

        if (!str_cmp(arg1, "generic"))
        {
                send_to_char
                        ("You can not edit the generic fail-safe prototype",
                         ch);
                return;
        }

        if (!str_cmp(arg2, "create"))
        {
                if ((ship = create_protoship()) == NULL)
                {
                        send_to_char("Error in creating protoship.", ch);
                        bug("Error in making protoship: do_setprotoship", 0);
                        return;
                }
                LINK(ship, first_protoship, last_protoship, next, prev);
                stralloc_printf(&ship->name, "%s", arg1);
                save_protoship(ship);
                write_protoship_list();
                send_to_char("Done.\n\r", ch);
        }

        if (!ship)
        {
                send_to_char("No such prototpye ship.\n\r", ch);
                return;
        }

        if (!str_cmp(arg2, "name"))
        {
                STRFREE(ship->name);
                ship->name = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                write_protoship_list();
                return;
        }

        if (!str_cmp(arg2, "desc"))
        {
                STRFREE(ship->description);
                ship->description = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }
#ifdef IMAGES
        if (!str_cmp(arg2, "class"))
        {
                if (!shipimage_exist(argument))
                {
                        send_to_char("There is no ship image with that name",
                                     ch);
                        return;
                }
                STRFREE(ship->shipclass);
                ship->shipclass = STRALLOC(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }
#endif
        if (!str_cmp(arg2, "minrooms"))
        {
                ship->minrooms = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxrooms"))
        {
                ship->maxrooms = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }
        if (!str_cmp(arg2, "mingroundspeed"))
        {
                ship->mingroundspeed = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxgroundspeed"))
        {
                ship->maxgroundspeed = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }
        if (!str_cmp(arg2, "mincomm"))
        {
                ship->mincomm = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxcomm"))
        {
                ship->maxcomm = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }
        if (!str_cmp(arg2, "minsensor"))
        {
                ship->minsensor = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxsensor"))
        {
                ship->maxsensor = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }
        if (!str_cmp(arg2, "minastro_array"))
        {
                ship->minastro_array = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxastro_array"))
        {
                ship->maxastro_array = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }
        if (!str_cmp(arg2, "minhyperspeed"))
        {
                ship->minhyperspeed = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxhyperspeed"))
        {
                ship->maxhyperspeed = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }
        if (!str_cmp(arg2, "minspeed"))
        {
                ship->minspeed = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxspeed"))
        {
                ship->maxspeed = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }
        if (!str_cmp(arg2, "minmissiles"))
        {
                ship->minmissiles = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxmissiles"))
        {
                ship->maxmissiles = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }
        if (!str_cmp(arg2, "mintorpedos"))
        {
                ship->mintorpedos = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxtorpedos"))
        {
                ship->maxtorpedos = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }
        if (!str_cmp(arg2, "minrockets"))
        {
                ship->minrockets = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxrockets"))
        {
                ship->maxrockets = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "minlasers"))
        {
                ship->minlasers = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxlasers"))
        {
                ship->maxlasers = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "mintractorbeam"))
        {
                ship->mintractorbeam = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxtractorbeam"))
        {
                ship->maxtractorbeam = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "minions"))
        {
                ship->minions = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxions"))
        {
                ship->maxions = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "minmanuever"))
        {
                ship->minmanuever = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxmanuever"))
        {
                ship->maxmanuever = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "mincargo"))
        {
                ship->mincargo = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxcargo"))
        {
                ship->maxcargo = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "minenergy"))
        {
                ship->minenergy = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxenergy"))
        {
                ship->maxenergy = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "minshield"))
        {
                ship->minshield = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxshield"))
        {
                ship->maxshield = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "minhull"))
        {
                ship->minhull = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxhull"))
        {
                ship->maxhull = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "minchaff"))
        {
                ship->minchaff = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxchaff"))
        {
                ship->maxchaff = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "minbattalions"))
        {
                ship->minbattalions = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        if (!str_cmp(arg2, "maxbattalions"))
        {
                ship->maxbattalions = atoi(argument);
                send_to_char("Done.\n\r", ch);
                save_protoship(ship);
                return;
        }

        return;
}

CMDF do_showprotoship(CHAR_DATA * ch, char *argument)
{
        PROTOSHIP_DATA *ship;

        if (IS_NPC(ch))
        {
                send_to_char("Huh?\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Usage: showprotoship <ship>\n\r", ch);
                return;
        }

        ship = get_protoship(argument);

        if (!ship)
        {
                send_to_char("No such prototype. Available options are:\n\r",
                             ch);
                for (ship = first_protoship; ship; ship = ship->next)
                        ch_printf(ch, "&B|| &w%s\n\r", ship->name);
                return;
        }
        if (!str_cmp(ship->name, "generic"))
        {
                send_to_char
                        ("You can not view the generic fail-safe prototype",
                         ch);
                return;
        }

        ch_printf(ch,
                  "&B|| N&zame:&w %s\n\r&B|| D&zescription:&w %s \n\r&B|| C&zlass: &w %s\n\r",
                  ship->name, ship->description, ship->shipclass);
        send_to_char("&B|| -------------------||---------||---------||\n\r",
                     ch);
        send_to_char
                ("&B|| &zField              &B||  &BM&zin    &B||  &BM&zax    &B||\n\r",
                 ch);
        send_to_char("&B|| -------------------||---------||---------||\n\r",
                     ch);
        ch_printf(ch,
                  "&B|| &zSensor:&w            &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minsensor, ship->maxsensor);
        ch_printf(ch,
                  "&B|| &zComm:&w              &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->mincomm, ship->maxcomm);
        ch_printf(ch,
                  "&B|| &zGround Speed:&w      &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->mingroundspeed, ship->maxgroundspeed);
        ch_printf(ch,
                  "&B|| &zRooms:&w             &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minrooms, ship->maxrooms);
        ch_printf(ch,
                  "&B|| &zAstro Array:&w       &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minastro_array, ship->maxastro_array);
        ch_printf(ch,
                  "&B|| &zHyper Speed:&w       &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minhyperspeed, ship->maxhyperspeed);
        ch_printf(ch,
                  "&B|| &zSpeed:&w             &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minspeed, ship->maxspeed);
        ch_printf(ch,
                  "&B|| &zMissiles:&w          &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minmissiles, ship->maxmissiles);
        ch_printf(ch,
                  "&B|| &zTopedos:&w           &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->mintorpedos, ship->maxtorpedos);
        ch_printf(ch,
                  "&B|| &zRockets:&w           &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minrockets, ship->maxrockets);
        ch_printf(ch,
                  "&B|| &zLasers:&w            &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minlasers, ship->maxlasers);
        ch_printf(ch,
                  "&B|| &zTractor Beams:&w     &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->mintractorbeam, ship->maxtractorbeam);
        ch_printf(ch,
                  "&B|| &zIon Cannons:&w       &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minions, ship->maxions);
        ch_printf(ch,
                  "&B|| &zManuever:&w          &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minmanuever, ship->maxmanuever);
        ch_printf(ch,
                  "&B|| &zCargo:&w             &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->mincargo, ship->maxcargo);
        ch_printf(ch,
                  "&B|| &zBattallions:&w       &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minbattalions, ship->maxbattalions);
        ch_printf(ch,
                  "&B|| &zEnergy:&w            &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minenergy, ship->maxenergy);
        ch_printf(ch,
                  "&B|| &zShields:&w           &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minshield, ship->maxshield);
        ch_printf(ch,
                  "&B|| &zHull:&w              &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minhull, ship->maxhull);
        ch_printf(ch,
                  "&B|| &zChaff:&w             &B||  &w%-7d&B||  &w%-7d&B||\n\r",
                  ship->minchaff, ship->maxchaff);
}

CMDF do_showability(CHAR_DATA * ch, char *argument)
{
        int       iclass = 0;
        RACE_DATA *race = NULL;

        if (argument[0] == '\0')
        {
                send_to_pager("showability <classname>", ch);
                return;
        }
        send_to_pager
                ("&B------------------&w<&B<&C<&w<&B<&C< &wAbility Info&B &w>&B>&C>&w>&B>&C>&B------------------\n\r",
                 ch);
        if (!str_cmp(argument, "all"))
        {
                int       total;

                send_to_char("&BR&zace Name:                   ", ch);
                for (iclass = 0; iclass < MAX_ABILITY; iclass++)
                        ch_printf(ch, "&B[&w%-3.3s&B]&z|",
                                  ability_name[iclass]);
                send_to_char("&B[&wTot&B]\n\r", ch);
                send_to_char("&B-----------------------------", ch);
                for (iclass = 0; iclass < MAX_ABILITY; iclass++)
                        ch_printf(ch, "-----&z+&B", ability_name[iclass]);
                send_to_char("-----\n\r", ch);
                FOR_EACH_LIST(RACE_LIST, races, race)
                {
                        total = 0;
                        ch_printf(ch, "&BN&zame&w: &B[&w%-20s&B]&z|",
                                  race->name());
                        for (iclass = 0; iclass < MAX_ABILITY; iclass++)
                        {
                                ch_printf(ch, "&B[&w%+3d&B]&z|&B",
                                          race->class_modifier(iclass));
                                total += race->class_modifier(iclass);
                        }
                        ch_printf(ch, "[&w%+3d&B]\n\r", total);
                }
                return;
        }

        /*
         * For all Classes 
         */
        for (iclass = 0; iclass < MAX_ABILITY; iclass++)
        {
                if (!str_cmp(ability_name[iclass], argument))
                {
                        FOR_EACH_LIST(RACE_LIST, races, race)
                        {
                                pager_printf(ch,
                                             "&BN&zame&w:     &B[&w%-20s&B] &BR&zace Modifier&w: &B[&w%-3d&B]\n\r",
                                             race->name(),
                                             race->class_modifier(iclass));
                        }
                        break;
                }
        }
        return;
}

ILLNESS_DATA *create_illness(void)
{
        ILLNESS_DATA *illness;

        CREATE(illness, ILLNESS_DATA, 1);
        illness->name = STRALLOC("");
        illness->next = NULL;
        illness->prev = NULL;
        illness->social1 = STRALLOC("");
        illness->social2 = STRALLOC("");
        illness->social3 = STRALLOC("");
        illness->message1 = STRALLOC("");
        illness->message2 = STRALLOC("");
        illness->message3 = STRALLOC("");
        illness->hploss = 0;
        illness->mvloss = 0;
        return illness;
}

void fwrite_illness_list(void)
{
        FILE     *fp;
        char      filename[256];
        ILLNESS_DATA *illness;

        snprintf(filename, 256, "%s%s", ILLNESS_DIR, ILLNESS_LIST);
        fp = fopen(filename, "w");
        if (!fp)
        {
                bug("FATAL: cannot open illness.lst for writing!\n\r", 0);
                return;
        }
        for (illness = first_illness; illness; illness = illness->next)
                fprintf(fp, "%s.illness\n", smash_space(illness->name));
        fprintf(fp, "$\n");
        FCLOSE(fp);
}

void fwrite_illness(ILLNESS_DATA * illness)
{
        FILE     *fp;
        char      filename[256];
        char      buf[MAX_STRING_LENGTH];

        if (!illness)
        {
                bug("fwrite_illness: null illness pointer!", 0);
                return;
        }

        if (!illness->name || illness->name[0] == '\0')
        {
                snprintf(buf, 256, "%s",
                         "fwrite_illness: illness has no name");
                bug(buf, 0);
                return;
        }

        snprintf(filename, MSL, "%s%s.illness", ILLNESS_DIR,
                 smash_space(illness->name));

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("fwrite_illness: fopen", 0);
                perror(filename);
        }
        else
        {
                fprintf(fp, "#ILLNESS\n");
                fprintf(fp, "Name              %s~\n", illness->name);
                if (illness->social1 && illness->social1[0] != '\0')
                        fprintf(fp, "Social1           %s~\n",
                                illness->social1);
                if (illness->social2 && illness->social2[0] != '\0')
                        fprintf(fp, "Social2           %s~\n",
                                illness->social2);
                if (illness->social3 && illness->social3[0] != '\0')
                        fprintf(fp, "Social3           %s~\n",
                                illness->social3);
                if (illness->message1 && illness->message1[0] != '\0')
                        fprintf(fp, "Message1          %s~\n",
                                illness->message1);
                if (illness->message2 && illness->message2[0] != '\0')
                        fprintf(fp, "Message2          %s~\n",
                                illness->message2);
                if (illness->message3 && illness->message3[0] != '\0')
                        fprintf(fp, "Message3          %s~\n",
                                illness->message3);
                fprintf(fp, "HPloss            %d\n", illness->hploss);
                fprintf(fp, "MVloss            %d\n", illness->mvloss);
                fprintf(fp, "Hunger            %d\n", illness->hunger);
                fprintf(fp, "Thirst            %d\n", illness->thirst);
                fprintf(fp, "End\n\n");
                fprintf(fp, "#END\n");
        }
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

void fread_illness(ILLNESS_DATA * illness, FILE * fp)
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
                case 'E':
                        if (!str_cmp(word, "End"))
                        {
                                if (illness->name == NULL)
                                        bug("Error loading race");
                                fMatch = TRUE;
                        }
                        return;
                case 'H':
                        KEY("HPloss", illness->hploss, fread_number(fp));
                        KEY("Hunger", illness->hunger, fread_number(fp));
                        break;
                case 'M':
                        KEY("MVloss", illness->mvloss, fread_number(fp));
                        KEY("Message1", illness->message1, fread_string(fp));
                        KEY("Message2", illness->message2, fread_string(fp));
                        KEY("Message3", illness->message3, fread_string(fp));
                        break;
                case 'N':
                        KEY("Name", illness->name, fread_string(fp));
                        break;
                case 'S':
                        KEY("Social1", illness->social1, fread_string(fp));
                        KEY("Social2", illness->social2, fread_string(fp));
                        KEY("Social3", illness->social3, fread_string(fp));
                        break;
                case 'T':
                        KEY("Thirst", illness->thirst, fread_number(fp));
                        break;
                }
                if (!fMatch)
                {
                        snprintf(buf, MSL, "Fread_illness: no match: %s",
                                 word);
                        bug(buf, 0);
                }
        }
}

bool load_illness_file(char *illnessfile)
{
        char      filename[256];
        ILLNESS_DATA *illness;

        FILE     *fp;

        snprintf(filename, 256, "%s%s", ILLNESS_DIR, illnessfile);

        if ((fp = fopen(filename, "r")) != NULL)
        {
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
                                bug("Load_illness_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "ILLNESS"))
                        {
                                illness = create_illness();
                                fread_illness(illness, fp);
                                LINK(illness, first_illness, last_illness,
                                     next, prev);
                                break;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                char      buf[MAX_STRING_LENGTH];

                                snprintf(buf, MSL,
                                         "Load_illness_file: bad section: %s.",
                                         word);
                                bug(buf, 0);
                                break;
                        }
                }
                FCLOSE(fp);
                return TRUE;
        }
        return FALSE;
}

void load_illness(void)
{
        FILE     *fpList;
        const char *filename;
        char      illnesslist[256];
        char      buf[MAX_STRING_LENGTH];

        first_illness = NULL;
        last_illness = NULL;

        snprintf(illnesslist, 256, "%s%s", ILLNESS_DIR, ILLNESS_LIST);
        FCLOSE(fpReserve);
        if ((fpList = fopen(illnesslist, "r")) == NULL)
        {
                perror(illnesslist);
                fpReserve = fopen(NULL_FILE, "r");
                return;
        }

        for (;;)
        {
                filename = feof(fpList) ? "$" : fread_word(fpList);
                if (filename[0] == '$')
                        break;

                if (!load_illness_file((char *) filename))
                {
                        snprintf(buf, MSL, "Cannot load illness file: %s",
                                 filename);
                        bug(buf, 0);
                }
        }
        FCLOSE(fpList);
        boot_log(" Done Illnesses");
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

ILLNESS_DATA *get_illness(char *string)
{
        ILLNESS_DATA *illness;

        for (illness = first_illness; illness; illness = illness->next)
                if (!str_cmp(string, illness->name))
                        return illness;
        return NULL;
}

CMDF do_makeillness(CHAR_DATA * ch, char *argument)
{
        ILLNESS_DATA *illness = NULL;

        if (!argument || argument[0] == '\0')
        {
                send_to_char("&BS&zyntax: makeillness &w<name>&R&W", ch);
                return;
        }
        for (illness = first_illness; illness; illness = illness->next)
        {
                if (illness->name && illness->name[0])
                        if (!str_cmp(illness->name, argument))
                        {
                                send_to_char("That illness already exists!",
                                             ch);
                                return;
                        }
        }

        illness = create_illness();
        LINK(illness, first_illness, last_illness, next, prev);
        stralloc_printf(&illness->name, "%s", argument);
        fwrite_illness(illness);
        fwrite_illness_list();
        ch_printf(ch, "Illness \"%s\" created and saved.\n\r",
                  illness ? illness->name ? illness->
                  name : "<ERROR1>" : "<ERROR2>");
}

CMDF do_showillness(CHAR_DATA * ch, char *argument)
{
        ILLNESS_DATA *illness;

        if (NULLSTR(argument) || (illness = get_illness(argument)) == NULL)
        {
                sh_int    count = 0;

                send_to_char("&BI&znvalid selection.\n\r", ch);
                send_to_char("&BS&zyntax: showillness &z<illness>&R&W\n\r",
                             ch);
                send_to_char("&BV&zalid illness are:\n\r", ch);
                for (illness = first_illness; illness;
                     illness = illness->next)
                {
                        ch_printf(ch, "\t&w%s\n\r", illness->name);
                        count++;
                }
                if (count == 0)
                        send_to_char("&BN&zo illnesses available.\n\r", ch);
                ch_printf(ch, "&BT&zotal illnesses created: &B[&w%d&B]\n\r",
                          count);
                return;
        }

        ch_printf(ch, "Name :          %s\n\r",
                  illness->name ? illness->name ? illness->
                  name : "No Name" : "No Name");
        ch_printf(ch, "Hit Point Loss: %d\n\r", illness->hploss);
        ch_printf(ch, "Movement Loss:  %d\n\r", illness->mvloss);
        ch_printf(ch, "Hunger Modifier:%d\n\r", illness->hunger);
        ch_printf(ch, "Thirst Modifier:%d\n\r", illness->thirst);
        ch_printf(ch, "Social 1: %s\n\r",
                  illness->social1[0] ==
                  '\0' ? "None set" : illness->social1);
        ch_printf(ch, "Social 2: %s\n\r",
                  illness->social2[0] ==
                  '\0' ? "None set" : illness->social2);
        ch_printf(ch, "Social 3: %s\n\r",
                  illness->social3[0] ==
                  '\0' ? "None set" : illness->social3);
        ch_printf(ch, "Message 1: %s\n\r",
                  illness->message1[0] ==
                  '\0' ? "None set" : illness->message1);
        ch_printf(ch, "Message 2: %s\n\r",
                  illness->message2[0] ==
                  '\0' ? "None set" : illness->message2);
        ch_printf(ch, "Message 3: %s\n\r",
                  illness->message3[0] ==
                  '\0' ? "None set" : illness->message3);
}

CMDF do_setillness(CHAR_DATA * ch, char *argument)
{
        ILLNESS_DATA *illness;
        char      arg1[MSL];
        char      arg2[MSL];

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);
        if (NULLSTR(argument) || NULLSTR(arg1) || NULLSTR(arg2)
            || (illness = get_illness(arg1)) == NULL)
        {
                sh_int    count = 0;

                send_to_char("&BI&znvalid selection.\n\r", ch);
                send_to_char
                        ("&BS&zyntax: setillness &z<illness> &zfield &w<value>&R&W\n\r",
                         ch);
                send_to_char("&BV&zalid illness are:\n\r", ch);
                for (illness = first_illness; illness;
                     illness = illness->next)
                {
                        ch_printf(ch, "\t&w%s\n\r", illness->name);
                        count++;
                }
                if (count == 0)
                        send_to_char("&BN&zo illnesses available.\n\r", ch);
                ch_printf(ch, "&BT&zotal illnesses created: &B[&w%d&B]\n\r",
                          count);
                send_to_char
                        ("&Bc&zurrent fields include: hunger, thirst, hploss, mvloss, name\n\r",
                         ch);
                send_to_char
                        ("                        social1, social2, social3\n\r",
                         ch);
                send_to_char
                        ("                        message1, message2, message3\n\r",
                         ch);
                return;
        }

        if (!str_cmp(arg2, "hunger"))
                illness->hunger = atoi(argument);
        else if (!str_cmp(arg2, "thirst"))
                illness->thirst = atoi(argument);
        else if (!str_cmp(arg2, "hploss"))
                illness->hploss = atoi(argument);
        else if (!str_cmp(arg2, "mvloss"))
                illness->mvloss = atoi(argument);
        else if (!str_cmp(arg2, "message1"))
                stralloc_printf(&illness->message1, "%s", argument);
        else if (!str_cmp(arg2, "message2"))
                stralloc_printf(&illness->message3, "%s", argument);
        else if (!str_cmp(arg2, "message3"))
                stralloc_printf(&illness->message3, "%s", argument);
        else if (!str_cmp(arg2, "social1"))
        {
                if (find_social(argument) == NULL)
                {
                        send_to_char("That is not a valid social.", ch);
                        return;
                }
                stralloc_printf(&illness->social1, "%s", argument);
        }
        else if (!str_cmp(arg2, "social2"))
        {
                if (find_social(argument) == NULL)
                {
                        send_to_char("That is not a valid social.", ch);
                        return;
                }
                stralloc_printf(&illness->social2, "%s", argument);
        }
        else if (!str_cmp(arg2, "social3"))
        {
                if (find_social(argument) == NULL)
                {
                        send_to_char("That is not a valid social.", ch);
                        return;
                }
                stralloc_printf(&illness->social3, "%s", argument);
        }
        else
        {
                do_setillness(ch, "");
                return;
        }
        fwrite_illness(illness);
        send_to_char("Illness set.", ch);
}

void free_protoship(PROTOSHIP_DATA * ship)
{
        if (ship->description)
                STRFREE(ship->description);
        if (ship->name)
                STRFREE(ship->name);
        if (ship->shipclass)
                STRFREE(ship->shipclass);
        DISPOSE(ship);
}

void free_illness(ILLNESS_DATA * illness)
{
        if (illness->name)
                STRFREE(illness->name);
        if (illness->social1)
                STRFREE(illness->social1);
        if (illness->social2)
                STRFREE(illness->social2);
        if (illness->social3)
                STRFREE(illness->social3);
        if (illness->message1)
                STRFREE(illness->message1);
        if (illness->message2)
                STRFREE(illness->message2);
        if (illness->message3)
                STRFREE(illness->message3);
        DISPOSE(illness);
}
