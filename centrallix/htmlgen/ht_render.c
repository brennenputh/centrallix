#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
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
/* Module: 	ht_render.h,ht_render.c					*/
/* Author:	Greg Beeley (GRB)					*/
/* Creation:	November 19, 1998					*/
/* Description:	HTML Page rendering engine that interacts with the 	*/
/*		various widget drivers to produce a dynamic HTML page.	*/
/************************************************************************/

/**CVSDATA***************************************************************

    $Id: ht_render.c,v 1.3 2002/03/09 19:21:20 gbeeley Exp $
    $Source: /srv/bld/centrallix-repo/centrallix/htmlgen/ht_render.c,v $

    $Log: ht_render.c,v $
    Revision 1.3  2002/03/09 19:21:20  gbeeley
    Basic security overhaul of the htmlgen subsystem.  Fixed many of my
    own bad sprintf habits that somehow worked their way into some other
    folks' code as well ;)  Textbutton widget had an inadequate buffer for
    the tb_init() call, causing various problems, including incorrect labels,
    and more recently, javascript errors.

    Revision 1.2  2001/10/22 17:19:42  gbeeley
    Added a few utility functions in ht_render to simplify the structure and
    authoring of widget drivers a bit.

    Revision 1.1.1.1  2001/08/13 18:00:48  gbeeley
    Centrallix Core initial import

    Revision 1.2  2001/08/07 19:31:52  gbeeley
    Turned on warnings, did some code cleanup...

    Revision 1.1.1.1  2001/08/07 02:30:53  gbeeley
    Centrallix Core Initial Import


 **END-CVSDATA***********************************************************/


/*** GLOBALS ***/
struct
    {
    XHashTable	WidgetDrivers;		/* widget name -> driver struct map */
    XArray	Drivers;		/* simple driver listing. */
    }
    HTR;


/*** htr_internal_AddTextToArray - adds a string of text to an array of 
 *** buffer blocks, allocating new blocks in the XArray if necessary.
 ***/
int
htr_internal_AddTextToArray(pXArray arr, char* txt)
    {
    int l,n,cnt;
    char* ptr;

    	/** Need new block? **/
	if (arr->nItems == 0)
	    {
	    ptr = (char*)nmMalloc(2048);
	    if (!ptr) return -1;
	    *(int*)ptr = 0;
	    l = 0;
	    xaAddItem(arr,ptr);
	    }
	else
	    {
	    ptr = (char*)(arr->Items[arr->nItems-1]);
	    l = *(int*)ptr;
	    }

	/** Copy into the blocks, allocating more as needed. **/
	n = strlen(txt);
	while(n)
	    {
	    cnt = n;
	    if (cnt > (2040-l)) cnt = 2040-l;
	    memcpy(ptr+l+8,txt,cnt);
	    n -= cnt;
	    txt += cnt;
	    l += cnt;
	    *(int*)ptr = l;
	    if (n)
	        {
		ptr = (char*)nmMalloc(2048);
		if (!ptr) return -1;
		*(int*)ptr = 0;
		l = 0;
		xaAddItem(arr,ptr);
		}
	    }

    return 0;
    }


/*** htrRenderWidget - generate a widget into the HtPage structure, given the
 *** widget's objectsystem descriptor...
 ***/
int
htrRenderWidget(pHtSession session, pObject widget_obj, int z, char* parentname, char* parentobj)
    {
    char* w_name;
    pHtDriver drv;

    	/** Get the name of the widget.. **/
	objGetAttrValue(widget_obj, "outer_type", POD(&w_name));
	if (strncmp(w_name,"widget/",7)) 
	    {
	    mssError(1,"HTR","Invalid content type for widget - must be widget/xxx");
	    return -1;
	    }

	/** Lookup the driver **/
	drv = (pHtDriver)xhLookup(&(HTR.WidgetDrivers),w_name+7);
	if (!drv) 
	    {
	    mssError(1,"HTR","Unknown widget content type");
	    return -1;
	    }

	/** Call the driver. **/
	drv->Render(session, widget_obj, z, parentname, parentobj);

    return 0;
    }


/*** htrAddHeaderItem -- copies html text into the buffers that will 
 *** eventually be output as the HTML header.
 ***/
