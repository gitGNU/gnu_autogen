/*
 *  $Id: defFind.c,v 3.5 2002/12/07 04:45:03 bkorb Exp $
 *  This module loads the definitions, calls yyparse to decipher them,
 *  and then makes a fixup pass to point all children definitions to
 *  their parent definition (except the fixed "rootEntry" entry).
 */

/*
 *  AutoGen copyright 1992-2002 Bruce Korb
 *
 *  AutoGen is free software.
 *  You may redistribute it and/or modify it under the terms of the
 *  GNU General Public License, as published by the Free Software
 *  Foundation; either version 2, or (at your option) any later version.
 *
 *  AutoGen is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with AutoGen.  See the file "COPYING".  If not,
 *  write to:  The Free Software Foundation, Inc.,
 *             59 Temple Place - Suite 330,
 *             Boston,  MA  02111-1307, USA.
 */
#include "autogen.h"
#include "assert.h"

struct defEntryList {
    size_t         allocCt;
    size_t         usedCt;
    tDefEntry**    papDefEntry;
    int            nestLevel;
};
typedef struct defEntryList tDefEntryList;

tSCC zNameRef[]   = "Ill formed name ``%s'' in %s line %d\n";

tSC zDefinitionName[ MAXPATHLEN ];

STATIC tDefEntry* findEntryByIndex( tDefEntry* pE, char* pzScan );


STATIC void
illFormedName( void )
{
    char* pz = aprf( zNameRef, zDefinitionName,
                         pCurTemplate->pzFileName, pCurMacro->lineNo );
    AG_ABEND( pz );
}


STATIC tDefEntry*
findEntryByIndex( tDefEntry* pE, char* pzScan )
{
    int  idx;

    /*
     *  '[]' means the first entry of whatever index number
     */
    if (*pzScan == ']')
        return pE;

    /*
     *  '[$]' means the last entry of whatever index number
     */
    if (*pzScan == '$') {
        while (isspace( *++pzScan )) ;
        if (*pzScan != ']')
            return NULL;

        if (pE->pEndTwin != NULL)
            return pE->pEndTwin;

        return pE;
    }

    /*
     *  '[nn]' means the specified index number
     */
    if (isdigit( *pzScan )) {
        char* pz;
        idx = strtol( pzScan, &pz, 0 );

        /*
         *  Skip over any trailing space and make sure we have a closer
         */
        while (isspace( *pz )) pz++;
        if (*pz != ']')
            return NULL;
    }

    else {
        /*
         *  '[XX]' means get the index from our definitions
         */
        char* pzDef = pzScan;
        const char* pzVal;

        if (! isalpha( *pzScan ))
            return NULL;

        while (ISNAMECHAR( *pzScan )) pzScan++;

        /*
         *  Temporarily remove the character under *pzScan and
         *  find the corresponding defined value.
         */
        {
            char  svch = *pzScan;
            *pzScan = NUL;
            pzVal    = getDefine( pzDef );
            *pzScan = svch;
        }

        /*
         *  Skip over any trailing space and make sure we have a closer
         */
        while (isspace( *pzScan )) pzScan++;
        if (*pzScan != ']')
            return NULL;

        /*
         *  make sure we found a defined value
         */
        if ((pzVal == NULL) || (*pzVal == NUL))
            return NULL;

        idx = strtol( pzVal, &pzDef, 0 );

        /*
         *  Make sure we got a legal number
         */
        if (*pzDef != NUL)
            return NULL;
    }

    /*
     *  Search for the entry with the specified index.
     */
    do  {
        if (pE->index > idx)
            return NULL;
        if (pE->index == idx)
            break;
        pE = pE->pTwin;
    } while (pE != NULL);

    return pE;
}


/*
 *  find entry support routines:
 *
 *  addResult:  place a new definition entry on the end of the
 *              list of found definitions (reallocating list size as needed).
 */
static void
addResult( tDefEntry* pDE, tDefEntryList* pDEL )
{
    if (++(pDEL->usedCt) > pDEL->allocCt) {
        pDEL->allocCt += pDEL->allocCt + 8; /* 8, 24, 56, ... */
        pDEL->papDefEntry = (tDefEntry**)
            AGREALOC( (void*)pDEL->papDefEntry,
                      pDEL->allocCt * sizeof( void* ),
                      "added find result" );
    }

    pDEL->papDefEntry[ pDEL->usedCt-1 ] = pDE;
}


STATIC size_t
badName( char* pzD, const char* pzS, size_t srcLen )
{
    memcpy( (void*)pzD, (void*)pzS, srcLen );
    pzD[ srcLen ] = NUL;
    fprintf( pfTrace, zNameRef, pzD,
             pCurTemplate->pzFileName, pCurMacro->lineNo );
    return srcLen + 1;
}


