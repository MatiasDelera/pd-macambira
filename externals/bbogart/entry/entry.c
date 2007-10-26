/* text entry widget for PD                                              *
 * Based on button from GGEE by Guenter Geiger                           *
 * Copyright Ben Bogart 2004 ben@ekran.org                               * 

 * This program is distributed under the terms of the GNU General Public *
 * License                                                               *

 * entry is free software; you can redistribute it and/or modify         *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *

 * entry is distributed in the hope that it will be useful,              *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          */

#include <m_pd.h>
#include <g_canvas.h>
#include <stdio.h>
#include <string.h>

/* TODO: make "display only" option, to force box to never accept focus */
/* TODO: make focus option only accept regular and shifted chars, not Cmd, Alt, Ctrl */
/* TODO: make entry_save include whole classname, including namespace prefix */
/* TODO: make [size( message redraw object */
/* TODO: set message doesnt work with a loadbang */
/* TODO: add [append( message to add contents after existing contents, just
 *      base it on the set method, without the preceeding delete  */

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

#ifndef IOWIDTH 
#define IOWIDTH 4
#endif

#define BACKGROUNDCOLOR "grey70"

#define DEBUG(x) x

typedef struct _entry
{
    t_object x_obj;
    
    t_glist * x_glist;
    int x_rect_width;
    int x_rect_height;
    t_symbol*  x_receive_name;

    char *x_widget_name;
    char *x_canvas_name;
	
/* TODO: these all should be settable by messages */
    int x_height;
    int x_width;

    t_symbol* x_bgcolour;
    t_symbol* x_fgcolour;
    
/* TODO: these all should be settable by messages
    t_symbol *x_font_face;
    t_float x_font_size;
    t_symbol *x_font_weight;

    t_float x_border;
    t_float x_highlightthickness;
    t_symbol *x_relief;
*/
    t_symbol* x_contents;

    t_outlet* x_data_outlet;
    t_outlet* x_status_outlet;
} t_entry;


static t_class *entry_class;


static t_symbol *backspace_symbol;
static t_symbol *return_symbol;
static t_symbol *space_symbol;
static t_symbol *tab_symbol;
static t_symbol *escape_symbol;
static t_symbol *left_symbol;
static t_symbol *right_symbol;
static t_symbol *up_symbol;
static t_symbol *down_symbol;

/* function prototypes */

static void entry_getrect(t_gobj *z, t_glist *owner, int *xp1, int *yp1, int *xp2, int *yp2);
static void entry_displace(t_gobj *z, t_glist *glist, int dx, int dy);
static void entry_select(t_gobj *z, t_glist *glist, int state);
static void entry_activate(t_gobj *z, t_glist *glist, int state);
static void entry_delete(t_gobj *z, t_glist *glist);
static void entry_vis(t_gobj *z, t_glist *glist, int vis);
static void entry_save(t_gobj *z, t_binbuf *b);


t_widgetbehavior   entry_widgetbehavior = {
w_getrectfn:  entry_getrect,
w_displacefn: entry_displace,
w_selectfn:   entry_select,
w_activatefn: entry_activate,
w_deletefn:   entry_delete,
w_visfn:      entry_vis,
w_clickfn:    NULL,
}; 

/* widget helper functions */

static int calculate_onset(t_entry *x, t_glist *glist, int i, int nplus)
{
    return(text_xpix(&x->x_obj, glist) + (x->x_rect_width - IOWIDTH) * i / nplus);
}


