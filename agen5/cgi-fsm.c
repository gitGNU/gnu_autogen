/*  
 *  EDIT THIS FILE WITH CAUTION  (cgi-fsm.c)
 *  
 *  It has been AutoGen-ed  Wednesday March  3, 2005 at 08:58:11 PM PST
 *  From the definitions    cgi.def
 *  and the template file   fsm
 *
 *  Automated Finite State Machine
 *
 *  Copyright (c) 2001-2005  by  Bruce Korb
 *
 *  AutoFSM is free software copyrighted by Bruce Korb.
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name ``Bruce Korb'' nor the name of any other
 *     contributor may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *  
 *  AutoFSM IS PROVIDED BY Bruce Korb ``AS IS'' AND ANY EXPRESS
 *  OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED.  IN NO EVENT SHALL Bruce Korb OR ANY OTHER CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 *  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#define DEFINE_FSM
#include "cgi-fsm.h"
#include <stdio.h>

/*
 *  Do not make changes to this file, except between the START/END
 *  comments, or it will be removed the next time it is generated.
 */
/* START === USER HEADERS === DO NOT CHANGE THIS COMMENT */

#include "autogen.h"

/* END   === USER HEADERS === DO NOT CHANGE THIS COMMENT */

#ifndef NULL
#  define NULL 0
#endif

#ifndef tSCC
#  define tSCC static const char
#endif

/*
 *  Enumeration of the valid transition types
 *  Some transition types may be common to several transitions.
 */
typedef enum {
    CGI_TR_INVALID,
    CGI_TR_NAME_EQUAL,
    CGI_TR_SEPARATE,
    CGI_TR_STASH,
    CGI_TR_VALUE_ESCAPE
} te_cgi_trans;
#define CGI_TRANSITION_CT  5

/*
 *  the state transition handling map
 *  This table maps the state enumeration + the event enumeration to
 *  the new state and the transition enumeration code (in that order).
 *  It is indexed by first the current state and then the event code.
 */
typedef struct cgi_transition t_cgi_transition;
struct cgi_transition {
    te_cgi_state  next_state;
    te_cgi_trans  transition;
};
static const t_cgi_transition
cgi_trans_table[ CGI_STATE_CT ][ CGI_EVENT_CT ] = {

  /* STATE 0:  CGI_ST_INIT */
  { { CGI_ST_NAME, CGI_TR_STASH },                  /* EVT:  alpha */
    { CGI_ST_INVALID, CGI_TR_INVALID },             /* EVT:  name_char */
    { CGI_ST_INVALID, CGI_TR_INVALID },             /* EVT:  = */
    { CGI_ST_INVALID, CGI_TR_INVALID },             /* EVT:  + */
    { CGI_ST_INVALID, CGI_TR_INVALID },             /* EVT:  % */
    { CGI_ST_INVALID, CGI_TR_INVALID },             /* EVT:  other */
    { CGI_ST_INVALID, CGI_TR_INVALID },             /* EVT:  & */
    { CGI_ST_INVALID, CGI_TR_INVALID }              /* EVT:  end */
  },


  /* STATE 1:  CGI_ST_NAME */
  { { CGI_ST_NAME, CGI_TR_STASH },                  /* EVT:  alpha */
    { CGI_ST_NAME, CGI_TR_STASH },                  /* EVT:  name_char */
    { CGI_ST_VALUE, CGI_TR_NAME_EQUAL },            /* EVT:  = */
    { CGI_ST_INVALID, CGI_TR_INVALID },             /* EVT:  + */
    { CGI_ST_INVALID, CGI_TR_INVALID },             /* EVT:  % */
    { CGI_ST_INVALID, CGI_TR_INVALID },             /* EVT:  other */
    { CGI_ST_INVALID, CGI_TR_INVALID },             /* EVT:  & */
    { CGI_ST_INVALID, CGI_TR_INVALID }              /* EVT:  end */
  },


  /* STATE 2:  CGI_ST_VALUE */
  { { CGI_ST_VALUE, CGI_TR_STASH },                 /* EVT:  alpha */
    { CGI_ST_VALUE, CGI_TR_STASH },                 /* EVT:  name_char */
    { CGI_ST_VALUE, CGI_TR_STASH },                 /* EVT:  = */
    { CGI_ST_VALUE, CGI_TR_STASH },                 /* EVT:  + */
    { CGI_ST_VALUE, CGI_TR_VALUE_ESCAPE },          /* EVT:  % */
    { CGI_ST_VALUE, CGI_TR_STASH },                 /* EVT:  other */
    { CGI_ST_INIT, CGI_TR_SEPARATE },               /* EVT:  & */
    { CGI_ST_DONE, CGI_TR_SEPARATE }                /* EVT:  end */
  }
};