/*
 *  canonicalizeName:  remove white space and roughly verify the syntax.
 *  This procedure will consume everything from the source string that
 *  forms a valid AutoGen compound definition name.
 *  We leave legally when:
 *  1.  the state is "CN_NAME_ENDED", AND
 *  2.  We stumble into a character that is not either '[' or '.'
 *      (always skipping white space).
 *  We start in CN_START.
 */
EXPORT int
canonicalizeName( char* pzD, const char* pzS, int srcLen )
{
    tSCC zNil[] = "";

    typedef enum {
        CN_START_NAME = 0,   /* must find a name */
        CN_NAME_ENDED,       /* must find '[' or '.' or we end */
        CN_INDEX,            /* must find name, number, '$' or ']' */
        CN_INDEX_CLOSE,      /* must find ']' */
        CN_INDEX_ENDED       /* must find '.' or we end */
    } teConState;

    teConState state = CN_START_NAME;

    const char* pzOri = pzS;
    char*       pzDst = pzD;
    size_t      stLen = srcLen;

    /*
     *  Before anything, skip a leading '.' as a special hack to force
     *  a current context lookup.
     */
    while (isspace( *pzS )) {
        if (--srcLen <= 0) {
            pzS = zNil;
            break;
        }
        pzS++;
    }

    if (*pzS == '.') {
        *(pzD++) = '.';
        pzS++;
    }

 nextSegment:
    /*
     *  The next segment may always start with an alpha character,
     *  but an index may also start with a number.  The full number
     *  validation will happen in findEntryByIndex().
     */
    while (isspace( *pzS )) {
        if (--srcLen <= 0) {
            pzS = zNil;
            break;
        }
        pzS++;
    }

    switch (state) {
    case CN_START_NAME:
        if (! isalpha( *pzS ))
            return badName( pzDst, pzOri, stLen );
        state = CN_NAME_ENDED;  /* we found the start of our first name */
        break;  /* fall through to name/number consumption code */

    case CN_NAME_ENDED:
        switch (*pzS++) {
        case '[':
            *(pzD++) = '[';
            state = CN_INDEX;
            break;

        case '.':
            *(pzD++) = '.';
            state = CN_START_NAME;
            break;

        default:
            /* legal exit -- we have a name already */
            *pzD = NUL;
            return srcLen;
        }

        if (--srcLen <= 0)
            return badName( pzDst, pzOri, stLen );
        goto nextSegment;

    case CN_INDEX:
        /*
         *  An index.  Valid syntaxes are:
         *
         *    '[' <#define-d name> ']'
         *    '[' <number> ']'
         *    '['  '$'  ']'
         *    '['       ']'
         *
         *  We will check for and handle the last case right here.
         *  The next cycle must find the index closer (']').
         */
        state = CN_INDEX_CLOSE;

        /*
         *  Numbers and #define-d names are handled at the end of the switch.
         *  '$' and ']' are handled immediately below.
         */
        if (isalnum( *pzS ))
            break;

        /*
         *  A solitary '$' is the highest index, whatever that happens to be
         *  We process that right here because down below we only accept
         *  name-type characters and this is not VMS.
         */
        if (*pzS == '$') {
            if (--srcLen < 0)
                return badName( pzDst, pzOri, stLen );

            *(pzD++) = *(pzS++);
            goto nextSegment;
        }
        /* FALLTHROUGH */

    case CN_INDEX_CLOSE:
        /*
         *  Nothing else is okay.
         */
        if ((*(pzD++) = *(pzS++)) != ']')
            return badName( pzDst, pzOri, stLen );

        if (--srcLen <= 0) {
            *pzD = NUL;
            return srcLen;
        }
        state = CN_INDEX_ENDED;
        goto nextSegment;

    case CN_INDEX_ENDED:
        if ((*pzS != '.') || (--srcLen < 0)) {
            *pzD = NUL;
            return srcLen;
        }
        *(pzD++) = *(pzS++);

        state = CN_START_NAME;
        goto nextSegment;
    }

    /*
     *  The next state must be either looking for what comes after the
     *  end of a name, or for the close bracket after an index.
     *  Whatever, the next token must be a name or a number.
     */
    assert((state == CN_NAME_ENDED) || (state == CN_INDEX_CLOSE));
    assert( isalnum( *pzS ));

    /*
     *  Copy the name/number.  We already know the first character is valid.
     *  However, we must *NOT* downcase #define names...
     */
    while (ISNAMECHAR( *pzS )) {
        char ch = *(pzS++);
        if ((state != CN_INDEX_CLOSE) && isupper( ch ))
            *(pzD++) = tolower( ch );

        else switch ( ch ) { /* force the separator chars to be '_' */
        case '-':
        case '^':
            *(pzD++) = '_';
            break;

        default:
            *(pzD++) = ch;
        }

        if (--srcLen <= 0) {
            pzS = zNil;
            break;
        }
    }

    goto nextSegment;
}


