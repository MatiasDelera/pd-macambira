/******************************************************
 *
 * midisettings - get/set midi preferences from within Pd-patches
 * Copyright (C) 2010 IOhannes m zm�lnig
 *
 *   forum::f�r::uml�ute
 *
 *   institute of electronic music and acoustics (iem)
 *   university of music and dramatic arts, graz (kug)
 *
 *
 ******************************************************
 *
 * license: GNU General Public License v.3 or later
 *
 ******************************************************/
#include "m_pd.h"
#include "s_stuff.h"
#include <stdio.h>
#include <string.h>

#define MAXNDEV 20
#define DEVDESCSIZE 80

#ifndef MAXMIDIINDEV
# define MAXMIDIINDEV 4
#endif
#ifndef MAXMIDIOUTDEV
# define MAXMIDIOUTDEV 4
#endif

extern int sys_midiapi;
static t_class *midisettings_class;

static t_symbol*s_pdsym=NULL;

typedef struct _as_drivers {
  t_symbol*name;
  int      id;

  struct _as_drivers *next;
} t_as_drivers;


t_as_drivers*as_finddriver(t_as_drivers*drivers, const t_symbol*name) {
  while(drivers) {
    if(name==drivers->name)return drivers;
    drivers=drivers->next;
  }
  return NULL;
}

t_as_drivers*as_finddriverid(t_as_drivers*drivers, const int id) {
  while(drivers) {
    if(id==drivers->id)return drivers;
    drivers=drivers->next;
  }
  return NULL;
}

t_as_drivers*as_adddriver(t_as_drivers*drivers, t_symbol*name, int id, int overwrite) {
  t_as_drivers*driver=as_finddriver(drivers, name);

  if(driver) {
    if(overwrite) {
      driver->name=name;
      driver->id  =id;
    }
    return drivers;
  }

  driver=(t_as_drivers*)getbytes(sizeof(t_as_drivers));
  driver->name=name;
  driver->id=id;
  driver->next=drivers;

  return driver;
}

t_as_drivers*as_driverparse(t_as_drivers*drivers, const char*buf) {
  int start=-1;
  int stop =-1;

  unsigned int index=0;
  int depth=0;
  const char*s;
  char substring[MAXPDSTRING];

  for(index=0, s=buf; 0!=*s; s++, index++) {
    if('{'==*s) {
      start=index;
      depth++;
    }
    if('}'==*s) {
      depth--;
      stop=index;

      if(start>=0 && start<stop) {
        char drivername[MAXPDSTRING];
        int  driverid;
        int length=stop-start;
        if(length>=MAXPDSTRING)length=MAXPDSTRING-1;
        snprintf(substring, length, "%s", buf+start+1);
        
        if(2==sscanf(substring, "%s %d", drivername, &driverid)) {
          drivers=as_adddriver(drivers, gensym(drivername), driverid, 0);
        } else {
          post("unparseable: '%s'", substring);
        }
      }
      start=-1;
      stop=-1;
    }
  }

  return drivers;
}

static t_as_drivers*DRIVERS=NULL;

static t_symbol*as_getdrivername(const int id) {
  t_as_drivers*driver=as_finddriverid(DRIVERS, id);
  if(driver) {
    return driver->name;
  } else {
    return gensym("<unknown>");
  }
}

static int as_getdriverid(const t_symbol*id) {
  t_as_drivers*driver=as_finddriver(DRIVERS, id);
  if(driver) {
    return driver->id;
  }
  return -1; /* unknown */
}



typedef struct _as_params {
  int indev[MAXMIDIINDEV], outdev[MAXMIDIOUTDEV];
  int inchannels, outchannels;
} t_as_params;
static void as_params_print(t_as_params*parms) {
  int i=0;
  post("\n=================================<");

  for(i=0; i<MAXMIDIINDEV; i++) {
    post("indev[%d]: %d", i, parms->indev[i]);
  }
  for(i=0; i<MAXMIDIOUTDEV; i++) {
    post("outdev[%d]: %d", i, parms->outdev[i]);
  }

  post("alsamidi: %d %d", parms->inchannels, parms->outchannels);

  post(">=================================\n");

}
static void as_params_get(t_as_params*parms) {
  int i=0;
  memset(parms, 0, sizeof(t_as_params));

  sys_get_midi_params(&parms->inchannels, parms->indev,
                      &parms->outchannels, parms->outdev);
#if 0
  for(i=parms->naudioindev; i<MAXAUDIOINDEV; i++) {
    parms->audioindev[i]=0;
    parms->chindev[i]=0;
  }
  for(i=parms->naudiooutdev; i<MAXAUDIOOUTDEV; i++) {
    parms->audiooutdev[i]=0;
    parms->choutdev[i]=0;
  }
#endif

  as_params_print(parms);
}