int 
htrAddHeaderItem(pHtSession s, char* html_text)
    {
    return htr_internal_AddTextToArray(&(s->Page.HtmlHeader), html_text);
    }


/*** htrAddBodyItem -- copies html text into the buffers that will 
 *** eventually be output as the HTML body.
 ***/
int 
htrAddBodyItem(pHtSession s, char* html_text)
    {
    return htr_internal_AddTextToArray(&(s->Page.HtmlBody), html_text);
    }


/*** htrAddBodyParam -- copies html text into the buffers that will
 *** eventually be output as the HTML body.  These are simple html tag
 *** parameters (i.e., "BGCOLOR=white")
 ***/
int 
htrAddBodyParam(pHtSession s, char* html_param)
    {
    return htr_internal_AddTextToArray(&(s->Page.HtmlBodyParams), html_param);
    }


/*** htrAddScriptFunction -- adds a script function to the list of functions
 *** that will be output.  Note that duplicate functions won't be added, so
 *** the widget drivers need not keep track of this.
 ***/
int 
htrAddScriptFunction(pHtSession s, char* fn_name, char* fn_text, int flags)
    {
    pStrValue sv;

    	/** Alloc the string val. **/
	if (xhLookup(&(s->Page.NameFunctions), fn_name)) return 0;
	sv = (pStrValue)nmMalloc(sizeof(StrValue));
	if (!sv) return -1;
	sv->Name = fn_name;
	sv->Value = fn_text;
	sv->Alloc = flags;

	/** Add to the hash table and array **/
	xhAdd(&(s->Page.NameFunctions), fn_name, (char*)sv);
	xaAddItem(&(s->Page.Functions), (char*)sv);

    return 0;
    }


/*** htrAddScriptGlobal -- adds a global variable to the list of variables
 *** to be output in the HTML JavaScript section.  Duplicates are suppressed.
 ***/
int 
htrAddScriptGlobal(pHtSession s, char* var_name, char* initialization, int flags)
    {
    pStrValue sv;

    	/** Alloc the string val. **/
	if (xhLookup(&(s->Page.NameGlobals), var_name)) return 0;
	sv = (pStrValue)nmMalloc(sizeof(StrValue));
	if (!sv) return -1;
	sv->Name = var_name;
	sv->NameSize = strlen(var_name)+1;
	sv->Value = initialization;
	sv->ValueSize = strlen(initialization)+1;
	sv->Alloc = flags;

	/** Add to the hash table and array **/
	xhAdd(&(s->Page.NameGlobals), var_name, (char*)sv);
	xaAddItem(&(s->Page.Globals), (char*)sv);

    return 0;
    }


/*** htrAddScriptInit -- adds some initialization text that runs outside of a
 *** function context in the HTML JavaScript.
 ***/
int 
htrAddScriptInit(pHtSession s, char* init_text)
    {
    return htr_internal_AddTextToArray(&(s->Page.Inits), init_text);
    }


/*** htrAddEventHandler - adds an event handler script code segment for a
 *** given event on a given object (usually the 'document').
 ***/
