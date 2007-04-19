#ifndef _XSTRING_H
#define _XSTRING_H

#ifdef CXLIB_INTERNAL
#include "cxsec.h"
#include "magic.h"
#else
#include "cxlib/cxsec.h"
#include "cxlib/magic.h"
#endif

/************************************************************************/
/* Centrallix Application Server System 				*/
/* Centrallix Base Library						*/
/* 									*/
/* Copyright (C) 1998-2001 LightSys Technology Services, Inc.		*/
/* 									*/
/* You may use these files and this library under the terms of the	*/
/* GNU Lesser General Public License, Version 2.1, contained in the	*/
/* included file "COPYING".						*/
/* 									*/
/* Module: 	xstring.c, xstring.h					*/
/* Author:	Greg Beeley (GRB)					*/
/* Creation:	December 17, 1998					*/
/* Description:	Extensible string support module.  Implements an auto-	*/
/*		realloc'ing string data structure.			*/
/************************************************************************/

/**CVSDATA***************************************************************

    $Id: xstring.h,v 1.12 2007/04/19 21:14:13 gbeeley Exp $
    $Source: /srv/bld/centrallix-repo/centrallix-lib/include/xstring.h,v $

    $Log: xstring.h,v $
    Revision 1.12  2007/04/19 21:14:13  gbeeley
    - (feature) adding &FILE and &PATH filters to qprintf.
    - (bugfix) include nLEN test earlier, make sure &FILE/PATH isn't tricked.
    - (tests) more tests cases (of course...)
    - (feature) adding qprintf functionality to XString.

    Revision 1.11  2005/02/26 04:32:02  gbeeley
    - moving include file install directory to include a "cxlib/" prefix
      instead of just putting 'em all in /usr/include with everything else.

    Revision 1.10  2005/02/06 02:35:41  gbeeley
    - Adding 'mkrpm' script for automating the RPM build process for this
      package (script is portable to other packages).
    - stubbed out pipe functionality in mtask (non-OS pipes; to be used
      between mtask threads)
    - added xsString(xstr) for getting the string instead of xstr->String.

    Revision 1.9  2003/04/03 21:38:30  gbeeley
    Adding xsSubst() to the xstring library.  Does a replacement of a
    string with another based on position, not content.

    Revision 1.8  2003/04/03 04:32:39  gbeeley
    Added new cxsec module which implements some optional-use security
    hardening measures designed to protect data structures and stack
    return addresses.  Updated build process to have hardening and
    optimization options.  Fixed some build-related dependency checking
    problems.  Updated mtask to put some variables in registers even
    when not optimizing with -O.  Added some security hardening features
    to xstring as an example.

    Revision 1.7  2002/11/22 20:56:57  gbeeley
    Added xsGenPrintf(), fdPrintf(), and supporting logic.  These routines
    basically allow printf() style functionality on top of any xxxWrite()
    type of routine (such as fdWrite, objWrite, etc).

    Revision 1.6  2002/09/28 01:08:07  jorupp
     * added xsFindRev()
     * fixed a couple bugs that pop up when using those functions I added
        the other day on null-terminated strings -- I may not have them all yet...

    Revision 1.5  2002/09/21 01:59:33  jorupp
     * added some functions to xstring -- helps quite a bit for string manipulation :)

    Revision 1.4  2002/08/06 16:00:28  lkehresman
    Added some xstring manipulation functions:
      xsRTrim - right trim whitespace
      xsLTrim - left trim whitespace
      xsTrim - xsRTrim && xsLTrim

    Revision 1.3  2001/10/03 15:48:09  gbeeley
    Added xsWrite() function to mimic fdWrite/objWrite for XStrings.

    Revision 1.2  2001/10/03 15:31:31  gbeeley
    Added xsPrintf and xsConcatPrintf functions to the xstring library.
    They currently support %s and %d with field width and precision.

    Revision 1.1.1.1  2001/08/13 18:04:20  gbeeley
    Centrallix Library initial import

    Revision 1.1.1.1  2001/07/03 01:03:03  gbeeley
    Initial checkin of centrallix-lib


 **END-CVSDATA***********************************************************/

#include <stdarg.h>

#define XS_BLK_SIZ	256
#define XS_PRINTF_BUFSIZ 1024

typedef struct _XS
    {
    Magic_t	Magic;
    CXSEC_DS_BEGIN;
    char	InitBuf[XS_BLK_SIZ];
    char*	String;
    int		Length;
    int		AllocLen;
    CXSEC_DS_END;
    }
    XString, *pXString;


/** XString methods **/
int xsInit(pXString this);
int xsDeInit(pXString this);
int xsCheckAlloc(pXString this, int addl_needed);
int xsConcatenate(pXString this, char* text, int len);
int xsCopy(pXString this, char* text, int len);
char* xsString(pXString this);
char* xsStringEnd(pXString this);
int xsPrintf(pXString this, char* fmt, ...);
int xsConcatPrintf(pXString this, char* fmt, ...);
int xsWrite(pXString this, char* buf, int len, int offset, int flags);
int xsRTrim(pXString this);
int xsLTrim(pXString this);
int xsTrim(pXString this);
int xsFind(pXString this,char* find,int findlen, int offset);
int xsFindRev(pXString this,char* find,int findlen, int offset);
int xsSubst(pXString this, int offset, int len, char* rep, int replen);
int xsReplace(pXString this, char* find, int findlen, int offset, char* rep, int replen);
int xsInsertAfter(pXString this, char* ins, int inslen, int offset);
int xsGenPrintf(int (*write_fn)(), void* write_arg, char** buf, int* buf_size, const char* fmt, ...);
int xsGenPrintf_va(int (*write_fn)(), void* write_arg, char** buf, int* buf_size, const char* fmt, va_list va);
int xsQPrintf(pXString this, char* fmt, ...);
int xsConcatQPrintf(pXString this, char* fmt, ...);

#define XS_U_SEEK	2

#endif /* _XSTRING_H */