typedef struct _midisettings
{
  t_object x_obj;
  t_outlet*x_info;

  t_as_params x_params;
} t_midisettings;

static void midisettings_params_init(t_midisettings*x) {
  as_params_get(&x->x_params);
}


static void midisettings_listdevices(t_midisettings *x)
{
  int i;

  char indevlist[MAXNDEV][DEVDESCSIZE], outdevlist[MAXNDEV][DEVDESCSIZE];
  int indevs = 0, outdevs = 0;

  t_atom atoms[3];
 
  sys_get_midi_devs((char*)indevlist, &indevs, 
                     (char*)outdevlist, &outdevs, 
                     MAXNDEV, DEVDESCSIZE);

  SETSYMBOL (atoms+0, gensym("driver"));
  SETSYMBOL (atoms+1, as_getdrivername(sys_midiapi));
  outlet_anything(x->x_info, gensym("device"), 2, atoms);

  SETSYMBOL(atoms+0, gensym("in"));

  SETSYMBOL(atoms+1, gensym("devices"));
  SETFLOAT (atoms+2, (t_float)indevs);
  outlet_anything(x->x_info, gensym("device"), 3, atoms);

  for(i=0; i<indevs; i++) {
    SETFLOAT (atoms+1, (t_float)i);
    SETSYMBOL(atoms+2, gensym(indevlist[i]));
    outlet_anything(x->x_info, gensym("device"), 3, atoms);
  }

  SETSYMBOL(atoms+0, gensym("out"));

  SETSYMBOL(atoms+1, gensym("devices"));
  SETFLOAT (atoms+2, (t_float)outdevs);
  outlet_anything(x->x_info, gensym("device"), 2, atoms);

  for(i=0; i<outdevs; i++) {
    SETFLOAT (atoms+1, (t_float)i);
    SETSYMBOL(atoms+2, gensym(outdevlist[i]));
    outlet_anything(x->x_info, gensym("device"), 3, atoms);
  }
}

static void midisettings_listparams(t_midisettings *x) {
  int i;
  t_atom atoms[4];

  midisettings_params_init(x);

  SETSYMBOL(atoms+0, gensym("in"));

  SETSYMBOL(atoms+1, gensym("devices"));
  SETFLOAT (atoms+2, (t_float)x->x_params.inchannels);
  outlet_anything(x->x_info, gensym("params"), 3, atoms);

  for(i=0; i<x->x_params.inchannels; i++) {
    SETFLOAT (atoms+1, (t_float)x->x_params.indev[i]);
    outlet_anything(x->x_info, gensym("params"), 2, atoms);
  }

  SETSYMBOL(atoms+0, gensym("out"));

  SETSYMBOL(atoms+1, gensym("devices"));
  SETFLOAT (atoms+2, (t_float)x->x_params.outchannels);
  outlet_anything(x->x_info, gensym("params"), 3, atoms);

  for(i=0; i<x->x_params.outchannels; i++) {
    SETFLOAT (atoms+1, (t_float)x->x_params.outdev[i]);
    outlet_anything(x->x_info, gensym("params"), 2, atoms);
  }
}

/*
 * TODO: provide a nicer interface than the "pd audio-dialog"
 */
static void midisettings_set(t_midisettings *x, 
                             t_symbol*s, 
                             int argc, t_atom*argv) {
/*
     "pd midi-dialog ..."
     #00: indev[0]
     #01: indev[1]
     #02: indev[2]
     #03: indev[3]
     #04: outdev[0]
     #05: outdev[1]
     #06: outdev[2]
     #07: outdev[3]
     #08: alsadevin
     #09: alsadevout
*/
/*
  PLAN:
    several messages that accumulate to a certain settings, and then "apply" them
*/

       
}




