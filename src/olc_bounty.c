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
 *                $Id: olc_bounty.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
#include <sys/types.h>
#include <list>
#include "mud.h"
#include "olc_bounty.h"
#include "races.h"
#include "utils.h"

OLC_BOUNTY_LIST olc_bounties;

OLC_BOUNTY_DATA::OLC_BOUNTY_DATA()
{
        this->_owner = 2;
        this->_vnum = 0;
        this->_type = 0;
        this->_amount = 0;
        this->_exp = 0;
        this->_corpse = 0;
}

OLC_BOUNTY_DATA::OLC_BOUNTY_DATA(int vnum)
{
        this->_owner = vnum;
        this->_vnum = 0;
        this->_type = 0;
        this->_amount = 0;
        this->_exp = 0;
        this->_corpse = 0;
}

OLC_BOUNTY_DATA::~OLC_BOUNTY_DATA()
{
        std::list < OLC_BOUNTY_DATA * >::iterator iter;
        OLC_BOUNTY_DATA *bounty;

        for (iter = olc_bounties.begin(); iter != olc_bounties.end(); iter++)
        {
                bounty = (*iter);

                if (bounty == this)
                {
                        olc_bounties.erase(iter);
                        return;
                }
        }
}

void OLC_BOUNTY_DATA::save()
{
        FILE     *fp;

        std::list < OLC_BOUNTY_DATA * >::iterator iter;
        OLC_BOUNTY_DATA *bounty;

        FCLOSE(fpReserve);
        if ((fp = fopen(OLC_BOUNTY_FILE, "w")) == NULL)
        {
                bug("fwrite_olc_bounty: fopen", 0);
                perror(OLC_BOUNTY_FILE);
                fpReserve = fopen(NULL_FILE, "r");
        }
        else
        {
                for (iter = olc_bounties.begin(); iter != olc_bounties.end();
                     iter++)
                {
                        bounty = (*iter);
                        fprintf(fp, "#OLC_BOUNTY\n");
                        fprintf(fp, "Owner             %d\n", bounty->_owner);
                        fprintf(fp, "Vnum              %d\n", bounty->_vnum);
                        fprintf(fp, "Corpse            %d\n",
                                bounty->_corpse);
                        fprintf(fp, "Type              %d\n", bounty->_type);
                        fprintf(fp, "Experience        %d\n", bounty->_exp);
                        fprintf(fp, "Amount            %d\n",
                                bounty->_amount);
                        fprintf(fp, "End\n\n");
                }
                fprintf(fp, "#END\n");
                FCLOSE(fp);
        }
        fpReserve = fopen(NULL_FILE, "r");
        return;
}

void OLC_BOUNTY_DATA::load(FILE * fp)
{
        const char *word;
        bool fMatch;

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
                        KEY("Amount", this->_amount, fread_number(fp));
                        break;
                case 'C':
                        KEY("Corpse", this->_corpse, fread_number(fp));
                        break;
                case 'E':
                        KEY("Experience", this->_exp, fread_number(fp));
                        if (!str_cmp(word, "End"))
                        {
                                return;
                        }
                        break;
                case 'O':
                        KEY("Owner", this->_owner, fread_number(fp));
                        break;
                case 'T':
                        KEY("Type", this->_type, fread_number(fp));
                        break;
                case 'V':
                        KEY("Vnum", this->_vnum, fread_number(fp));
                        break;
                }

                if (!fMatch)
                {
                        bug("OLC_BOUNTY_DATA::load: no match: %s", word);
                }
        }
}

