/* Copyright (c) 1999 Guenter Geiger and others.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*
 * This file implements the loader for linux, which includes
 * a little bit of path handling.
 *
 * Generalized by MSP to provide an open_via_path function
 * and lists of files for all purposes.
 */ 

/* #define DEBUG(x) x */
#define DEBUG(x)
void readsf_banana( void);    /* debugging */

#include <stdlib.h>
#ifdef UNISTD
#include <unistd.h>
#include <sys/stat.h>
#endif
#ifdef MSW
#include <io.h>
#endif

#include <string.h>
#include "m_pd.h"
#include "m_imp.h"
#include "s_stuff.h"
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>

t_namelist *sys_externlist;
t_namelist *sys_searchpath;
t_namelist *sys_helppath;

    /* change '/' characters to the system's native file separator */
void sys_bashfilename(const char *from, char *to)
{
    char c;
    while (c = *from++)
    {
#ifdef MSW
        if (c == '/') c = '\\';
#endif
        *to++ = c;
    }
    *to = 0;
}

    /* change the system's native file separator to '/' characters  */
void sys_unbashfilename(const char *from, char *to)
{
    char c;
    while (c = *from++)
    {
#ifdef MSW
        if (c == '\\') c = '/';
#endif
        *to++ = c;
    }
    *to = 0;
}

/*******************  Utility functions used below ******************/

/* copy until delimiter and return position after delimiter in string */
/* if it was the last substring, return NULL */

static const char* strtokcpy(char *to, const char *from, int delim)
{
    int size = 0;

    while (from[size] != (char)delim && from[size] != '\0')
        size++;

    strncpy(to,from,size);
    to[size] = '\0';
    if (from[size] == '\0') return NULL;
    if (size) return from+size+1;
    else return NULL;
}

/* add a single item to a namelist.  If "allowdup" is true, duplicates
may be added; othewise they're dropped.  */
  
t_namelist *namelist_append(t_namelist *listwas, const char *s, int allowdup)
{
    t_namelist *nl, *nl2;
    nl2 = (t_namelist *)(getbytes(sizeof(*nl)));
    nl2->nl_next = 0;
    nl2->nl_string = (char *)getbytes(strlen(s) + 1);
    strcpy(nl2->nl_string, s);
    sys_unbashfilename(nl2->nl_string, nl2->nl_string);
    if (!listwas)
        return (nl2);
    else
    {
        for (nl = listwas; ;)
        {
            if (!allowdup && !strcmp(nl->nl_string, s))
                return (listwas);
            if (!nl->nl_next)
                break;
            nl = nl->nl_next;
        }
        nl->nl_next = nl2;
    }
    return (listwas);
}

/* add a colon-separated list of names to a namelist */

#ifdef MSW
#define SEPARATOR ';'   /* in MSW the natural separator is semicolon instead */
#else
#define SEPARATOR ':'
#endif

t_namelist *namelist_append_files(t_namelist *listwas, const char *s)
{
    const char *npos;
    char temp[MAXPDSTRING];
    t_namelist *nl = listwas, *rtn = listwas;
    
    npos = s;
    do
    {
        npos = strtokcpy(temp, npos, SEPARATOR);
        if (! *temp) continue;
        nl = namelist_append(nl, temp, 0);
    }
        while (npos);
    return (nl);
}

void namelist_free(t_namelist *listwas)
{
    t_namelist *nl, *nl2;
    for (nl = listwas; nl; nl = nl2)
    {
        nl2 = nl->nl_next;
        t_freebytes(nl->nl_string, strlen(nl->nl_string) + 1);
        t_freebytes(nl, sizeof(*nl));
    }
}

char *namelist_get(t_namelist *namelist, int n)
{
    int i;
    t_namelist *nl;
    for (i = 0, nl = namelist; i < n && nl; i++, nl = nl->nl_next)
        ;
    return (nl ? nl->nl_string : 0);
}

static t_namelist *pd_extrapath;

int sys_usestdpath = 1;

