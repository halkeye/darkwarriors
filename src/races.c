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
 *                $Id: races.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#include "mud.h"
#include "races.h"

RACE_LIST races;
int get_partflag args((char *flag));

/*
 * Cleaner to all be in one place - Gavin
 */
RACE_DATA::RACE_DATA()
{
        this->_name = NULL;
        this->_affected = 0;
        this->_con_plus = 0;
        this->_str_plus = 0;
        this->_wis_plus = 0;
        this->_dex_plus = 0;
        this->_cha_plus = 0;
        this->_int_plus = 0;
        this->_frc_plus = 0;
        this->_lck_plus = 0;
        this->_hit = 0;
        this->_endurance = 0;
        this->_language = get_language("basic");
        this->_home = -1;
        this->_death_age = 800; /* FIXME */
        this->_hunger_mod = 0;
        this->_thirst_mod = 0;
        this->_lang_bonus = 0;
        this->_rpneeded = 0;
        this->_start_age = 17;
        xCLEAR_BITS(this->_body_parts);
        for (int imod = 0; imod <= MAX_ATTR; imod++)
                this->_attr_mod[imod] = 0;
        for (int iclass = 0; iclass <= MAX_ABILITY; iclass++)
                this->_class_modifier[iclass] = 0;
}

RACE_DATA::~RACE_DATA()
{
        if (this->_name)
			STRFREE(this->_name);
}