void load_olc_bounties(void)
{
        FILE     *fp;
        OLC_BOUNTY_DATA *bounty;

        if ((fp = fopen(OLC_BOUNTY_FILE, "r")) != NULL)
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
                                bug("Load_olc_bounties: # not found.", 0);
                                break;
                        }

                        word = fread_word(fp);
                        if (!str_cmp(word, "OLC_BOUNTY"))
                        {
                                bounty = new OLC_BOUNTY_DATA;
                                bounty->load(fp);
                                olc_bounties.push_back(bounty);
                                continue;
                        }
                        else if (!str_cmp(word, "END"))
                                break;
                        else
                        {
                                bug("load_olc_bounties: bad section.", 0);
                                continue;
                        }
                }
                FCLOSE(fp);
        }
        else
        {
				if (!file_exist(OLC_BOUNTY_FILE))
				{
  				    bug("olcbounty.dat does not exist", 0);
  				    bug("Creating empty file for future use", 0);
					bounty->save();
				}
				else
			    {
					bug("olcbounty.dat exists, but cannot be opened", 0);
                    exit(0);
				}
        }
}

bool check_olc_bounties(ROOM_INDEX_DATA * room)
{
        CHAR_DATA *mob;

        if (!room)
        {
                return FALSE;
        }

        for (mob = room->first_person; mob; mob = mob->next_in_room)
        {
                if (!IS_NPC(mob))
                {
                        continue;
                }
                if (mob->spec_fun == spec_lookup("spec_bountyhunter"))
                {
                        return TRUE;
                }
        }
        return FALSE;
}

bool check_olc_bounties(CHAR_DATA * mob)
{

        if (!IS_NPC(mob))
        {
                return FALSE;
        }
        if (mob->spec_fun == spec_lookup("spec_bountyhunter"))
        {
                return TRUE;
        }
        return FALSE;
}

int print_olc_bounties(CHAR_DATA * ch)
{
        int count = 0;
        CHAR_DATA *mob;

        std::list < OLC_BOUNTY_DATA * >::const_iterator iter;
        OLC_BOUNTY_DATA *bounty;
        MOB_INDEX_DATA *imob;

        if (!ch)
        {
                return FALSE;
        }

        for (mob = ch->in_room->first_person; mob; mob = mob->next_in_room)
        {
                if (!IS_NPC(mob))
                {
                        continue;
                }
                if (mob->spec_fun == spec_lookup("spec_bountyhunter"))
                {
                        for (iter = olc_bounties.begin();
                             iter != olc_bounties.end(); iter++)
                        {
                                bounty = (*iter);
                                if (bounty->owner() == mob->pIndexData->vnum)
                                {
                                        if ((imob =
                                             get_mob_index(bounty->vnum())) !=
                                            NULL)
                                        {
                                                ch_printf(ch,
                                                          "&R%-26s   %-15d&B[&R%s&B]\n\r",
                                                          imob->short_descr,
                                                          bounty->amount(),
                                                          olc_bounty_types
                                                          [bounty->type()]);
                                                count++;
                                        }
                                }
                        }
                }
        }
        return count;
}

void print_olc_bounties_mob(CHAR_DATA * ch, CHAR_DATA * mob)
{
        int count = 0;

        std::list < OLC_BOUNTY_DATA * >::const_iterator iter;
        OLC_BOUNTY_DATA *bounty;

        if (!ch)
        {
                return;
        }

        if (mob->spec_fun == spec_lookup("spec_bountyhunter"))
        {
                for (iter = olc_bounties.begin(); iter != olc_bounties.end();
                     iter++)
                {
                        bounty = (*iter);
                        if (bounty->owner() == mob->pIndexData->vnum)
                        {
                                count++;
                                ch_printf(ch,
                                          "&B[&w%3.3d&B] &BType: &w%5.5s &BAmount: &w%10.10d &BExperience: &w%10.10d &BVnum: &w%7.7d  &BCorpse: &w%7.7d\n\r",
                                          count,
                                          olc_bounty_types[bounty->type()],
                                          bounty->amount(),
                                          bounty->experience(),
                                          bounty->vnum(), bounty->corpse());
                        }
                }
        }
}

OLC_BOUNTY_DATA *has_olc_bounty(CHAR_DATA * victim)
{
        std::list < OLC_BOUNTY_DATA * >::iterator iter;
        OLC_BOUNTY_DATA *bounty;

        if (!IS_NPC(victim))
        {
                return NULL;
        }
        for (iter = olc_bounties.begin(); iter != olc_bounties.end(); iter++)
        {
                bounty = (*iter);
                if (victim->pIndexData->vnum == bounty->vnum()
                    && bounty->type() == BOUNTY_ALIVE)
                {
                        return bounty;
                }
        }
        return NULL;
}