void sys_setextrapath(const char *p)
{
    namelist_free(pd_extrapath);
    pd_extrapath = namelist_append(0, p, 0);
}

#ifdef MSW
#define MSWOPENFLAG(bin) (bin ? _O_BINARY : _O_TEXT)
#else
#define MSWOPENFLAG(bin) 0
#endif

/* search for a file in a specified directory, then along the globally
defined search path, using ext as filename extension.  Exception:
if the 'name' starts with a slash or a letter, colon, and slash in MSW,
there is no search and instead we just try to open the file literally.  The
fd is returned, the directory ends up in the "dirresult" which must be at
least "size" bytes.  "nameresult" is set to point to the filename, which
ends up in the same buffer as dirresult. */

int open_via_path(const char *dir, const char *name, const char* ext,
    char *dirresult, char **nameresult, unsigned int size, int bin)
{
    t_namelist *nl, thislist;
    int fd = -1;
    char listbuf[MAXPDSTRING];

    if (name[0] == '/' 
#ifdef MSW
        || (name[1] == ':' && name[2] == '/')
#endif
            )
    {
        thislist.nl_next = 0;
        thislist.nl_string = listbuf;
        listbuf[0] = 0;
    }
    else
    {
        thislist.nl_string = listbuf;
        thislist.nl_next = sys_searchpath;
        strncpy(listbuf, dir, MAXPDSTRING);
        listbuf[MAXPDSTRING-1] = 0;
        sys_unbashfilename(listbuf, listbuf);
    }

        /* search, first, through the search path (to which we prepended the
        current directory), then, if enabled, look in the "extra" dir */
    for (nl = &thislist; nl;
        nl = (nl->nl_next ? nl->nl_next :
            (nl == pd_extrapath ? 0 : 
                (sys_usestdpath ? pd_extrapath : 0))))
    {
        if (strlen(nl->nl_string) + strlen(name) + strlen(ext) + 4 >
            size)
                continue;
        strcpy(dirresult, nl->nl_string);
        if (*dirresult && dirresult[strlen(dirresult)-1] != '/')
               strcat(dirresult, "/");
        strcat(dirresult, name);
        strcat(dirresult, ext);
        sys_bashfilename(dirresult, dirresult);

        DEBUG(post("looking for %s",dirresult));
            /* see if we can open the file for reading */
        if ((fd=open(dirresult,O_RDONLY | MSWOPENFLAG(bin))) >= 0)
        {
                /* in unix, further check that it's not a directory */
#ifdef UNISTD
            struct stat statbuf;
            int ok =  ((fstat(fd, &statbuf) >= 0) &&
                !S_ISDIR(statbuf.st_mode));
            if (!ok)
            {
                if (sys_verbose) post("tried %s; stat failed or directory",
                    dirresult);
                close (fd);
                fd = -1;
            }
            else
#endif
            {
                char *slash;
                if (sys_verbose) post("tried %s and succeeded", dirresult);
                sys_unbashfilename(dirresult, dirresult);
                slash = strrchr(dirresult, '/');
                if (slash)
                {
                    *slash = 0;
                    *nameresult = slash + 1;
                }
                else *nameresult = dirresult;
                
                return (fd);  
            }
        }
        else
        {
            if (sys_verbose) post("tried %s and failed", dirresult);
        }
    }
    *dirresult = 0;
    *nameresult = dirresult;
    return (-1);
}

