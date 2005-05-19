/******************************************************
 *
 * zexy - implementation file
 *
 * copyleft (c) IOhannes m zm�lnig
 *
 *   1999:forum::f�r::uml�ute:2004
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/

/* 2305:forum::f�r::uml�ute:2001 */


#include "zexy.h"
#include <string.h>

/* ------------------------- lifop ------------------------------- */

/*
 * a LIFO (last-in first-out) with priorities
 *
 * an incoming list is added to a lifo (based on its priority)
 * "bang" outputs the last element of the non-empty lifo with the highest priority
 *
 * high priority means low numeric value
 */

static t_class *lifop_class;

typedef struct _lifop_list {
  int                 argc;
  t_atom             *argv;
  struct _lifop_list *next;
} t_lifop_list;

typedef struct _lifop_prioritylist {
  t_float                     priority;
  t_lifop_list               *lifo_start;
  struct _lifop_prioritylist *next;
} t_lifop_prioritylist;
typedef struct _lifop
{
  t_object              x_obj;
  t_lifop_prioritylist *lifo_list;
  t_float               priority; /* current priority */
} t_lifop;

static t_lifop_prioritylist*lifop_genprioritylist(t_lifop*x, t_float priority)
{
  t_lifop_prioritylist*result=0, *dummy=0;

  if(x->lifo_list!=0)
    {
      /*
       * do we already have this priority ?
       * if so, just return a pointer to that lifo
       * else set the dummy-pointer to the lifo BEFORE the new one
       */
      dummy=x->lifo_list;
      while(dummy!=0){
        t_float prio=dummy->priority;
        if(prio==priority)return dummy;
        if(prio>priority)break;
        result=dummy;
        dummy=dummy->next;
      }
      dummy=result;
    }
  /* create a new priority list */
  result = (t_lifop_prioritylist*)getbytes(sizeof( t_lifop_prioritylist));
  result->priority=priority;
  result->lifo_start=0;

  /* insert it into the list of priority lists */
  if(dummy!=0){
    result->next=dummy->next;
    dummy->next =result;
  } else {
    result->next=0;
  }
  if(x->lifo_list==0){
    x->lifo_list=result;
  }

  /* return the result */
  return result;
}

static int add2lifo(t_lifop_prioritylist*lifoprio, int argc, t_atom *argv)
{
  t_atom*buffer=0;
  t_lifop_list*lifo=0;
  t_lifop_list*entry=0;

  if(lifoprio==0){
    error("plifo: no lifos available");
    return -1;
  }

  /* create an entry for the lifo */
  if(!(entry = (t_lifop_list*)getbytes(sizeof(t_lifop_list))))
    {
      error("plifo: couldn't add entry to end of lifo");
      return -1;
    }
  if(!(entry->argv=(t_atom*)getbytes(argc*sizeof(t_atom)))){
    error("plifo: couldn't add list to lifo!");
    return -1;
  }
  memcpy(entry->argv, argv, argc*sizeof(t_atom));
  entry->argc=argc;
  entry->next=0;

  entry->next=lifoprio->lifo_start;
  lifoprio->lifo_start=entry;
}
static t_lifop_prioritylist*getLifo(t_lifop_prioritylist*plifo)
{
  if(plifo==0)return 0;
  /* get the highest non-empty lifo */
  while(plifo->lifo_start==0 && plifo->next!=0)plifo=plifo->next;
  return plifo;
}

static void lifop_list(t_lifop *x, t_symbol *s, int argc, t_atom *argv)
{
  t_lifop_prioritylist*plifo=0;
  if(!(plifo=lifop_genprioritylist(x, x->priority))) {
    error("[lifop]: couldn't get priority lifo");
    return;
  }
  add2lifo(plifo, argc, argv);
}
static void lifop_bang(t_lifop *x)
{
  t_lifop_prioritylist*plifo=0;
  t_lifop_list*lifo=0;
  t_atom*argv=0;
  int argc=0;

  if(!(plifo=getLifo(x->lifo_list))){
    return;
  }
  if(!(lifo=plifo->lifo_start)){
    return;
  }

  plifo->lifo_start=lifo->next;

  /* get the list from the entry */
  argc=lifo->argc;
  argv=lifo->argv;

  lifo->argc=0;
  lifo->argv=0;
  lifo->next=0;

  /* destroy the lifo-entry (important for recursion! */
  freebytes(lifo, sizeof(t_lifop_list));

  /* output the list */
  outlet_list(x->x_obj.ob_outlet, &s_list, argc, argv);

  /* free the list */
  freebytes(argv, argc*sizeof(t_atom));
}

static void lifop_free(t_lifop *x)
{
  t_lifop_prioritylist *lifo_list=x->lifo_list;
  while(lifo_list){
    t_lifop_prioritylist *lifo_list2=lifo_list;

    t_lifop_list*lifo=lifo_list2->lifo_start;
    lifo_list=lifo_list->next;

    while(lifo){
      t_lifop_list*lifo2=lifo;
      lifo=lifo->next;

      if(lifo2->argv)freebytes(lifo2->argv, lifo2->argc*sizeof(t_atom));
      lifo2->argv=0;
      lifo2->argc=0;
      lifo2->next=0;
      freebytes(lifo2, sizeof(t_lifop_list));
    }
    lifo_list2->priority  =0;
    lifo_list2->lifo_start=0;
    lifo_list2->next      =0;
    freebytes(lifo_list2, sizeof( t_lifop_prioritylist));
  }
  x->lifo_list=0;
}

static void *lifop_new(t_symbol *s, int argc, t_atom *argv)
{
  t_lifop *x = (t_lifop *)pd_new(lifop_class);

  outlet_new(&x->x_obj, 0);
  floatinlet_new(&x->x_obj, &x->priority);

  x->lifo_list = 0;
  x->priority=0;

  return (x);
}

void lifop_setup(void)
{
  lifop_class = class_new(gensym("lifop"), (t_newmethod)lifop_new,
                             (t_method)lifop_free, sizeof(t_lifop), 0, A_GIMME, 0);

  class_addbang    (lifop_class, lifop_bang);
  class_addlist    (lifop_class, lifop_list);

  class_sethelpsymbol(lifop_class, gensym("zexy/lifop"));
  zexy_register("lifop");
}

void z_lifop_setup(void)
{
  lifop_setup();
}
