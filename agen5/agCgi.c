
/*
 *  agCgi.c
 *  $Id: agCgi.c,v 3.2 2001/12/24 14:13:32 bkorb Exp $
 *
 *  This is a CGI wrapper for AutoGen.  It will take POST-method
 *  name-value pairs and emit AutoGen definitions to a spawned
 *  AutoGen process.
 */

/*
 *  AutoGen-cgi copyright 2001 Bruce Korb
 *
 *  AutoGen-cgi is free software.
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

#include "cgi-fsm.h"

typedef struct {
    const char*  pzName;
    char*        pzValue;
} tNameMap;

#define ENV_TABLE \
    _ET_( SERVER_SOFTWARE ) \
    _ET_( SERVER_NAME ) \
    _ET_( GATEWAY_INTERFACE ) \
    _ET_( SERVER_PROTOCOL ) \
    _ET_( SERVER_PORT ) \
    _ET_( REQUEST_METHOD ) \
    _ET_( PATH_INFO ) \
    _ET_( PATH_TRANSLATED ) \
    _ET_( SCRIPT_NAME ) \
    _ET_( QUERY_STRING ) \
    _ET_( REMOTE_HOST ) \
    _ET_( REMOTE_ADDR ) \
    _ET_( AUTH_TYPE ) \
    _ET_( REMOTE_USER ) \
    _ET_( REMOTE_IDENT ) \
    _ET_( CONTENT_TYPE ) \
    _ET_( CONTENT_LENGTH ) \
    _ET_( HTTP_ACCEPT ) \
    _ET_( HTTP_USER_AGENT ) \
    _ET_( HTTP_REFERER )

static tNameMap nameValueMap[] = {
#define _ET_(n) { #n, NULL },
    ENV_TABLE
#undef _ET_
    { NULL, NULL }
};

typedef enum {
#define _ET_(n) n ## _IDX,
    ENV_TABLE
#undef _ET_
    NAME_CT
} tNameIdx;

#define pzMethod nameValueMap[ REQUEST_METHOD_IDX ].pzValue
#define pzQuery  nameValueMap[ QUERY_STRING_IDX   ].pzValue


static const char zOops[] =
"Content-type: text/plain\n\n"
"Form processing error:\n";
#define zNil     ((char*)(zOops + sizeof( zOops ) - 1))

static char* parseInput( char* pzSrc, int len );

EXPORT void
loadCgi()
{
    int textLen = 0;
    char* pzText  = NULL;

    fclose( stderr );
    {
        FILE* fp = fdopen( STDOUT_FILENO, "w" );
        if (fileno( fp ) < STDERR_FILENO)
            fp = fdopen( STDOUT_FILENO, "w" );
        if (fileno( fp ) < STDERR_FILENO)
            AG_ABEND_STR( "Cannot redirect stderr to stdout" );
    }

    {
        tNameMap* pNM = nameValueMap;
        tNameIdx  ix  = SERVER_SOFTWARE_IDX;

        do  {
            pNM->pzValue = getenv( pNM->pzName );
            if (pNM->pzValue == NULL)
                pNM->pzValue = zNil;
        } while (pNM++, ++ix < NAME_CT);
    }

    textLen = atoi( nameValueMap[ CONTENT_LENGTH_IDX ].pzValue );
    if (textLen == 0)
        AG_ABEND_STR( "No data were received" );

    if (strcasecmp( pzMethod, "POST" ) == 0) {
        pzText  = malloc( (textLen + 32) & ~0x000F );
        if (pzText == NULL) {
            AG_ABEND_START( zAllocErr );
            fprintf( stderr, zAllocWhat, (textLen + 32) & ~0x000F, "input" );
            AG_ABEND;
        }

        if (fread( pzText, 1, textLen, stdin ) != textLen) {
            AG_ABEND_START( "Could not read" );
            fprintf( stderr, "\t%d bytes of input\n", textLen );
            AG_ABEND;
        }
        pzText[ textLen ] = '\0';

        pzQuery = pzText;
        pzText  = parseInput( pzText, textLen );
		AGFREE( pzQuery );

    } else if (strcasecmp( pzMethod, "GET" ) == 0) {
        pzText = parseInput( pzQuery, textLen );

    } else
        AG_ABEND_STR( "invalid request method" );

    pzOopsPrefix = zOops;
}


static char*
parseInput( char* pzSrc, int len )
{
    tSCC zDef[] = "Autogen Definitions cgi;\n";
    int   outlen = (len * 2) + sizeof( zDef );
    char* pzRes = malloc( outlen );

    if (pzRes == NULL)
        pzRes = "cannout allocate output buffer\n";

    else {
        strcpy( pzRes, zDef );
        if (cgi_run_fsm( pzSrc, len, pzRes + sizeof( zDef ) - 1, outlen )
            == CGI_ST_DONE)
            return pzRes;
    }

    AG_ABEND_STR( pzRes );
}

/*
 * Local Variables:
 * c-file-style: "stroustrup"
 * indent-tabs-mode: nil
 * End:
 * end of autogen.c */