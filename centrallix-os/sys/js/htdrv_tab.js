// Copyright (C) 1998-2004 LightSys Technology Services, Inc.
//
// You may use these files and this library under the terms of the
// GNU Lesser General Public License, Version 2.1, contained in the
// included file "COPYING" or http://www.gnu.org/licenses/lgpl.txt.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.


// Sets the value of the current tab (but not the appearance), without
// triggering on-change events.
function tc_set_tab_unwatched()
    {
    htr_unwatch(this.tabctl,"selected","tc_selection_changed");
    htr_unwatch(this.tabctl,"selected_index","tc_selection_changed");
    this.tabctl.selected_index = this.tabindex;
    this.tabctl.selected = this.tabname;
    this.tabctl.current_tab = this;
    htr_watch(this.tabctl,"selected", "tc_selection_changed");
    htr_watch(this.tabctl,"selected_index", "tc_selection_changed");
    }


// Makes the given tab current.
function tc_makecurrent()
    {
    var t;
    if (this.tabctl.tloc != 4 && htr_getzindex(this) > htr_getzindex(this.tabctl)) return 0;
    for(var i=0;i<this.tabctl.tabs.length;i++)
	{
	t = this.tabctl.tabs[i];
	if (t != this && (t.tabctl.tloc == 4 || htr_getzindex(t) > htr_getzindex(this)))
	    {
	    htr_setzindex(t.tabpage,htr_getzindex(this.tabctl) - 1);
	    htr_setvisibility(t.tabpage,'hidden');
	    if (t.tabctl.tloc != 4)
		{
		htr_setzindex(t,htr_getzindex(this.tabctl) - 1);
		t.marker_image.src = '/sys/images/tab_lft3.gif';
		moveBy(t, this.tabctl.xo, this.tabctl.yo);
		setClipItem(t,t.tabctl.cl, getClipItem(t,t.tabctl.cl) + t.tabctl.ci);
		if (this.tabctl.inactive_bgColor) htr_setbgcolor(t,this.tabctl.inactive_bgColor);
		if (this.tabctl.inactive_bgnd) htr_setbgimage(t,this.tabctl.inactive_bgnd);
		}
	    }
	}
    htr_setzindex(this.tabpage, htr_getzindex(this.tabctl) + 1);
    htr_setvisibility(this.tabpage,'inherit');
    if (this.tabctl.tloc != 4)
	{
	if (this.tabctl.main_bgColor) htr_setbgcolor(this,this.tabctl.main_bgColor);
	if (this.tabctl.main_bgnd) htr_setbgimage(this,this.tabctl.main_bgnd);
	htr_setzindex(this, htr_getzindex(this.tabctl) + 1);
	this.marker_image.src = '/sys/images/tab_lft2.gif';
	moveBy(this, -this.tabctl.xo, -this.tabctl.yo);
	setClipItem(this,this.tabctl.cl, getClipItem(this,this.tabctl.cl) - this.tabctl.ci);
	}
    this.setTabUnwatched();
    }


