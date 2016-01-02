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
 *                $Id: wedding.c 1330 2005-12-05 03:23:24Z halkeye $                *
 ****************************************************************************************/
/***************************************************************************
*	MARRY.C written by Ryouga for Vilaross Mud (baby.indstate.edu 4000)	   *
*	Please leave this and all other credit include in this package.        *
*	Email questions/comments to ryouga@jessi.indstate.edu		           *
****************************************************************************
*	Modified By Gavin(ur_gavin@hotmail.com) for Unkown Regions             *
***************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


CMDF do_propose(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;

        if (IS_NPC(ch))
                return;

        if (IS_SET(ch->pcdata->flags, PCFLAG_MARRIED))
        {
                send_to_char("But you are already married!\n\r", ch);
                return;
        }

        if (ch->pcdata->spouse && ch->pcdata->spouse[0] != '\0')
        {
                send_to_char("But you are already engaged!\n\r", ch);
                return;
        }

        if (argument[0] == '\0')
        {
                send_to_char("Who do you wish to propose marriage to?\n\r",
                             ch);
                return;
        }

        if ((victim = get_char_room(ch, argument)) == NULL && victim != ch)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (victim->pcdata->spouse && victim->pcdata->spouse[0] != '\0')
        {
                if (IS_SET(victim->pcdata->flags, PCFLAG_MARRIED))
                        send_to_char("But they are already married!\n\r", ch);
                else
                        send_to_char("But they are already engaged!\n\r", ch);
                return;
        }

/*	if ( get_age(ch) < race_table[ch->race]->consent )
	{
		send_to_char("You must mature a little more before you start thinking about that.\n\r",ch);
		return;
	}
	if ( get_age(victim) < race_table[victim->race]->consent )
	{
		send_to_char("They must mature a little more before they should start thinking about that.\n\r",ch);
		return;
	}*/

/*	if ( ch->sex == victim->sex && ch->sex != SEX_NEUTRAL)
	{
		send_to_char("Not on this mud bub!\n\r",ch);
		return;
	}*/
        if (ch == victim)
        {
                send_to_char("Not on this mud bub!\n\r", ch);
                return;
        }

        ch->pcdata->propose = victim;
        victim->pcdata->propose = ch;
        act(AT_WHITE, "You propose marriage to $M.", ch, NULL, victim,
            TO_CHAR);
        act(AT_WHITE, "$n gets down on one knee and proposes to $N.", ch,
            NULL, victim, TO_NOTVICT);
        act(AT_WHITE, "$n asks you quietly 'Will you marry me?'", ch, NULL,
            victim, TO_VICT);
        return;
}

CMDF do_accept(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;

        argument = NULL;

        if (IS_NPC(ch))
                return;

        if ((victim = ch->pcdata->propose) == NULL)
        {
                send_to_char("Nobody proposed to you.", ch);
                return;
        }

        if (victim->pcdata->propose != ch)
        {
                send_to_char
                        ("They seemed to have proposed to someone else in the mean time.",
                         ch);
                return;
        }

        if (victim == NULL || victim->in_room != ch->in_room)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }

        if (IS_SET(ch->pcdata->flags, PCFLAG_MARRIED))
        {
                send_to_char("You are already married!\n\r", ch);
                return;
        }

        if (IS_SET(victim->pcdata->flags, PCFLAG_MARRIED))
        {
                send_to_char("They are already married!\n\r", ch);
                return;
        }

        victim->pcdata->propose = NULL;
        ch->pcdata->propose = NULL;
        REMOVE_BIT(ch->pcdata->flags, PCFLAG_MARRIED);
        REMOVE_BIT(victim->pcdata->flags, PCFLAG_MARRIED);
        if (victim->pcdata->spouse)
                STRFREE(victim->pcdata->spouse);
        if (ch->pcdata->spouse)
                STRFREE(ch->pcdata->spouse);
        victim->pcdata->spouse = STRALLOC(ch->name);
        ch->pcdata->spouse = STRALLOC(victim->name);
        act(AT_WHITE, "You accept $S offer of marriage.", ch, NULL, victim,
            TO_CHAR);
        act(AT_WHITE, "$n accepts $N's offer of marriage.", ch, NULL, victim,
            TO_NOTVICT);
        act(AT_WHITE, "$n accepts your offer of marriage.", ch, NULL, victim,
            TO_VICT);
        act(AT_WHITE, "$n and $N are now engaged!", ch, NULL, victim,
            TO_NOTVICT);
        save_char_obj(victim);
        save_char_obj(ch);
        return;
}


