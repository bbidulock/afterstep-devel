/*
 * Copyright (C) 2000 Sasha Vasko <sasha at aftercode.net>
 * Copyright (c) 1999 Ethan Fischer <allanon@crystaltokyo.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define LOCAL_DEBUG
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include "astypes.h"
#include "output.h"
#include "parse.h"
#include "safemalloc.h"
#include "xwrap.h"
#include "xprop.h"
#include "audit.h"

/* X property access : */
Bool
intern_atom_list (AtomXref * list)
{
	Bool          res = False;
#ifndef X_DISPLAY_MISSING
	int           nitems = 0, i = 0;
	char        **names;
	Atom         *atoms;

	if (list)
	{
		for (i = 0; list[i].name != NULL; i++);
		nitems = i;
		if (nitems > 0)
		{
			names = (char **)safemalloc (sizeof (char *) * nitems);

			atoms = (Atom *) safemalloc (sizeof (Atom) * nitems);
			memset (atoms, 0x00, sizeof (Atom) * nitems);
			for (i = 0; i < nitems; i++)
				names[i] = list[i].name;

			res = (XInternAtoms (dpy, names, nitems, False, atoms) != 0);
			for (i = 0; i < nitems; i++)
			{
                LOCAL_DEBUG_OUT( "Atom \"%s\" interned as 0x%lX", list[i].name, atoms[i] );
                list[i].atom = atoms[i];
				*(list[i].variable) = atoms[i];
			}
			free (atoms);
			free (names);
		}
	}
#endif
	return res;
}

void
translate_atom_list (ASFlagType *trg, AtomXref * xref, unsigned long* list, long nitems)
{
	if (trg && list && xref && nitems > 0)
	{
		register int  i;
		register AtomXref *curr;

		for (i = 0; i < nitems && list[i] != None; i++)
			for (curr = xref; curr->atom != None; curr++)
				if (curr->atom == list[i])
				{
					set_flags (*trg, curr->flag);
					break;
				}
	}
}

void
print_list_hints( stream_func func, void* stream, ASFlagType flags, AtomXref *xref, const char *prompt )
{
    register int i ;
    ASFlagType  effective_flags = 0 ;
    if( !pre_print_check( &func, &stream, (void*)flags, NULL ) ) return ;

    for( i = 0 ; xref[i].name ; i++ )
        if( get_flags(flags, xref[i].flag) )
            set_flags( effective_flags, xref[i].flag);
    func( stream, "%s.flags = 0x%lX;\n", prompt, effective_flags );
    for( i = 0 ; xref[i].name ; i++ )
    {
        LOCAL_DEBUG_OUT("comparing flag 0x%lX, name \"%s\";", xref[i].flag, xref[i].name );
        if( get_flags(flags, xref[i].flag) )
            func( stream, "%s.atom[%d] = %s;\n", prompt, i, xref[i].name );
    }
}

void
encode_atom_list ( AtomXref * xref, unsigned long **list, long *nitems, ASFlagType flags)
{
	if ( list && xref && nitems )
	{
		register int  i, k = 0;

	    for( i = 0 ; xref[i].name ; i++ )
        {
            if( get_flags(flags, xref[i].flag) )
            {
                LOCAL_DEBUG_OUT( "flag %lX matches", xref[i].flag );
                k++;
            }
        }
		*list = NULL;
		*nitems = k ;
		if( k > 0 )
		{
            *list = safecalloc( k, sizeof(unsigned long));
			k = 0 ;
		    for( i = 0 ; xref[i].name ; i++ )
				if( get_flags(flags, xref[i].flag) )
				{
                    LOCAL_DEBUG_OUT( "flag %lX encoded as atom \"%s\"(0x%lX)", xref[i].flag, XGetAtomName(dpy,xref[i].atom), xref[i].atom );
                    (*list)[k] = xref[i].atom;
					k++;
				}
		}
        LOCAL_DEBUG_OUT( "list = %p, count = %ld", *list, *nitems );
	}
}



Bool
read_32bit_proplist (Window w, Atom property, long estimate, unsigned long ** list, long *nitems)
{
	Bool          res = False;

#ifndef X_DISPLAY_MISSING
	if (w != None && property != None && list && nitems)
	{
		Atom          actual_type;
		int           actual_format;
        ASFlagType bytes_after;
		unsigned long unitems = 0 ;

		if (estimate <= 0)
			estimate = 1;
		res =
			(XGetWindowProperty
			 (dpy, w, property, 0, estimate, False, AnyPropertyType,
			  &actual_type, &actual_format, &unitems, &bytes_after, (unsigned char **)list) == 0);
		/* checking property sanity */
		res = (res && unitems > 0 && actual_format == 32);

		if (bytes_after > 0 && res)
		{
			XFree (*list);
			res =
				(XGetWindowProperty
				 (dpy, w, property, 0, estimate + (bytes_after >> 2), False,
				  actual_type, &actual_type, &actual_format, &unitems, &bytes_after, (unsigned char **)list) == 0);
			res = (res && (unitems > 0));	   /* bad property */
		}

		if (!res)
		{
			if (*list )
				XFree (*list);
			*nitems = 0;
			*list = NULL;
		}else
			*nitems = unitems ;
	}
#endif
	return res;
}