static void draw_inlets(t_entry *x, t_glist *glist, int firsttime, int nin, int nout)
{
    DEBUG(post("draw_inlets in: %d  out: %d", nin, nout););

    int nplus, i, onset;

    nplus = (nin == 1 ? 1 : nin-1);
    /* inlets */
    for (i = 0; i < nin; i++)
    {
        onset = calculate_onset(x,glist,i,nplus);
        if (firsttime)
        {
            DEBUG(post("%s create rectangle %d %d %d %d -tags {%xi%d %xi}\n",
                       x->x_canvas_name,
                       onset, text_ypix(&x->x_obj, glist) - 2,
                       onset + IOWIDTH, text_ypix(&x->x_obj, glist) - 1,
                       x, i, x););
            sys_vgui("%s create rectangle %d %d %d %d -tags {%xi%d %xi}\n",
                     x->x_canvas_name,
                     onset, text_ypix(&x->x_obj, glist) - 2,
                     onset + IOWIDTH, text_ypix(&x->x_obj, glist) - 1,
                     x, i, x);
        }
        else
        {
            DEBUG(post("%s coords %xi%d %d %d %d %d\n",
                       x->x_canvas_name, x, i,
                       onset, text_ypix(&x->x_obj, glist) - 2,
                       onset + IOWIDTH, text_ypix(&x->x_obj, glist) - 1););
            sys_vgui("%s coords %xi%d %d %d %d %d\n",
                     x->x_canvas_name, x, i,
                     onset, text_ypix(&x->x_obj, glist) - 2,
                     onset + IOWIDTH, text_ypix(&x->x_obj, glist)- 1);
        }
    }
    nplus = (nout == 1 ? 1 : nout-1);
    for (i = 0; i < nout; i++) /* outlets */
    {
        onset = calculate_onset(x,glist,i,nplus);
        if (firsttime)
        {
            DEBUG(post("%s create rectangle %d %d %d %d -tags {%xo%d %xo}\n",
                       x->x_canvas_name,
                       onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 2,
                       onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_rect_height-1,
                       x, i, x););
            sys_vgui("%s create rectangle %d %d %d %d -tags {%xo%d %xo}\n",
                     x->x_canvas_name,
                     onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 2,
                     onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_rect_height-1,
                     x, i, x);
        }
        else
        {
            DEBUG(post("%s coords %xo%d %d %d %d %d\n",
                       x->x_canvas_name, x, i,
                       onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 2,
                       onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_rect_height-1););
            sys_vgui("%s coords %xo%d %d %d %d %d\n",
                     x->x_canvas_name, x, i,
                     onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 2,
                     onset + IOWIDTH, text_ypix(&x->x_obj, glist) + x->x_rect_height-1);
        }
    }
    DEBUG(post("draw inlet end"););
}

/* currently unused
   static void draw_handle(t_entry *x, t_glist *glist, int firsttime) {
   int onset = text_xpix(&x->x_obj, glist) + (x->x_rect_width - IOWIDTH+2);

   if (firsttime)
   sys_vgui("%s create rectangle %d %d %d %d -tags %xhandle\n",
   x->x_canvas_name,
   onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 12,
   onset + IOWIDTH-2, text_ypix(&x->x_obj, glist) + x->x_rect_height-4,
   x);
   else
   sys_vgui("%s coords %xhandle %d %d %d %d\n",
   x->x_canvas_name, x, 
   onset, text_ypix(&x->x_obj, glist) + x->x_rect_height - 12,
   onset + IOWIDTH-2, text_ypix(&x->x_obj, glist) + x->x_rect_height-4);
   }
*/

static void create_widget(t_entry *x, t_glist *glist)
{
    DEBUG(post("create_widget"););
    /* I guess this is for fine-tuning of the rect size based on width and height? */
    x->x_rect_width = x->x_width;
    x->x_rect_height =  x->x_height+2;
	
    DEBUG(post("namespace eval entry%lx {} \n", x););
    sys_vgui("namespace eval entry%lx {} \n", x);

    /* Seems we have to delete the widget in case it already exists (Provided by Guenter)*/
    DEBUG(post("destroy %s\n", x->x_widget_name););
    sys_vgui("destroy %s\n", x->x_widget_name);

    DEBUG(post("text %s -font {helvetica 10} -border 1 -highlightthickness 1 -relief sunken -bg \"%s\" -fg \"%s\" \n",
               x->x_widget_name,x->x_bgcolour->s_name,x->x_fgcolour->s_name););
    sys_vgui("text %s -font {helvetica 10} -border 1 -highlightthickness 1 -relief sunken -bg \"%s\" -fg \"%s\" \n",
             x->x_widget_name,x->x_bgcolour->s_name,x->x_fgcolour->s_name);
    DEBUG(post("bind %s <KeyRelease> {+pd %s keyup %%N \\;} \n", 
               x->x_widget_name, x->x_receive_name->s_name););
    sys_vgui("bind %s <KeyRelease> {+pd %s keyup %%N \\;} \n", 
             x->x_widget_name, x->x_receive_name->s_name);
    DEBUG(post("bind %s <Leave> {focus [winfo parent %s]} \n", 
               x->x_widget_name, x->x_widget_name);); 
    sys_vgui("bind %s <Leave> {focus [winfo parent %s]} \n", 
             x->x_widget_name, x->x_widget_name); 
}