// Adds a new tab to the tab control
function tc_addtab(l_tab, l_page, l, nm)
    {
    var newx;
    var newy;
    if (!l_tab) l_tab = new Object();
    l_tab.tabname = nm;
    l_tab.tabindex = this.tabs.length+1;
    htr_init_layer(l_page,l,'tc_pn');
    if (l.tloc != 4) 
	{
	htr_init_layer(l_tab,l,'tc');
	if (l.tloc == 0 || l.tloc == 1) // top or bottom
	    {
	    if (this.tabs.length > 0)
		{
		newx = getPageX(this.tabs[this.tabs.length-1]) + htr_getphyswidth(this.tabs[this.tabs.length-1]) + 1;
		if (htr_getvisibility(this.tabs[this.tabs.length-1].tabpage) == 'inherit') newx += l.xo;
		}
	    else
		newx = getPageX(this);
	    }
	else if (l.tloc == 2) // left
	    newx = getPageX(this)- htr_getviswidth(l_tab) + 1;
	else if (l.tloc == 3) // right
	    newx = getPageX(this) + htr_getviswidth(this) - 1;

	if (l.tloc == 2 || l.tloc == 3) // left or right
	    {
	    if (this.tabs.length > 0)
		{
		newy = getPageY(this.tabs[this.tabs.length-1]) + 26;
		if (htr_getvisibility(this.tabs[this.tabs.length-1].tabpage) == 'inherit') newy += l.yo;
		}
	    else
		newy = getPageY(this);
	    }
	else if (l.tloc == 1) // bottom
	    newy = getPageY(this)+ htr_getvisheight(this) - 1;
	else // top
	    newy = getPageY(this) - 24;
	}
    else
	{
	newx = 0;
	newy = 0;
	}

    if (htr_getvisibility(l_page) != 'inherit')
	{
	if (l.tloc != 4)
	    {
	    newx += l.xo;
	    newy += l.yo;
	    setClipItem(l_tab, l.cl, getClipItem(l_tab, l.cl) + l.ci);
	    if (l.inactive_bgColor) htr_setbgcolor(l_tab, l.inactive_bgColor);
	    else if (l.main_bgColor) htr_setbgcolor(l_tab, l.main_bgColor);
	    if (l.inactive_bgnd) htr_setbgimage(l_tab, l.inactive_bgnd);
	    else if (l.main_bgnd) htr_setbgimage(l_tab, l.main_bgnd);
	    }
	}
    else
	{
	htr_unwatch(l,"selected","tc_selection_changed");
	htr_unwatch(l,"selected_index","tc_selection_changed");
	l.selected = l_tab.tabname;
	l.selected_index = l_tab.tabindex;
	l.current_tab = l_tab;
	l.init_tab = l_tab;
	pg_addsched_fn(window,"pg_reveal_event",new Array(l_tab,l_tab,'Reveal'), 0);
	htr_watch(l,"selected", "tc_selection_changed");
	htr_watch(l,"selected_index", "tc_selection_changed");
	if (l.tloc != 4)
	    {
	    if (l.main_bgColor) htr_setbgcolor(l_tab, l.main_bgColor);
	    if (l.main_bgnd) htr_setbgimage(l_tab, l.main_bgnd);
	    }
	}
    if (l.tloc != 4)
	{
	var images = pg_images(l_tab);
	for(var i=0;i<images.length;i++)
	    {
	    images[i].layer = l_tab;
	    images[i].kind = 'tc';
	    if  (images[i].width == 5) l_tab.marker_image = images[i];
	    }
	}
    this.tabs[this.tabs.length++] = l_tab;
    l_tab.tabpage = l_page;
    l_tab.tabctl = this;
    l_tab.makeCurrent = tc_makecurrent;
    l_tab.setTabUnwatched = tc_set_tab_unwatched;
    if (l.tloc != 4)
	{
	moveToAbsolute(l_tab, newx + 0.001, newy + 0.001);
	}
    l_page.tabctl = this;
    l_page.tab = l_tab;
    setClipWidth(l_page, getClipWidth(this)-2);
    setClipHeight(l_page, getClipHeight(this)-2);

    // Indicate that we generate reveal/obscure notifications
    l_tab.Reveal = tc_cb_reveal;
    pg_reveal_register_triggerer(l_tab);
    //if (htr_getvisibility(l_page) == 'inherit') pg_addsched("pg_reveal(" + l_tab.tabname + ")");

    // Show Container API
    l_page.showcontainer = tc_showcontainer;

    return l_tab;
    }

function tc_selection_changed(prop,o,n)
    {
    var tabindex = null;
    if (o == n) return n;
    // find index if name specified
    if (prop == 'selected')
	{
	for (var i=0; i<this.tabs.length; i++)
	    if (this.tabs[i].tabname == n)
		{
		tabindex = i+1;
		break;
		}
	}
    else
	tabindex = n;
    if (tabindex < 1 || tabindex > this.tabs.length) return o;

    // okay to change tab.
    //this.tabs[tabindex-1].makeCurrent();
    pg_addsched_fn(this,"ChangeSelection1", new Array(this.tabs[tabindex-1].tabpage), 0);
    return n;
    }

function tc_action_set_tab(aparam)
    {
    if (aparam.Tab) this.selected = aparam.Tab;
    else if (aparam.TabIndex) this.selected_index = parseInt(aparam.TabIndex);
    }

function tc_showcontainer()
    {
    if (htr_getvisibility(this) != 'inherit')
	this.tabctl.ChangeSelection1(this);
    return true;
    }

