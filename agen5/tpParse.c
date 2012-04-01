
/**
 * @file tpParse.c
 *
 * Time-stamp:        "2012-03-31 13:53:03 bkorb"
 *
 *  This module will load a template and return a template structure.
 *
 * This file is part of AutoGen.
 * Copyright (c) 1992-2012 Bruce Korb - all rights reserved
 *
 * AutoGen is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AutoGen is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#if defined(DEBUG_ENABLED)
static int tplNestLevel = 0;

static char const zTDef[] = "%-10s (%d) line %d end=%d, strlen=%d\n";
#endif

/* = = = START-STATIC-FORWARD = = = */
static mac_func_t
func_code(char const ** ppzScan);

static char const *
find_mac_end(char const ** ppzMark);

static char const *
find_mac_start(char const * pz, macro_t** ppM, templ_t* pTpl);

static char const *
find_macro(templ_t * pTpl, macro_t ** ppM, char const ** ppzScan);
/* = = = END-STATIC-FORWARD = = = */

/*
 *  Return the enumerated function type corresponding
 *  to a name pointed to by the input argument.
 */
static mac_func_t
func_code(char const ** ppzScan)
{
    fn_name_type_t const * pNT;
    char const *      pzFuncName = *ppzScan;
    int               hi, lo, av;
    int               cmp;

    /*
     *  IF the name starts with a punctuation, then it is some sort of
     *  alias.  Find the function in the alias portion of the table.
     */
    if (IS_PUNCTUATION_CHAR(*pzFuncName)) {
        hi = FUNC_ALIAS_HIGH_INDEX;
        lo = FUNC_ALIAS_LOW_INDEX;
        do  {
            av  = (hi + lo)/2;
            pNT = fn_name_types + av;
            cmp = (int)(*(pNT->pName)) - (int)(*pzFuncName);

            /*
             *  For strings that start with a punctuation, we
             *  do not need to test for the end of token
             *  We will not strip off the marker and the load function
             *  will figure out what to do with the code.
             */
            if (cmp == 0)
                return pNT->fType;
            if (cmp > 0)
                 hi = av - 1;
            else lo = av + 1;
        } while (hi >= lo);
        return FTYP_BOGUS;
    }

    if (! IS_VAR_FIRST_CHAR(*pzFuncName))
        return FTYP_BOGUS;

    hi = FUNC_NAMES_HIGH_INDEX;
    lo = FUNC_NAMES_LOW_INDEX;

    do  {
        av  = (hi + lo)/2;
        pNT = fn_name_types + av;
        cmp = strneqvcmp(pNT->pName, pzFuncName, (int)pNT->cmpLen);
        if (cmp == 0) {
            /*
             *  Make sure we matched to the end of the token.
             */
            if (IS_VARIABLE_NAME_CHAR(pzFuncName[pNT->cmpLen]))
                break;

            /*
             *  Advance the scanner past the macro name.
             *  The name is encoded in the "fType".
             */
            *ppzScan = pzFuncName + pNT->cmpLen;
            return pNT->fType;
        }
        if (cmp > 0)
             hi = av - 1;
        else lo = av + 1;
    } while (hi >= lo);

    /*
     *  Save the name for later lookup
     */
    cur_macro->md_name_off = (current_tpl->td_scan - current_tpl->td_text);
    {
        char * pzCopy = current_tpl->td_scan;
        char * pe = SPN_VALUE_NAME_CHARS(pzFuncName);
        size_t l  = pe - pzFuncName;
        memcpy(pzCopy, pzFuncName, l);
        pzCopy     += l;
        pzFuncName += l;

        /*
         *  Names are allowed to contain colons, but not end with them.
         */
        if (pzCopy[-1] == ':')
            pzCopy--, pzFuncName--;

        *(pzCopy++) = NUL;
        *ppzScan = pzFuncName;
        current_tpl->td_scan = pzCopy;
    }

    /*
     *  "Unknown" means we have to check again before we
     *  know whether to assign it to "FTYP_INVOKE" or "FTYP_COND".
     *  That depends on whether or not we find a named template
     *  at template instantiation time.
     */
    return FTYP_UNKNOWN;
}