tSCC zCgiStrings[] =
       "** OUT-OF-RANGE **" "\0"
       "FSM Error:  in state %d (%s), event %d (%s) is invalid\n" "\0"
       "invalid" "\0"
       "init" "\0"
       "name" "\0"
       "value" "\0"
       "alpha" "\0"
       "name_char" "\0"
       "=" "\0"
       "+" "\0"
       "%" "\0"
       "other" "\0"
       "&" "\0"
       "end" "\0";

#define CgiBogus_off 0
#define CgiFsmErr_off 19
#define CgiEvInvalid_off 75
#define CgiStInit_off 83
#define CgiStName_off 88
#define CgiStValue_off 93
#define CgiEvAlpha_off 99
#define CgiEvName_Char_off 105
#define CgiEvEqual_off 115
#define CgiEvSpace_off 117
#define CgiEvEscape_off 119
#define CgiEvOther_off 121
#define CgiEvSeparator_off 127
#define CgiEvEnd_off 129


static const size_t aszCgiStates[] = {
    CgiStInit_off,  CgiStName_off,  CgiStValue_off };

static const size_t aszCgiEvents[] = {
    CgiEvAlpha_off,     CgiEvName_Char_off, CgiEvEqual_off,
    CgiEvSpace_off,     CgiEvEscape_off,    CgiEvOther_off,
    CgiEvSeparator_off, CgiEvEnd_off,       CgiEvInvalid_off };

#define CGI_EVT_NAME(t) ( (((unsigned)(t)) >= CGI_EV_INVALID) \
    ? zCgiStrings : zCgiStrings + aszCgiEvents[t])

#define CGI_STATE_NAME(s) ( (((unsigned)(s)) > CGI_ST_INVALID) \
    ? zCgiStrings : zCgiStrings + aszCgiStates[s])

#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

/* * * * * * * * * THE CODE STARTS HERE * * * * * * * *
 *
 *  Print out an invalid transition message and return EXIT_FAILURE
 */
int
cgi_invalid_transition( te_cgi_state st, te_cgi_event evt )
{
    /* START == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */
    char* pz = aprf( zCgiStrings + CgiFsmErr_off, st, CGI_STATE_NAME( st ),
                     evt, CGI_EVT_NAME( evt ));

    AG_ABEND( aprf( "CGI parsing error:  %s", pz ));
    /* END   == INVALID TRANS MSG == DO NOT CHANGE THIS COMMENT */

    return EXIT_FAILURE;
}

/*
 *  Run the FSM.  Will return CGI_ST_DONE or CGI_ST_INVALID
 */
