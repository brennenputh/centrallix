#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "ht_render.h"
#include "obj.h"
#include "mtask.h"
#include "xarray.h"
#include "xhash.h"
#include "xstring.h"
#include "mtsession.h"

/************************************************************************/
/* Centrallix Application Server System 				*/
/* Centrallix Core       						*/
/* 									*/
/* Copyright (C) 1998-2004 LightSys Technology Services, Inc.		*/
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
/* Module: 	htdrv_hints.c						*/
/* Author:	Greg Beeley (GRB)					*/
/* Creation:	February 22, 2004 					*/
/* Description:	HTML Widget driver for a 'hints', which is a pseudo-	*/
/*		widget allowing the association of presentation hints	*/
/*		with a form element widget				*/
/************************************************************************/

/**CVSDATA***************************************************************

    $Id: htdrv_hints.c,v 1.1 2004/02/24 19:59:30 gbeeley Exp $
    $Source: /srv/bld/centrallix-repo/centrallix/htmlgen/htdrv_hints.c,v $

    $Log: htdrv_hints.c,v $
    Revision 1.1  2004/02/24 19:59:30  gbeeley
    - adding component-declaration widget driver
    - adding image widget driver
    - adding app-level presentation hints pseudo-widget driver

 **END-CVSDATA***********************************************************/

/** globals **/
static struct 
    {
    int		idcnt;
    }
    HTHINT;


/*** hthintVerify - not written yet.
 ***/
int
hthintVerify()
    {
    return 0;
    }


/*** hthintRender - generate the HTML code for the page.
 ***/
int
hthintRender(pHtSession s, pObject w_obj, int z, char* parentname, char* parentobj)
    {
    int id;
    pObjPresentationHints hints;
    XString xs;

    	/** Get an id for this. **/
	id = (HTHINT.idcnt++);

	/** Convert the object data into hints data **/
	hints = hntObjToHints(w_obj);
	if (!hints)
	    {
	    mssError(0, "HTHINT", "Error in presentation hints data");
	    return -1;
	    }

	/** Serialize the hints data and add the script init for it **/
	xsInit(&xs);
	hntEncodeHints(hints, &xs);
	htrAddScriptInit_va(s, "    cx_set_hints(%s, '%s', 'app');\n",
		parentobj, xs.String);
	xsDeInit(&xs);

	/** Release the hints data structure **/
	objFreeHints(hints);

    return 0;
    }


/*** hthintInitialize - register with the ht_render module.
 ***/
int
hthintInitialize()
    {
    pHtDriver drv;

    	/** Allocate the driver **/
	drv = htrAllocDriver();
	if (!drv) return -1;

	/** Fill in the structure. **/
	strcpy(drv->Name,"Presentation Hints pseudo-widget Driver");
	strcpy(drv->WidgetName,"hints");
	drv->Render = hthintRender;
	drv->Verify = hthintVerify;

	/** Register. **/
	htrRegisterDriver(drv);

	htrAddSupport(drv, "dhtml");

	HTHINT.idcnt = 0;

    return 0;
    }
