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
/* Module: 	htdrv_treeview.c        				*/
/* Author:	Greg Beeley (GRB)					*/
/* Creation:	December 1, 1998 					*/
/* Description:	HTML Widget driver for a treeview.			*/
/************************************************************************/

/**CVSDATA***************************************************************

    $Id: htdrv_treeview.c,v 1.3 2002/03/09 19:21:20 gbeeley Exp $
    $Source: /srv/bld/centrallix-repo/centrallix/htmlgen/htdrv_treeview.c,v $

    $Log: htdrv_treeview.c,v $
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

    Revision 1.1.1.1  2001/08/13 18:00:52  gbeeley
    Centrallix Core initial import

    Revision 1.2  2001/08/07 19:31:53  gbeeley
    Turned on warnings, did some code cleanup...

    Revision 1.1.1.1  2001/08/07 02:30:56  gbeeley
    Centrallix Core Initial Import


 **END-CVSDATA***********************************************************/

/** globals **/
static struct 
    {
    int		idcnt;
    }
    HTTREE;


/*** httreeVerify - not written yet.
 ***/
int
httreeVerify()
    {
    return 0;
    }


/*** httreeRender - generate the HTML code for the page.
 ***/
int
httreeRender(pHtSession s, pObject w_obj, int z, char* parentname, char* parentobj)
    {
    char* ptr;
    char name[64];
    char sbuf[160];
    char src[128];
    pObject sub_w_obj;
    pObjQuery qy;
    int x,y,w;
    int id;
    char* nptr;

    	/** Get an id for this. **/
	id = (HTTREE.idcnt++);

    	/** Get x,y,w,h of this object **/
	if (objGetAttrValue(w_obj,"x",POD(&x)) != 0) 
	    {
	    mssError(1,"HTTREE","TreeView widget must have an 'x' property");
	    return -1;
	    }
	if (objGetAttrValue(w_obj,"y",POD(&y)) != 0)
	    {
	    mssError(1,"HTTREE","TreeView widget must have a 'y' property");
	    return -1;
	    }
	if (objGetAttrValue(w_obj,"width",POD(&w)) != 0)
	    {
	    mssError(1,"HTTREE","TreeView widget must have a 'width' property");
	    return -1;
	    }

	/** Get name **/
	if (objGetAttrValue(w_obj,"name",POD(&ptr)) != 0) return -1;
	memccpy(name,ptr,0,63);
	name[63] = 0;

	/** Get source directory tree **/
	if (objGetAttrValue(w_obj,"source",POD(&ptr)) != 0)
	    {
	    mssError(1,"HTTREE","TreeView widget must have a 'source' property");
	    return -1;
	    }
	memccpy(src,ptr,0,127);
	src[127]=0;

	/** Ok, write the style header items. **/
	snprintf(sbuf,160,"    <STYLE TYPE=\"text/css\">\n");
	htrAddHeaderItem(s,sbuf);
	snprintf(sbuf,160,"\t#tv%droot { POSITION:absolute; VISIBILITY:inherit; LEFT:%d; TOP:%d; WIDTH:%d; Z-INDEX:%d; }\n",id,x,y,w,z);
	htrAddHeaderItem(s,sbuf);
	snprintf(sbuf,160,"\t#tv%dload { POSITION:absolute; VISIBILITY:hidden; LEFT:0; TOP:0; clip:rect(1,1); Z-INDEX:0; }\n",id);
	htrAddHeaderItem(s,sbuf);
	snprintf(sbuf,160,"    </STYLE>\n");
	htrAddHeaderItem(s,sbuf);

	/** Write globals for internal use **/
	htrAddScriptGlobal(s, "tv_tgt_layer", "null", 0);
	htrAddScriptGlobal(s, "tv_target_img","null",0);
	htrAddScriptGlobal(s, "tv_layer_cache","null",0);
	htrAddScriptGlobal(s, "tv_alloc_cnt","0",0);
	htrAddScriptGlobal(s, "tv_cache_cnt","0",0);

	/** Write named global **/
	nptr = (char*)nmMalloc(strlen(name)+1);
	strcpy(nptr,name);
	htrAddScriptGlobal(s, nptr, "null", HTR_F_NAMEALLOC);

	/** Utility function... **/
	htrAddScriptFunction(s, "subst_last", "\n"
		"function subst_last(str,subst)\n"
		"    {\n"
		"    return str.substring(0,str.length-subst.length)+subst;\n"
		"    }\n", 0);
	htrAddScriptFunction(s, "show_obj", "\n"
		"function show_obj(obj)\n"
		"    {\n"
		"    var result='';\n"
		"    for (var i in obj) result += 'obj.' + i + ' = ' + obj[i] + '\\n';\n"
		"    return result;\n"
		"    }\n", 0);

	/** Layer memory management. **/
	htrAddScriptFunction(s, "tv_new_layer", "\n"
		"function tv_new_layer(width,pdoc)\n"
		"    {\n"
		"    var nl;\n"
		"    if (pdoc.tv_layer_cache != null)\n"
		"        {\n"
		"        nl = pdoc.tv_layer_cache;\n"
		"        pdoc.tv_layer_cache = nl.next;\n"
		"        nl.next = null;\n"
		"        tv_cache_cnt--;\n"
		"        }\n"
		"    else\n"
		"        {\n"
		"        nl = new Layer(width,pdoc.tv_layer_tgt);\n"
		/*"        nl.captureEvents(Event.CLICK | Event.MOUSEDOWN);\n"
		"        nl.onclick = tv_click;\n"
		"        nl.onmousedown = tv_rclick;\n"*/
		"        tv_alloc_cnt++;\n"
		"        }\n"
		"    return nl;\n"
		"    }\n" ,0);

	htrAddScriptFunction(s, "tv_cache_layer", "\n"
		"function tv_cache_layer(l,pdoc)\n"
		"    {\n"
		"    l.next = pdoc.tv_layer_cache;\n"
		"    pdoc.tv_layer_cache = l;\n"
		"    l.visibility = 'hidden';\n"
		"    tv_cache_cnt++;\n"
		"    }\n", 0);

	/** This function handles a click event on the treeview item **/
	htrAddScriptFunction(s, "tv_click", "\n"
		"function tv_click(e)\n"
		"    {\n"
		"    if (e.which == 3)\n"
		"        {\n"
		"        tv_rclick(e);\n"
		"        return false;\n"
		"        }\n"
		"    if (e.target.href != null)\n"
		"        {\n"
		"        eparam = new Object();\n"
		"        eparam.Pathname = e.target.href;\n"
		"        eparam.Caller = e.target.layer.root;\n"
		"        if (e.target.layer.root.EventClickItem != null)\n"
		"            cn_activate(e.target.layer.root,'ClickItem', eparam);\n"
		"        delete eparam;\n"
		"        }\n"
		"    return false;\n"
		"    }\n", 0);

	/** This function handles the rightclick event on an item. **/
	htrAddScriptFunction(s, "tv_rclick", "\n"
		"function tv_rclick(e)\n"
		"    {\n"
		"    if (e.target.layer.document.links != null && e.which == 3)\n"
		"        {\n"
		"        hr = e.target.layer.document.links[0].href;\n"
		"        eparam = new Object();\n"
		"        eparam.Pathname = hr;\n"
		"        eparam.Caller = e.target.layer.root;\n"
		"        eparam.X = e.pageX;\n"
		"        eparam.Y = e.pageY;\n"
		"        if (e.target.layer.root.EventRightClickItem != null)\n"
		"            {\n"
		"            cn_activate(e.target.layer.root, 'RightClickItem', eparam);\n"
		"            delete eparam;\n"
		"            return false;\n"
		"            }\n"
		"        delete eparam;\n"
		"        }\n"
		"    return true;\n"
		"    }\n", 0);
	
	/** This runs when the hidden layer loads the directory **/
	htrAddScriptFunction(s, "tv_loaded", "\n"
		"function tv_loaded(e)\n"
		"    {\n"
		"    one_layer=null;\n"
		"    cnt=0;\n"
		"    nullcnt=0;\n"
		"    last = tv_tgt_layer.ld.document.links.length - 1;\n"
		"    linkcnt = tv_tgt_layer.ld.document.links.length-1;\n"
		"    if (linkcnt < 0) linkcnt = 0;\n"
		"    if (window != tv_tgt_layer.pdoc.tv_layer_tgt)\n"
		"            tv_tgt_layer.pdoc.tv_layer_tgt.clip.height += 20*(linkcnt);\n"
		"    for (j=0;j<tv_tgt_layer.pdoc.layers.length;j++)\n"
		"        {\n"
		"        sl = tv_tgt_layer.pdoc.layers[j];\n"
		"        if (sl.pageY >= tv_tgt_layer.pageY + 20 && sl != tv_tgt_layer && sl.visibility == 'inherit')\n"
		"            {\n"
		"            sl.pageY += 20*(linkcnt);\n"
		"            }\n"
		"        }\n"
		"    for (i=1;i<=linkcnt;i++)\n"
		"        {\n"
		"        link_txt = tv_tgt_layer.ld.document.links[i].text;\n"
		"        link_href = tv_tgt_layer.ld.document.links[i].href;\n"
		"        one_link = link_href.substring(link_href.lastIndexOf('/')+1,link_href.length);\n"
		"        if (one_link[0] == ' ') one_link = one_link.substring(1,one_link.length);\n"
		"        one_layer = tv_new_layer(tv_tgt_layer.clip.width,tv_tgt_layer.pdoc);\n"
		"        im = '01';\n"
		"        if (link_txt == '' || link_txt == null) link_txt = one_link;\n"
		"        else link_txt = one_link + '&nbsp;-&nbsp;' + link_txt;\n"
		"        if (one_link.lastIndexOf('/') > 0) im = '02';\n"
		"        else one_link = one_link + '/';\n"
		"        imgs = '';\n"

		/*"        if (last == i)\n"
		"            {\n"
		"            imgs = tv_tgt_layer.childimgs + '<IMG SRC=/sys/images/tv1.gif align=left>';\n"
		"            one_layer.childimgs = tv_tgt_layer.childimgs + '<IMG SRC=/sys/images/tv3.gif align=left>';\n"
		"            }\n"
		"        else\n"
		"            {\n"
		"            imgs = tv_tgt_layer.childimgs + '<IMG SRC=/sys/images/tv2.gif align=left>';\n"
		"            one_layer.childimgs = tv_tgt_layer.childimgs + '<IMG SRC=/sys/images/tv0.gif align=left>';\n"
		"            }\n"*/

		"        one_layer.fname = tv_tgt_layer.fname + one_link;\n"
		"        //alert(one_layer.fname);\n"
		/*"        one_layer.document.writeln(\"<HTML><HEAD><META HTTP-EQUIV=\\\"Pragma\\\" CONTENT=\\\"no-cache\\\"></HEAD><BODY>\" + imgs + \"<IMG SRC=/sys/images/ico\" + im + \"b.gif align=left>&nbsp;<A HREF=\" + link_href + \">\" + link_txt + \"</A></BODY></HTML>\");\n"*/
		"        tvtext = imgs + \"<IMG SRC=/sys/images/ico\" + im + \"b.gif align=left>&nbsp;<A HREF=\" + link_href + \">\" + link_txt + \"</A>\";\n"
		"        if (one_layer.tvtext != tvtext)\n"
		"            {\n"
		"            one_layer.tvtext = tvtext;\n"
		"            one_layer.document.writeln(one_layer.tvtext);\n"
		"            one_layer.document.close();\n"
		"            }\n"
		"        one_layer.type = im;\n"
		"        one_layer.moveTo(tv_tgt_layer.x + 20, tv_tgt_layer.y + 20*i);\n"
		"        one_layer.visibility = 'inherit';\n"

		/*"        one_layer.moveTo(tv_tgt_layer.x, tv_tgt_layer.y + 20*i);\n"*/

		"        one_layer.img = one_layer.document.images[one_layer.document.images.length-1];\n"
		"        one_layer.img.kind = 'tv';\n"
		"        if (one_layer.document.links.length != 0)\n"
		"            {\n"
		"            one_layer.document.links[0].kind = 'tv';\n"
		"            one_layer.document.links[0].layer = one_layer;\n"
		"            }\n"
		"        one_layer.expanded = 0;\n"
		"        one_layer.img.layer = one_layer;\n"
		"        one_layer.zIndex = tv_tgt_layer.zIndex;\n"
		"        one_layer.pdoc = tv_tgt_layer.pdoc;\n"
		"        one_layer.ld = tv_tgt_layer.ld;\n"
		"        one_layer.root = tv_tgt_layer.root;\n"
		"        if (one_layer.clip.width != tv_tgt_layer.clip.width - one_layer.pageX + tv_tgt_layer.pageX)\n"
		"            one_layer.clip.width = tv_tgt_layer.clip.width - one_layer.pageX + tv_tgt_layer.pageX;\n"
		"        cnt++;\n"
		/*"        tv_tgt_layer.pdoc.height += 20;\n"*/
		/*"        if (window != tv_tgt_layer.pdoc.tv_layer_tgt)\n"
		"            tv_tgt_layer.pdoc.tv_layer_tgt.clip.height += 20;\n"*/
		/*"        for (j=0;j<tv_tgt_layer.pdoc.layers.length;j++)\n"
		"            {\n"
		"            sl = tv_tgt_layer.pdoc.layers[j];\n"
		"            if (sl.pageY >= one_layer.pageY && sl != one_layer && sl.visibility == 'inherit')\n"
		"                {\n"
		"                sl.pageY += 20;\n"
		"                }\n"
		"            }\n"*/
		"        }\n"
		"    pg_resize(tv_tgt_layer.parentLayer);\n"
		"    tv_tgt_layer.img.src = tv_tgt_layer.img.realsrc;\n"
		"    tv_tgt_layer.img.src = subst_last(tv_tgt_layer.img.src,'b.gif');\n"
		"    tv_tgt_layer.img.realsrc = null;\n"
		"    tv_tgt_layer = null;\n"
		"    return false;\n"
		"    }\n", 0);

	/** Our initialization processor function. **/
	htrAddScriptFunction(s, "tv_init", "\n"
		"function tv_init(l,fname,loader,pdoc,w,p)\n"
		"    {\n"
	     /*	"    l.nofocus = true;\n" */
	        "    l.LSParent = p;\n"
		"    l.fname = fname;\n"
		"    l.expanded = 0;\n"
		"    l.type = '02';\n"
		"    l.img = l.document.images[0];\n"
		"    l.img.layer = l;\n"
		"    l.img.kind = 'tv';\n"
		"    l.kind = 'tv';\n"
		"    l.pdoc = pdoc;\n"
		"    l.ld = loader;\n"
		"    l.root = l;\n"
		"    pdoc.tv_layer_cache = null;\n"
		"    pdoc.tv_layer_tgt = l.parentLayer;\n"
		"    l.clip.width = w;\n"
		"    l.childimgs = '';\n"
		"    }\n" ,0);

	/** Script initialization call. **/
	snprintf(sbuf,160,"    %s = %s.layers.tv%droot;\n",nptr, parentname, id);
	htrAddScriptInit(s, sbuf);
	snprintf(sbuf,160,"    tv_init(%s,\"%s\",%s.layers.tv%dload,%s,%d,%s);\n",
		nptr, src, parentname, id, parentname, w, parentobj);
	htrAddScriptInit(s, sbuf);

	/** HTML body <DIV> elements for the layers. **/
	snprintf(sbuf,160,"<DIV ID=\"tv%droot\"><IMG SRC=/sys/images/ico02b.gif align=left>&nbsp;%s</DIV>\n",id,src);
	htrAddBodyItem(s, sbuf);
	snprintf(sbuf,160,"<DIV ID=\"tv%dload\"></DIV>\n",id);
	htrAddBodyItem(s, sbuf);

	/** Event handler for click-on-url **/
	htrAddEventHandler(s, "document","CLICK","tv",
		"    if (e.target != null && e.target.kind == 'tv' && e.target.href != null)\n"
		"        {\n"
		"        return tv_click(e);\n"
		"        }\n");

	/** Add the event handling scripts **/
	htrAddEventHandler(s, "document","MOUSEDOWN","tv",
		"    if (e.target != null && e.target.kind=='tv' && e.target.href == null)\n"
		"        {\n"
		"        if (e.which == 3)\n"
		"            {\n"
		"            return false; //return tv_rclick(e);\n"
		"            }\n"
		"        else\n"
		"            {\n"
		"            tv_target_img = e.target;\n"
		"            tv_target_img.src = subst_last(tv_target_img.src,'c.gif');\n"
		"            }\n"
		"        }\n");

	htrAddEventHandler(s, "document","MOUSEUP","tv",
		"    if (e.target != null && e.target.kind == 'tv' && e.which == 3) return false;\n"
		"    if (tv_target_img != null && tv_target_img.kind == 'tv')\n"
		"        {\n"
		"        l = tv_target_img.layer;\n"
		"        tv_target_img = null;\n"
		"        if (l==null) return false;\n"
		"        if (l.img.realsrc != null) return false;\n"
		"        l.img.realsrc=l.img.src;\n"
		"        l.img.src='/sys/images/ico11c.gif';\n"
		"        if (l.expanded == 0)\n"
		"            {\n"
		"            if (l.fname.substr(l.fname.length-1,1) == '/' && l.fname.length > 1)\n"
		"                use_fname = l.fname.substr(0,l.fname.length-1);\n"
		"            else\n"
		"                use_fname = l.fname;\n"
		"            if (use_fname.lastIndexOf('?') > use_fname.lastIndexOf('/', use_fname.length-2))\n"
		"                l.ld.src = use_fname + '&ls__mode=list';\n"
		"            else\n"
		"                l.ld.src = use_fname + '?ls__mode=list';\n"
		"            l.ld.onload = tv_loaded;\n"
		"            tv_tgt_layer = l;\n"
		"            l.expanded = 1;\n"
		"            }\n"
		"        else\n"
		"            {\n"
		"            l.expanded = 0;\n"
		"            cnt = 0;\n"
		"            for(i=l.pdoc.layers.length-1;i>=0;i--)\n"
		"                {\n"
		"                sl = l.pdoc.layers[i];\n"
		"                if (sl.fname!=null && sl!=l && l.fname==sl.fname.substring(0,l.fname.length))\n"
		"                    {\n"
		"                    tv_cache_layer(sl,l.pdoc);\n"
		"                    delete l.pdoc.layers[i];\n"
		"                    sl.fname = null;\n"
		"                    sl.document.onmouseup = 0;\n"
		"                    cnt++;\n"
		"                    }\n"
		"                }\n"
		"            for (j=0;j<l.pdoc.layers.length;j++)\n"
		"                {\n"
		"                sl = l.pdoc.layers[j];\n"
		"                if (sl.pageY > l.pageY && sl.visibility == 'inherit')\n"
		"                    {\n"
		"                    sl.pageY -= 20*cnt;\n"
		"                    }\n"
		"                }\n"
		"            pg_resize(l.parentLayer);\n"
		"            l.img.src=l.img.realsrc;\n"
		"            l.img.src = subst_last(l.img.src,'b.gif');\n"
		"            l.img.realsrc = null;\n"
		"            }\n"
		"        }\n");

	/** Check for more sub-widgets within the treeview. **/
	qy = objOpenQuery(w_obj,"",NULL,NULL,NULL);
	if (qy)
	    {
	    while((sub_w_obj = objQueryFetch(qy, O_RDONLY)))
	        {
		htrRenderWidget(s, sub_w_obj, z+2, parentname, nptr);
		objClose(sub_w_obj);
		}
	    objQueryClose(qy);
	    }

    return 0;
    }