static int do_open_via_helppath(const char *realname, t_namelist *listp)
{
    t_namelist *nl;
    int fd = -1;
    char dirresult[MAXPDSTRING], realdir[MAXPDSTRING];
    for (nl = listp; nl; nl = nl->nl_next)
    {
        strcpy(dirresult, nl->nl_string);
        strcpy(realdir, dirresult);
        if (*dirresult && dirresult[strlen(dirresult)-1] != '/')
               strcat(dirresult, "/");
        strcat(dirresult, realname);
        sys_bashfilename(dirresult, dirresult);

        DEBUG(post("looking for %s",dirresult));
            /* see if we can open the file for reading */
        if ((fd=open(dirresult,O_RDONLY | MSWOPENFLAG(0))) >= 0)
        {
                /* in unix, further check that it's not a directory */
#ifdef UNISTD
            struct stat statbuf;
            int ok =  ((fstat(fd, &statbuf) >= 0) &&
                !S_ISDIR(statbuf.st_mode));
            if (!ok)
            {
                if (sys_verbose) post("tried %s; stat failed or directory",
                    dirresult);
                close (fd);
                fd = -1;
            }
            else
#endif
            {
                char *slash;
                if (sys_verbose) post("tried %s and succeeded", dirresult);
                sys_unbashfilename(dirresult, dirresult);
                close (fd);
                glob_evalfile(0, gensym((char*)realname), gensym(realdir));
                return (1);
            }
        }
        else
        {
            if (sys_verbose) post("tried %s and failed", dirresult);
        }
    }
    return (0);
}

    /* LATER make this use open_via_path above.  We expect the ".pd"
    suffix here, even though we have to tear it back off for one of the
    search attempts. */
void open_via_helppath(const char *name, const char *dir)
{
    t_namelist *nl, thislist, *listp;
    int fd = -1;
    char dirbuf2[MAXPDSTRING], realname[MAXPDSTRING];

        /* if directory is supplied, put it at head of search list. */
    if (*dir)
    {
        thislist.nl_string = dirbuf2;
        thislist.nl_next = sys_helppath;
        strncpy(dirbuf2, dir, MAXPDSTRING);
        dirbuf2[MAXPDSTRING-1] = 0;
        sys_unbashfilename(dirbuf2, dirbuf2);
        listp = &thislist;
    }
    else listp = sys_helppath;
        /* 1. "objectname-help.pd" */
    strncpy(realname, name, MAXPDSTRING-10);
    realname[MAXPDSTRING-10] = 0;
    if (strlen(realname) > 3 && !strcmp(realname+strlen(realname)-3, ".pd"))
        realname[strlen(realname)-3] = 0;
    strcat(realname, "-help.pd");
    if (do_open_via_helppath(realname, listp))
        return;
        /* 2. "help-objectname.pd" */
    strcpy(realname, "help-");
    strncat(realname, name, MAXPDSTRING-10);
    realname[MAXPDSTRING-1] = 0;
    if (do_open_via_helppath(realname, listp))
        return;
        /* 3. "objectname.pd" */
    if (do_open_via_helppath(name, listp))
        return;
    post("sorry, couldn't find help patch for \"%s\"", name);
    return;
}


/* Startup file reading for linux and MACOSX.  As of 0.38 this will be
deprecated in favor of the "settings" mechanism */

#ifndef MSW

#define STARTUPNAME ".pdrc"
#define NUMARGS 1000

int sys_argparse(int argc, char **argv);

int sys_rcfile(void)
{
    FILE* file;
    int i;
    int k;
    int rcargc;
    char* rcargv[NUMARGS];
    char* buffer;
    char  fname[MAXPDSTRING], buf[1000], *home = getenv("HOME");

    /* parse a startup file */
    
    *fname = '\0'; 

    strncat(fname, home? home : ".", MAXPDSTRING-10);
    strcat(fname, "/");

    strcat(fname, STARTUPNAME);

    if (!(file = fopen(fname, "r")))
        return 1;

    post("reading startup file: %s", fname);

    rcargv[0] = ".";    /* this no longer matters to sys_argparse() */

    for (i = 1; i < NUMARGS-1; i++)
    {
        if (fscanf(file, "%999s", buf) < 0)
            break;
        buf[1000] = 0;
        if (!(rcargv[i] = malloc(strlen(buf) + 1)))
            return (1);
        strcpy(rcargv[i], buf);
    }
    if (i >= NUMARGS-1)
        fprintf(stderr, "startup file too long; extra args dropped\n");
    rcargv[i] = 0;

    rcargc = i;

    /* parse the options */

    fclose(file);
    if (sys_verbose)
    {
        if (rcargv)
        {
            post("startup args from RC file:");
            for (i = 1; i < rcargc; i++)
                post("%s", rcargv[i]);
        }
        else post("no RC file arguments found");
    }
    if (sys_argparse(rcargc, rcargv))
    {
        post("error parsing RC arguments");
        return (1);
    }
    return (0);
}
#endif /* MSW */