te_cgi_state
cgi_run_fsm(
    const char* pzSrc,
    int inlen,
    char* pzOut,
    int outlen )
{
    te_cgi_state cgi_state = CGI_ST_INIT;
    te_cgi_event trans_evt;
    te_cgi_state nxtSt, firstNext;
    te_cgi_trans trans;

    while (cgi_state < CGI_ST_INVALID) {

        /* START == FIND TRANSITION == DO NOT CHANGE THIS COMMENT */

        char  curCh;
        if (--inlen < 0) {
            trans_evt = CGI_EV_END;
            curCh = NUL;

        } else {
            if (outlen < 4) {
                strcpy( pzOut, "output space exhausted\n" );
                return CGI_ST_INVALID;
            }
            curCh = *(pzSrc++);
            if (isalpha( curCh ))
                trans_evt = CGI_EV_ALPHA;
            else if (isdigit( curCh ))
                trans_evt = CGI_EV_NAME_CHAR;
            else switch (curCh) {
            case '_': trans_evt = CGI_EV_NAME_CHAR; break;
            case '=': trans_evt = CGI_EV_EQUAL;     break;
            case '+': trans_evt = CGI_EV_SPACE;     curCh = ' '; break;
            case '%': trans_evt = CGI_EV_ESCAPE;    break;
            case '&': trans_evt = CGI_EV_SEPARATOR; break;
            default:  trans_evt = CGI_EV_OTHER;     break;
            }
        }

        /* END   == FIND TRANSITION == DO NOT CHANGE THIS COMMENT */

        if (trans_evt >= CGI_EV_INVALID) {
            nxtSt = CGI_ST_INVALID;
            trans = CGI_TR_INVALID;
        } else {
            const t_cgi_transition* pTT =
                cgi_trans_table[ cgi_state ] + trans_evt;
            nxtSt = firstNext = pTT->next_state;
            trans = pTT->transition;
        }

#ifdef DEBUG
        printf( "in state %s(%d) step %s(%d) to %s(%d)\n",
            CGI_STATE_NAME( cgi_state ), cgi_state,
            CGI_EVT_NAME( trans_evt ), trans_evt,
            CGI_STATE_NAME( nxtSt ), nxtSt );
#endif

        switch (trans) {
        case CGI_TR_INVALID:
            /* START == INVALID == DO NOT CHANGE THIS COMMENT */
            exit( cgi_invalid_transition( cgi_state, trans_evt ));
            /* END   == INVALID == DO NOT CHANGE THIS COMMENT */
            break;


        case CGI_TR_NAME_EQUAL:
            /* START == NAME_EQUAL == DO NOT CHANGE THIS COMMENT */
            strcpy( pzOut, "='" );
            outlen -= 2;
            pzOut += 2;
            /* END   == NAME_EQUAL == DO NOT CHANGE THIS COMMENT */
            break;


        case CGI_TR_SEPARATE:
            /* START == SEPARATE == DO NOT CHANGE THIS COMMENT */
            strcpy( pzOut, "';\n" );
            outlen -= 2;
            pzOut += 3;
            /* END   == SEPARATE == DO NOT CHANGE THIS COMMENT */
            break;


        case CGI_TR_STASH:
            /* START == STASH == DO NOT CHANGE THIS COMMENT */
            *(pzOut++) = curCh;
            outlen--;
            /* END   == STASH == DO NOT CHANGE THIS COMMENT */
            break;


        case CGI_TR_VALUE_ESCAPE:
            /* START == VALUE_ESCAPE == DO NOT CHANGE THIS COMMENT */
            {
            char z[4];
            if (inlen < 2)
                exit( cgi_invalid_transition( cgi_state, trans_evt ));

            z[0] = *(pzSrc++);
            z[1] = *(pzSrc++);
            z[2] = NUL;
            inlen -= 2;

            /*
             *  We must backslash quote certain characters that are %-quoted
             *  in the input string:
             */
            switch (*(pzOut++) = (int)strtol( z, NULL, 16 )) {
            case '\'':
            case '\\':
            case '#':
                pzOut[0]  = pzOut[-1];
                pzOut[-1] = '\\';
                pzOut++;
            }
            }
            /* END   == VALUE_ESCAPE == DO NOT CHANGE THIS COMMENT */
            break;


        default:
            /* START == BROKEN MACHINE == DO NOT CHANGE THIS COMMENT */
            exit( cgi_invalid_transition( cgi_state, trans_evt ));
            /* END   == BROKEN MACHINE == DO NOT CHANGE THIS COMMENT */
        }
#ifdef DEBUG
        if (nxtSt != firstNext)
            printf( "transition code changed destination state to %s(%d)\n",
                CGI_STATE_NAME( nxtSt ), nxtSt );
#endif
        cgi_state = nxtSt;
    }
    return cgi_state;
}
/* end of cgi-fsm.c */