static void entry_drawme(t_entry *x, t_glist *glist, int firsttime)
{
    DEBUG(post("entry_drawme"););
    t_canvas *canvas=glist_getcanvas(glist);
    DEBUG(post("drawme %d",firsttime););
    if (firsttime) 
    {
        create_widget(x,glist);	       
        DEBUG(post("%s create window %d %d -anchor nw -window %s -tags %xS -width %d -height %d \n", 
                   x->x_canvas_name,text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
                   x->x_widget_name,x, x->x_width, x->x_height););
        sys_vgui("%s create window %d %d -anchor nw -window %s -tags %xS -width %d -height %d \n",
                 x->x_canvas_name,text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist),
                 x->x_widget_name,x, x->x_width, x->x_height);
    }     
    else 
    {
        DEBUG(post("%s coords %xS %d %d\n", x->x_canvas_name, x,
                   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)););
        sys_vgui("%s coords %xS %d %d\n", x->x_canvas_name, x,
                 text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist));
    }
    draw_inlets(x, glist, firsttime, 1,2);
    //     draw_handle(x, glist, firsttime);
}


static void entry_erase(t_entry* x,t_glist* glist)
{
    DEBUG(post("entry_erase"););
    DEBUG(post("destroy %s\n",x->x_widget_name););
    sys_vgui("destroy %s\n",x->x_widget_name);

    DEBUG(post("%s delete %xS\n", x->x_canvas_name, x););
    sys_vgui("%s delete %xS\n", x->x_canvas_name, x);

    /* inlets and outlets */
     
/* Added tag for all inlets of one instance */
    DEBUG(post("%s delete %xi\n", x->x_canvas_name,x););
    sys_vgui("%s delete %xi\n", x->x_canvas_name,x); 
    DEBUG(post("%s delete %xo\n", x->x_canvas_name,x););
    sys_vgui("%s delete %xo\n", x->x_canvas_name,x); 
/* Added tag for all outlets of one instance */
    DEBUG(post("%s delete  %xhandle\n", x->x_canvas_name,x,0););
    sys_vgui("%s delete  %xhandle\n", x->x_canvas_name,x,0);
}
	


/* ------------------------ text widgetbehaviour----------------------------- */


static void entry_getrect(t_gobj *z, t_glist *owner,
                          int *xp1, int *yp1, int *xp2, int *yp2)
{
    int width, height;
    t_entry* s = (t_entry*)z;

    width = s->x_rect_width;
    height = s->x_rect_height;
    *xp1 = text_xpix(&s->x_obj, owner);
    *yp1 = text_ypix(&s->x_obj, owner) - 1;
    *xp2 = text_xpix(&s->x_obj, owner) + width;
    *yp2 = text_ypix(&s->x_obj, owner) + height;
}