void sys_doflags( void)
{
    int i, beginstring = 0, state = 0, len = strlen(sys_flags->s_name);
    int rcargc = 0;
    char *rcargv[MAXPDSTRING];

    if (len > MAXPDSTRING)
    {
        post("flags: %s: too long", sys_flags->s_name);
        return;
    }
    for (i = 0; i < len; i++)
    {
        int c = sys_flags->s_name[i];
        if (state == 0)
        {
            if (c && !isspace(c))
            {
                beginstring = i;
                state = 1;
            }
        }
        else
        {
            if (!c || isspace(c))
            {
                char *foo = malloc(i - beginstring + 1);
                if (!foo)
                    return;
                strncpy(foo, sys_flags->s_name + beginstring, i - beginstring);
                foo[i - beginstring] = 0;
                rcargv[rcargc] = foo;
                rcargc++;
                if (rcargc >= MAXPDSTRING)
                    break;
            }
        }
    }
    if (sys_argparse(rcargc, rcargv))
        post("error parsing RC arguments");
}

    /* start a search path dialog window */
void glob_start_path_dialog(t_pd *dummy)
{
    char buf[MAXPDSTRING];
    int i;
    t_namelist *nl;
    
    for (nl = sys_searchpath, i = 0; nl && i < 10; nl = nl->nl_next, i++)
        sys_vgui("pd_set pd_path%d \"%s\"\n", i, nl->nl_string);
    for (; i < 10; i++)
        sys_vgui("pd_set pd_path%d \"\"\n", i);

    sprintf(buf, "pdtk_path_dialog %%s %d %d\n", sys_usestdpath, sys_verbose);
    gfxstub_new(&glob_pdobject, glob_start_path_dialog, buf);
}

    /* new values from dialog window */
void glob_path_dialog(t_pd *dummy, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    namelist_free(sys_searchpath);
    sys_searchpath = 0;
    sys_usestdpath = atom_getintarg(0, argc, argv);
    sys_verbose = atom_getintarg(1, argc, argv);
    for (i = 0; i < argc; i++)
    {
        t_symbol *s = atom_getsymbolarg(i+2, argc, argv);
        if (*s->s_name)
            sys_searchpath = namelist_append_files(sys_searchpath, s->s_name);
    }
}

    /* start a startup dialog window */
void glob_start_startup_dialog(t_pd *dummy)
{
    char buf[MAXPDSTRING];
    int i;
    t_namelist *nl;
    
    for (nl = sys_externlist, i = 0; nl && i < 10; nl = nl->nl_next, i++)
        sys_vgui("pd_set pd_startup%d \"%s\"\n", i, nl->nl_string);
    for (; i < 10; i++)
        sys_vgui("pd_set pd_startup%d \"\"\n", i);

    sprintf(buf, "pdtk_startup_dialog %%s %d \"%s\"\n", sys_defeatrt,
        sys_flags->s_name);
    gfxstub_new(&glob_pdobject, glob_start_startup_dialog, buf);
}

    /* new values from dialog window */
void glob_startup_dialog(t_pd *dummy, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    namelist_free(sys_externlist);
    sys_externlist = 0;
    sys_defeatrt = atom_getintarg(0, argc, argv);
    sys_flags = atom_getsymbolarg(1, argc, argv);
    for (i = 0; i < argc; i++)
    {
        t_symbol *s = atom_getsymbolarg(i+2, argc, argv);
        if (*s->s_name)
            sys_externlist = namelist_append_files(sys_externlist, s->s_name);
    }
}


