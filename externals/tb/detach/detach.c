/* 
* 
* detatch
* Copyright (C) 2005  Tim Blechmann
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; see the file COPYING.  If not, write to
* the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
*/

#include "m_pd.h"
#include "m_fifo.h"

#include "pthread.h"


static t_class *detatch_class;

typedef t_fifo fifo_t; /* for emacs syntax highlighting */

typedef struct _detatch
{
    t_object x_obj;

	t_outlet * x_outlet;

	pthread_t x_thread;
	pthread_mutex_t x_mutex;
    pthread_cond_t x_cond;

	fifo_t * x_fifo;

} detatch_t;

typedef struct _detatch_content
{
	enum { BANG, 
		   POINTER,
		   FLOAT,
		   SYMBOL,
		   LIST,
		   ANYTHING,
		   CANCEL} type;
	int argc;
	t_atom * argv;
} detatch_content_t;


static void detatch_thread(detatch_t* x)
{
	detatch_content_t * me;
	while(1)
	{
		pthread_cond_wait(&x->x_cond, &x->x_mutex);
		
		me = (detatch_content_t*) fifo_get(x->x_fifo);
		
		while (me != NULL)
		{
			/* run function */
			switch (me->type)
			{
			case BANG:
				outlet_bang(x->x_outlet);
				break;
			case FLOAT:
				outlet_float(x->x_outlet, atom_getfloat(me->argv));
				break;
			case SYMBOL:
				outlet_symbol(x->x_outlet, atom_getsymbol(me->argv));
				break;
			case LIST:
				outlet_list(x->x_outlet, 0, me->argc, me->argv);
				freebytes(me->argv, me->argc * sizeof(t_atom));
				break;
			case ANYTHING:
				outlet_anything(x->x_outlet, 0, me->argc, me->argv);
				freebytes(me->argv, me->argc * sizeof(t_atom));
				break;
			case CANCEL:
				goto done;
				
			}
			
			/* free */
			if (me->argc)
				freebytes(me->argv, me->argc * sizeof (t_atom));
			freebytes (me, sizeof(detatch_content_t));
			
			me = (detatch_content_t*) fifo_get(x->x_fifo);
		}
	}
	
 done: /* free the fifo and exit */

	do 
	{
		if (me->argc)
			freebytes(me->argv, me->argc * sizeof (t_atom));
		freebytes (me, sizeof(detatch_content_t));
	}
	while (me = (detatch_content_t*) fifo_get(x->x_fifo));

	fifo_destroy(x->x_fifo);
	return;
}

/* todo: take argument for thread priority */
static detatch_t * detatch_new(void)
{
	detatch_t *x = (detatch_t*) pd_new(detatch_class);
	pthread_attr_t thread_attr;
	struct sched_param thread_sched;
	int status;

	x->x_outlet = outlet_new(&x->x_obj, NULL);
	x->x_fifo = fifo_init();

	/* thread initialisation */
	pthread_mutex_init (&x->x_mutex,NULL);
	pthread_mutex_unlock (&x->x_mutex);
	pthread_cond_init (&x->x_cond,NULL);

	pthread_attr_init(&thread_attr);
#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
    thread_sched.sched_priority=sched_get_priority_min(SCHED_OTHER);
    pthread_attr_setschedparam(&thread_attr,&thread_sched);
#endif
	status = pthread_create(&x->x_thread, &thread_attr,
							(void*)detatch_thread, x);

#if 1
	if (status == 0)
		post("detatching thread");
#endif
	return x;
}


static void detatch_free(detatch_t * x)
{
	detatch_content_t * me = getbytes(sizeof(detatch_content_t));
	
	me->type = CANCEL;
	me->argc = 0;
	fifo_put(x->x_fifo, me);
	pthread_cond_broadcast(&x->x_cond);
}


static void detatch_bang(detatch_t * x)
{
	detatch_content_t * me = getbytes(sizeof(detatch_content_t));
	
	me->type = BANG;
	me->argc = 0;
	fifo_put(x->x_fifo, me);
	
	pthread_cond_broadcast(&x->x_cond);
}


static void detatch_float(detatch_t * x, t_float f)
{
	detatch_content_t * me = getbytes(sizeof(detatch_content_t));
	
	me->type = FLOAT;
	me->argc = 1;
	me->argv = getbytes(sizeof(t_atom));
	SETFLOAT(me->argv, f);
	fifo_put(x->x_fifo, me);
	
	pthread_cond_broadcast(&x->x_cond);
}

static void detatch_pointer(detatch_t * x, t_gpointer* gp)
{
	detatch_content_t * me = getbytes(sizeof(detatch_content_t));
	
	me->type = POINTER;
	me->argc = 1;
	me->argv = getbytes(sizeof(t_atom));
	SETPOINTER(me->argv, gp);
	fifo_put(x->x_fifo, me);
	
	pthread_cond_broadcast(&x->x_cond);
}

static void detatch_symbol(detatch_t * x, t_symbol * s)
{
	detatch_content_t * me = getbytes(sizeof(detatch_content_t));
	
	me->type = SYMBOL;
	me->argc = 1;
	me->argv = getbytes(sizeof(t_atom));
	SETSYMBOL(me->argv, s);
	fifo_put(x->x_fifo, me);
	
	pthread_cond_broadcast(&x->x_cond);
}


static void detatch_list(detatch_t * x, t_symbol * s, 
						 int argc, t_atom* argv)
{
	detatch_content_t * me = getbytes(sizeof(detatch_content_t));
	
	me->type = LIST;
	me->argc = argc;
	me->argv = copybytes(argv, argc * sizeof(t_atom));
	fifo_put(x->x_fifo, me);
	
	pthread_cond_broadcast(&x->x_cond);
}

static void detatch_anything(detatch_t * x, t_symbol * s, 
							 int argc, t_atom* argv)
{
	detatch_content_t * me = getbytes(sizeof(detatch_content_t));
	
	me->type = ANYTHING;
	me->argc = argc;
	me->argv = copybytes(argv, argc * sizeof(t_atom));
	fifo_put(x->x_fifo, me);
	
	pthread_cond_broadcast(&x->x_cond);
}


void detatch_setup(void)
{
	detatch_class = class_new(gensym("detatch"), (t_newmethod)detatch_new,
							  (t_method)detatch_free, sizeof(detatch_t),
							  CLASS_DEFAULT, 0);
	class_addbang(detatch_class, detatch_bang);
    class_addfloat(detatch_class, detatch_float);
    class_addpointer(detatch_class, detatch_pointer);
    class_addsymbol(detatch_class, detatch_symbol);
    class_addlist(detatch_class, detatch_list);
    class_addanything(detatch_class, detatch_anything);
}