/*
 *  findDefEntry
 *
 *  Find the definition entry for the name passed in.
 *  It is okay to find block entries IFF they are found on the
 *  current level.  Once you start traversing up the tree,
 *  the macro must be a text macro.  Return an indicator saying if
 *  the element has been indexed (so the caller will not try
 *  to traverse the list of twins).
 */
STATIC tDefEntry*
defEntrySearch( char* pzName, tDefStack* pDefStack, ag_bool* pIsIndexed )
{
    char*        pcBrace;
    char         breakCh;
    tDefEntry*   pE;
    ag_bool      dummy;
    ag_bool      noNesting    = AG_FALSE;

    static int   nestingDepth = 0;

    /*
     *  IF we are at the start of a search, then canonicalize the name
     *  we are hunting for, copying it to a modifiable buffer, and
     *  initialize the "indexed" boolean to false (we have not found
     *  an index yet).
     */
    if (nestingDepth == 0) {
        canonicalizeName( zDefinitionName, pzName, strlen( pzName ));
        pzName = zDefinitionName;

        if (pIsIndexed != NULL)
             *pIsIndexed = AG_FALSE;
        else pIsIndexed  = &dummy;

        if (*pzName == '.') {
            noNesting = AG_TRUE;
            pzName++;
        }
    }

    pcBrace  = pzName + strcspn( pzName, "[." );
    breakCh  = *pcBrace;
    *pcBrace = NUL;

    *pIsIndexed |= (breakCh == '[');

    for (;;) {
        /*
         *  IF we are at the end of the definitions (reached ROOT),
         *  THEN it is time to bail out.
         */
        pE = pDefStack->pDefs;
        if (pE == NULL)
            return NULL;

        do  {
            /*
             *  IF the name matches
             *  THEN break out of the double loop
             */
            if (strcmp( pE->pzDefName, pzName ) == 0)
                goto found_def_entry;

            pE = pE->pNext;
        } while (pE != NULL);

        /*
         *  IF we are nested, then we cannot change the definition level.
         *  So, we did not find anything.
         */
        if ((nestingDepth != 0) || noNesting)
            return NULL;

        /*
         *  Let's go try the definitions at the next higher level.
         */
        pDefStack = pDefStack->pPrev;
        if (pDefStack == NULL)
            return NULL;
    } found_def_entry:;

    /*
     *  At this point, we have found the entry that matches the supplied
     *  name, up to the '[' or '.' or NUL character.  It *must* be one of
     *  those three characters.
     */
    *pcBrace = breakCh;

    switch (breakCh) {
    case NUL:
        return pE;

    case '[':
        /*
         *  We have to find a specific entry in a list.
         */
        while (isspace( *++pcBrace )) ;

        pE = findEntryByIndex( pE, pcBrace );
        if (pE == NULL)
            return pE;

        /*
         *  We must find the closing brace, or there is an error
         */
        pcBrace = strchr( pcBrace, ']' );
        if (pcBrace == NULL)
            illFormedName();

        /*
         *  IF we are at the end of the definition,
         *  THEN return what we found
         */
        switch (*++pcBrace) {
        case NUL:
            return pE;

        case '.':
            break;

        default:
            illFormedName();
        }
        /* FALLTHROUGH */

    case '.':
        /*
         *  It is a segmented value name.  Set the name pointer
         *  to the next segment and search starting from the newly
         *  available set of definitions.
         */
        pzName = pcBrace + 1;
        break;

    default:
        illFormedName();
    }

    /*
     *  We cannot find a member of a non-block type macro definition.
     */
    if (pE->valType != VALTYP_BLOCK)
        return NULL;

    /*
     *  Loop through all the twins of the entry we found until
     *  we find the entry we want.  We ignore twins if we just
     *  used a subscript.
     */
    nestingDepth++;
    {
        tDefStack stack = { NULL, &currDefCtx };

        stack.pDefs = (tDefEntry*)pE->pzValue;

        for (;;) {
            tDefEntry* res;

            res = defEntrySearch( pzName, &stack, pIsIndexed );
            if ((res != NULL) || (breakCh == '[')) {
                nestingDepth--;
                return res;
            }
            pE = pE->pTwin;
            if (pE == NULL)
                break;
            stack.pDefs = (tDefEntry*)pE->pzValue;
        }
    }

    nestingDepth--;
    return NULL;
}


EXPORT tDefEntry*
findDefEntry( char* pzName, ag_bool* pIsIndexed )
{
    return defEntrySearch( pzName, &currDefCtx, pIsIndexed );
}