bool check_given_bounty(CHAR_DATA * ch, CHAR_DATA * hunter, OBJ_DATA * obj)
{
        std::list < OLC_BOUNTY_DATA * >::iterator iter;
        OLC_BOUNTY_DATA *bounty;
        char buf[MSL];

        if (!IS_NPC(hunter))
        {
                return FALSE;
        }
        for (iter = olc_bounties.begin(); iter != olc_bounties.end(); iter++)
        {
                bounty = (*iter);
                if (obj->pIndexData->vnum == bounty->corpse()
                    && bounty->type() == BOUNTY_DEAD)
                {
                        command_printf(hunter,
                                       "sayto %s Well done %s, you've killed it.",
                                       ch->name, ch->race->name());
                        act(AT_ACTION,
                            "$n takes the corpse and puts it in the back.",
                            hunter, NULL, NULL, TO_ROOM);
                        command_printf(hunter,
                                       "sayto %s Here is the payment for the kill.",
                                       ch->name);
                        ch->gold += bounty->amount();
                        sprintf(buf, "$n give you %d credits.",
                                bounty->amount());
                        act(AT_ACTION, buf, hunter, NULL, ch, TO_VICT);
                        gain_exp(ch, bounty->experience(), HUNTING_ABILITY);
                        ch_printf(ch,
                                  "You gain %d bounty hunting experience.",
                                  bounty->experience());
                        command_printf(hunter, "mpjunk %s", obj->short_descr);
                        return TRUE;
                }
        }
        return FALSE;
}

