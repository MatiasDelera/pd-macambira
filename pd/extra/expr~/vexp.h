/*
 * jMax
 * Copyright (C) 1994, 1995, 1998, 1999 by IRCAM-Centre Georges Pompidou, Paris, France.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * See file LICENSE for further informations on licensing terms.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 * Based on Max/ISPW by Miller Puckette.
 *
 * Authors: Maurizio De Cecco, Francois Dechelle, Enzo Maggi, Norbert Schnell.
 *
 */

/* "expr" was written by Shahrokh Yadegari c. 1989. -msp */
/* "expr~" and "fexpr~" conversion by Shahrokh Yadegari c. 1999,2000 */

#define MSP
#ifdef PD
#undef MSP
#endif

#ifdef PD
#include "m_pd.h"
#else /* MSP */
#include "ext.h"
#include "z_dsp.h"
#endif

#include "fts_to_pd.h"
/* This is put in fts_to_pd.h

#ifdef MSP
#define	t_atom	Atom
#define	t_symbol Symbol
#define	pd_new(x)	newobject(x);
#define t_outlet void
#endif
*/

/*
 * Currently the maximum number of variables (inlets) that are supported
 * is 10.
 */

#define	MAX_VARS	9
#define	MINODES		10 /* was 200 */

/* terminal defines */

/*
 * operations
 * (x<<16|y) x defines the level of precedence,
 * the lower the number the lower the precedence
 * separators are defines as operators just for convenience
 */

#define	OP_SEMI		((long)(1<<16|1))		/* ; */
#define	OP_COMMA	((long)(2<<16|2))		/* , */
#define	OP_LOR		((long)(3<<16|3))		/* || */
#define	OP_LAND		((long)(4<<16|4))		/* && */
#define	OP_OR		((long)(5<<16|5))		/* | */
#define	OP_XOR		((long)(6<<16|6))		/* ^ */
#define	OP_AND		((long)(7<<16|7))		/* & */
#define	OP_NE		((long)(8<<16|8))		/* != */
#define	OP_EQ		((long)(8<<16|9))		/* == */
#define	OP_GE		((long)(9<<16|10))		/* >= */
#define	OP_GT		((long)(9<<16|11))		/* > */
#define	OP_LE		((long)(9<<16|12))		/* <= */
#define	OP_LT		((long)(9<<16|13))		/* < */
#define	OP_SR		((long)(10<<16|14))		/* >> */
#define	OP_SL		((long)(10<<16|15))		/* << */
#define	OP_SUB		((long)(11<<16|16))		/* - */
#define	OP_ADD		((long)(11<<16|17))		/* + */
#define	OP_MOD		((long)(12<<16|18))		/* % */
#define	OP_DIV		((long)(12<<16|19))		/* / */
#define	OP_MUL		((long)(12<<16|20))		/* * */
#define	OP_UMINUS	((long)(13<<16|21))		/* - unary minus */
#define	OP_NEG		((long)(13<<16|22))		/* ~ one complement */
#define	OP_NOT		((long)(13<<16|23))		/* ! */
#define	OP_RB		((long)(14<<16|24))		/* ] */
#define	OP_LB		((long)(14<<16|25))		/* [ */
#define	OP_RP		((long)(14<<16|26))		/* ) */
#define	OP_LP		((long)(14<<16|27))		/* ( */
#define	HI_PRE		((long)(100<<16))	/* infinite precedence */
#define	PRE_MASK	((long)0xffff0000)	/* precedence level mask */

struct ex_ex;

#define	name_ok(c)	(((c)=='_') || ((c)>='a' && (c)<='z') || \
			((c)>='A' && (c)<='Z') || ((c) >= '0' && (c) <= '9'))
#define unary_op(x)	((x) == OP_NOT || (x) == OP_NEG || (x) == OP_UMINUS)

struct ex_ex {
	union {
		long v_int;
		float v_flt;
		t_float *v_vec;		/* this is an for allocated vector */
		long op;
		char *ptr;
	} ex_cont;		/* content */
#define	ex_int		ex_cont.v_int
#define	ex_flt		ex_cont.v_flt
#define	ex_vec		ex_cont.v_vec
#define	ex_op		ex_cont.op
#define	ex_ptr		ex_cont.ptr
	long ex_type;		/* type of the node */
};
#define	exNULL	((struct ex_ex *)0)

/* defines for ex_type */
#define	ET_INT		0x1		/* an int */
#define	ET_FLT		0x2		/* a float */
#define ET_OP		0x3		/* operator */
#define	ET_STR		0x4		/* string */
#define ET_TBL		0x5		/* a table, the content is a pointer */
#define	ET_FUNC		0x6		/* a function */
#define	ET_SYM		0x7		/* symbol ("string") */
#define	ET_VSYM		0x8		/* variable symbol ("$s?") */
				/* we treat parenthesis and brackets */
				/* special to keep a pointer to their */
				/* match in the content */
