/*
 *  Copyright (C) 2017 Masatoshi Teruya
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 *
 *
 *  pcre2.c
 *  lua-pcre2
 *
 *  Created by Masatoshi Teruya on 17/05/29.
 *
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <lauxhlib.h>
#include "config.h"

#define PCRE2_CODE_UNIT_WIDTH 8
#include "pcre2.h"


// MARK: lua binding
#define MODULE_MT   "pcre2"

#define REGEX_MODE_JIT  0x1

typedef struct {
    uint32_t mode;
    pcre2_code *code;
} regex_t;


typedef struct {
    int len;
    const char msg[256];
} regex_error_t;


static inline void regex_strerror( regex_error_t *err, int errnum )
{
    err->len = pcre2_get_error_message( errnum, (PCRE2_UCHAR*)err->msg,
                                        sizeof( err->msg ) );
}


static int match_lua( lua_State *L )
{
    regex_t *re = lauxh_checkudata( L, 1, MODULE_MT );
    size_t len = 0;
    PCRE2_SPTR sbj = (PCRE2_SPTR)lauxh_checklstring( L, 2, &len );
    uint32_t opts = lauxh_optflags( L, 3 );
    lua_Integer offset = lauxh_optinteger( L, 4, 0 );
    pcre2_match_data *data = pcre2_match_data_create_from_pattern( re->code,
                                                                   NULL );

    if( data )
    {
        int rc = 0;

        if( re->mode & REGEX_MODE_JIT ){
            rc = pcre2_jit_match( re->code, sbj, len, offset, opts, data, NULL );
        }
        else {
            rc = pcre2_match( re->code, sbj, len, offset, opts, data, NULL );
        }

        if( rc > 0 )
        {
            PCRE2_SIZE *ovec = pcre2_get_ovector_pointer( data );
            int i = 0;

            // create head and tail arrays
            lua_createtable( L, rc, 0 );
            lua_createtable( L, rc, 0 );
            for(; i < rc; i++ ){
                lua_pushinteger( L, ovec[i*2] + 1 );
                lua_rawseti( L, -3, i + 1 );
                lua_pushinteger( L, ovec[i*2+1] );
                lua_rawseti( L, -2, i + 1 );
            }
            rc = 2;
        }
        // got error
        else
        {
            regex_error_t err;

            lua_pushnil( L );
            lua_pushnil( L );
            switch( rc ){
                case PCRE2_ERROR_NOMATCH:
                    rc = 2;
                break;

                default:
                    regex_strerror( &err, rc );
                    lua_pushlstring( L, err.msg, err.len );
                    rc = 3;
                break;
            }
        }

        pcre2_match_data_free( data );

        return rc;
    }

    // got mem error
    lua_pushnil( L );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


static int gc_lua( lua_State *L )
{
    regex_t *re = lua_touserdata( L, 1 );

    pcre2_code_free( re->code );

    return 0;
}


static int tostring_lua( lua_State *L )
{
    lua_pushfstring( L, MODULE_MT ": %p", lua_touserdata( L, 1 ) );
    return 1;
}


static int new_lua( lua_State *L )
{
    size_t len = 0;
    const char *pattern = lauxh_checklstring( L, 1, &len );
    uint32_t flgs = lauxh_optflags( L, 2 );
    uint32_t jitflgs = lauxh_optflags( L, 3 );
    regex_t *re = lua_newuserdata( L, sizeof( regex_t ) );

    if( re )
    {
        int rc = 0;
        PCRE2_SIZE offset = 0;

        // compile pattern
        if( !( re->code = pcre2_compile( (PCRE2_SPTR)pattern, len, flgs, &rc,
                                         &offset, NULL ) ) ){
            regex_error_t err;

            regex_strerror( &err, rc );
            lua_pushnil( L );
            lua_pushfstring( L, "PCRE2 compilation failed at offset %d: %s",
                            (int)offset, err.msg );
            return 2;
        }
        // jit-compile if jitflgs specified
        else if( jitflgs )
        {
            if( ( rc = pcre2_jit_compile( re->code, jitflgs ) ) ){
                regex_error_t err;

                regex_strerror( &err, rc );
                lua_pushnil( L );
                lua_pushfstring( L, "PCRE2 JIT compilation failed: %s", err.msg );
            }

            re->mode |= REGEX_MODE_JIT;
        }
        else {
            re->mode = 0;
        }

        lauxh_setmetatable( L, MODULE_MT );

        return 1;
    }

    // got mem error
    lua_pushnil( L );
    lua_pushstring( L, strerror( errno ) );

    return 2;
}


LUALIB_API int luaopen_pcre2( lua_State *L )
{
    struct luaL_Reg mmethod[] = {
        { "__gc", gc_lua },
        { "__tostring", tostring_lua },
        { NULL, NULL }
    };
    struct luaL_Reg method[] = {
        { "match", match_lua },
        { NULL, NULL }
    };
    struct luaL_Reg *ptr = mmethod;

    // create metatable
    luaL_newmetatable( L, MODULE_MT );
    while( ptr->name ){
        lauxh_pushfn2tbl( L, ptr->name, ptr->func );
        ptr++;
    }
    lua_pushstring( L, "__index" );
    lua_newtable( L );
    ptr = method;
    while( ptr->name ){
        lauxh_pushfn2tbl( L, ptr->name, ptr->func );
        ptr++;
    }
    lua_rawset( L, -3 );
    lua_pop( L, 1 );


    lua_newtable( L );
    lauxh_pushfn2tbl( L, "new", new_lua );

    // flags for pcre2_compile(), pcre2_match(), or pcre2_dfa_match
    lauxh_pushint2tbl( L, "ANCHORED", PCRE2_ANCHORED );
    lauxh_pushint2tbl( L, "NO_UTF_CHECK", PCRE2_NO_UTF_CHECK );

    // flags for pcre2_compile()
    lauxh_pushint2tbl( L, "ALLOW_EMPTY_CLASS", PCRE2_ALLOW_EMPTY_CLASS );
    lauxh_pushint2tbl( L, "ALT_BSUX", PCRE2_ALT_BSUX );
    lauxh_pushint2tbl( L, "AUTO_CALLOUT", PCRE2_AUTO_CALLOUT );
    lauxh_pushint2tbl( L, "CASELESS", PCRE2_CASELESS );
    lauxh_pushint2tbl( L, "DOLLAR_ENDONLY", PCRE2_DOLLAR_ENDONLY );
    lauxh_pushint2tbl( L, "DOTALL", PCRE2_DOTALL );
    lauxh_pushint2tbl( L, "DUPNAMES", PCRE2_DUPNAMES );
    lauxh_pushint2tbl( L, "EXTENDED", PCRE2_EXTENDED );
    lauxh_pushint2tbl( L, "FIRSTLINE", PCRE2_FIRSTLINE );
    lauxh_pushint2tbl( L, "MATCH_UNSET_BACKREF", PCRE2_MATCH_UNSET_BACKREF );
    lauxh_pushint2tbl( L, "MULTILINE", PCRE2_MULTILINE );
    lauxh_pushint2tbl( L, "NEVER_UCP", PCRE2_NEVER_UCP );
    lauxh_pushint2tbl( L, "NEVER_UTF", PCRE2_NEVER_UTF );
    lauxh_pushint2tbl( L, "AUTO_CAPTURE", PCRE2_NO_AUTO_CAPTURE );
    lauxh_pushint2tbl( L, "NO_AUTO_POSSESS", PCRE2_NO_AUTO_POSSESS );
    lauxh_pushint2tbl( L, "NO_DOTSTAR_ANCHOR", PCRE2_NO_DOTSTAR_ANCHOR );
    lauxh_pushint2tbl( L, "NO_START_OPTIMIZE", PCRE2_NO_START_OPTIMIZE );
    lauxh_pushint2tbl( L, "UCP", PCRE2_UCP );
    lauxh_pushint2tbl( L, "UNGREEDY", PCRE2_UNGREEDY );
    lauxh_pushint2tbl( L, "UTF", PCRE2_UTF );
    lauxh_pushint2tbl( L, "NEVER_BACKSLASH_C", PCRE2_NEVER_BACKSLASH_C );
    lauxh_pushint2tbl( L, "ALT_CIRCUMFLEX", PCRE2_ALT_CIRCUMFLEX );
    lauxh_pushint2tbl( L, "ALT_VERBNAMES", PCRE2_ALT_VERBNAMES );
    lauxh_pushint2tbl( L, "USE_OFFSET_LIMIT", PCRE2_USE_OFFSET_LIMIT );

    // flags for pcre2_jit_compile()
    lauxh_pushint2tbl( L, "JIT_COMPLETE", PCRE2_JIT_COMPLETE );
    lauxh_pushint2tbl( L, "JIT_PARTIAL_SOFT", PCRE2_JIT_PARTIAL_SOFT );
    lauxh_pushint2tbl( L, "JIT_PARTIAL_HARD", PCRE2_JIT_PARTIAL_HARD );

    // flags for pcre2_match(), pcre2_dfa_match(), and pcre2_jit_match()
    lauxh_pushint2tbl( L, "NOTBOL", PCRE2_NOTBOL );
    lauxh_pushint2tbl( L, "NOTEOL", PCRE2_NOTEOL );
    lauxh_pushint2tbl( L, "NOTEMPTY", PCRE2_NOTEMPTY );
    lauxh_pushint2tbl( L, "NOTEMPTY_ATSTART", PCRE2_NOTEMPTY_ATSTART );
    lauxh_pushint2tbl( L, "PARTIAL_SOFT", PCRE2_PARTIAL_SOFT );
    lauxh_pushint2tbl( L, "PARTIAL_HARD", PCRE2_PARTIAL_HARD );

    // flags for pcre2_dfa_match()
    lauxh_pushint2tbl( L, "DFA_RESTART", PCRE2_DFA_RESTART );
    lauxh_pushint2tbl( L, "DFA_SHORTEST", PCRE2_DFA_SHORTEST );

    // flags for pcre2_substitute()
    // which passes any others through to pcre2_match()
    lauxh_pushint2tbl( L, "SUBSTITUTE_GLOBAL", PCRE2_SUBSTITUTE_GLOBAL );
    lauxh_pushint2tbl( L, "SUBSTITUTE_EXTENDED", PCRE2_SUBSTITUTE_EXTENDED );
    lauxh_pushint2tbl( L, "SUBSTITUTE_UNSET_EMPTY", PCRE2_SUBSTITUTE_UNSET_EMPTY );
    lauxh_pushint2tbl( L, "SUBSTITUTE_UNKNOWN_UNSET", PCRE2_SUBSTITUTE_UNKNOWN_UNSET );
    lauxh_pushint2tbl( L, "SUBSTITUTE_OVERFLOW_LENGTH", PCRE2_SUBSTITUTE_OVERFLOW_LENGTH );

    // flags for pcre2_match()
    // not allowed for pcre2_dfa_match()
    // ignored for pcre2_jit_match()
    lauxh_pushint2tbl( L, "NO_JIT", PCRE2_NO_JIT );

    // Request types for pcre2_pattern_info()
    lauxh_pushint2tbl( L, "INFO_ALLOPTIONS", PCRE2_INFO_ALLOPTIONS );
    lauxh_pushint2tbl( L, "INFO_ARGOPTIONS", PCRE2_INFO_ARGOPTIONS );
    lauxh_pushint2tbl( L, "INFO_BACKREFMAX", PCRE2_INFO_BACKREFMAX );
    lauxh_pushint2tbl( L, "INFO_BSR", PCRE2_INFO_BSR );
    lauxh_pushint2tbl( L, "INFO_CAPTURECOUNT", PCRE2_INFO_CAPTURECOUNT );
    lauxh_pushint2tbl( L, "INFO_FIRSTCODEUNIT", PCRE2_INFO_FIRSTCODEUNIT );
    lauxh_pushint2tbl( L, "INFO_FIRSTCODETYPE", PCRE2_INFO_FIRSTCODETYPE );
    lauxh_pushint2tbl( L, "INFO_FIRSTBITMAP", PCRE2_INFO_FIRSTBITMAP );
    lauxh_pushint2tbl( L, "INFO_HASCRORLF", PCRE2_INFO_HASCRORLF );
    lauxh_pushint2tbl( L, "INFO_JCHANGED", PCRE2_INFO_JCHANGED );
    lauxh_pushint2tbl( L, "INFO_JITSIZE", PCRE2_INFO_JITSIZE );
    lauxh_pushint2tbl( L, "INFO_LASTCODEUNIT", PCRE2_INFO_LASTCODEUNIT );
    lauxh_pushint2tbl( L, "INFO_LASTCODETYPE", PCRE2_INFO_LASTCODETYPE );
    lauxh_pushint2tbl( L, "INFO_MATCHEMPTY", PCRE2_INFO_MATCHEMPTY );
    lauxh_pushint2tbl( L, "INFO_MATCHLIMIT", PCRE2_INFO_MATCHLIMIT );
    lauxh_pushint2tbl( L, "INFO_MAXLOOKBEHIND", PCRE2_INFO_MAXLOOKBEHIND );
    lauxh_pushint2tbl( L, "INFO_MINLENGTH", PCRE2_INFO_MINLENGTH );
    lauxh_pushint2tbl( L, "INFO_NAMECOUNT", PCRE2_INFO_NAMECOUNT );
    lauxh_pushint2tbl( L, "INFO_NAMEENTRYSIZE", PCRE2_INFO_NAMEENTRYSIZE );
    lauxh_pushint2tbl( L, "INFO_NAMETABLE", PCRE2_INFO_NAMETABLE );
    lauxh_pushint2tbl( L, "INFO_NEWLINE", PCRE2_INFO_NEWLINE );
    lauxh_pushint2tbl( L, "INFO_RECURSIONLIMIT", PCRE2_INFO_RECURSIONLIMIT );
    lauxh_pushint2tbl( L, "INFO_SIZE", PCRE2_INFO_SIZE );
    lauxh_pushint2tbl( L, "INFO_HASBACKSLASHC", PCRE2_INFO_HASBACKSLASHC );

    return 1;
}