static void entry_displace(t_gobj *z, t_glist *glist,
                           int dx, int dy)
{
    t_entry *x = (t_entry *)z;
    DEBUG(post("displace"););
    x->x_obj.te_xpix += dx;
    x->x_obj.te_ypix += dy;
    if (glist_isvisible(glist))
    {
        DEBUG(post("%s coords %xSEL %d %d %d %d\n", x->x_canvas_name, x,
                   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)-1,
                   text_xpix(&x->x_obj, glist) + x->x_rect_width, 
                   text_ypix(&x->x_obj, glist) + x->x_rect_height-2););
        sys_vgui("%s coords %xSEL %d %d %d %d\n", x->x_canvas_name, x,
                 text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)-1,
                 text_xpix(&x->x_obj, glist) + x->x_rect_width, 
                 text_ypix(&x->x_obj, glist) + x->x_rect_height-2);
      
        entry_drawme(x, glist, 0);
        canvas_fixlinesfor(glist_getcanvas(glist),(t_text*) x);
    }
    DEBUG(post("displace end");)
        }

static void entry_select(t_gobj *z, t_glist *glist, int state)
{
    DEBUG(post("entry_select"););
    t_entry *x = (t_entry *)z;
    if (state) {
        DEBUG(post("%s create rectangle %d %d %d %d -tags %xSEL -outline blue\n",
                   x->x_canvas_name,
                   text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)-1,
                   text_xpix(&x->x_obj, glist) + x->x_rect_width, text_ypix(&x->x_obj, glist) + x->x_rect_height-2,
                   x););
        sys_vgui("%s create rectangle %d %d %d %d -tags %xSEL -outline blue\n",
                 x->x_canvas_name,
                 text_xpix(&x->x_obj, glist), text_ypix(&x->x_obj, glist)-1,
                 text_xpix(&x->x_obj, glist) + x->x_rect_width, text_ypix(&x->x_obj, glist) + x->x_rect_height-2,
                 x);
    }
    else {
        DEBUG(post("%s delete %xSEL\n", x->x_canvas_name, x););
        sys_vgui("%s delete %xSEL\n", x->x_canvas_name, x);
    }
}

static void entry_activate(t_gobj *z, t_glist *glist, int state)
{
/* this is currently unused
   t_text *x = (t_text *)z;
   t_rtext *y = glist_findrtext(glist, x);
   if (z->g_pd != gatom_class) rtext_activate(y, state);
*/
}

static void entry_delete(t_gobj *z, t_glist *glist)
{
    t_text *x = (t_text *)z;
    canvas_deletelinesfor(glist_getcanvas(glist), x);
}

       
static void entry_vis(t_gobj *z, t_glist *glist, int vis)
{
    DEBUG(post("entry_vis"););
    t_entry* s = (t_entry*)z;
    t_rtext *y;
    DEBUG(post("vis: %d",vis););
    if (vis) {
        y = (t_rtext *) rtext_new(glist, (t_text *)z);
        entry_drawme(s, glist, 1);
    }
    else {
        y = glist_findrtext(glist, (t_text *)z);
        entry_erase(s,glist);
        rtext_free(y);
    }
}

static void entry_add(t_entry* x,  t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("entry_add"););
    int i;
    t_symbol *tmp = s; /* <-- this gets rid of the unused variable warning */

    for(i=0; i<argc ; i++)
    {
        tmp = atom_getsymbol(argv+i);
        DEBUG(post("lappend ::entry%lx::list %s \n", x, tmp->s_name ););
        sys_vgui("lappend ::entry%lx::list %s \n", x, tmp->s_name );
    }
    DEBUG(post("%s insert end $::entry%lx::list ; unset ::entry%lx::list \n", 
               x->x_widget_name, x, x ););
    sys_vgui("%s insert end $::entry%lx::list ; unset ::entry%lx::list \n", 
             x->x_widget_name, x, x );
}

/* Function to reset the contents of the entry box */
static void entry_set(t_entry* x,  t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("entry_set"););
    int i;

    DEBUG(post("%s delete 0.0 end \n", x->x_widget_name););
    sys_vgui("%s delete 0.0 end \n", x->x_widget_name);
    entry_add(x, s, argc, argv);
}