static char const *
find_mac_end(char const ** ppzMark)
{
    char const * pzMark = *ppzMark + st_mac_len;
    char const * pzFunc;
    char const * pzNextMark;
    char const * pzEndMark;

    /*
     *  Set our pointers to the start of the macro text
     */
    for (;;) {
        pzMark = SPN_NON_NL_WHITE_CHARS(pzMark);
        if (*pzMark != NL)
            break;
        tpl_line++;
        pzMark++;
    }

    pzFunc             = pzMark;
    cur_macro->md_code = func_code(&pzMark);
    cur_macro->md_line = tpl_line;
    *ppzMark           = pzMark;

    /*
     *  Find the end.  (We must.)  If the thing is empty, treat as a comment,
     *  but warn about it.
     */
    pzEndMark = strstr(pzMark, end_mac_mark);
    if (pzEndMark == NULL)
        AG_ABEND(FIND_MAC_END_NOPE);

    if (pzEndMark == pzFunc) {
        cur_macro->md_code = FTYP_COMMENT;
        fprintf(trace_fp, FIND_MAC_END_EMPTY,
                current_tpl->td_file, tpl_line);
        return pzEndMark;
    }

    /*
     *  Back up over a preceding backslash.  It is a flag to indicate the
     *  removal of the end of line white space.
     */
    if (pzEndMark[-1] == '\\')
        pzEndMark--;

    pzNextMark = strstr(pzMark, st_mac_mark);
    if (pzNextMark == NULL)
        return pzEndMark;

    if (pzEndMark > pzNextMark)
        AG_ABEND(FIND_MAC_END_NESTED);

    return pzEndMark;
}

static char const *
find_mac_start(char const * pz, macro_t** ppM, templ_t* pTpl)
{
    char *       pzCopy;
    char const * pzEnd;
    char const * res = strstr(pz, st_mac_mark);
    macro_t *     pM  = *ppM;

    if (res == pz)
        return res;

    /*
     *  There is some text here.  Make a text macro entry.
     */
    pzCopy      = pTpl->td_scan;
    pzEnd       = (res != NULL) ? res : pz + strlen(pz);
    pM->md_txt_off = pzCopy - pTpl->td_text;
    pM->md_code = FTYP_TEXT;
    pM->md_line = tpl_line;

#if defined(DEBUG_ENABLED)
    if (HAVE_OPT(SHOW_DEFS)) {
        int ct = tplNestLevel;
        fprintf(trace_fp, "%3u ", (unsigned int)(pM - pTpl->td_macros));
        do { fputs("  ", trace_fp); } while (--ct > 0);

        fprintf(trace_fp, zTDef, apzFuncNames[ FTYP_TEXT ], FTYP_TEXT,
                pM->md_line, pM->md_end_idx, (unsigned int)(pzEnd - pz));
    }
#endif

    do  {
        if ((*(pzCopy++) = *(pz++)) == NL)
            tpl_line++;
    } while (pz < pzEnd);

    *(pzCopy++)   = NUL;
    *ppM          = pM + 1;
    pTpl->td_scan = pzCopy;

    return res;  /* may be NULL, if there are no more macros */
}

