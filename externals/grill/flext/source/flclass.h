/* 

flext - C++ layer for Max/MSP and pd (pure data) externals

Copyright (c) 2001,2002 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

*/

/*! \file flclass.h
	\brief User accessible flext base classes
    
*/

#ifndef __FLCLASS_H
#define __FLCLASS_H

// include the header file declaring the base classes
#include "flbase.h"
#include "flsupport.h"

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#ifdef __BORLANDC__
#pragma warn -8008 // Condition is always false
#pragma warn -8057 // Parameter is never used
#pragma warn -8066 // Unreachable code
#endif


// === flext_base ==================================================

//class qmsg;
//class thr_entry;

/*! \brief Flext message only base object
*/


class flext_base: 
	public flext_obj
{
	FLEXT_HEADER_S(flext_base,flext_obj,Setup)
	
	friend class flext_obj;

	/*!	\defgroup FLEXT_CLASS Flext base class
		@{ 
	*/
public:

	/*!	\defgroup FLEXT_C_BASE Basic class functionality
		@{ 
	*/

// ---  compatibility mode ----------------------------------------

	/*! \brief Cross-platform compatibility flag.
		If set flext allows only operations valid for all platforms.
		Set to true by default!
	*/
	static bool compatibility;  

//!		@} FLEXT_C_BASE

// --- inheritable virtual methods --------------------------------

	/*!	\defgroup FLEXT_C_VIRTUAL Virtual base class functions

		@{ 
	*/

	//! called on "help" message: should post some text
	virtual void m_help();
	
	//! called on patcher load (not on mere object creation!)
	virtual void m_loadbang() {}

	//! quickhelp for inlets/outlets (gets called in Max/MSP only)
	virtual void m_assist(long /*msg*/,long /*arg*/,char * /*s*/) {}

	/*!	\brief Called for every incoming message.
		All method handling is done in there
		\return True if a handler was found and called
		\todo Once, there should be a const modifier for argv
	*/
	virtual bool m_methodmain(int inlet,const t_symbol *s,int argc,const t_atom *argv);

	/*! \brief Called for every unhandled message (by m_methodmain)
		\todo Once, there should be a const modifier for argv
	*/
	virtual bool m_method_(int inlet,const t_symbol *s,int argc,const t_atom *argv);

//!		@} FLEXT_C_VIRTUAL


// --- inlet/outlet stuff -----------------------------------------	

	/*!	\defgroup FLEXT_C_INOUT Flext in-/outlet functions
		\note These must be called in the class' constructor
		\note All (also default) inlets must be defined
		@{ 
	*/

	/*!	\defgroup FLEXT_C_IO_ADD Announce in-/outlet functions
		@{ 
	*/

	// argument m specifies multiple inlet/outlet count
	
//	void AddInDef() { AddInlet(xlet::tp_def,1); }

	/*! \brief Add inlet(s) for anythings
		\remark That's the one to choose for the left-most (first) inlet unless it's a signal inlet.
	*/
	void AddInAnything(int m = 1) { AddInlet(xlet::tp_any,m); } 
	/*! \brief Add inlet(s) for anythings (with description)
		\remark That's the one to choose for the left-most (first) inlet unless it's a signal inlet.
	*/
	void AddInAnything(const char *desc,int m = 1) { AddInlet(xlet::tp_any,m,desc); } 
	//! Add inlet(s) for floats
	void AddInFloat(int m = 1) { AddInlet(xlet::tp_float,m); }
	//! Add inlet(s) for floats (with description)
	void AddInFloat(const char *desc,int m = 1) { AddInlet(xlet::tp_float,m,desc); }
	//! Add inlet(s) for integers
	void AddInInt(int m = 1) { AddInlet(xlet::tp_int,m); }
	//! Add inlet(s) for integers (with description)
	void AddInInt(const char *desc,int m = 1) { AddInlet(xlet::tp_int,m,desc); }
	//! Add inlet(s) for symbols
	void AddInSymbol(int m = 1) { AddInlet(xlet::tp_sym,m); }
	//! Add inlet(s) for symbol (with description)
	void AddInSymbol(const char *desc,int m = 1) { AddInlet(xlet::tp_sym,m,desc); }
	//! Add inlet(s) for bang
	void AddInBang(int m = 1) { AddInlet(xlet::tp_sym,m); }
	//! Add inlet(s) for bangs (with description)
	void AddInBang(const char *desc,int m = 1) { AddInlet(xlet::tp_sym,m,desc); }
	//! Add inlet(s) for lists
	void AddInList(int m = 1) { AddInlet(xlet::tp_list,m); }  
	//! Add inlet(s) for lists (with description)
	void AddInList(const char *desc,int m = 1) { AddInlet(xlet::tp_list,m,desc); }  
	
	//! Add outlet(s) for anythings
	void AddOutAnything(int m = 1) { AddOutlet(xlet::tp_any,m); }
	//! Add outlet(s) for anythings (with description)
	void AddOutAnything(const char *desc,int m = 1) { AddOutlet(xlet::tp_any,m,desc); }
	//! Add outlet(s) for floats
	void AddOutFloat(int m = 1) { AddOutlet(xlet::tp_float,m); }
	//! Add outlet(s) for floats (with description)
	void AddOutFloat(const char *desc,int m = 1) { AddOutlet(xlet::tp_float,m,desc); }
	//! Add outlet(s) for integers
	void AddOutInt(int m = 1) { AddOutlet(xlet::tp_int,m); }
	//! Add outlet(s) for integers (with description)
	void AddOutInt(const char *desc,int m = 1) { AddOutlet(xlet::tp_int,m,desc); }
	//! Add outlet(s) for symbols
	void AddOutSymbol(int m = 1) { AddOutlet(xlet::tp_sym,m); }
	//! Add outlet(s) for symbols (with description)
	void AddOutSymbol(const char *desc,int m = 1) { AddOutlet(xlet::tp_sym,m,desc); }
	//! Add outlet(s) for bangs
	void AddOutBang(int m = 1) { AddOutlet(xlet::tp_sym,m); }
	//! Add outlet(s) for bangs (with description)
	void AddOutBang(const char *desc,int m = 1) { AddOutlet(xlet::tp_sym,m,desc); }
	//! Add outlet(s) for lists
	void AddOutList(int m = 1) { AddOutlet(xlet::tp_list,m); }
	//! Add outlet(s) for lists (with description)
	void AddOutList(const char *desc,int m = 1) { AddOutlet(xlet::tp_list,m,desc); }

	//! \deprecated inlets and outlets are now set up automatically
	bool SetupInOut() { return true; }

	//!	@} FLEXT_C_IO_ADD 

	/*!	\defgroup FLEXT_C_IO_MISC Miscellanous in-/outlet functionality
		@{ 
	*/

	//! Get number of inlets
	int CntIn() const { return incnt; }
	//! Get number of outlets
	int CntOut() const { return outcnt; }
	//! Get number of signal inlets
	int CntInSig() const { return insigs; }
	//! Get number of signal outlets
	int CntOutSig() const { return outsigs; }

	class outlet;

	//! Retrieve currently processed message tag (NULL if no message processing)
	const t_symbol *thisTag() const { return curtag; }

	//! Get pointer to outlet (not in the constructor!)
	outlet *GetOut(int ix) const { return (outlets && ix < outcnt)?outlets[ix]:NULL; }

	//! Get pointer to attribute outlet 
	outlet *GetOutAttr() const { return outattr; }

	//! @} FLEXT_C_IO_MISC

	/*!	\defgroup FLEXT_C_IO_OUT Output data to inlets/outlets
		@{ 
	*/

	// output messages 

	//! Output bang (to appointed outlet)
	void ToOutBang(outlet *o) const;
	//! Output bang (index n starts with 0)
	void ToOutBang(int n) const { outlet *o = GetOut(n); if(o) ToOutBang(o); }

	//! Output float (to appointed outlet)
	void ToOutFloat(outlet *o,float f) const; 
	//! Output float (index n starts with 0)
	void ToOutFloat(int n,float f) const { outlet *o = GetOut(n); if(o) ToOutFloat(o,f); }

	//! Output integer (to appointed outlet)
	void ToOutInt(outlet *o,int f) const; 
	//! Output integer (index n starts with 0)
	void ToOutInt(int n,int f) const { outlet *o = GetOut(n); if(o) ToOutInt(o,f); }
	
	//! Output symbol (to appointed outlet)
	void ToOutSymbol(outlet *o,const t_symbol *s) const; 
	//! Output symbol (index n starts with 0)
	void ToOutSymbol(int n,const t_symbol *s) const { outlet *o = GetOut(n); if(o) ToOutSymbol(o,s); }
	//! Output string aka symbol (to appointed outlet)
	void ToOutString(outlet *o,const char *s) const { ToOutSymbol(o,MakeSymbol(s)); }
	//! Output string aka symbol (index n starts with 0)
	void ToOutString(int n,const char *s) const { outlet *o = GetOut(n); if(o) ToOutString(o,s); }

	//! Output list (to appointed outlet)
	void ToOutList(outlet *o,int argc,const t_atom *argv) const; 
	//! Output list (index n starts with 0)
	void ToOutList(int n,int argc,const t_atom *argv) const { outlet *o = GetOut(n); if(o) ToOutList(o,argc,argv); }
	//! Output list (index n starts with 0)
	void ToOutList(int n,const AtomList &list) const { ToOutList(n,list.Count(),list.Atoms()); }
	
	//! Output anything (to appointed outlet)
	void ToOutAnything(outlet *o,const t_symbol *s,int argc,const t_atom *argv) const; 
	//! Output anything (index n starts with 0)
	void ToOutAnything(int n,const t_symbol *s,int argc,const t_atom *argv) const  { outlet *o = GetOut(n); if(o) ToOutAnything(o,const_cast<t_symbol *>(s),argc,argv); }
	//! Output anything (index n starts with 0)
	void ToOutAnything(int n,const AtomAnything &any) const { ToOutAnything(n,any.Header(),any.Count(),any.Atoms()); }
	
	//! @} FLEXT_C_IO_OUT

	/*!	\defgroup FLEXT_C_IO_QUEUE Low-priority output of data to inlets/outlets
		@{ 
	*/

	//! Output bang (to appointed outlet)
	void ToQueueBang(outlet *o) const; 
	//! Output bang (index n starts with 0)
	void ToQueueBang(int n) const { outlet *o = GetOut(n); if(o) ToQueueBang(o); }

	//! Output float (to appointed outlet)
	void ToQueueFloat(outlet *o,float f) const; 
	//! Output float (index n starts with 0)
	void ToQueueFloat(int n,float f) const { outlet *o = GetOut(n); if(o) ToQueueFloat(o,f); }

	//! Output integer (to appointed outlet)
	void ToQueueInt(outlet *o,int f) const; 
	//! Output integer (index n starts with 0)
	void ToQueueInt(int n,int f) const { outlet *o = GetOut(n); if(o) ToQueueInt(o,f); }

	//! Output symbol (to appointed outlet)
	void ToQueueSymbol(outlet *o,const t_symbol *s) const; 
	//! Output symbol (index n starts with 0)
	void ToQueueSymbol(int n,const t_symbol *s) const { outlet *o = GetOut(n); if(o) ToQueueSymbol(o,s); }
	//! Output string aka symbol (to appointed outlet)
	void ToQueueString(outlet *o,const char *s) const { ToQueueSymbol(o,MakeSymbol(s)); }
	//! Output string aka symbol (to appointed outlet)
	void ToQueueString(int n,const char *s) const { ToQueueSymbol(n,MakeSymbol(s)); }

	//! Output list (to appointed outlet)
	void ToQueueList(outlet *o,int argc,const t_atom *argv) const; 
	//! Output list (to appointed outlet)
	void ToQueueList(outlet *o,const AtomList &list) const { ToQueueList(o,list.Count(),list.Atoms()); }
	//! Output list (index n starts with 0)
	void ToQueueList(int n,int argc,const t_atom *argv) const { outlet *o = GetOut(n); if(o) ToQueueList(o,argc,argv); }
	//! Output list (index n starts with 0)
	void ToQueueList(int n,const AtomList &list) const  { ToQueueList(n,list.Count(),list.Atoms()); }

	//! Output anything (to appointed outlet)
	void ToQueueAnything(outlet *o,const t_symbol *s,int argc,const t_atom *argv) const; 
	//! Output anything (to appointed outlet)
	void ToQueueAnything(outlet *o,const AtomAnything &any) const  { ToQueueAnything(o,any.Header(),any.Count(),any.Atoms()); }
	//! Output anything (index n starts with 0)
	void ToQueueAnything(int n,const t_symbol *s,int argc,const t_atom *argv)  const { outlet *o = GetOut(n); if(o) ToQueueAnything(o,s,argc,argv); }
	//! Output anything (index n starts with 0)
	void ToQueueAnything(int n,const AtomAnything &any) const  { ToQueueAnything(n,any.Header(),any.Count(),any.Atoms()); }

//!		@} FLEXT_C_IO_QUEUE

//!	@} FLEXT_C_INOUT


// --- message handling -------------------------------------------

	enum metharg {
		a_null = 0,
		a_float,a_int,
		a_symbol,a_pointer,
		a_list,a_any,
		a_LIST,a_ANY
	};

	typedef bool (*methfun)(flext_base *c);

	/*!	\defgroup FLEXT_C_ADDMETHOD Method handling
		\internal
		@{ 
	*/

	void AddMethodDef(int inlet,const char *tag = NULL); // call virtual function for tag && inlet
	void AddMethod(int inlet,const char *tag,methfun fun,metharg tp,...); 

	void AddMethod(int inlet,bool (*m)(flext_base *,int,t_atom *)) { AddMethod(inlet,"list",(methfun)m,a_list,a_null); }
	void AddMethod(int inlet,bool (*m)(flext_base *,int,const t_atom *)) { AddMethod(inlet,"list",(methfun)m,a_list,a_null); }
	void AddMethod(int inlet,const char *tag,bool (*m)(flext_base *)) { AddMethod(inlet,tag,(methfun)m,a_null); }  // pure method
	void AddMethod(int inlet,bool (*m)(flext_base *,t_symbol *,int,t_atom *)) { AddMethod(inlet,"anything",(methfun)m,a_any,a_null); } // anything
	void AddMethod(int inlet,bool (*m)(flext_base *,const t_symbol *,int,const t_atom *)) { AddMethod(inlet,"anything",(methfun)m,a_any,a_null); } // anything
	void AddMethod(int inlet,bool (*m)(flext_base *,t_symbol *&)) { AddMethod(inlet,"symbol",(methfun)m,a_symbol,a_null); } // single symbol
	void AddMethod(int inlet,bool (*m)(flext_base *,const t_symbol *&)) { AddMethod(inlet,"symbol",(methfun)m,a_symbol,a_null); } // single symbol
	void AddMethod(int inlet,bool (*m)(flext_base *,float &)) { AddMethod(inlet,"float",(methfun)m,a_float,a_null); }  // single float
	void AddMethod(int inlet,bool (*m)(flext_base *,float &,float &)) { AddMethod(inlet,"list",(methfun)m,a_float,a_float,a_null); } // list of 2 floats
	void AddMethod(int inlet,bool (*m)(flext_base *,float &,float &,float &)) { AddMethod(inlet,"list",(methfun)m,a_float,a_float,a_float,a_null); } // list of 3 floats
#if FLEXT_SYS == FLEXT_SYS_PD
	void AddMethod(int inlet,bool (*m)(flext_base *,int &)) { AddMethod(inlet,"float",(methfun)m,a_int,a_null); }  // single float
#elif FLEXT_SYS == FLEXT_SYS_MAX
	void AddMethod(int inlet,bool (*m)(flext_base *,int &)) { AddMethod(inlet,"int",(methfun)m,a_int,a_null); }  // single float
#else
#error
#endif
	void AddMethod(int inlet,bool (*m)(flext_base *,int &,int &)) { AddMethod(inlet,"list",(methfun)m,a_int,a_int,a_null); } // list of 2 floats
	void AddMethod(int inlet,bool (*m)(flext_base *,int &,int &,int &)) { AddMethod(inlet,"list",(methfun)m,a_int,a_int,a_int,a_null); } // list of 3 floats
	void AddMethod(int inlet,const char *tag,bool (*m)(flext_base *,int,t_atom *)) { AddMethod(inlet,tag,(methfun)m,a_list,a_null); } // method+gimme
	void AddMethod(int inlet,const char *tag,bool (*m)(flext_base *,int,const t_atom *)) { AddMethod(inlet,tag,(methfun)m,a_list,a_null); } // method+gimme
	void AddMethod(int inlet,const char *tag,bool (*m)(flext_base *,t_symbol *,int,t_atom *)) { AddMethod(inlet,tag,(methfun)m,a_any,a_null); } // method+gimme 
	void AddMethod(int inlet,const char *tag,bool (*m)(flext_base *,const t_symbol *,int,const t_atom *)) { AddMethod(inlet,tag,(methfun)m,a_any,a_null); } // method+gimme 
	void AddMethod(int inlet,const char *tag,bool (*m)(flext_base *,t_symbol *&)) { AddMethod(inlet,tag,(methfun)m,a_symbol,a_null); } // method+symbol
	void AddMethod(int inlet,const char *tag,bool (*m)(flext_base *,const t_symbol *&)) { AddMethod(inlet,tag,(methfun)m,a_symbol,a_null); } // method+symbol
	void AddMethod(int inlet,const char *tag,bool (*m)(flext_base *,float &)) { AddMethod(inlet,tag,(methfun)m,a_float,a_null); }  // method+float
	void AddMethod(int inlet,const char *tag,bool (*m)(flext_base *,int &)) { AddMethod(inlet,tag,(methfun)m,a_int,a_null); } // method+int

	//! Set Max/MSP style of distributing list elements over (message) inlets
	void SetDist(bool d = true) { distmsgs = d; }

//!		@} FLEXT_C_ADDMETHOD

// --- bind/unbind ---------------------------------------

	/*!	\defgroup FLEXT_C_BIND Methods for binding a flext class to a symbol

		@{ 
	*/

#if FLEXT_SYS == FLEXT_SYS_PD
	//! Bind object to a symbol
	bool Bind(const t_symbol *s) { pd_bind(&thisHdr()->ob_pd,const_cast<t_symbol *>(s)); return true; }
	//! Unbind object from a symbol
	bool Unbind(const t_symbol *s) { pd_unbind(&thisHdr()->ob_pd,const_cast<t_symbol *>(s)); return true; }
#else
	//! Bind object to a symbol
	bool Bind(const t_symbol *s) { if(s->s_thing) return false; else { const_cast<t_symbol *>(s)->s_thing = (t_object *)thisHdr(); return true; } }
	//! Unbind object from a symbol
	bool Unbind(const t_symbol *s) { if(s->s_thing != (t_object *)thisHdr()) return false; else { const_cast<t_symbol *>(s)->s_thing = NULL; return true; } }
#endif
	//! Bind object to a symbol (as string)
	bool Bind(const char *c) { return Bind(MakeSymbol(c)); }  
	//! Unbind object from a symbol (as string)
	bool Unbind(const char *c) { return Unbind(MakeSymbol(c)); }  

/*
	// Low level

	//! Bind object to a symbol
	static void DoUnbind(t_symbol *s,flext_obj *o);
	//! Unbind object from a symbol
	static void DoBind(const t_symbol *s,flext_obj *o);
	//! Get bound object of a symbol
	static t_class **GetBound(const t_symbol *s) { return (t_class **)s->s_thing; }
*/

//!		@} FLEXT_C_BIND

// --- thread stuff -----------------------------------------------

#ifdef FLEXT_THREADS
	/*!	\defgroup FLEXT_C_THREAD Thread handling 

		@{ 
	*/

	//! Check if current thread should terminate
	bool ShouldExit() const;

#endif // FLEXT_THREADS

//!		@}  FLEXT_C_THREAD

// xxx internal stuff xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

protected:

// --- thread stuff -----------------------------------------------

#ifdef FLEXT_THREADS

	/*! \brief Thread parameters
		\internal
	*/
	class thr_params:
		public flext 
	{
	public:
		thr_params(flext_base *c,int n = 1);
		~thr_params();

		void set_any(const t_symbol *s,int argc,const t_atom *argv);
		void set_list(int argc,const t_atom *argv);

		flext_base *cl;
		union _data {
			bool _bool;
			float _float;
			int _int;
			t_symptr _t_symptr;
			struct { AtomAnything *args; } _any;
			struct { AtomList *args; } _list;
			struct { void *data; } _ext;
		} *var;
	};

	/*! \brief This represents an entry to the list of active method threads
		\internal
	*/
	class thr_entry 
	{
	public:
		thr_entry(flext_base *t,void *(*m)(thr_params *),thr_params *p,pthread_t id = pthread_self());

		//! \brief Check if this class represents the current thread
		bool Is(pthread_t id = pthread_self()) const { return pthread_equal(thrid,id) != 0; }

		bool active,shouldexit;
		pthread_t thrid;
		void *(*meth)(thr_params *);
		thr_params *params;
		flext_base *th;
		thr_entry *nxt;
	};

	/*! \brief Add current thread to list of active threads
		\return true on success
		\internal
	*/
	bool PushThread();

	/*! \brief Remove current thread from list of active threads
		\internal
	*/
	void PopThread();

public:
	/*! \brief Start a method thread
		\internal
	*/
	bool StartThread(void *(*meth)(thr_params *p),thr_params *p,char *methname);

#endif

protected:

	flext_base();
	virtual ~flext_base();

// inlets and outlets
		
	/*! \brief Set up inlets and outlets
		\ingroup FLEXT_C_INOUT 
		\return True on successful creation of all inlets and outlets
	*/
	virtual bool Init();

	//! \brief This represents either an inlet or outlet
	struct xlet {	
		enum type {
			tp_none = 0,
			tp_float,tp_int,tp_sym,tp_list,tp_any,
			tp_LIST,tp_ANY, // use AtomList and AtomAnything
			tp_sig
		};

		xlet(type t,const char *desc = NULL);
		~xlet();
		
		char *desc;
		type tp;
		xlet *nxt;
	};

	/*!	\defgroup FLEXT_C_ATTR Attribute handling methods
		@{ 
	*/

	void AddAttrib(const char *attr,bool (*get)(flext_base *,float &),bool (*set)(flext_base *,float &)) { AddAttrib(attr,a_float,(methfun)get,(methfun)set); }
	void AddAttrib(const char *attr,bool (*get)(flext_base *,int &),bool (*set)(flext_base *,int &)) { AddAttrib(attr,a_int,(methfun)get,(methfun)set); }
	void AddAttrib(const char *attr,bool (*get)(flext_base *,const t_symbol *&),bool (*set)(flext_base *,const t_symbol *&)) { AddAttrib(attr,a_symbol,(methfun)get,(methfun)set); }
	void AddAttrib(const char *attr,bool (*get)(flext_base *,t_symbol *&),bool (*set)(flext_base *,t_symbol *&)) { AddAttrib(attr,a_symbol,(methfun)get,(methfun)set); }
	void AddAttrib(const char *attr,bool (*get)(flext_base *,AtomList *&),bool (*set)(flext_base *,AtomList *&)) { AddAttrib(attr,a_LIST,(methfun)get,(methfun)set); }
	void AddAttrib(const char *attr,bool (*get)(flext_base *,AtomAnything *&),bool (*set)(flext_base *,AtomAnything *&)) { AddAttrib(attr,a_ANY,(methfun)get,(methfun)set); }

//!		@} FLEXT_C_ATTR

	/*!	\addtogroup FLEXT_C_INOUT 
		@{ 
	*/

	//! \brief get a code for a list of inlets or outlets
	unsigned long XletCode(xlet::type tp = xlet::tp_none,...); // end list with 0 (= tp_none) !!

	/*! \brief Add some inlets by a special code representing the types
		\remark use XletCode function to get code value
	*/
	void AddInlets(unsigned long code); 

	//! \brief Add one or more inlet(s)
	void AddInlet(xlet::type tp,int mult = 1,const char *desc = NULL) { AddXlet(tp,mult,desc,inlist); }

	/*! \brief Add some inlets by a special code representing the types
		\remark use XletCode function to get code value
	*/
	void AddOutlets(unsigned long code); 

	//! \brief Add one or more outlet(s)
	void AddOutlet(xlet::type tp,int mult = 1,const char *desc = NULL) { AddXlet(tp,mult,desc,outlist); }

	//! \brief Set the description of an indexed inlet
	void DescInlet(int ix,const char *desc) { DescXlet(ix,desc,inlist); }

	//! \brief Set the description of an indexed outlet
	void DescOutlet(int ix,const char *desc) { DescXlet(ix,desc,outlist); }

//!		@} FLEXT_C_INOUT

// method handling

	//! \brief This represents an item of the method list
	class methitem { 
	public:
		methitem(int inlet,const t_symbol *t);
		~methitem();

		void SetArgs(methfun fun,int argc,metharg *args);

		const t_symbol *tag;
		int inlet;
		int argc;
		metharg *args;
		methfun fun;
		
		methitem *nxt;
	};
	
	//! \brief This represents an item of the attribute list
	class attritem { 
	public:
		attritem(const t_symbol *tag,const t_symbol *gtag,metharg tp,methfun gfun,methfun sfun);
		~attritem();

		const t_symbol *tag,*gtag;
		metharg argtp;
		methfun	gfun,sfun;

		attritem *nxt;
	};

//!		@} FLEXT_CLASS

private:

	static void Setup(t_class *c);

	xlet *inlist,*outlist;
	const t_symbol *curtag;
	int incnt,outcnt,insigs,outsigs;
	outlet **outlets,*outattr;
	bool distmsgs;

	void AddXlet(xlet::type tp,int mult,const char *desc,xlet *&root);	
	void DescXlet(int ix,const char *desc,xlet *&root);	

	union t_any {
		float ft;
		int it;
		t_symbol *st;
#if FLEXT_SYS == FLEXT_SYS_PD
		t_gpointer *pt;
#endif
		void *vt;
	};

	typedef bool (*methfun_V)(flext_base *c,int argc,t_atom *argv);
	typedef bool (*methfun_A)(flext_base *c,const t_symbol *s,int argc,t_atom *argv);
	typedef bool (*methfun_0)(flext_base *c);
	typedef bool (*methfun_1)(flext_base *c,t_any &);
	typedef bool (*methfun_2)(flext_base *c,t_any &,t_any &);
	typedef bool (*methfun_3)(flext_base *c,t_any &,t_any &,t_any &);
	typedef bool (*methfun_4)(flext_base *c,t_any &,t_any &,t_any &,t_any &);
	typedef bool (*methfun_5)(flext_base *c,t_any &,t_any &,t_any &,t_any &,t_any &);
	
	methitem *methhead;
	void AddMethItem(methitem *m);

	attritem *attrhead;
	int attrcnt;
	void AddAttrItem(attritem *m);

	void AddAttrib(const char *attr,metharg tp,methfun gfun,methfun sfun);

	static int CheckAttrib(int argc,const t_atom *argv);
	bool InitAttrib(int argc,const t_atom *argv);

	bool ListAttrib();
	bool GetAttrib(const t_symbol *s,int argc,const t_atom *argv);
	bool SetAttrib(const t_symbol *s,int argc,const t_atom *argv);

	static bool cb_ListAttrib(flext_base *c) { return c->ListAttrib(); }
	static bool cb_GetAttrib(flext_base *c,const t_symbol *s,int argc,const t_atom *argv) { return c->GetAttrib(s,argc,argv); }
	static bool cb_SetAttrib(flext_base *c,const t_symbol *s,int argc,const t_atom *argv) { return c->SetAttrib(s,argc,argv); }

#ifdef FLEXT_THREADS
	ThrMutex qmutex;

	static thr_entry *thrhead,*thrtail;
	static ThrMutex tlmutex;

	static pthread_t thrhelpid;
	static bool StartHelper();
	static bool StopHelper();
	static void ThrHelper(void *);

	void TermThreads();
#endif

	class qmsg;
	qmsg *qhead,*qtail;
	t_qelem *qclk;
#if FLEXT_SYS == FLEXT_SYS_MAX
	t_clock *yclk;
	static void YTick(flext_base *th);
#endif

	static void QTick(flext_base *th);
	void Queue(qmsg *m);

#if FLEXT_SYS == FLEXT_SYS_PD
	// proxy object (for additional inlets) stuff
	struct px_object;
	friend struct px_object;
#elif FLEXT_SYS == FLEXT_SYS_MAX
	typedef object px_object;
	static void cb_px_float(t_class *c,float f);
	static void cb_px_int(t_class *c,int v);
	static void cb_px_bang(t_class *c);

	static void cb_px_in1(t_class *c,int v);
	static void cb_px_in2(t_class *c,int v);
	static void cb_px_in3(t_class *c,int v);
	static void cb_px_in4(t_class *c,int v);
	static void cb_px_in5(t_class *c,int v);
	static void cb_px_in6(t_class *c,int v);
	static void cb_px_in7(t_class *c,int v);
	static void cb_px_in8(t_class *c,int v);
	static void cb_px_in9(t_class *c,int v);
#else
#error
#endif
	static void cb_px_anything(t_class *c,const t_symbol *s,int argc,t_atom *argv);

	static void cb_px_ft1(t_class *c,float f);
	static void cb_px_ft2(t_class *c,float f);
	static void cb_px_ft3(t_class *c,float f);
	static void cb_px_ft4(t_class *c,float f);
	static void cb_px_ft5(t_class *c,float f);
	static void cb_px_ft6(t_class *c,float f);
	static void cb_px_ft7(t_class *c,float f);
	static void cb_px_ft8(t_class *c,float f);
	static void cb_px_ft9(t_class *c,float f);

	px_object **inlets;

	// callback functions

	static void cb_help(t_class *c);

	static void cb_loadbang(t_class *c);
#if FLEXT_SYS == FLEXT_SYS_MAX
	static void cb_assist(t_class *c,void *b,long msg,long arg,char *s);
#endif
};

#endif