/*** httreeInitialize - register with the ht_render module.
 ***/
int
httreeInitialize()
    {
    pHtDriver drv;
    pHtEventAction event;
    pHtParam value;

    	/** Allocate the driver **/
	drv = (pHtDriver)nmMalloc(sizeof(HtDriver));
	if (!drv) return -1;

	/** Fill in the structure. **/
	strcpy(drv->Name,"HTML Treeview Widget Driver");
	strcpy(drv->WidgetName,"treeview");
	drv->Render = httreeRender;
	drv->Verify = httreeVerify;
	xaInit(&(drv->PosParams),16);
	xaInit(&(drv->Properties),16);
	xaInit(&(drv->Events),16);
	xaInit(&(drv->Actions),16);

	/** Add the 'click item' event **/
	event = (pHtEventAction)nmSysMalloc(sizeof(HtEventAction));
	strcpy(event->Name,"ClickItem");
	value = (pHtParam)nmSysMalloc(sizeof(HtParam));
	strcpy(value->ParamName,"Pathname");
	value->DataType = DATA_T_STRING;
	xaAddItem(&event->Parameters,(void*)value);
	xaAddItem(&drv->Events,(void*)event);

	/** Add the 'rightclick item' event **/
	event = (pHtEventAction)nmSysMalloc(sizeof(HtEventAction));
	strcpy(event->Name,"RightClickItem");
	value = (pHtParam)nmSysMalloc(sizeof(HtParam));
	strcpy(value->ParamName,"Pathname");
	value->DataType = DATA_T_STRING;
	xaAddItem(&event->Parameters,(void*)value);
	xaAddItem(&drv->Events,(void*)event);

	/** Register. **/
	htrRegisterDriver(drv);

	HTTREE.idcnt = 0;

    return 0;
    }