Bool read_text_property (Window w, Atom property, XTextProperty ** trg)
{
	Bool          res = False;
#ifndef X_DISPLAY_MISSING
	if (w != None && property != None && trg)
	{
		if (*trg)
		{
			if ((*trg)->value)
				XFree ((*trg)->value);
		} else
			*trg = (XTextProperty *) safemalloc (sizeof (XTextProperty));

		if (XGetTextProperty (dpy, w, *trg, property) == 0)
		{
			free ((*trg));
			*trg = NULL;
		} else
			res = True;
	}
#endif
	return res;
}

Bool read_string_property (Window w, Atom property, char **trg)
{
	Bool          res = False;
#ifndef X_DISPLAY_MISSING
    if (w != None && property != None && trg)
	{
        int           actual_format;
        Atom          actual_type;
        unsigned long junk;

		if (*trg)
		{
            XFree (*trg);
            *trg = NULL ;
        }

        if (XGetWindowProperty(dpy, w, property, 0, ~0, False, AnyPropertyType, &actual_type,
             &actual_format, &junk, &junk, (unsigned char **)trg) == Success)
        {
            if (actual_type != XA_STRING || actual_format != 8)
            {
                XFree (*trg);
                *trg = NULL ;
            }else
                res = True;
        }
	}
#endif
	return res;
}

void
print_text_property( stream_func func, void* stream, XTextProperty *tprop, const char *prompt )
{
    if( pre_print_check( &func, &stream, tprop, NULL ) )
    {
        func( stream, "%s.value = \"%s\";\n", prompt, tprop->value );
        func( stream, "%s.format = %d;\n", prompt, tprop->format );
    }
}

void
free_text_property (XTextProperty ** trg)
{
    if( *trg )
    {
#ifndef X_DISPLAY_MISSING
        if( (*trg)->value ) XFree ((*trg)->value);
#else
        if( (*trg)->value ) free ((*trg)->value);
#endif
        free( *trg );
        *trg = NULL ;
    }
}

Bool read_32bit_property (Window w, Atom property, unsigned long* trg)
{
	Bool          res = False;

#ifndef X_DISPLAY_MISSING
	if (w != None && property != None && trg)
	{
		Atom          actual_type;
		int           actual_format;
        ASFlagType bytes_after;
        unsigned long *data = NULL;
		unsigned long nitems;

		res =
			(XGetWindowProperty
			 (dpy, w, property, 0, 1, False, AnyPropertyType, &actual_type,
			  &actual_format, &nitems, &bytes_after, (unsigned char **)&data) == 0);

		/* checking property sanity */
		res = (res && nitems > 0 && actual_format == 32);

		if (res)
			*trg = *data;
		if (data && nitems > 0)
			XFree (data);
	}
#endif
	return res;
}
/**** conversion routines taking in consideration different encoding of X properties ****/
char *
text_property2string( XTextProperty *tprop)
{
    char *text = NULL ;
    if( tprop )
    {
        if (tprop->value)
		{
#if defined(I18N) && !defined(X_DISPLAY_MISSING)
            if (tprop->encoding != XA_STRING)
			{
                char  **list;
                int     num, res ;

                tprop->nitems = strlen (tprop->value);
                res = XmbTextPropertyToTextList (dpy, &tprop, &list, &num);
                if ( res >= Success && num > 0 && *list)
                    text = stripcpy( *list );
                if ( res >= Success && *list)
                    XFreeStringList( list );
            }
#endif
            if( text == NULL && *(tprop->value) )
                text = stripcpy((const char*)(tprop->value));
        }
    }
    return text;
}

/* AfterStep specific property : */
unsigned long *
get_as_property ( Window w, Atom property, size_t * data_size, unsigned long *version)
{
    unsigned long *data = NULL;
#ifndef X_DISPLAY_MISSING
    unsigned long *header;
	int           actual_format;
	Atom          actual_type;
    unsigned long junk, size;

    if( w == None || property == None )
        return False;
	/* try to get the property version and data size */
    if (XGetWindowProperty (dpy, w, property, 0, 2, False, AnyPropertyType, &actual_type,
		 &actual_format, &junk, &junk, (unsigned char **)&header) != Success)
        return False;
	if (header == NULL)
        return False;

    if( version )
        *version   = (unsigned long)header[0];
    size = (unsigned long)header[1];
    if( data_size )
        *data_size = size;
    size /= sizeof(unsigned long);

	XFree (header);
	if (actual_type == XA_INTEGER)
	{
		/* try to get the actual information */
        if (XGetWindowProperty(dpy, w, property, 2, size, False,
                               AnyPropertyType, &actual_type, &actual_format, &size, &junk, (unsigned char **)&data) != Success)
			data = NULL;
	}
#endif
	return data ;
}

