
/*
 *  stack.c
 *  $Id: stack.c,v 2.4 2000/03/11 18:31:30 bruce Exp $
 *  This is a special option processing routine that will save the
 *  argument to an option in a FIFO queue.
 */

/*
 *  Automated Options copyright 1992-1999 Bruce Korb
 *
 *  Automated Options is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  Automated Options is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Automated Options.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 *
 * As a special exception, Bruce Korb gives permission for additional
 * uses of the text contained in his release of AutoOpts.
 *
 * The exception is that, if you link the AutoOpts library with other
 * files to produce an executable, this does not by itself cause the
 * resulting executable to be covered by the GNU General Public License.
 * Your use of that executable is in no way restricted on account of
 * linking the AutoOpts library code into it.
 *
 * This exception does not however invalidate any other reasons why
 * the executable file might be covered by the GNU General Public License.
 *
 * This exception applies only to the code released by Bruce Korb under
 * the name AutoOpts.  If you copy code from other sources under the
 * General Public License into a copy of AutoOpts, as the General Public
 * License permits, the exception does not apply to the code that you add
 * in this way.  To avoid misleading anyone as to the status of such
 * modified files, you must delete this exception notice from them.
 *
 * If you write modifications of your own for AutoOpts, it is your choice
 * whether to permit this exception to apply to your modifications.
 * If you do not wish that, delete this exception notice.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "compat/compat.h"

#ifdef WITH_INCLUDED_REGEX
#  define REGEX_MALLOC
#  include "compat/gnu-regex.c"
#else
#  include <regex.h>
#endif

#include "autoopts.h"


    void
unstackOptArg( tOptions*  pOpts, tOptDesc*  pOptDesc )
{
    /*
     *  IF the target system is too feeble to have a POSIX regex,
     *  THEN we use a simple string compare for entry removal
     */
#ifdef HAVE_POSIX_REGCOMP
    regex_t   re;
    tArgList* pAL = (tArgList*)pOptDesc->optCookie;
    int       ct, sIdx, dIdx;
    char*     pzSrc;
    char*     pzEq;
    int       res;

    if (pAL == (tArgList*)NULL)
        return;

    if (regcomp( &re, pOptDesc->pzLastArg, REG_NOSUB ) != 0)
        return;

    /*
     *  search the list for the entry(s) to remove.  Entries that
     *  are removed are *not* copied into the result.  The source
     *  index is incremented every time.  The destination only when
     *  we are keeping a define.
     */
    for (sIdx = 0, dIdx = 0, ct = pAL->useCt; --ct >= 0; sIdx++) {
        pzSrc = pAL->apzArgs[ sIdx ];
        pzEq  = strchr( pzSrc, '=' );

        if (pzEq != (char*)NULL)
            *pzEq = NUL;

        res = regexec( &re, pzSrc, (size_t)0, (regmatch_t*)NULL, 0 );
        switch (res) {
        case 0:
            /*
             *  Remove this entry by reducing the in-use count
             *  and *not* putting the string pointer back into
             *  the list.
             */
            pAL->useCt--;
            break;

        default:
        case REG_NOMATCH:
            if (pzEq != (char*)NULL)
                *pzEq = '=';

            /*
             *  IF we have dropped an entry
             *  THEN we have to move the current one.
             */
            if (dIdx != sIdx)
                pAL->apzArgs[ dIdx ] = pzSrc;
            dIdx++;
        }
    }

    regfree( &re );
#endif
}


    void
stackOptArg( tOptions*  pOpts, tOptDesc* pOptDesc )
{
    tArgList* pAL;

    if (pOptDesc->optArgType == ARG_NONE)
        return;

    /*
     *  Being called is the most authoritative way to be sure an
     *  option wants to have its argument values stacked...
     */
    pOptDesc->fOptState |= OPTST_STACKED;

    /*
     *  IF this is a negated ('+'-marked) option
     *  THEN we unstack the argument
     */
    if (DISABLED_OPT( pOptDesc )) {
        unstackOptArg( pOpts, pOptDesc );
        return;
    }

    pAL = (tArgList*)pOptDesc->optCookie;

    if (pOptDesc->pzLastArg == (char*)NULL)
        return;

    /*
     *  IF we have never allocated one of these,
     *  THEN allocate one now
     */
    if (pAL == (tArgList*)NULL) {
        pAL = (tArgList*)AGALOC( sizeof( *pAL ));
        if (pAL == (tArgList*)NULL)
            return;
        pAL->useCt   = 0;
        pAL->allocCt = MIN_ARG_ALLOC_CT;
    }

    /*
     *  ELSE if we are out of room
     *  THEN make it bigger
     */
    else if (pAL->useCt >= pAL->allocCt) {
        pAL->allocCt += INCR_ARG_ALLOC_CT;

        /*
         *  The base structure contains space for MIN_ARG_ALLOC_CT
         *  pointers.  We subtract it off to find our augment size.
         */
        pAL = (tArgList*)AGREALOC( (void*)pAL,
                sizeof(*pAL) + (sizeof(char*) * (pAL->allocCt
                             - MIN_ARG_ALLOC_CT)) );
        if (pAL == (tArgList*)NULL)
            return;
    }

    /*
     *  Insert the new argument into the list
     */
    pAL->apzArgs[ (pAL->useCt)++ ] = pOptDesc->pzLastArg;
    pOptDesc->optCookie = (void*)pAL;
}
/* stack.c ends here */