int
htrAddEventHandler(pHtSession s, char* event_src, char* event, char* drvname, char* handler_code)
    {
    pHtNameArray obj, evt, drv;

    	/** Is this object already listed? **/
	obj = (pHtNameArray)xhLookup(&(s->Page.EventScripts.HashTable), event_src);

	/** If not, create new object for this event source **/
	if (!obj)
	    {
	    obj = (pHtNameArray)nmMalloc(sizeof(HtNameArray));
	    if (!obj) return -1;
	    memccpy(obj->Name, event_src, 0, 127);
	    obj->Name[127] = '\0';
	    xhInit(&(obj->HashTable),31,0);
	    xaInit(&(obj->Array),16);
	    xhAdd(&(s->Page.EventScripts.HashTable), obj->Name, (void*)(obj));
	    xaAddItem(&(s->Page.EventScripts.Array), (void*)obj);
	    }

	/** Is this event name already listed? **/
	evt = (pHtNameArray)xhLookup(&(obj->HashTable), event);
	
	/** If not already, create new. **/
	if (!evt)
	    {
	    evt = (pHtNameArray)nmMalloc(sizeof(HtNameArray));
	    if (!evt) return -1;
	    memccpy(evt->Name, event,0,127);
	    evt->Name[127] = '\0';
	    xhInit(&(evt->HashTable),31,0);
	    xaInit(&(evt->Array),16);
	    xhAdd(&(obj->HashTable), evt->Name, (void*)evt);
	    xaAddItem(&(obj->Array), (void*)evt);
	    }

	/** Is the driver name already listed? **/
	drv = (pHtNameArray)xhLookup(&(evt->HashTable),drvname);

	/** If not already, add new. **/
	if (!drv)
	    {
	    drv = (pHtNameArray)nmMalloc(sizeof(HtNameArray));
	    if (!drv) return -1;
	    memccpy(drv->Name, drvname, 0, 127);
	    drv->Name[127] = '\0';
	    xaInit(&(drv->Array),16);
	    xhAdd(&(evt->HashTable), drv->Name, (void*)drv);
	    xaAddItem(&(evt->Array), (void*)drv);
	   
	    /** Ok, got event and object.  Now, add script text. **/
            htr_internal_AddTextToArray(&(drv->Array), handler_code);
	    }

    return 0;
    }


/*** htrDisableBody - disables the <BODY> </BODY> tags so that, for instance,
 *** a frameset item can be used.
 ***/
int
htrDisableBody(pHtSession s)
    {
    s->DisableBody = 1;
    return 0;
    }


/*** htrAddEvent - adds an event to a driver.
 ***/
int
htrAddEvent(pHtDriver drv, char* event_name)
    {
    pHtEventAction event;

	/** Create the action **/
	event = (pHtEventAction)nmSysMalloc(sizeof(HtEventAction));
	if (!event) return -1;
	memccpy(event->Name, event_name, 0, 31);
	event->Name[31] = '\0';
	xaAddItem(&drv->Events, (void*)event);

    return 0;
    }


/*** htrAddAction - adds an action to a widget.
 ***/
int
htrAddAction(pHtDriver drv, char* action_name)
    {
    pHtEventAction action;

	/** Create the action **/
	action = (pHtEventAction)nmSysMalloc(sizeof(HtEventAction));
	if (!action) return -1;
	memccpy(action->Name, action_name, 0, 31);
	action->Name[31] = '\0';
	xaAddItem(&drv->Actions, (void*)action);

    return 0;
    }


/*** htrAddParam - adds a parameter to a widget's action or event
 ***/
int
htrAddParam(pHtDriver drv, char* eventaction, char* param_name, int datatype)
    {
    pHtEventAction ea = NULL;
    int i;
    pHtParam p;

	/** Look for a matching event/action **/
	for(i=0;i<drv->Actions.nItems;i++)
	    {
	    if (!strcmp(((pHtEventAction)(drv->Actions.Items[i]))->Name, eventaction))
	        {
		ea = (pHtEventAction)(drv->Actions.Items[i]);
		break;
		}
	    }
	if (!ea) for(i=0;i<drv->Events.nItems;i++)
	    {
	    if (!strcmp(((pHtEventAction)(drv->Events.Items[i]))->Name, eventaction))
	        {
		ea = (pHtEventAction)(drv->Events.Items[i]);
		break;
		}
	    }
	if (!ea) return -1;

	/** Add the parameter **/
	p = nmSysMalloc(sizeof(HtParam));
	if (!p) return -1;
	memccpy(p->ParamName, param_name, 0, 31);
	p->ParamName[31] = '\0';
	p->DataType = datatype;
	xaAddItem(&(ea->Parameters), (void*)p);

    return 0;
    }


/*** htrRenderSubwidgets - generates the code for all subwidgets within
 *** the current widget.  This is  a generic function that does not 
 *** necessarily apply to all widgets that contain other widgets, but 
 *** is useful for your basic ordinary "container" type widget, such
 *** as panes and tab pages.
 ***/
int
htrRenderSubwidgets(pHtSession s, pObject widget_obj, char* docname, char* layername, int zlevel)
    {
    pObjQuery qy;
    pObject sub_widget_obj;

	/** Open the query for subwidgets **/
	qy = objOpenQuery(widget_obj, "", NULL, NULL, NULL);
	if (qy)
	    {
	    while((sub_widget_obj = objQueryFetch(qy, O_RDONLY)))
	        {
		htrRenderWidget(s, sub_widget_obj, zlevel, docname, layername);
		objClose(sub_widget_obj);
		}
	    objQueryClose(qy);
	    }

    return 0;
    }