void RACE_DATA::save()
{
        FILE     *fp;
        char filename[256];
        char buf[MAX_STRING_LENGTH];
        int iclass;

        if (!this)
        {
                bug("fwrite_race: null race pointer!", 0);
                return;
        }

        if (!this->_name || this->_name[0] == '\0')
        {
                snprintf(buf, MSL, "%s", "fwrite_race: race has no name");
                bug(buf, 0);
                return;
        }

        snprintf(filename, 256, "%s%s.race", RACES_DIR,
                 smash_space(this->_name));

        FCLOSE(fpReserve);
        if ((fp = fopen(filename, "w")) == NULL)
        {
                bug("fwrite_race: fopen", 0);
                perror(filename);
                fpReserve = fopen(NULL_FILE, "r");
        }
        else
        {
                fprintf(fp, "#RACE\n");
                fprintf(fp, "Name              %s~\n", this->_name);
                fprintf(fp, "Affect            %d\n", this->_affected);
                fprintf(fp, "Strplus           %d\n", this->_str_plus);
                fprintf(fp, "Wisplus           %d\n", this->_wis_plus);
                fprintf(fp, "Dexplus           %d\n", this->_dex_plus);
                fprintf(fp, "Conplus           %d\n", this->_con_plus);
                fprintf(fp, "Intplus           %d\n", this->_int_plus);
                fprintf(fp, "Chaplus           %d\n", this->_cha_plus);
                fprintf(fp, "Lckplus           %d\n", this->_lck_plus);
                fprintf(fp, "Frcplus           %d\n", this->_frc_plus);
                fprintf(fp, "Home              %d\n", this->_home);
                fprintf(fp, "Hit               %d\n", this->_hit);
                fprintf(fp, "Endurance              %d\n", this->_endurance);
                fprintf(fp, "DeathAge          %d\n", this->_death_age);
                fprintf(fp, "StartAge          %d\n", this->_start_age);
                fprintf(fp, "HungerMod         %d\n", this->_hunger_mod);
                fprintf(fp, "ThirstMod         %d\n", this->_thirst_mod);
                fprintf(fp, "LangBonus         %d\n", this->_lang_bonus);
                fprintf(fp, "RPneeded          %d\n", this->_rpneeded);
                fprintf(fp, "Language          %s~\n", this->_language->name);
                fprintf(fp, "BodyParts         %s\n",
                        print_bitvector(&this->_body_parts));
                fprintf(fp, "ClassRestricted   %d\n",
                        this->_class_restriction);
                for (iclass = 0; iclass <= MAX_ABILITY; iclass++)
                        fprintf(fp, "Class             %d\n",
                                this->_class_modifier[iclass]);
                fprintf(fp, "End\n\n");
                fprintf(fp, "#END\n");
        }
        FCLOSE(fp);
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

void RACE_DATA::load(FILE * fp)
{
        char buf[MAX_STRING_LENGTH];
        const char *word;
        bool fMatch;
        int iclass = 0;

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
                        KEY("Affect", this->_affected, fread_number(fp));
                        break;
                case 'B':
                        KEY("BodyParts", this->_body_parts,
                            fread_bitvector(fp));
                        break;
                case 'C':
                        KEY("Conplus", this->_con_plus, fread_number(fp));
                        KEY("ClassRestricted", this->_class_restriction,
                            fread_number(fp));
                        if (!str_cmp(word, "Class"))
                        {
                                fMatch = TRUE;
                                if (iclass > MAX_ABILITY)
                                        break;
                                this->_class_modifier[iclass] =
                                        fread_number(fp);
                                iclass++;
                                break;
                        }
                        KEY("Chaplus", this->_cha_plus, fread_number(fp));
                        break;
                case 'D':
                        KEY("DeathAge", this->_death_age, fread_number(fp));
                        KEY("Dexplus", this->_dex_plus, fread_number(fp));
                        break;
                case 'E':
                        KEY("Endurance", this->_endurance, fread_number(fp));
                        if (!str_cmp(word, "End"))
                        {
                                if (this->_name == NULL)
                                        bug("Error loading race");
                                this->_attr_mod[ATTR_STRENGTH] =
                                        this->_str_plus;
                                this->_attr_mod[ATTR_CONSTITUTION] =
                                        this->_con_plus;
                                this->_attr_mod[ATTR_DEXTERITY] =
                                        this->_dex_plus;
                                this->_attr_mod[ATTR_WISDOM] =
                                        this->_wis_plus;
                                this->_attr_mod[ATTR_INTELLIGENCE] =
                                        this->_int_plus;
                                this->_attr_mod[ATTR_CHARISMA] =
                                        this->_cha_plus;
                                this->_attr_mod[ATTR_LUCK] = this->_lck_plus;
                                this->_attr_mod[ATTR_FORCE] = this->_frc_plus;

                        }
                        return;
                case 'F':
                        KEY("Frcplus", this->_frc_plus, fread_number(fp));
                        break;
                case 'H':
                        KEY("Hit", this->_hit, fread_number(fp));
                        KEY("Home", this->_home, fread_number(fp));
                        KEY("HungerMod", this->_hunger_mod, fread_number(fp));
                        break;
                case 'I':
                        KEY("Intplus", this->_int_plus, fread_number(fp));
                        break;
                case 'L':
                        if (!str_cmp(word, "Language"))
                        {
                                char     *temp = fread_string_nohash(fp);

                                this->_language = NULL;
                                this->_language = get_language(temp);
                                if (this->_language == NULL)
                                        bug("Error loading language %s",
                                            temp);
                                DISPOSE(temp);
                                fMatch = TRUE;
                                break;
                        }
                        KEY("Lckplus", this->_lck_plus, fread_number(fp));
                        KEY("LangBonus", this->_lang_bonus, fread_number(fp));
                        break;
                case 'M':
                        KEY("Mana", this->_endurance, fread_number(fp));
                        break;
                case 'N':
                        KEY("Name", this->_name, fread_string(fp));
                        break;
                case 'R':
                        KEY("RPneeded", this->_rpneeded, fread_number(fp));
                        break;
                case 'S':
                        KEY("Strplus", this->_str_plus, fread_number(fp));
                        KEY("Startage", this->_start_age, fread_number(fp));
                        break;
                case 'T':
                        KEY("ThirstMod", this->_thirst_mod, fread_number(fp));
                        break;

                case 'W':
                        KEY("Wisplus", this->_wis_plus, fread_number(fp));
                        break;
                }
                if (!fMatch)
                {
                        snprintf(buf, MSL, "Fread_race: no match: %s", word);
                        bug(buf, 0);
                }
        }
}