/* Clear the contents of the text widget */
static void entry_clear(t_entry* x)
{
    DEBUG(post("%s delete 0.0 end \n", x->x_widget_name););
    sys_vgui("%s delete 0.0 end \n", x->x_widget_name);
}

/* Output the symbol */
/* , t_symbol *s, int argc, t_atom *argv) */
static void entry_output(t_entry* x, t_symbol *s, int argc, t_atom *argv)
{
    outlet_list(x->x_data_outlet, s, argc, argv );
}

/* Pass the contents of the text widget onto the entry_output fuction above */
static void entry_bang_output(t_entry* x)
{
    /* With "," and ";" escaping thanks to JMZ */
    DEBUG(post("pd [concat %s output [string map {\",\" \"\\\\,\" \";\" \"\\\\;\"} [%s get 0.0 end]] \\;]\n", 
               x->x_receive_name->s_name, x->x_widget_name););
    sys_vgui("pd [concat %s output [string map {\",\" \"\\\\,\" \";\" \"\\\\;\"} [%s get 0.0 end]] \\;]\n", 
             x->x_receive_name->s_name, x->x_widget_name);

    DEBUG(post("bind %s <Leave> {focus [winfo parent %s]} \n", 
               x->x_widget_name, x->x_widget_name););
    sys_vgui("bind %s <Leave> {focus [winfo parent %s]} \n", 
             x->x_widget_name, x->x_widget_name);
}

static void entry_keyup(t_entry *x, t_float f)
{
/*     DEBUG(post("entry_keyup");); */
    int keycode = (int) f;
    char buf[10];
    t_symbol *output_symbol;

    if( (keycode > 32 ) && (keycode < 65288) )
    {
        snprintf(buf, 2, "%c", keycode);
        output_symbol = gensym(buf);
    } else
        switch(keycode)
        {
        case 32: /* space */
            output_symbol = space_symbol;
            break;
        case 65293: /* return */
            output_symbol = return_symbol;
            break;
        case 65288: /* backspace */
            output_symbol = backspace_symbol;
            break;
        case 65289: /* tab */
            output_symbol = tab_symbol;
            break;
        case 65307: /* escape */
            output_symbol = escape_symbol;
            break;
        case 65361: /* left */
            output_symbol = left_symbol;
            break;
        case 65363: /* right */
            output_symbol = right_symbol;
            break;
        case 65362: /* up */
            output_symbol = up_symbol;
            break;
        case 65364: /* down */
            output_symbol = down_symbol;
            break;
        default:
            snprintf(buf, 10, "key_%d", keycode);
            DEBUG(post("keyup: %d", keycode););
            output_symbol = gensym(buf);
        }
    outlet_symbol(x->x_status_outlet, output_symbol);
}

static void entry_save(t_gobj *z, t_binbuf *b)
{
    t_entry *x = (t_entry *)z;

    binbuf_addv(b, "ssiisiiss", gensym("#X"),gensym("obj"),
                x->x_obj.te_xpix, x->x_obj.te_ypix ,  
                gensym("entry"), x->x_width, x->x_height, x->x_bgcolour, x->x_fgcolour);
    binbuf_addv(b, ";");
//    post("objectclass: %s", ((t_pd *)x)->);
}

/* function to change colour of text background */
void entry_bgcolour(t_entry* x, t_symbol* bgcol)
{
	x->x_bgcolour = bgcol;
	DEBUG(post("%s configure -background \"%s\" \n", x->x_widget_name, x->x_bgcolour->s_name););
	sys_vgui("%s configure -background \"%s\" \n", x->x_widget_name, x->x_bgcolour->s_name);
}

/* function to change colour of text foreground */
void entry_fgcolour(t_entry* x, t_symbol* fgcol)
{
	x->x_fgcolour = fgcol;
	DEBUG(post("%s configure -foreground \"%s\" \n", x->x_widget_name, x->x_fgcolour->s_name););
	sys_vgui("%s configure -foreground \"%s\" \n", x->x_widget_name, x->x_fgcolour->s_name);
}