/*** htrRender - generate an HTML document given the app structure subtree
 *** as an open ObjectSystem object.
 ***/
int
htrRender(pFile output, pObject appstruct)
    {
    pHtSession s;
    int i,n,j,k,l;
    pStrValue tmp;
    char* ptr;
    pStrValue sv;
    char sbuf[HT_SBUF_SIZE];
    char ename[40];
    pHtNameArray tmp_a, tmp_a2, tmp_a3;

    	/** Initialize the session **/
	s = (pHtSession)nmMalloc(sizeof(HtSession));
	if (!s) return -1;

	/** Setup the page structures **/
	xhInit(&(s->Page.NameFunctions),31,0);
	xaInit(&(s->Page.Functions),16);
	xhInit(&(s->Page.NameGlobals),31,0);
	xaInit(&(s->Page.Globals),16);
	xaInit(&(s->Page.Inits),16);
	xaInit(&(s->Page.HtmlBody),16);
	xaInit(&(s->Page.HtmlHeader),16);
	xaInit(&(s->Page.HtmlBodyParams),16);
	xaInit(&(s->Page.EventScripts.Array),16);
	xhInit(&(s->Page.EventScripts.HashTable),31,0);
	s->Page.HtmlBodyFile = NULL;
	s->Page.HtmlHeaderFile = NULL;
	s->DisableBody = 0;

	/** Render the top-level widget. **/
	htrRenderWidget(s, appstruct, 10, "document", "document");

	/** Write the HTML out... **/
	fdWrite(output, "<HTML>\n<HEAD>\n",14,0,0);

	/** Write the HTML header items. **/
	for(i=0;i<s->Page.HtmlHeader.nItems;i++)
	    {
	    ptr = (char*)(s->Page.HtmlHeader.Items[i]);
	    n = *(int*)ptr;
	    fdWrite(output, ptr+8, n,0,0);
	    }
	fdWrite(output, "\n</HEAD>\n",9,0,0);

	/** Write the script globals **/
	fdWrite(output, "<SCRIPT language=javascript>\n\n\n", 31,0,0);
	for(i=0;i<s->Page.Globals.nItems;i++)
	    {
	    sv = (pStrValue)(s->Page.Globals.Items[i]);
	    if (sv->Value[0])
		snprintf(sbuf,HT_SBUF_SIZE,"var %s = %s;\n", sv->Name, sv->Value);
	    else
		snprintf(sbuf,HT_SBUF_SIZE,"var %s;\n", sv->Name);
	    fdWrite(output, sbuf, strlen(sbuf),0,0);
	    }

	/** Write the script functions **/
	for(i=0;i<s->Page.Functions.nItems;i++)
	    {
	    sv = (pStrValue)(s->Page.Functions.Items[i]);
	    fdWrite(output, sv->Value, strlen(sv->Value),0,0);
	    }

	/** Write the event scripts themselves. **/
	for(i=0;i<s->Page.EventScripts.Array.nItems;i++)
	    {
	    tmp_a = (pHtNameArray)(s->Page.EventScripts.Array.Items[i]);
	    for(j=0;j<tmp_a->Array.nItems;j++)
	        {
	        tmp_a2 = (pHtNameArray)(tmp_a->Array.Items[j]);
	        snprintf(sbuf,HT_SBUF_SIZE,"\nfunction e%d_%d(e)\n    {\n",i,j);
		fdWrite(output,sbuf,strlen(sbuf),0,0);
		for(k=0;k<tmp_a2->Array.nItems;k++)
		    {
		    tmp_a3 = (pHtNameArray)(tmp_a2->Array.Items[k]);
		    for(l=0;l<tmp_a3->Array.nItems;l++)
		        {
		        ptr = (char*)(tmp_a3->Array.Items[l]);
		        fdWrite(output,ptr+8,*(int*)ptr,0,0);
			}
		    }
		fdWrite(output,"    return true;\n    }\n",23,0,0);
		}
	    }

	/** Write the event capture lines **/
	fdWrite(output,"\nfunction events()\n    {\n", 25,0,0);
	for(i=0;i<s->Page.EventScripts.Array.nItems;i++)
	    {
	    tmp_a = (pHtNameArray)(s->Page.EventScripts.Array.Items[i]);
	    snprintf(sbuf,HT_SBUF_SIZE,"    %.64s.captureEvents(",tmp_a->Name);
	    for(j=0;j<tmp_a->Array.nItems;j++)
	        {
	        tmp_a2 = (pHtNameArray)(tmp_a->Array.Items[j]);
		if (j!=0) strcat(sbuf," | ");
		strcat(sbuf,"Event.");
		strcat(sbuf,tmp_a2->Name);
		}
	    strcat(sbuf,");\n");
	    fdWrite(output,sbuf,strlen(sbuf),0,0);
	    for(j=0;j<tmp_a->Array.nItems;j++)
	        {
	        tmp_a2 = (pHtNameArray)(tmp_a->Array.Items[j]);
		n = strlen(tmp_a2->Name);
		for(k=0;k<=n;k++) ename[k] = tolower(tmp_a2->Name[k]);
		snprintf(sbuf,HT_SBUF_SIZE,"    %.64s.on%s=e%d_%d;\n",tmp_a->Name,ename,i,j);
		fdWrite(output,sbuf,strlen(sbuf),0,0);
		}
	    }
	fdWrite(output,"    }\n",6,0,0);

	/** Write the initialization lines **/
	fdWrite(output,"\nfunction startup()\n    {\n",26,0,0);
	for(i=0;i<s->Page.Inits.nItems;i++)
	    {
	    ptr = (char*)(s->Page.Inits.Items[i]);
	    n = *(int*)ptr;
	    fdWrite(output, ptr+8, n,0,0);
	    }
	fdWrite(output,"    events();\n", 14,0,0);
	fdWrite(output,"    }\n",6,0,0);

	/** If the body part is disabled, skip over body section generation **/
	if (s->DisableBody == 0)
	    {
	    /** Write the HTML body params **/
	    fdWrite(output, "\n</SCRIPT>\n<BODY", 16,0,0);
	    for(i=0;i<s->Page.HtmlBodyParams.nItems;i++)
	        {
	        ptr = (char*)(s->Page.HtmlBodyParams.Items[i]);
	        n = *(int*)ptr;
	        fdWrite(output, ptr+8, n,0,0);
	        }
	    fdWrite(output, " onLoad=\"startup();\">\n", 22,0,0);
	    }
	else
	    {
	    fdWrite(output, "\n</SCRIPT>\n",11,0,0);
	    }

	/** Write the HTML body. **/
	for(i=0;i<s->Page.HtmlBody.nItems;i++)
	    {
	    ptr = (char*)(s->Page.HtmlBody.Items[i]);
	    n = *(int*)ptr;
	    fdWrite(output, ptr+8, n,0,0);
	    }

	if (s->DisableBody == 0)
	    {
	    fdWrite(output, "</BODY>\n</HTML>\n",16,0,0);
	    }
	else
	    {
	    fdWrite(output, "\n</HTML>\n",9,0,0);
	    }

	/** Deinitialize the session and page structures **/
	for(i=0;i<s->Page.Functions.nItems;i++)
	    {
	    tmp = (pStrValue)(s->Page.Functions.Items[i]);
	    xhRemove(&(s->Page.NameFunctions),tmp->Name);
	    if (tmp->Alloc & HTR_F_NAMEALLOC) nmFree(tmp->Name,tmp->NameSize);
	    if (tmp->Alloc & HTR_F_VALUEALLOC) nmFree(tmp->Value,tmp->ValueSize);
	    nmFree(tmp,sizeof(StrValue));
	    }
	xaDeInit(&(s->Page.Functions));
	xhDeInit(&(s->Page.NameFunctions));
	for(i=0;i<s->Page.Globals.nItems;i++)
	    {
	    tmp = (pStrValue)(s->Page.Globals.Items[i]);
	    xhRemove(&(s->Page.NameGlobals),tmp->Name);
	    if (tmp->Alloc & HTR_F_NAMEALLOC) nmFree(tmp->Name,tmp->NameSize);
	    if (tmp->Alloc & HTR_F_VALUEALLOC) nmFree(tmp->Value,tmp->ValueSize);
	    nmFree(tmp,sizeof(StrValue));
	    }
	xaDeInit(&(s->Page.Globals));
	xhDeInit(&(s->Page.NameGlobals));
	for(i=0;i<s->Page.Inits.nItems;i++) nmFree(s->Page.Inits.Items[i],2048);
	xaDeInit(&(s->Page.Inits));
	for(i=0;i<s->Page.HtmlBody.nItems;i++) nmFree(s->Page.HtmlBody.Items[i],2048);
	xaDeInit(&(s->Page.HtmlBody));
	for(i=0;i<s->Page.HtmlHeader.nItems;i++) nmFree(s->Page.HtmlHeader.Items[i],2048);
	xaDeInit(&(s->Page.HtmlHeader));
	for(i=0;i<s->Page.HtmlBodyParams.nItems;i++) nmFree(s->Page.HtmlBodyParams.Items[i],2048);
	xaDeInit(&(s->Page.HtmlBodyParams));

	/** Clean up the event script structure, which is multi-level. **/
	for(i=0;i<s->Page.EventScripts.Array.nItems;i++)
	    {
	    tmp_a = (pHtNameArray)(s->Page.EventScripts.Array.Items[i]);
	    for(j=0;j<tmp_a->Array.nItems;j++)
	        {
	        tmp_a2 = (pHtNameArray)(tmp_a->Array.Items[j]);
		for(k=0;k<tmp_a2->Array.nItems;k++) 
		    {
		    tmp_a3 = (pHtNameArray)(tmp_a2->Array.Items[k]);
		    for(l=0;l<tmp_a3->Array.nItems;l++)
		        {
			nmFree(tmp_a3->Array.Items[l],2048);
			}
	            xaDeInit(&(tmp_a3->Array));
	            xhRemove(&(tmp_a2->HashTable),tmp_a3->Name);
		    nmFree(tmp_a3,sizeof(HtNameArray));
		    }
	        xaDeInit(&(tmp_a2->Array));
		xhDeInit(&(tmp_a2->HashTable));
	        xhRemove(&(tmp_a->HashTable),tmp_a2->Name);
	        nmFree(tmp_a2,sizeof(HtNameArray));
		}
	    xaDeInit(&(tmp_a->Array));
	    xhDeInit(&(tmp_a->HashTable));
	    xhRemove(&(s->Page.EventScripts.HashTable),tmp_a->Name);
	    nmFree(tmp_a,sizeof(HtNameArray));
	    }
	xhDeInit(&(s->Page.EventScripts.HashTable));
	xaDeInit(&(s->Page.EventScripts.Array));

	nmFree(s,sizeof(HtSession));

    return 0;
    }