Bool
read_as_property ( Window w, Atom property, size_t * data_size, unsigned long *version, unsigned long **trg)
{
#ifndef X_DISPLAY_MISSING
    unsigned long  *data = get_as_property( w, property, data_size, version );
    int             size = (*data_size)/sizeof(unsigned long);

    if( data )
    {
        *trg = safemalloc( size*sizeof(unsigned long));
        while( --size >= 0 )
            (*trg)[size] = (unsigned long) (data[size]) ;
        XFree( data );
    }
    return True;
#else
	return False ;
#endif
}

/*************************************************************************/
/* Writing properties here :                                             */
/*************************************************************************/
void
set_32bit_property (Window w, Atom property, Atom type, unsigned long data)
{
    if (w != None && property != None )
	{
#ifndef X_DISPLAY_MISSING
        XChangeProperty (dpy, w, property, type?type:XA_CARDINAL, 32,
                         PropModeReplace, (unsigned char *)&data, 1);
#endif
    }
}

void
set_multi32bit_property (Window w, Atom property, Atom type, int items, ...)
{
#ifndef X_DISPLAY_MISSING
    if (w != None && property != None )
	{
        if( items > 0 )
        {
            unsigned long *data = safemalloc( items*sizeof(unsigned long));
            register int i = 0;
            va_list ap;

            va_start(ap,items);
            while( i < items )
                data[i++] = va_arg(ap,unsigned long);
            va_end(ap);

            XChangeProperty (dpy, w, property, type?type:XA_CARDINAL, 32,
                             PropModeReplace, (unsigned char *)&data, items);
			free(data);
        }else
        {
            XChangeProperty (dpy, w, property,
                             type?type:XA_CARDINAL, 32, PropModeReplace, NULL, 0);
        }
    }
#endif
}

void
set_32bit_proplist (Window w, Atom property, Atom type, unsigned long* list, long nitems)
{
#ifndef X_DISPLAY_MISSING
    if (w != None && property != None )
	{
        if( nitems > 0 )
        {
            XChangeProperty (dpy, w, property, type?type:XA_CARDINAL, 32,
                             PropModeReplace, (unsigned char *)list, nitems);
        }else
        {
            XChangeProperty (dpy, w, property,
                             type?type:XA_CARDINAL, 32, PropModeReplace, NULL, 0);
        }
    }
#endif
}

void
set_string_property (Window w, Atom property, char *data)
{
#ifndef X_DISPLAY_MISSING
    if (w != None && property != None && data)
	{
LOCAL_DEBUG_OUT( "setting property %lX on %lX to \"%s\"", property, w, data );
        XChangeProperty (dpy, w, property, XA_STRING, 8,
                         PropModeReplace, (unsigned char *)data, strlen (data));
    }
#endif
}

void
set_text_property (Window w, Atom property, char** data, int items_num, ASTextEncoding encoding )
{
#ifndef X_DISPLAY_MISSING
	if (w != None && property != None && data && items_num > 0)
	{
		XTextProperty prop ;
#ifdef I18N
        if (encoding != TPE_UTF8)
		{
            char  **list;
            int     num, res ;
/*			static Atom _XA_UTF8_STRING = XInternAtom(dpy, "UTF8_STRING", False) ; */
			if( XmbTextListToTextProperty(dpy, data, items_num, XUTF8String, &prop) == Success )
			{
				XSetTextProperty (dpy, w, &prop, property);
				XFree ((char *)prop.value);
				return;
			}
        }
#endif
		XStringListToTextProperty (data, items_num, &prop);
		XSetTextProperty (dpy, w, &prop, property);
		XFree ((char *)prop.value);
	}
#endif
}

/* AfterStep specific property : */
void
set_as_property ( Window w, Atom property, unsigned long *data, size_t data_size, unsigned long version)
{
#ifndef X_DISPLAY_MISSING
    unsigned long *buffer;

    buffer = safemalloc (2 * sizeof (unsigned long) + data_size);
	/* set the property version to 1.0 */
	buffer[0] = version;
	/* the size of meaningful data to store */
	buffer[1] = data_size;
	/* fill in the properties */
	memcpy (&(buffer[2]), data, data_size);

    XChangeProperty (dpy, w, property, XA_INTEGER, 32, PropModeReplace,
					 (unsigned char *)buffer, 2 + data_size / sizeof (unsigned long));
	free (buffer);
#endif
}