/*
 *  findEntryList
 *
 *  Find the definition entry for the name passed in.  It is okay to find
 *  block entries IFF they are found on the current level.  Once you start
 *  traversing up the tree, the macro must be a text macro.  Return an
 *  indicator saying if the element has been indexed (so the caller will
 *  not try to traverse the list of twins).
 */
STATIC tDefEntry**
entryListSearch( char* pzName, tDefStack* pDefStack )
{
    static tDefEntryList defList = { 0, 0, NULL, 0 };

    char*      pcBrace;
    char       breakCh;
    tDefEntry* pE;
    ag_bool    noNesting = AG_FALSE;

    /*
     *  IF we are at the start of a search, then canonicalize the name
     *  we are hunting for, copying it to a modifiable buffer, and
     *  initialize the "indexed" boolean to false (we have not found
     *  an index yet).
     */
    if (defList.nestLevel == 0) {
        canonicalizeName( zDefinitionName, pzName, strlen( pzName ));
        pzName = zDefinitionName;
        defList.usedCt = 0;

        if (*pzName == '.') {
            noNesting = AG_TRUE;
            pzName++;
        }
    }

    pcBrace  = pzName + strcspn( pzName, "[." );
    breakCh  = *pcBrace;
    *pcBrace = NUL;

    for (;;) {
        /*
         *  IF we are at the end of the definitions (reached ROOT),
         *  THEN it is time to bail out.
         */
        pE = pDefStack->pDefs;
        if (pE == NULL) {
            /*
             *  Make sure we are not nested.  Once we start to nest,
             *  then we cannot "change definition levels"
             */
        not_found:
            if (defList.nestLevel != 0)
                illFormedName();

            /*
             *  Don't bother returning zero entry list.  Just return NULL.
             */
            return NULL;
        }

        do  {
            /*
             *  IF the name matches
             *  THEN go add it, plus all its twins
             */
            if (strcmp( pE->pzDefName, pzName ) == 0)
                goto found_def_entry;

            pE = pE->pNext;
        } while (pE != NULL);

        /*
         *  IF we are nested, then we cannot change the definition level.
         *  Just go and return what we have found so far.
         */
        if ((defList.nestLevel != 0) || noNesting)
            goto returnResult;

        /*
         *  Let's go try the definitions at the next higher level.
         */
        pDefStack = pDefStack->pPrev;
        if (pDefStack == NULL)
            goto not_found;
    } found_def_entry:;

    /*
     *  At this point, we have found the entry that matches the supplied
     *  name, up to the '[' or '.' or NUL character.  It *must* be one of
     *  those three characters.
     */
    *pcBrace = breakCh;

    switch (breakCh) {
    case NUL:
        do  {
            addResult( pE, &defList );
            pE = pE->pTwin;
        } while (pE != NULL);
        goto returnResult;

    case '[':
        /*
         *  We have to find a specific entry in a list.
         */
        while (isspace( *++pcBrace )) ;

        pE = findEntryByIndex( pE, pcBrace );
        if (pE == NULL)
            goto returnResult;

        /*
         *  We must find the closing brace, or there is an error
         */
        pcBrace = strchr( pcBrace, ']' );
        if (pcBrace == NULL)
            illFormedName();

        /*
         *  IF we are at the end of the definition,
         *  THEN return what we found
         */
        switch (*++pcBrace) {
        case NUL:
            goto returnResult;

        case '.':
            break;

        default:
            illFormedName();
        }
        /* FALLTHROUGH */

    case '.':
        /*
         *  It is a segmented value name.  Set the name pointer
         *  to the next segment and search starting from the newly
         *  available set of definitions.
         */
        pzName = pcBrace + 1;
        break;

    default:
        illFormedName();
    }

    /*
     *  We cannot find a member of a non-block type macro definition.
     */
    if (pE->valType != VALTYP_BLOCK)
        return NULL;

    /*
     *  Loop through all the twins of the entry.  We ignore twins if we just
     *  used a subscript.
     */
    defList.nestLevel++;
    {
        tDefStack stack = { NULL, &currDefCtx };

        stack.pDefs = (tDefEntry*)pE->pzValue;

        for (;;) {
            (void)entryListSearch( pzName, &stack );
            if (breakCh == '[')
                break;
            pE = pE->pTwin;
            if (pE == NULL)
                break;
            stack.pDefs = (tDefEntry*)pE->pzValue;
        }
    }
    defList.nestLevel--;

 returnResult:
    if (defList.nestLevel == 0)
        addResult( NULL, &defList );

    return defList.papDefEntry;
}


EXPORT tDefEntry**
findEntryList( char* pzName )
{
    return entryListSearch( pzName, &currDefCtx );
}
/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of defFind.c */