/*** htrAllocDriver - allocates a driver structure that can later be
 *** registered by using the next function below, htrRegisterDriver().
 ***/
pHtDriver
htrAllocDriver()
    {
    pHtDriver drv;

	/** Allocate the driver structure **/
	drv = (pHtDriver)nmMalloc(sizeof(HtDriver));
	if (!drv) return NULL;

	/** Init some of the basic array structures **/
	xaInit(&(drv->PosParams),16);
	xaInit(&(drv->Properties),16);
	xaInit(&(drv->Events),16);
	xaInit(&(drv->Actions),16);

    return drv;
    }


/*** htrRegisterDriver - register a new driver with the rendering system
 *** and map the widget name to the driver's structure for later access.
 ***/
int 
htrRegisterDriver(pHtDriver drv)
    {

    	/** Add to the drivers listing and the widget name map. **/
	xaAddItem(&(HTR.Drivers),(void*)drv);
	xhAdd(&(HTR.WidgetDrivers),drv->WidgetName,(char*)drv);

    return 0;
    }


/*** htrInitialize - initialize the system and the global variables and
 *** structures.
 ***/
int
htrInitialize()
    {

    	/** Initialize the global hash tables and arrays **/
	xaInit(&(HTR.Drivers),64);
	xhInit(&(HTR.WidgetDrivers),255,0);

    return 0;
    }