static void entry_size(t_entry *x, t_float width, t_float height)
{
    DEBUG(post("entry_size"););
    x->x_height = height;
    x->x_width = width;
}

static void entry_free(t_entry *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_receive_name);
}

static void *entry_new(t_symbol *s, int argc, t_atom *argv)
{
    DEBUG(post("entry_new"););
    t_entry *x = (t_entry *)pd_new(entry_class);
    char buf[MAXPDSTRING];

    x->x_glist = canvas_getcurrent();
    snprintf(buf, MAXPDSTRING, ".x%lx.c", (long unsigned int) glist_getcanvas(x->x_glist));
    x->x_canvas_name = getbytes(strlen(buf));
    strncpy(x->x_canvas_name, buf, strlen(buf));
    snprintf(buf, MAXPDSTRING, "%s.s%lx", x->x_canvas_name, (long unsigned int) x);
    x->x_widget_name = getbytes(strlen(buf));
    strncpy(x->x_widget_name, buf, strlen(buf));

    x->x_height = 1;
	
	if (argc < 4)
	{
		post("entry: You must enter at least 4 arguments. Default values used.");
		x->x_width = 124;
		x->x_height = 100;
		x->x_bgcolour = gensym("grey70");
		x->x_fgcolour = gensym("black");
		
	} else {
		/* Copy args into structure */
		x->x_width = atom_getint(argv);
		x->x_height = atom_getint(argv+1);
		x->x_bgcolour = atom_getsymbol(argv+2);
		x->x_fgcolour = atom_getsymbol(argv+3);
	}	

    x->x_data_outlet = outlet_new(&x->x_obj, &s_float);
    x->x_status_outlet = outlet_new(&x->x_obj, &s_symbol);

    snprintf(buf,MAXPDSTRING,"#entry%lx",(long unsigned int)x);
    x->x_receive_name = gensym(buf);
    pd_bind(&x->x_obj.ob_pd, x->x_receive_name);

    return (x);
}

void entry_setup(void) {
    entry_class = class_new(gensym("entry"), (t_newmethod)entry_new, 
                            (t_method)entry_free, sizeof(t_entry),0,A_GIMME,0);
				
	class_addbang(entry_class, (t_method)entry_bang_output);

    class_addmethod(entry_class, (t_method)entry_keyup,
                    gensym("keyup"),
                    A_DEFFLOAT,
                    0);

    class_addmethod(entry_class, (t_method)entry_size,
                    gensym("size"),
                    A_DEFFLOAT,
                    A_DEFFLOAT,
                    0);
	
	class_addmethod(entry_class, (t_method)entry_output,
                    gensym("output"),
                    A_GIMME,
                    0);
								  
	class_addmethod(entry_class, (t_method)entry_set,
                    gensym("set"),
                    A_GIMME,
                    0);
								  
	class_addmethod(entry_class, (t_method)entry_add,
                    gensym("add"),
                    A_GIMME,
                    0);
								  
	class_addmethod(entry_class, (t_method)entry_clear,
                    gensym("clear"),
                    0);
								  
	class_addmethod(entry_class, (t_method)entry_bgcolour,
                    gensym("bgcolour"),
                    A_DEFSYMBOL,
                    0);
								  
	class_addmethod(entry_class, (t_method)entry_fgcolour,
                    gensym("fgcolour"),
                    A_DEFSYMBOL,
                    0);
								  
    class_setwidget(entry_class,&entry_widgetbehavior);
    class_setsavefn(entry_class,&entry_save);

    backspace_symbol = gensym("backspace");
    return_symbol = gensym("return");
	space_symbol = gensym("space");
	tab_symbol = gensym("tab");
	escape_symbol = gensym("escape");
	left_symbol = gensym("left");
	right_symbol = gensym("right");
	up_symbol = gensym("up");
	down_symbol = gensym("down");
    
	post("Text v0.1 Ben Bogart.\nCVS: $Revision: 1.14 $ $Date: 2007-10-26 04:49:50 $");
}


