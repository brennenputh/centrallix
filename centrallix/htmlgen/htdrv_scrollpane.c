#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "ht_render.h"
#include "obj.h"
#include "mtask.h"
#include "xarray.h"
#include "xhash.h"
#include "mtsession.h"

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
/* Module: 	htdrv_scrollpane.c      				*/
/* Author:	Greg Beeley (GRB)					*/
/* Creation:	November 30, 1998					*/
/* Description:	HTML Widget driver for a scrollpane -- a css layer with	*/
/*		a scrollable layer and a scrollbar for scrolling the	*/
/*		layer.  Can contain most objects, except for framesets.	*/
/************************************************************************/

/**CVSDATA***************************************************************

    $Id: htdrv_scrollpane.c,v 1.14 2002/12/04 00:19:11 gbeeley Exp $
    $Source: /srv/bld/centrallix-repo/centrallix/htmlgen/htdrv_scrollpane.c,v $

    $Log: htdrv_scrollpane.c,v $
    Revision 1.14  2002/12/04 00:19:11  gbeeley
    Did some cleanup on the user agent selection mechanism, moving to a
    bitmask so that drivers don't have to register twice.  Theme will be
    handled differently, but provision is made for 'classes' of widgets
    such as dhtml vs. xul.  Started work on some utility functions to
    resolve some ns47 vs. w3c issues.

    Revision 1.13  2002/09/27 22:26:05  gbeeley
    Finished converting over to the new obj[GS]etAttrValue() API spec.  Now
    my gfingrersd asre soi rtirewd iu'm hjavimng rto trype rthius ewithj nmy
    mnodse...

    Revision 1.12  2002/08/13 04:17:35  gbeeley
    Fixed problem with the scrollpane that caused it to shift static content
    down an amount equal to the entire inner dimensions of the scrollpane
    (thus virtually hiding its content).  Static content includes static
    mode widget/html and static mode widget/table.  Example of problem
    was centrallix-os/samples/ReportingSystem.app.

    Revision 1.11  2002/08/01 18:24:44  pfinley
    Added a table to the body of the sparea layer so that it would span the
    entire height and width of the widget (-2 for the border).  Prior it was
    cutting off the pgarea boxes of text areas.

    Revision 1.10  2002/08/01 14:48:46  pfinley
    Added events to the scrollpane widget:
       MouseUp,MouseDown,MouseOver,MouseOut,MouseMove

    Revision 1.9  2002/07/19 21:17:49  mcancel
    Changed widget driver allocation to use the nifty function htrAllocDriver instead of calling nmMalloc.

    Revision 1.8  2002/07/16 18:23:20  lkehresman
    Added htrAddStylesheetItem() function to help consolidate the output of
    the html generator.  Now, all stylesheet definitions are included in the
    same <style></style> tags rather than each widget having their own.  I
    have modified the current widgets to take advantage of this.  In the
    future, do not use htrAddHeaderItem(), but use this new function.

    NOTE:  There is also a htrAddStylesheetItem_va() function if you need it.

    Revision 1.7  2002/07/16 17:52:00  lkehresman
    Updated widget drivers to use include files

    Revision 1.6  2002/06/19 19:08:55  lkehresman
    Changed all snprintf to use the *_va functions

    Revision 1.5  2002/06/19 16:31:04  lkehresman
    * Changed snprintf to *_va functions in several places
    * Allow fading to both static and dynamic pages

    Revision 1.4  2002/06/09 23:44:46  nehresma
    This is the initial cut of the browser detection code.  Note that each widget
    needs to register which browser and style is supported.  The GNU regular
    expression library is also needed (comes with GLIBC).

    Revision 1.3  2002/03/09 19:21:20  gbeeley
    Basic security overhaul of the htmlgen subsystem.  Fixed many of my
    own bad sprintf habits that somehow worked their way into some other
    folks' code as well ;)  Textbutton widget had an inadequate buffer for
    the tb_init() call, causing various problems, including incorrect labels,
    and more recently, javascript errors.

    Revision 1.2  2001/11/03 02:09:54  gbeeley
    Added timer nonvisual widget.  Added support for multiple connectors on
    one event.  Added fades to the html-area widget.  Corrected some
    pg_resize() geometry issues.  Updated several widgets to reflect the
    connector widget changes.

    Revision 1.1.1.1  2001/08/13 18:00:50  gbeeley
    Centrallix Core initial import

    Revision 1.2  2001/08/07 19:31:53  gbeeley
    Turned on warnings, did some code cleanup...

    Revision 1.1.1.1  2001/08/07 02:30:55  gbeeley
    Centrallix Core Initial Import


 **END-CVSDATA***********************************************************/

