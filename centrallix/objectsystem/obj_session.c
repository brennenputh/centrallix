#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "obj.h"
#include "mtask.h"
#include "xarray.h"
#include "xhash.h"
#include "magic.h"

/************************************************************************/
/* Centrallix Application Server System 				*/
/* Centrallix Core       						*/
/* 									*/
/* Copyright (C) 1998-2001 LightSys Technology Services, Inc.		*/
/* 									*/
/* This program is free software; you can redistribute it and/or modify	*/
/* it under the terms of the GNU General Public License as published by	*/
/* the Free Software Foundation; either version 2 of the License, or	*/
/* (at your option) any later version.					*/
/* 									*/
/* This program is distributed in the hope that it will be useful,	*/
/* but WITHOUT ANY WARRANTY; without even the implied warranty of	*/
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	*/
/* GNU General Public License for more details.				*/
/* 									*/
/* You should have received a copy of the GNU General Public License	*/
/* along with this program; if not, write to the Free Software		*/
/* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  		*/
/* 02111-1307  USA							*/
/*									*/
/* A copy of the GNU General Public License has been included in this	*/
/* distribution in the file "COPYING".					*/
/* 									*/
/* Module: 	obj.h, obj_*.c    					*/
/* Author:	Greg Beeley (GRB)					*/
/* Creation:	October 26, 1998					*/
/* Description:	Implements the ObjectSystem part of the Centrallix.    */
/*		The various obj_*.c files implement the various parts of*/
/*		the ObjectSystem interface.				*/
/*		--> obj_session.c: implements the session-management of	*/
/*		the objectsystem.					*/
/************************************************************************/

/**CVSDATA***************************************************************

    $Id: obj_session.c,v 1.3 2002/05/03 03:51:21 gbeeley Exp $
    $Source: /srv/bld/centrallix-repo/centrallix/objectsystem/obj_session.c,v $

    $Log: obj_session.c,v $
    Revision 1.3  2002/05/03 03:51:21  gbeeley
    Added objUnmanageObject() and objUnmanageQuery() which cause an object
    or query to not be closed automatically on session close.  This should
    NEVER be used with the intent of keeping an object or query open after
    session close, but rather it is used when the object or query would be
    closed in some other way, such as 'hidden' objects and queries that the
    multiquery layer opens behind the scenes (closing the multiquery objects
    and queries will cause the underlying ones to be closed).
    Also fixed some problems in the OSML where some objects and queries
    were not properly being added to the session's open objects and open
    queries lists.

    Revision 1.2  2002/04/25 17:59:59  gbeeley
    Added better magic number support in the OSML API.  ObjQuery and
    ObjSession structures are now protected with magic numbers, and
    support for magic numbers in Object structures has been improved
    a bit.

    Revision 1.1.1.1  2001/08/13 18:00:59  gbeeley
    Centrallix Core initial import

    Revision 1.1.1.1  2001/08/07 02:31:01  gbeeley
    Centrallix Core Initial Import


 **END-CVSDATA***********************************************************/


/*** objOpenSession - start a new session with a specific current
 *** directory.  If current_dir is NULL, then the new session is started
 *** in the home directory of the calling user.
 ***/
pObjSession 
objOpenSession(char* current_dir)
    {
    pObjSession this;

	/** Allocate the session **/
	this = (pObjSession)nmMalloc(sizeof(ObjSession));
	if (!this) return NULL;

	/** Setup the session structure **/
	xaInit(&(this->OpenObjects),16);
	xaInit(&(this->OpenQueries),16);
	strncpy(this->CurrentDirectory,current_dir,255);
	this->CurrentDirectory[255]=0;
	this->Trx = NULL;
	this->Magic = MGK_OBJSESSION;

	/** Add to the sessions list **/
	xaAddItem(&(OSYS.OpenSessions),(void*)this);

    return this;
    }


/*** objCloseSession - closes an open session and closes any objects,
 *** queries, and attributes that were open via this session.
 ***/
int 
objCloseSession(pObjSession this)
    {

	ASSERTMAGIC(this, MGK_OBJSESSION);

	/** Close any open queries **/
	while(this->OpenQueries.nItems)
	    {
	    objQueryClose((pObjQuery)(this->OpenQueries.Items[0]));
	    }

	/** Close any open objects **/
	while(this->OpenObjects.nItems)
	    {
	    objClose((pObject)(this->OpenObjects.Items[0]));
	    }

	/** Remove from the session list **/
	xaRemoveItem(&(OSYS.OpenSessions),xaFindItem(&(OSYS.OpenSessions),(void*)this));

	/** Free the memory **/
	if (this->Trx) obj_internal_FreeTree(this->Trx);
	xaDeInit(&(this->OpenObjects));
	xaDeInit(&(this->OpenQueries));
	nmFree(this,sizeof(ObjSession));

    return 0;
    }


/*** objSetWD - sets the current working directory to that of an open
 *** object.
 ***/
int
objSetWD(pObjSession this, pObject wd)
    {
    ASSERTMAGIC(this, MGK_OBJSESSION);
    if (!strcmp(wd->Pathname->Pathbuf,".")) strcpy(this->CurrentDirectory,"/");
    else strcpy(this->CurrentDirectory, wd->Pathname->Pathbuf+1);
    return 0;
    }


/*** objGetWD - gets the current working directory.
 ***/
char*
objGetWD(pObjSession this)
    {
    ASSERTMAGIC(this, MGK_OBJSESSION);
    return this->CurrentDirectory;
    }


/*** objUnmanageObject - removes an object from the list of open objects
 *** associated with this session.  This basically means that the object
 *** won't be auto-closed when the session is closed.  This should ONLY
 *** be used when a module can guarantee that the object will otherwise
 *** be closed when the session is closed, such as with open objects
 *** and queries used internally by the multiquery module.
 ***/
int
objUnmanageObject(pObjSession this, pObject obj)
    {
    xaRemoveItem(&(this->OpenObjects), xaFindItem(&(this->OpenObjects), (void*)obj));
    return 0;
    }


/*** objUnmanageQuery - removes a query from the list of open queries
 *** associated with this session.  This basically means that the query
 *** won't be auto-closed when the session is closed.  This should ONLY
 *** be used when a module can guarantee that the query will otherwise
 *** be closed when the session is closed, such as with open objects
 *** and queries used internally by the multiquery module.
 ***/
int
objUnmanageQuery(pObjSession this, pObjQuery qy)
    {
    xaRemoveItem(&(this->OpenQueries), xaFindItem(&(this->OpenQueries), (void*)qy));
    return 0;
    }