CMDF do_refuse(CHAR_DATA * ch, char *argument)
{
        CHAR_DATA *victim;

        argument = NULL;

        if (IS_NPC(ch))
                return;

        if ((victim = ch->pcdata->propose) == NULL)
        {
                send_to_char("Nobody proposed to you.", ch);
                return;
        }

        if (victim->pcdata->propose != ch)
        {
                send_to_char
                        ("They seemed to have proposed to someone else in the mean time.",
                         ch);
                return;
        }

        if (victim == NULL || victim->in_room != ch->in_room)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }

        if (IS_SET(ch->pcdata->flags, PCFLAG_MARRIED))
        {
                send_to_char("You are already married!\n\r", ch);
                return;
        }

        if (IS_SET(victim->pcdata->flags, PCFLAG_MARRIED))
        {
                send_to_char("They are already married!\n\r", ch);
                return;
        }

        victim->pcdata->propose = NULL;
        ch->pcdata->propose = NULL;

        act(AT_WHITE, "$N refused $n's offer of engagement!", ch, NULL,
            victim, TO_NOTVICT);
        act(AT_WHITE, "$N refuses your offer of engagement!", ch, NULL,
            victim, TO_CHAR);
        act(AT_WHITE, "You refuse $N's offer of engagement!", ch, NULL,
            victim, TO_VICT);
}

CMDF do_marry(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim1;
        CHAR_DATA *victim2;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char
                        ("&RSyntax: &Gmarry &C<&cperson&C> &C<&cperson&C>\n\r",
                         ch);
                return;
        }

        if ((victim1 = get_char_room(ch, arg)) == NULL && victim1 != ch)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }

        if ((victim2 = get_char_room(ch, argument)) == NULL && victim2 != ch)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim1) || IS_NPC(victim2))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }

        if (IS_SET(victim1->pcdata->flags, PCFLAG_MARRIED) ||
            IS_SET(victim2->pcdata->flags, PCFLAG_MARRIED))
        {
                send_to_char("They are already married!\n\r", ch);
                return;
        }

        if (!str_cmp(victim1->name, victim2->pcdata->spouse) &&
            !str_cmp(victim2->name, victim1->pcdata->spouse))
        {
                SET_BIT(victim1->pcdata->flags, PCFLAG_MARRIED);
                SET_BIT(victim2->pcdata->flags, PCFLAG_MARRIED);
                save_char_obj(victim1);
                save_char_obj(victim2);
                act(AT_WHITE, "You are now married to $N! Congrats!", victim1,
                    NULL, victim2, TO_VICT);
                act(AT_WHITE, "You are now married to $n! Congrats!", victim1,
                    NULL, victim2, TO_CHAR);
                act(AT_WHITE, "$n and $N are now married!", victim1, NULL,
                    victim2, TO_NOTVICT);
                return;
        }

        send_to_char("You can't do that to them! They are not ready.\n\r",
                     ch);
        return;
}

CMDF do_divorce(CHAR_DATA * ch, char *argument)
{
        char      arg[MAX_INPUT_LENGTH];
        CHAR_DATA *victim1;
        CHAR_DATA *victim2;

        argument = one_argument(argument, arg);

        if (arg[0] == '\0' || argument[0] == '\0')
        {
                send_to_char
                        ("&RSyntax: &Gdivorce &C<&cperson&C> &C<&cperson&C>\n\r",
                         ch);
                return;
        }

        if ((victim1 = get_char_room(ch, arg)) == NULL && victim1 != ch)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }

        if ((victim2 = get_char_room(ch, argument)) == NULL && victim2 != ch)
        {
                send_to_char("They are not here.\n\r", ch);
                return;
        }

        if (IS_NPC(victim1) || IS_NPC(victim2))
        {
                send_to_char("Not on NPC's.\n\r", ch);
                return;
        }


        if (!str_cmp(victim1->name, victim2->pcdata->spouse) &&
            !str_cmp(victim2->name, victim1->pcdata->spouse))
        {
                if (!IS_SET(victim1->pcdata->flags, PCFLAG_MARRIED) ||
                    !IS_SET(victim2->pcdata->flags, PCFLAG_MARRIED))
                {
                        send_to_char("They are not married!\n\r", ch);
                        return;
                }

                REMOVE_BIT(victim1->pcdata->flags, PCFLAG_MARRIED);
                REMOVE_BIT(victim2->pcdata->flags, PCFLAG_MARRIED);
                STRFREE(victim1->pcdata->spouse);
                STRFREE(victim2->pcdata->spouse);

                victim1->pcdata->spouse = STRALLOC("");
                victim2->pcdata->spouse = STRALLOC("");

                save_char_obj(victim1);
                save_char_obj(victim2);
                act(AT_WHITE, "You are now divorced from $N!", victim1, NULL,
                    victim2, TO_VICT);
                act(AT_WHITE, "You are now divorced from $n!", victim1, NULL,
                    victim2, TO_CHAR);
                act(AT_WHITE, "$n and $N are now divorced!", victim1, NULL,
                    victim2, TO_NOTVICT);
                return;
        }

        send_to_char("They are not married!\n\r", ch);
        return;
}