/** globals **/
static struct 
    {
    int		idcnt;
    }
    HTSPANE;


/*** htspaneVerify - not written yet.
 ***/
int
htspaneVerify()
    {
    return 0;
    }


/*** htspaneRender - generate the HTML code for the page.
 ***/
int
htspaneRender(pHtSession s, pObject w_obj, int z, char* parentname, char* parentobj)
    {
    char* ptr;
    char name[64];
    char sbuf[160];
    pObject sub_w_obj;
    pObjQuery qy;
    int x,y,w,h;
    int id;
    int visible = 1;
    char* nptr;
    char bcolor[64] = "";
    char bimage[64] = "";

    	/** Get an id for this. **/
	id = (HTSPANE.idcnt++);

    	/** Get x,y,w,h of this object **/
	if (objGetAttrValue(w_obj,"x",DATA_T_INTEGER,POD(&x)) != 0) 
	    {
	    mssError(1,"HTSPANE","ScrollPane widget must have an 'x' property");
	    return -1;
	    }
	if (objGetAttrValue(w_obj,"y",DATA_T_INTEGER,POD(&y)) != 0)
	    {
	    mssError(1,"HTSPANE","ScrollPane widget must have a 'y' property");
	    return -1;
	    }
	if (objGetAttrValue(w_obj,"width",DATA_T_INTEGER,POD(&w)) != 0)
	    {
	    mssError(1,"HTSPANE","ScrollPane widget must have a 'width' property");
	    return -1;
	    }
	if (objGetAttrValue(w_obj,"height",DATA_T_INTEGER,POD(&h)) != 0)
	    {
	    mssError(1,"HTSPANE","ScrollPane widget must have a 'height' property");
	    return -1;
	    }

	/** Get name **/
	if (objGetAttrValue(w_obj,"name",DATA_T_STRING,POD(&ptr)) != 0) return -1;
	memccpy(name,ptr,'\0',63);
	name[63]=0;

	/** Check background color **/
	if (objGetAttrValue(w_obj,"bgcolor",DATA_T_STRING,POD(&ptr)) == 0)
	    {
	    memccpy(bcolor,ptr,'\0',63);
	    bcolor[63]=0;
	    }
	if (objGetAttrValue(w_obj,"background",DATA_T_STRING,POD(&ptr)) == 0)
	    {
	    memccpy(bimage,ptr,'\0',63);
	    bimage[63]=0;
	    }

	/** Marked not visible? **/
	if (objGetAttrValue(w_obj,"visible",DATA_T_STRING,POD(&ptr)) == 0)
	    {
	    if (!strcmp(ptr,"false")) visible = 0;
	    }

	/** Ok, write the style header items. **/
	htrAddStylesheetItem_va(s,"\t#sp%dpane { POSITION:absolute; VISIBILITY:%s; LEFT:%d; TOP:%d; WIDTH:%d; HEIGHT:%d; clip:rect(%d,%d); Z-INDEX:%d; }\n",id,visible?"inherit":"hidden",x,y,w,h,w,h, z);
	htrAddStylesheetItem_va(s,"\t#sp%darea { POSITION:absolute; VISIBILITY:inherit; LEFT:0; TOP:0; WIDTH:%d; Z-INDEX:%d; }\n",id,w-18,z+1);
	htrAddStylesheetItem_va(s,"\t#sp%dthum { POSITION:absolute; VISIBILITY:inherit; LEFT:%d; TOP:18; WIDTH:18; Z-INDEX:%d; }\n",id,w-18,z+1);

	/** Write globals for internal use **/
	htrAddScriptGlobal(s, "sp_target_img", "null", 0);
	htrAddScriptGlobal(s, "sp_click_x","0",0);
	htrAddScriptGlobal(s, "sp_click_y","0",0);
	htrAddScriptGlobal(s, "sp_thum_y","0",0);
	htrAddScriptGlobal(s, "sp_mv_timeout","null",0);
	htrAddScriptGlobal(s, "sp_mv_incr","0",0);
	htrAddScriptGlobal(s, "sp_cur_mainlayer","null",0);

	/** Write named global **/
	nptr = (char*)nmMalloc(strlen(name)+1);
	strcpy(nptr,name);
	htrAddScriptGlobal(s, nptr, "null", HTR_F_NAMEALLOC);

	htrAddScriptInclude(s, "/sys/js/htdrv_scrollpane.js", 0);
	htrAddScriptInclude(s, "/sys/js/ht_utils_string.js", 0);

	/** Script initialization call. **/
	htrAddScriptInit_va(s,"    sp_init(%s.layers.sp%dpane,\"sp%darea\",\"sp%dthum\",%s);\n", parentname,id,id,id,parentobj);
	htrAddScriptInit_va(s,"    %s=%s.layers.sp%dpane;\n",name,parentname,id);

	/** HTML body <DIV> elements for the layers. **/
	htrAddBodyItem_va(s,"<DIV ID=\"sp%dpane\"><TABLE %s%s %s%s border=0 cellspacing=0 cellpadding=0 width=%d>",id,(*bcolor)?"bgcolor=":"",bcolor, (*bimage)?"background=":"",bimage, w);
	htrAddBodyItem_va(s,"<TR><TD align=right><IMG SRC=/sys/images/ico13b.gif NAME=u></TD></TR><TR><TD align=right>");
	htrAddBodyItem_va(s,"<IMG SRC=/sys/images/trans_1.gif height=%d width=18 name='b'>",h-36);
	htrAddBodyItem_va(s,"</TD></TR><TR><TD align=right><IMG SRC=/sys/images/ico12b.gif NAME=d></TD></TR></TABLE>\n");
	htrAddBodyItem_va(s,"<DIV ID=\"sp%dthum\"><IMG SRC=/sys/images/ico14b.gif NAME=t></DIV>\n<DIV ID=\"sp%darea\"><table border=0 cellpadding=0 cellspacing=0 width=%d height=%d><tr><td>",id,id,w-2,h-2);

	/** Add the event handling scripts **/

	htrAddEventHandler(s, "document","MOUSEDOWN","sp",
		"    sp_target_img=e.target;\n"
		"    if (sp_target_img != null && sp_target_img.kind=='sp' && (sp_target_img.name=='u' || sp_target_img.name=='d'))\n"
		"        {\n"
		"        if (sp_target_img.name=='u') sp_mv_incr=-10; else sp_mv_incr=+10;\n"
		"        sp_target_img.src = htutil_subst_last(sp_target_img.src,\"c.gif\");\n"
		"        do_mv();\n"
		"        sp_mv_timeout = setTimeout(tm_mv,300);\n"
		"        }\n"
		"    else if (sp_target_img != null && sp_target_img.kind=='sp' && sp_target_img.name=='t')\n"
		"        {\n"
		"        sp_click_x = e.pageX;\n"
		"        sp_click_y = e.pageY;\n"
		"        sp_thum_y = sp_target_img.thum.pageY;\n"
		"        }\n"
		"    else if (sp_target_img != null && sp_target_img.kind=='sp' && sp_target_img.name=='b')\n"
		"        {\n"
		"        sp_mv_incr=sp_target_img.height+36;\n"
		"        if (e.pageY < sp_target_img.thum.pageY+9) sp_mv_incr = -sp_mv_incr;\n"
		"        do_mv();\n"
		"        sp_mv_timeout = setTimeout(tm_mv,300);\n"
		"        }\n"
		"    else sp_target_img = null;\n"
		"    if (ly.kind == 'sp') cn_activate(ly.mainlayer, 'MouseDown');");
	htrAddEventHandler(s, "document","MOUSEMOVE","sp",
		"    ti=sp_target_img;\n"
		"    if (ti != null && ti.kind=='sp' && ti.name=='t')\n"
		"        {\n"
		"        v=ti.pane.clip.height-(3*18);\n"
		"        new_y=sp_thum_y + (e.pageY-sp_click_y);\n"
		"        if (new_y > ti.pane.pageY+18+v) new_y=ti.pane.pageY+18+v;\n"
		"        if (new_y < ti.pane.pageY+18) new_y=ti.pane.pageY+18;\n"
		"        ti.thum.pageY=new_y;\n"
		"        h=ti.area.clip.height;\n"
		"        d=h-ti.pane.clip.height;\n"
		"        if (d<0) d=0;\n"
		"        yincr = (((ti.thum.y-18)/v)*-d) - ti.area.y;\n"
		"        for(i=0;i<ti.pane.document.layers.length;i++) if (ti.pane.document.layers[i] != ti.thum)\n"
		"            ti.pane.document.layers[i].y+=yincr;\n"
		"        return false;\n"
		"        }\n"
		"    if (ly.kind == 'sp') cn_activate(ly.mainlayer, 'MouseMove');\n"
		"    if (sp_cur_mainlayer && ly.kind != 'sp')\n"
		"        {\n"
		"        cn_activate(sp_cur_mainlayer, 'MouseOut');\n"
		"        sp_cur_mainlayer = null;\n"
		"        }\n");
	htrAddEventHandler(s, "document","MOUSEUP","sp",
		"    if (sp_mv_timeout != null)\n"
		"        {\n"
		"        clearTimeout(sp_mv_timeout);\n"
		"        sp_mv_timeout = null;\n"
		"        sp_mv_incr = 0;\n"
		"        }\n"
		"    if (sp_target_img != null)\n"
		"        {\n"
		"        if (sp_target_img.name != 'b')\n"
		"            sp_target_img.src = htutil_subst_last(sp_target_img.src,\"b.gif\");\n"
		"        sp_target_img = null;\n"
		"        }\n"
		"    if (ly.kind == 'sp') cn_activate(ly.mainlayer, 'MouseUp');\n");
	htrAddEventHandler(s, "document", "MOUSEOVER", "sp",
		"    if (ly.kind == 'sp')\n"
		"        {\n"
		"        if (!sp_cur_mainlayer)\n"
		"            {\n"
		"            cn_activate(ly.mainlayer, 'MouseOver');\n"
		"            sp_cur_mainlayer = ly.mainlayer;\n"
		"            }\n"
		"        }\n");
//	htrAddEventHandler(s, "document", "MOUSEOUT", "sp",
//		"");

	/** Check for more sub-widgets within the page. **/
	qy = objOpenQuery(w_obj,"",NULL,NULL,NULL);
	snprintf(sbuf,160,"%s.document.layers.sp%darea.document",name,id);
	if (qy)
	    {
	    while((sub_w_obj = objQueryFetch(qy, O_RDONLY)))
	        {
		htrRenderWidget(s, sub_w_obj, z+2, sbuf, name);
		objClose(sub_w_obj);
		}
	    objQueryClose(qy);
	    }

	/** Finish off the last <DIV> **/
	htrAddBodyItem(s,"</td></tr></table></DIV></DIV>\n");

    return 0;
    }


/*** htspaneInitialize - register with the ht_render module.
 ***/
int
htspaneInitialize()
    {
    pHtDriver drv;

    	/** Allocate the driver **/
	drv = htrAllocDriver();
	if (!drv) return -1;

	/** Fill in the structure. **/
	strcpy(drv->Name,"HTML ScrollPane Widget Driver");
	strcpy(drv->WidgetName,"scrollpane");
	drv->Render = htspaneRender;
	drv->Verify = htspaneVerify;
	htrAddSupport(drv, HTR_UA_NETSCAPE_47);

	/** Events **/ 
	htrAddEvent(drv,"Click");
	htrAddEvent(drv,"MouseUp");
	htrAddEvent(drv,"MouseDown");
	htrAddEvent(drv,"MouseOver");
	htrAddEvent(drv,"MouseOut");
	htrAddEvent(drv,"MouseMove");

	/** Register. **/
	htrRegisterDriver(drv);

	HTSPANE.idcnt = 0;

    return 0;
    }