static void midisettings_params_apply(t_midisettings*x) {
/*
     "pd midi-dialog ..."
     #00: indev[0]
     #01: indev[1]
     #02: indev[2]
     #03: indev[3]
     #04: outdev[0]
     #05: outdev[1]
     #06: outdev[2]
     #07: outdev[3]
     #08: alsadevin
     #09: alsadevout
*/

  int alsamidi=0;

  t_atom argv [MAXMIDIINDEV+MAXMIDIOUTDEV+2];
  int    argc= MAXMIDIINDEV+MAXMIDIOUTDEV+2;

  int i=0;

  as_params_print(&x->x_params);


  for(i=0; i<MAXMIDIINDEV; i++) {
    SETFLOAT(argv+i+0*MAXMIDIINDEV, (t_float)(alsamidi?0:x->x_params.indev[i]));
  }
  for(i=0; i<MAXMIDIOUTDEV; i++) {
    SETFLOAT(argv+i+1*MAXMIDIINDEV, (t_float)(alsamidi?0:x->x_params.outdev[i]));
  }

  
  SETFLOAT(argv+2*MAXMIDIINDEV+2*MAXMIDIOUTDEV+0,(t_float)(alsamidi?x->x_params.inchannels:0));
  SETFLOAT(argv+2*MAXMIDIINDEV+2*MAXMIDIOUTDEV+1,(t_float)(alsamidi?x->x_params.outchannels:0));

  if (s_pdsym->s_thing) typedmess(s_pdsym->s_thing, 
				  gensym("midi-dialog"), 
				  argc,
				  argv);
}


/* find the beginning of the next parameter in the list */
typedef enum {
  PARAM_INPUT,
  PARAM_OUTPUT,
  PARAM_INVALID
} t_paramtype;
static t_paramtype midisettings_setparams_id(t_symbol*s) {
  if(gensym("@input")==s) {
    return PARAM_INPUT;
  } else if(gensym("@output")==s) {
    return PARAM_OUTPUT;
  }
  return PARAM_INVALID;
}

/* find the beginning of the next parameter in the list */
static int midisettings_setparams_next(int argc, t_atom*argv) {
  int i=0;
  for(i=0; i<argc; i++) {
    if(A_SYMBOL==argv[i].a_type) {
      t_symbol*s=atom_getsymbol(argv+i);
      if('@'==s->s_name[0])
	return i;
    }
  }
  return i;
}

/* [<device> <channels>]* ... */
static int midisettings_setparams_input(t_midisettings*x, int argc, t_atom*argv) {
  int length=midisettings_setparams_next(argc, argv);
  int i;
  int numpairs=length/2;

  if(length%2)return length;

  if(numpairs>MAXMIDIINDEV)
    numpairs=MAXMIDIINDEV;

  for(i=0; i<numpairs; i++) {
    int dev=0;
    int ch=0;

    if(A_FLOAT==argv[2*i+0].a_type) {
      dev=atom_getint(argv);
    } else if (A_SYMBOL==argv[2*i+0].a_type) {
      // LATER: get the device-id from the device-name
      continue;
    } else {
      continue;
    }
    ch=atom_getint(argv+2*i+1);

    x->x_params.indev[i]=dev;
  }

  return length;
}

static int midisettings_setparams_output(t_midisettings*x, int argc, t_atom*argv) {
  int length=midisettings_setparams_next(argc, argv);
  int i;
  int numpairs=length/2;

  if(length%2)return length;

  if(numpairs>MAXMIDIOUTDEV)
    numpairs=MAXMIDIOUTDEV;

  for(i=0; i<numpairs; i++) {
    int dev=0;
    int ch=0;

    if(A_FLOAT==argv[2*i+0].a_type) {
      dev=atom_getint(argv);
    } else if (A_SYMBOL==argv[2*i+0].a_type) {
      // LATER: get the device-id from the device-name
      continue;
    } else {
      continue;
    }
    ch=atom_getint(argv+2*i+1);

    x->x_params.outdev[i]=dev;
  }

  return length;
}

static void midisettings_setparams(t_midisettings *x, t_symbol*s, int argc, t_atom*argv) {
/*
 * PLAN:
 *   several messages that accumulate to a certain settings, and then "apply" them
 *
 * normal midi: list up to four devices (each direction)
 * alsa   midi: choose number of ports (each direction)
    
*/
  int apply=1;
  int advance=0;
  t_paramtype param=PARAM_INVALID;

  advance=midisettings_setparams_next(argc, argv);
  while((argc-=advance)>0) {
    argv+=advance;
    s=atom_getsymbol(argv);
    param=midisettings_setparams_id(s);

    argv++;
    argc--;

    switch(param) {
    case PARAM_INPUT:
      advance=midisettings_setparams_input(x, argc, argv);
      break;
    case PARAM_OUTPUT:
      advance=midisettings_setparams_output(x, argc, argv);
      break;
    default:
      pd_error(x, "unknown parameter"); postatom(1, argv);endpost();
      break;
    }

    argc-=advance;
    argv+=advance;
    advance=midisettings_setparams_next(argc, argv);
  }
  if(apply) {
    //    midisettings_params_apply(x);
    midisettings_params_init (x); /* re-initialize to what we got */
  }
}




