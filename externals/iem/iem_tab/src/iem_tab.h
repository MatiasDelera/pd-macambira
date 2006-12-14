/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_tab written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#ifndef __IEMTAB_H__
#define __IEMTAB_H__

typedef struct
{
    float real;
    float imag;
}
TAB_COMPLEX;

int iem_tab_check_arrays(t_symbol *obj_name, t_symbol *array_name, t_float **beg_mem, int *array_size, int max_index);

#endif