function tc_init(param)
    {
    // Basic stuff...
    var l = param.layer; 
    htr_init_layer(l,l,'tc');
    ifc_init_widget(l);
    l.tabs = new Array();
    l.addTab = tc_addtab;
    l.current_tab = null;
    l.init_tab = null;
    l.tloc = param.tloc;
    if (tc_tabs == null) tc_tabs = new Array();
    tc_tabs[tc_tabs.length++] = l;

    // Background color/image selection...
    l.main_bgColor = htr_extract_bgcolor(param.mainBackground);
    l.main_bgnd = htr_extract_bgimage(param.mainBackground);
    l.inactive_bgColor = htr_extract_bgcolor(param.inactiveBackground);
    l.inactive_bgnd = htr_extract_bgimage(param.inactiveBackground);

    // Properties available to other widgets, that can be used to
    // change current tab index as well.
    l.selected = null;
    l.selected_index = 1;
    htr_watch(l,"selected", "tc_selection_changed");
    htr_watch(l,"selected_index", "tc_selection_changed");
    l.tc_selection_changed = tc_selection_changed;

    // Actions
    var ia = l.ifcProbeAdd(ifAction);
    ia.Add("SetTab", tc_action_set_tab);

    // Events
    var ie = l.ifcProbeAdd(ifEvent);
    ie.Add("MouseDown");
    ie.Add("MouseUp");
    ie.Add("MouseOver");
    ie.Add("MouseOut");
    ie.Add("MouseMove");

    // Reveal/Obscure mechanism
    l.ChangeSelection1 = tc_changeselection_1;
    l.ChangeSelection2 = tc_changeselection_2;
    l.ChangeSelection3 = tc_changeselection_3;

    // Movement geometries and clipping for tabs
    switch(l.tloc)
	{
	case 0: // top
	    l.xo = +1;
	    l.yo = +2;
	    l.cl = "bottom";
	    l.ci = -2;
	    break;
	case 1: // bottom
	    l.xo = +1;
	    l.yo = -2;
	    l.cl = "top";
	    l.ci = +2;
	    break;
	case 2: // left
	    l.xo = +2;
	    l.yo = +1;
	    l.cl = "right";
	    l.ci = -2;
	    break;
	case 3: // right
	    l.xo = -2;
	    l.yo = +1;
	    l.cl = "left";
	    l.ci = +2;
	    break;
	case 4: // none
	    l.xo = 0;
	    l.yo = 0;
	    l.cl = "bottom";
	    l.ci = 0;
	    break;
	}
    return l;
    }


// Reveal() interface function - called when a triggerer event occurs.
// c == tab (not tabpage) to make current.
// this == tab (not tabpage) event (revealcheck/obscurecheck) was processed for.
function tc_cb_reveal(e)
    {
    switch(e.eventName)
	{
	case 'ObscureOK':
	    this.tabctl.ChangeSelection2(e.c);
	    break;
	case 'RevealOK':
	    this.tabctl.ChangeSelection3(e.c);
	    break;
	case 'ObscureFailed':
	case 'RevealFailed':
	    this.tabctl.current_tab.setTabUnwatched();
	    break;
	}
    return true;
    }

// change selection - first function, called initially.
function tc_changeselection_1(c)
    {
    pg_reveal_event(c.tabctl.current_tab, c.tab, 'ObscureCheck');
    }
// change selection - second function, called when ObscureCheck succeeds.
function tc_changeselection_2(c)
    {
    pg_reveal_event(c, c, 'RevealCheck');
    }
// change selection - third function, called when RevealCheck succeeds.
function tc_changeselection_3(c)
    {
    pg_reveal_event(c, c, 'Reveal');
    pg_reveal_event(c.tabctl.current_tab, c, 'Obscure');
    c.makeCurrent();
    }

function tc_mousedown(e)
    {
    if (e.mainlayer && e.mainkind == 'tc')
	 cn_activate(e.mainlayer, 'MouseDown');
    if (e.kind == 'tc' && e.layer.tabctl)
	 e.layer.tabctl.ChangeSelection1(e.layer.tabpage);
    return EVENT_CONTINUE | EVENT_ALLOW_DEFAULT_ACTION;
    }

function tc_mouseup(e)
    {
    if (e.mainlayer && e.mainkind == 'tc') 
	cn_activate(e.mainlayer, 'MouseUp');
    return EVENT_CONTINUE | EVENT_ALLOW_DEFAULT_ACTION;
    }

function tc_mousemove(e)
    {
    if (!e.mainlayer || e.mainkind != 'tc')
        {
        if (tc_cur_mainlayer) cn_activate(tc_cur_mainlayer, 'MouseOut');
            tc_cur_mainlayer = null;
        }
    else if (e.kind && e.kind.substr(0,2) == 'tc')
	{
        cn_activate(e.mainlayer, 'MouseMove');
        }
    return EVENT_CONTINUE | EVENT_ALLOW_DEFAULT_ACTION;
    }

function tc_mouseover(e)
    {
    if (e.kind && e.kind.substr(0,2) == 'tc')
        {
        cn_activate(e.mainlayer, 'MouseOver');
        tc_cur_mainlayer = e.mainlayer;
        }
    return EVENT_CONTINUE | EVENT_ALLOW_DEFAULT_ACTION;
    }