bool RACE_DATA::load_race_file(char *racefile)
{
        char filename[256];
        RACE_DATA *race;

        FILE     *fp;

        snprintf(filename, 256, "%s%s", RACES_DIR, racefile);

        if ((fp = fopen(filename, "r")) != NULL)
        {
                for (;;)
                {
                        char letter;
                        char     *word;

                        letter = fread_letter(fp);
                        if (letter == '*')
                        {
                                fread_to_eol(fp);
                                continue;
                        }

                        if (letter != '#')
                        {
                                bug("Load_race_file: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "RACE"))
                        {
                                race = new RACE_DATA;
                                race->load(fp);
                                races.push_back(race);
                                break;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                bug("Load_race_file: bad section: %s.", word);
                                break;
                        }
                }
                FCLOSE(fp);
                return TRUE;
        }
        return FALSE;
}

void RACE_DATA::fwrite_race_list(void)
{
        FILE     *fp;
        char filename[256];
        RACE_DATA *race = NULL;

        snprintf(filename, 256, "%s%s", RACES_DIR, FILE_RACE_LIST);
        fp = fopen(filename, "w");
        if (!fp)
        {
                bug("FATAL: cannot open race.lst for writing!\n\r", 0);
                return;
        }
        FOR_EACH_LIST(RACE_LIST, races, race)
                fprintf(fp, "%s.race\n", smash_space(race->name()));
        fprintf(fp, "$\n");
        FCLOSE(fp);
}

void RACE_DATA::load_races(void)
{
        FILE     *fpList;
        const char *filename;
        char racelist[256];
        char buf[MAX_STRING_LENGTH];

        snprintf(racelist, 256, "%s%s", RACES_DIR, FILE_RACE_LIST);
        FCLOSE(fpReserve);
        if ((fpList = fopen(racelist, "r")) == NULL)
        {
                perror(racelist);
                fpReserve = fopen(NULL_FILE, "r");
                return;
        }

        for (;;)
        {
                filename = feof(fpList) ? "$" : fread_word(fpList);
                if (filename[0] == '$')
                        break;

                if (!load_race_file((char *) filename))
                {
                        snprintf(buf, MSL, "Cannot load race file: %s",
                                 filename);
                        bug(buf, 0);
                }
        }
        FCLOSE(fpList);
        boot_log(" Done races");
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

RACE_DATA *get_race(char *string)
{
        RACE_DATA *race = NULL;

        FOR_EACH_LIST(RACE_LIST, races, race)
                if (!str_cmp(string, race->name()))
                return race;

        FOR_EACH_LIST(RACE_LIST, races, race)
                if (!str_prefix(string, race->name()))
                return race;
        return NULL;
}

RACE_DATA *get_race_number(int number)
{
        RACE_DATA *race = NULL;

        FOR_EACH_LIST(RACE_LIST, races, race)
                if (!str_cmp(race->name(), race_table[number].race_name))
                return race;
        return get_race("human");
}

CMDF do_setrace(CHAR_DATA * ch, char *argument)
{
        RACE_DATA *race = NULL;
        char arg1[MSL];
        char arg2[MSL];

        argument = one_argument(argument, arg1);
        argument = one_argument(argument, arg2);

        if (arg1[0] == '\0' || arg2[0] == '\0')
        {
                send_to_char
                        ("&cInvalid syntax, usage: setrace <race>  <field> <value>\n\r&YValid fields are:\n\r",
                         ch);
                send_to_char
                        ("&RAffected, Constitution, Strength, Wisdom, Intelligence, Dexteritry, Charisma\n\r",
                         ch);
                send_to_char
                        ("&Home, DeathAge, StartAge, HungerMod, ThirstMod, Lang_bonus, Body Parts, rpneeded\n\r",
                         ch);
                send_to_char
                        ("&RHit, Language, Luck, Force, Endurance, Name, \"save <race>/all\"\n\r",
                         ch);
                send_to_char
                        ("&RAlso available: Class modifier, IE, setrace <race> class <class> <modifier>\n\r",
                         ch);
                return;
        }

        if (!str_cmp(arg1, "save"))
        {
                if (!str_cmp(arg2, "all"))
                {
                        FOR_EACH_LIST(RACE_LIST, races, race) race->save();
                        return;
                }
        }

        if ((race = get_race(arg1)) == NULL)
        {
                send_to_char
                        ("&RThat is not a valid race, choose one of the following:\n\r",
                         ch);
                FOR_EACH_LIST(RACE_LIST, races, race)
                        ch_printf(ch, "&W\t%s\n\r", race->name());
                return;
        }

        if (!str_cmp(arg2, "save"))
        {
                /*
                 * Nothing 'cause it'll save 
                 */
        }
        else if (!str_cmp(arg2, "affected"))
        {
                int value;
                char arg3[MSL];

                if (!str_cmp(argument, "clear"))
                {
                        race->affected(0);
                }
                else
                {
                        while (argument[0] != '\0')
                        {
                                argument = one_argument(argument, arg3);
                                value = get_aflag(arg3);
                                if (value < 0 || value > 31)
                                        ch_printf(ch, "Unknown flag: %s\n\r",
                                                  arg3);
                                else
                                {
                                        int affected = race->affected();

                                        TOGGLE_BIT(affected, 1 << value);
                                        race->affected(affected);
                                }
                        }
                }
        }
        else if (!str_cmp(arg2, "home"))
        {
                race->home(atoi(argument));
        }
        else if (!str_cmp(arg2, "deathage"))
        {
                race->death_age(atoi(argument));
        }
        else if (!str_cmp(arg2, "hungermod"))
        {
                race->hunger_mod(atoi(argument));
        }
        else if (!str_cmp(arg2, "thirstmod"))
        {
                race->thirst_mod(atoi(argument));
        }
        else if (!str_cmp(arg2, "constitution"))
        {
                race->attr_modifier(ATTR_CONSTITUTION, atoi(argument));
        }
        else if (!str_cmp(arg2, "strength"))
        {
                race->attr_modifier(ATTR_STRENGTH, atoi(argument));
        }
        else if (!str_cmp(arg2, "wisdom"))
        {
                race->attr_modifier(ATTR_WISDOM, atoi(argument));
        }
        else if (!str_cmp(arg2, "intelligence"))
        {
                race->attr_modifier(ATTR_INTELLIGENCE, atoi(argument));
        }
        else if (!str_cmp(arg2, "lang_bonus"))
        {
                race->lang_bonus(atoi(argument));
        }
        else if (!str_cmp(arg2, "dexterity"))
        {
                race->attr_modifier(ATTR_DEXTERITY, atoi(argument));
        }
        else if (!str_cmp(arg2, "charisma"))
        {
                race->attr_modifier(ATTR_CHARISMA, atoi(argument));
        }
        else if (!str_cmp(arg2, "hit"))
        {
                race->hit(atoi(argument));
        }
        else if (!str_cmp(arg2, "classes"))
        {
                int i = 0;

                for (i = 0; i < MAX_ABILITY; i++)
                {
                        if (!str_cmp(argument, ability_name[i]))
                        {
                                int a = race->class_restriction();

                                TOGGLE_BIT(a, 1 << i);
                                race->class_restriction(a);
                                send_to_char("Done.\n\r", ch);
                                return;
                        }
                }
                send_to_char("No such class.\n\r", ch);
                return;
        }
        else if (!str_cmp(arg2, "language"))
        {
                race->language(get_language(argument));
                if (race->language() == NULL)
                {
                        send_to_char
                                ("Not a valid language, defaulting to basic",
                                 ch);
                        race->language(get_language("basic"));
                        return;
                }
        }
        else if (!str_cmp(arg2, "luck"))
        {
                race->attr_modifier(ATTR_LUCK, atoi(argument));
        }
        else if (!str_cmp(arg2, "force"))
        {
                race->attr_modifier(ATTR_FORCE, atoi(argument));
        }
        else if (!str_cmp(arg2, "endurance"))
        {
                race->endurance(atoi(argument));
        }
        else if (!str_cmp(arg2, "startage"))
        {
                race->start_age(atoi(argument));
        }
        else if (!str_cmp(arg2, "rpneeded"))
        {
                race->rpneeded(atoi(argument));
        }
        else if (!str_cmp(arg2, "name"))
        {
                race->name(argument);
                RACE_DATA::fwrite_race_list();
        }
        else if (!str_cmp(arg2, "class"))
        {
                char arg3[MSL];
                int iclass;

                argument = one_argument(argument, arg3);
                if (argument[0] == '\0')
                {
                        send_to_char
                                ("Usage: setrace <race> class <class> argument",
                                 ch);
                        return;
                }
                for (iclass = 0; iclass < MAX_ABILITY; iclass++)
                        if (!str_prefix(arg3, ability_name[iclass]))
                                break;
                if (iclass == MAX_ABILITY)
                {
                        send_to_char("That is not a valid class.", ch);
                        return;
                }
                race->class_modifier(iclass, atoi(argument));
        }
        else if (!str_cmp(arg2, "parts"))
        {
                char arg3[MSL];
                int value;

                while (argument[0] != '\0')
                {
                        argument = one_argument(argument, arg3);
                        value = get_partflag(arg3);
                        if (value < 0 || value > MAX_BITS)
                                ch_printf(ch, "Unknown flag: %s\n\r", arg3);
                        else
                                xTOGGLE_BIT(race->body_parts(), value);
                }
        }
        else
        {
                send_to_char("Huh? Pick a valid one.", ch);
                return;
        }

        race->save();
        send_to_char("Race set.", ch);
}

CMDF do_showrace(CHAR_DATA * ch, char *argument)
{
        RACE_DATA *race;
        char arg1[MSL];
        int iclass;

        argument = one_argument(argument, arg1);

        if (arg1[0] == '\0')
        {
                send_to_char("&cInvalid syntax, usage: showrace <race>\n\r",
                             ch);
                return;
        }

        if ((race = get_race(arg1)) == NULL)
        {
                send_to_char
                        ("&RThat is not a valid race, choose one of the following:\n\r",
                         ch);
                FOR_EACH_LIST(RACE_LIST, races, race)
                        ch_printf(ch, "&W\t%s\n\r", race->name());
                return;
        }

        /*
         * Anyone else find this a waste of bandwidth ? - Gavin 
         */
        ch_printf(ch,
                  "\n\r&c==== &B%-15s &c====== &BLanguage: &w%-12s &c===========\n\r",
                  race->name(), capitalize(race->language()->name));
        send_to_char
                ("&c==============================================================\n\r"
                 "&c======= &BModifiers &c============================================\n\r"
                 "&c==============================================================\n\r",
                 ch);

        ch_printf(ch,
                  "&c==== &BStrength     : &w%-5d &c====================================\n\r",
                  race->attr_modifier(ATTR_STRENGTH));
        ch_printf(ch,
                  "&c==== &BConstitution : &w%-5d &c====================================\n\r",
                  race->attr_modifier(ATTR_CONSTITUTION));
        ch_printf(ch,
                  "&c==== &BDexterity    : &w%-5d &c====================================\n\r",
                  race->attr_modifier(ATTR_DEXTERITY));
        ch_printf(ch,
                  "&c==== &BIntelligence : &w%-5d &c====================================\n\r",
                  race->attr_modifier(ATTR_INTELLIGENCE));
        ch_printf(ch,
                  "&c==== &BWisdom       : &w%-5d &c====================================\n\r",
                  race->attr_modifier(ATTR_WISDOM));
        ch_printf(ch,
                  "&c==== &BCharisma     : &w%-5d &c====================================\n\r",
                  race->attr_modifier(ATTR_CHARISMA));
        ch_printf(ch,
                  "&c==== &BLuck         : &w%-5d &c====================================\n\r",
                  race->attr_modifier(ATTR_LUCK));
        ch_printf(ch,
                  "&c==== &BForce        : &w%-5d &c====================================\n\r",
                  race->attr_modifier(ATTR_FORCE));
        ch_printf(ch,
                  "&c==== &BHit Points   : &w%-5d &c====================================\n\r",
                  race->hit());
        ch_printf(ch,
                  "&c==== &BEndurance    : &w%-5d &c====================================\n\r",
                  race->endurance());
        /*
         * Home Planet / Number 
         */
        ch_printf(ch,
                  "&c==== &BHome         : &w%-5d &c====================================\n\r",
                  race->home());
        /*
         * Death Age 
         */
        ch_printf(ch,
                  "&c==== &BDeath Age    : &w%-5d &c====================================\n\r",
                  race->death_age());
        ch_printf(ch,
                  "&c==== &BStart Age    : &w%-5d &c====================================\n\r",
                  race->start_age());
        /*
         * Hunger Mod 
         */
        ch_printf(ch,
                  "&c==== &BHunger Mod   : &w%-5d &c====================================\n\r",
                  race->hunger_mod());
        /*
         * Thirst Mod 
         */
        ch_printf(ch,
                  "&c==== &BThirst Mod   : &w%-5d &c====================================\n\r",
                  race->thirst_mod());
        ch_printf(ch,
                  "&c==== &BRP Needed    : &w%-5d &c====================================\n\r",
                  race->rpneeded());
        ch_printf(ch,
                  "&c==== &BLang Bonus   : &w%-5d &c====================================\n\r",
                  race->lang_bonus());
        ch_printf(ch,
                  "&c==== &BAffected     : &w%-10s &c===============================\n\r",
                  affect_bit_name(race->affected()));

        if (race->class_restriction())
        {
                int iClass = 0;

                send_to_char
                        ("&c==============================================================\n\r"
                         "&c======= &BClass Restrictions &c=====================================\n\r"
                         "&c==============================================================\n\r",
                         ch);
                for (iClass = 0; iClass < MAX_ABILITY; iClass++)
                {
                        if (IS_SET(race->class_restriction(), 1 << iClass))
                        {
                                ch_printf(ch,
                                          "&c==== &BClass Name   : &w%-10s &c===============================\n\r",
                                          ability_name[iClass]);
                        }
                }

        }
        send_to_char
                ("&c==============================================================\n\r"
                 "&c======= &BClass Modifiers &c======================================\n\r"
                 "&c==============================================================\n\r",
                 ch);

        for (iclass = 0; iclass < MAX_ABILITY; iclass++)
                ch_printf(ch,
                          "&c==== &B%-15s : &w%-2d &c====================================\n\r",
                          capitalize(ability_name[iclass]),
                          race->class_modifier(iclass));

        send_to_char
                ("&c==============================================================\n\r"
                 "&c======= &BBody Parts &c===========================================\n\r"
                 "&c==============================================================\n\r",
                 ch);

        ch_printf(ch, "&c==== &w%-54s &c==\n\r",
                  xIS_EMPTY(race->
                            body_parts())? "None" : ext_flag_string(&race->
                                                                    body_parts
                                                                    (),
                                                                    part_flags));
        send_to_char
                ("&c==============================================================\n\r",
                 ch);
}

CMDF do_makerace(CHAR_DATA * ch, char *argument)
{
        RACE_DATA *race = NULL;

        if (!argument || argument[0] == '\0')
        {
                send_to_char("&BS&zyntax: makerace &w<race>&R&W", ch);
                return;
        }

        FOR_EACH_LIST(RACE_LIST, races, race)
        {
                if (race->name() && race->name()[0])

                        if (!str_cmp(race->name(), argument))
                        {
                                send_to_char("That race already exists!", ch);
                                return;
                        }
        }

        race = new RACE_DATA;
        races.push_back(race);
        race->name(argument);
        race->save();
        RACE_DATA::fwrite_race_list();
        ch_printf(ch, "Race \"%s\" created and saved.\n\r",
                  race ? race->name()? race->
                  name() : "<ERROR1>" : "<ERROR2>");
}