#define	ET_LP		0x9		/* left parenthesis */
#define	ET_LB		0x10		/* left bracket */
#define	ET_II		0x11		/* and integer inlet */
#define	ET_FI		0x12		/* float inlet */
#define	ET_SI		0x13		/* string inlet */
#define	ET_VI		0x14		/* signal inlet */
#define	ET_VEC		0x15		/* allocated signal vector */
				/* special types for fexpr~ */
#define	ET_VO		0x16		/* vector output for fexpr~ */
#define	ET_XI		0x17		/* vector input for fexpr~ */
#define	ET_XI0		0x18		/* shorthand for $x?[0] */

/* defines for ex_flags */
#define EF_TYPE_MASK	0x07	/* first three bits define the type of expr */
#define EF_EXPR		0x01	/* expr  - control in and out */
#define EF_EXPR_TILDE	0x02	/* expr~ signal and control in, signal out */
#define EF_FEXPR_TILDE	0x04	/* fexpr~ filter expression */

#define	EF_STOP		0x08	/* is it stopped used for expr~ and fexpr~ */

#define	IS_EXPR(x)	  ((((x)->exp_flags&EF_TYPE_MASK)|EF_EXPR) == EF_EXPR)
#define	IS_EXPR_TILDE(x)  \
                 ((((x)->exp_flags&EF_TYPE_MASK)|EF_EXPR_TILDE)==EF_EXPR_TILDE)
#define	IS_FEXPR_TILDE(x) \
               ((((x)->exp_flags&EF_TYPE_MASK)|EF_FEXPR_TILDE)==EF_FEXPR_TILDE)

#define	SET_EXPR(x)	(x)->exp_flags |= EF_EXPR; \
			(x)->exp_flags &= ~EF_EXPR_TILDE;  \
			(x)->exp_flags &= ~EF_FEXPR_TILDE; 

#define	SET_EXPR_TILDE(x)	(x)->exp_flags &= ~EF_EXPR; \
				(x)->exp_flags |= EF_EXPR_TILDE;  \
				(x)->exp_flags &= ~EF_FEXPR_TILDE; 

#define	SET_FEXPR_TILDE(x)	(x)->exp_flags &= ~EF_EXPR; \
				(x)->exp_flags &= ~EF_EXPR_TILDE;  \
				(x)->exp_flags |= EF_FEXPR_TILDE; 

/*
 * defines for expr_error
 */
#define	EE_DZ		0x01	/* divide by zero error */
#define	EE_BI_OUTPUT	0x02	/* Bad output index */
#define	EE_BI_INPUT	0x04	/* Bad input index */
#define	EE_NOTABLE	0x08	/* NO TABLE */

typedef struct expr {
#ifdef PD
	t_object exp_ob;
#else /* MSP */
	t_pxobject exp_ob;
#endif
	int	exp_flags;		/* are we expr~, fexpr~, or expr */
	int	exp_error;		/* reported errors */
	t_outlet *exp_outlet;
#ifdef PD
	struct _exprproxy *exp_proxy;
#else /* MAX */
	void *exp_proxy[MAX_VARS];
	long exp_proxy_id;
#endif
	struct ex_ex *exp_stack;
	struct ex_ex exp_var[MAX_VARS];
	struct ex_ex exp_res;		/* the evluation result */
	t_float *exp_p_var[MAX_VARS];
	t_float *exp_p_res;		/* the previous evaluation result */
	t_float *exp_tmpres;		/* temporty result for fexpr~ */
	int exp_vsize;			/* the size of the signal vector */
	int exp_nivec;			/* # of vector inlets */
	float exp_f;   		/* control value to be transformed to signal */
} t_expr;

typedef struct ex_funcs {
	char *f_name;					/* function name */
	void (*f_func)(t_expr *, long, struct ex_ex *, struct ex_ex *);
	  /* the real function performing the function (void, no return!!!) */
	long f_argc;				/* number of arguments */
} t_ex_func;

/* function prototypes for pd-related functions called withing vexp.h */

extern int max_ex_tab(struct expr *expr, t_symbol *s, struct ex_ex *arg, struct ex_ex *optr);
extern int ex_getsym(char *p, t_symbol **s);
extern const char *ex_symname(t_symbol *s);
void ex_mkvector(t_float *fp, t_float x, int size);
extern void ex_size(t_expr *expr, long int argc, struct ex_ex *argv,
							struct ex_ex *optr);
extern void ex_sum(t_expr *expr, long int argc, struct ex_ex *argv,									struct ex_ex *optr);
extern void ex_Sum(t_expr *expr, long int argc, struct ex_ex *argv,									struct ex_ex *optr);
extern void ex_avg(t_expr *expr, long int argc, struct ex_ex *argv,									struct ex_ex *optr);
extern void ex_Avg(t_expr *expr, long int argc, struct ex_ex *argv,									struct ex_ex *optr);
extern void ex_store(t_expr *expr, long int argc, struct ex_ex *argv,									struct ex_ex *optr);

#ifdef NT
#pragma warning (disable: 4305 4244)

#define abort ABORT
void ABORT(void);
#endif