static char const *
find_macro(templ_t * pTpl, macro_t ** ppM, char const ** ppzScan)
{
    char const * pzScan = *ppzScan;
    char const * pzMark;

    pzMark = find_mac_start(pzScan, ppM, pTpl);

    /*
     *  IF no more macro marks are found, THEN we are done...
     */
    if (pzMark == NULL)
        return pzMark;

    /*
     *  Find the macro code and the end of the macro invocation
     */
    cur_macro = *ppM;
    pzScan    = find_mac_end(&pzMark);

    /*
     *  Count the lines in the macro text and advance the
     *  text pointer to after the marker.
     */
    {
        char const *  pzMacEnd = pzScan;
        char const *  pz       = pzMark;

        for (;;pz++) {
            pz = strchr(pz, NL);
            if ((pz == NULL) || (pz > pzMacEnd))
                break;
            tpl_line++;
        }

        /*
         *  Strip white space from the macro
         */
        pzMark = SPN_WHITESPACE_CHARS(pzMark);

        if (pzMark != pzMacEnd) {
            pzMacEnd = SPN_WHITESPACE_BACK( pzMark, pzMacEnd);
            (*ppM)->md_txt_off = (uintptr_t)pzMark;
            (*ppM)->md_res     = (long)(pzMacEnd - pzMark);
        }
    }

    /*
     *  IF the end macro mark was preceded by a backslash, then we remove
     *  trailing white space from there to the end of the line.
     */
    if ((*pzScan != '\\') || (strncmp(end_mac_mark, pzScan, end_mac_len) == 0))
        pzScan += end_mac_len;

    else {
        char const * pz;
        pzScan += end_mac_len + 1;
        pz = SPN_NON_NL_WHITE_CHARS(pzScan);
        if (*pz == NL) {
            pzScan = pz + 1;
            tpl_line++;
        }
    }

    *ppzScan = pzScan;
    return pzMark;
}

LOCAL macro_t *
parse_tpl(macro_t * mac, char const ** p_scan)
{
    char const * scan = *p_scan;
    templ_t *    tpl  = current_tpl;

#if defined(DEBUG_ENABLED)
    static char const zTUndef[] = "%-10s (%d) line %d - MARKER\n";

    #define DEBUG_DEC(l)  l--

    if (  ((tplNestLevel++) > 0)
       && HAVE_OPT(SHOW_DEFS)) {
        int ct = tplNestLevel;
        macro_t* pPm = mac-1;

        fprintf(trace_fp, "%3u ", (unsigned int)(pPm - tpl->td_macros));
        do { fputs("  ", trace_fp); } while (--ct > 0);

        fprintf(trace_fp, zTUndef, apzFuncNames[ pPm->md_code ],
                pPm->md_code, pPm->md_line);
    }
#else
    #define DEBUG_DEC(l)
#endif

    for (;;) {
        {
            char const * mark = find_macro(tpl, &mac, &scan);
            if (mark == NULL)
                break;
        }

        /*
         *  IF the called function returns a NULL next macro pointer,
         *  THEN some block has completed.  The returned scanning pointer
         *       will be non-NULL.
         */
        {
            tpLoadProc const fn = load_proc_table[mac->md_code];
            macro_t *   nxt_mac = fn(tpl, mac, &scan);

#if defined(DEBUG_ENABLED)
            if (HAVE_OPT(SHOW_DEFS)) {
                mac_func_t ft  = mac->md_code;
                int        ln  = mac->md_line;
                int ct = tplNestLevel;
                if (mac->md_code == FTYP_BOGUS)
                     fputs("    ", trace_fp);
                else fprintf(trace_fp, "%3u ",
                             (unsigned int)(mac - tpl->td_macros));

                do { fputs("  ", trace_fp); } while (--ct > 0);

                if (mac->md_code == FTYP_BOGUS)
                     fprintf(trace_fp, zTUndef, apzFuncNames[ ft ], ft, ln);
                else {
                    char const * pz;
                    if (ft >= FUNC_CT)
                        ft = FTYP_SELECT;
                    pz = (mac->md_txt_off == 0)
                        ? zNil
                        : (tpl->td_text + mac->md_txt_off);
                    fprintf(trace_fp, zTDef, apzFuncNames[ft], mac->md_code,
                            ln, mac->md_end_idx, (unsigned int)strlen(pz));
                }
            }
#endif

            if (nxt_mac == NULL) {
                *p_scan = scan;
                DEBUG_DEC(tplNestLevel);
                return mac;
            }
            mac = nxt_mac;
        }
    }

    DEBUG_DEC(tplNestLevel);

    /*
     *  We reached the end of the input string.
     *  Return a NULL scanning pointer and a pointer to the end.
     */
    *p_scan = NULL;
    return mac;
}
/*
 * Local Variables:
 * mode: C
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of agen5/tpParse.c */