/*
 */
static void midisettings_listdrivers(t_midisettings *x)
{

  t_as_drivers*driver=NULL;

  for(driver=DRIVERS; driver; driver=driver->next) {
    t_atom ap[1];
    SETSYMBOL(ap, driver->name);
    outlet_anything(x->x_info, gensym("driver"), 1, ap);    
  }
}

static void midisettings_bang(t_midisettings *x) {
  midisettings_listdrivers(x);
  midisettings_listdevices(x);
  midisettings_listparams(x);
}


static void midisettings_free(t_midisettings *x){

}


static void *midisettings_new(void)
{
  t_midisettings *x = (t_midisettings *)pd_new(midisettings_class);
  x->x_info=outlet_new(&x->x_obj, 0);

  char buf[MAXPDSTRING];
  sys_get_midi_apis(buf);

  midisettings_params_init (x); /* re-initialize to what we got */

  DRIVERS=as_driverparse(DRIVERS, buf);

  //  sys_get_midi_drivers(buf);
  //  DRIVERS=as_driverparse(DRIVERS, buf);


  return (x);
}

static void midisettings_testdevices(t_midisettings *x);

void midisettings_setup(void)
{
  s_pdsym=gensym("pd");

  post("midisettings: midi settings manager");
  post("          Copyright (C) 2010 IOhannes m zmoelnig");
  post("          for the IntegraLive project");
  post("          institute of electronic music and acoustics (iem)");
  post("          published under the GNU General Public License version 3 or later");
#ifdef MIDISETTINGS_VERSION
  startpost("          version:"MIDISETTINGS_VERSION);
#endif
  post("\tcompiled: "__DATE__"");

  midisettings_class = class_new(gensym("midisettings"), (t_newmethod)midisettings_new, (t_method)midisettings_free,
			     sizeof(t_midisettings), 0, 0);

  class_addbang(midisettings_class, (t_method)midisettings_bang);
  class_addmethod(midisettings_class, (t_method)midisettings_listdrivers, gensym("listdrivers"), A_NULL);
  class_addmethod(midisettings_class, (t_method)midisettings_listdevices, gensym("listdevices"), A_NULL);
  class_addmethod(midisettings_class, (t_method)midisettings_listparams, gensym("listparams"), A_NULL);



  class_addmethod(midisettings_class, (t_method)midisettings_setparams, gensym("params"), A_GIMME);

  class_addmethod(midisettings_class, (t_method)midisettings_testdevices, gensym("testdevices"), A_NULL);
}



static void midisettings_testdevices(t_midisettings *x)
{
  int i;

  char indevlist[MAXNDEV][DEVDESCSIZE], outdevlist[MAXNDEV][DEVDESCSIZE];
  int indevs = 0, outdevs = 0;
 
  sys_get_midi_devs((char*)indevlist, &indevs, (char*)outdevlist, &outdevs, MAXNDEV, DEVDESCSIZE);

  post("%d midi indevs", indevs);
  for(i=0; i<indevs; i++)
    post("\t#%02d: %s", i, indevlist[i]);

  post("%d midi outdevs", outdevs);
  for(i=0; i<outdevs; i++)
    post("\t#%02d: %s", i, outdevlist[i]);

  endpost();
  int nmidiindev, midiindev[MAXMIDIINDEV];
  int nmidioutdev, midioutdev[MAXMIDIOUTDEV];
  sys_get_midi_params(&nmidiindev, midiindev,
                       &nmidioutdev, midioutdev);
  
  post("%d midiindev (parms)", nmidiindev);
  for(i=0; i<nmidiindev; i++) {
    post("\t#%02d: %d %d", i, midiindev[i]);
  }
  post("%d midioutdev (parms)", nmidioutdev);
  for(i=0; i<nmidioutdev; i++) {
    post("\t#%02d: %d %d", i, midioutdev[i]);
  }
}