void mset_bounty(CHAR_DATA * ch, CHAR_DATA * mob, char *argument)
{
        OLC_BOUNTY_DATA *bounty;

        std::list < OLC_BOUNTY_DATA * >::iterator iter;
        char arg1[MAX_INPUT_LENGTH];
        char arg2[MAX_INPUT_LENGTH];
        char arg3[MAX_INPUT_LENGTH];
        int number = 0, count = 0, i = 0;

        if (NULLSTR(argument))
        {
                send_to_char("Usage: mset <victim> bounty create\n\r", ch);
                send_to_char
                        ("Usage: mset <victim> bounty <number> delete\n\r",
                         ch);
                send_to_char
                        ("Usage: mset <victim> bounty <number> <field> <value>\n\r",
                         ch);
                send_to_char
                        ("       Field being one of: vnum amount experience type corpse\n\r",
                         ch);
                return;
        }

        argument = one_argument(argument, arg1);
        if (NULLSTR(arg1))
        {
                send_to_char("No arg 1", ch);
                mset_bounty(ch, mob, "");
                return;
        }
        if (!str_cmp(arg1, "create"))
        {
                bounty = new OLC_BOUNTY_DATA(mob->pIndexData->vnum);
                olc_bounties.push_back(bounty);
                bounty->save();
                return;
        }

        if (!is_number(arg1))
        {
                send_to_char
                        ("You must specify which bounty with a number\n\r",
                         ch);
                mset_bounty(ch, mob, "");
                return;
        }

        number = atoi(arg1);

        for (iter = olc_bounties.begin(); iter != olc_bounties.end(); iter++)
        {
                bounty = (*iter);
                if (bounty->owner() == mob->pIndexData->vnum)
                        count++;
                if (count == number)
                {
                        break;
                }
        }
        if (number != count)
        {
                send_to_char
                        ("Please select a valid number for the bounty you want.\n\r",
                         ch);
                mset_bounty(ch, mob, "");
                return;
        }

        argument = one_argument(argument, arg2);

        if (NULLSTR(arg2))
        {
                send_to_char
                        ("Please specify the field that you want to edit.\n\r",
                         ch);
                mset_bounty(ch, mob, "");
                return;
        }

        if (!str_cmp(arg2, "delete"))
        {
                delete bounty;

                bounty->save();
                send_to_char("Bounty has been deleted.\n\r", ch);
                return;
        }

        argument = one_argument(argument, arg3);
        if (NULLSTR(arg3))
        {
                send_to_char
                        ("You must specify what you want the value to be.\n\r",
                         ch);
                mset_bounty(ch, mob, "");
                return;
        }

        if (!str_cmp(arg2, "amount"))
        {
                if (!is_number(arg3))
                {
                        send_to_char
                                ("You must specify a number value for the amount.\n\r",
                                 ch);
                        mset_bounty(ch, mob, "");
                        return;
                }
                number = atoi(arg3);
                bounty->amount(number);
                bounty->save();
                send_to_char("Bounty has been updated.\n\r", ch);
                return;
        }
        if (!str_cmp(arg2, "vnum"))
        {
                if (!is_number(arg3))
                {
                        send_to_char
                                ("You must specify a number value for the amount.\n\r",
                                 ch);
                        mset_bounty(ch, mob, "");
                        return;
                }
                number = atoi(arg3);
                bounty->vnum(number);
                bounty->save();
                send_to_char("Bounty has been updated.\n\r", ch);
                return;
        }
        if (!str_cmp(arg2, "corpse"))
        {
                if (!is_number(arg3))
                {
                        send_to_char
                                ("You must specify a number value for the amount.\n\r",
                                 ch);
                        mset_bounty(ch, mob, "");
                        return;
                }
                number = atoi(arg3);
                bounty->corpse(number);
                bounty->save();
                send_to_char("Bounty has been updated.\n\r", ch);
                return;
        }
        if (!str_cmp(arg2, "experience"))
        {
                if (!is_number(arg3))
                {
                        send_to_char
                                ("You must specify a number value for the amount.\n\r",
                                 ch);
                        mset_bounty(ch, mob, "");
                        return;
                }
                number = atoi(arg3);
                bounty->experience(number);
                bounty->save();
                send_to_char("Bounty has been updated.\n\r", ch);
                return;
        }
        if (!str_cmp(arg2, "type"))
        {
                if (is_number(arg3))
                {
                        send_to_char
                                ("You must specify a string value for the amount.\n\rAllowed types are:\n\r",
                                 ch);
                        for (i = 0; i < (NUMITEMS(olc_bounty_types) - 1); i++)
                        {
                                ch_printf(ch, "\t%s\n\r",
                                          olc_bounty_types[i]);
                        }
                        mset_bounty(ch, mob, "");
                        return;
                }

                for (i = 0; i < (NUMITEMS(olc_bounty_types) - 1); i++)
                {
                        if (!str_cmp(arg3, olc_bounty_types[i]))
                                break;
                }

                if (i == (NUMITEMS(olc_bounty_types) - 1))
                {
                        send_to_char
                                ("You must specify a string value for the amount.\n\rAllowed types are:\n\r",
                                 ch);
                        for (i = 0; i < (NUMITEMS(olc_bounty_types) - 1); i++)
                        {
                                ch_printf(ch, "\t%s\n\r",
                                          olc_bounty_types[i]);
                        }
                        mset_bounty(ch, mob, "");
                        return;
                }

                bounty->type(i);
                bounty->save();
                send_to_char("Bounty has been updated.\n\r", ch);
                return;
        }
        mset_bounty(ch, mob, "");
}

CMDF do_showallbounties(CHAR_DATA * ch, char *argument)
{
        OLC_BOUNTY_DATA *bounty;

        std::list < OLC_BOUNTY_DATA * >::iterator iter;

        if (olc_bounties.empty())
        {
                send_to_char("No OLC bounties.\n\r", ch);
                return;
        }

        for (iter = olc_bounties.begin(); iter != olc_bounties.end(); iter++)
        {
                bounty = (*iter);
                ch_printf(ch,
                          "&BType: &w%5.5s &BAmount: &w%10.10d &BExperience: &w%10.10d &BVnum: &w%7.7d &BCorpse: &w%7.7d &BOwner: &w%7.7d\n\r",
                          olc_bounty_types[bounty->type()], bounty->amount(),
                          bounty->experience(), bounty->vnum(),
                          bounty->corpse(), bounty->owner());

        }
}
