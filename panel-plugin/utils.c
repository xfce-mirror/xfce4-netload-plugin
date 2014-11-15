/*
 * Copyright 2003, 2005 Bernhard Walle <bernhard@bwalle.de>
 * -------------------------------------------------------------------------------------------------
 * 
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 675 Mass
 * Ave, Cambridge, MA 02139, USA.
 *
 * ------------------------------------------------------------------------------------------------- 
 */
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <locale.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef BUFSIZ
#define BUFSIZ 512
#endif

/* ---------------------------------------------------------------------------------------------- */
unsigned long min_array( unsigned long array[], int size )
{
    int i;
    unsigned long min = array[0];
    
    for (i = 1; i < size; i++)
    {
        if (array[i] < min)
        {
            min = array[i];
        }
    }
    return min;
}


/* ---------------------------------------------------------------------------------------------- */
unsigned long max_array( unsigned long array[], int size )
{
    int i;
    unsigned long max = array[0];
    
    for (i = 1; i < size; i++)
    {
        if( array[i] > max )
        {
            max = array[i];
        }
    }
    
    return max;
}


/* ---------------------------------------------------------------------------------------------- */
char* format_byte_humanreadable(char* string, int stringsize, double number, int digits)
{
    char* str = string;
    char buffer[BUFSIZ], formatstring[BUFSIZ];
    char* bufptr = buffer;
    char* unit_names[] = { N_("B"), N_("KiB"), N_("MiB"), N_("GiB") };
    unsigned int uidx = 0;
    double number_displayed = number / 1024.0;
    unsigned int i;
    int numberOfIntegerChars, count;
    struct lconv* localeinfo = localeconv();
    int grouping = (int)localeinfo->grouping[0] == 0 ? INT_MAX : (int)localeinfo->grouping[0];
    
    
    /* sensible value for digits */
    if (digits < 0 || digits >= 10)
    {
        digits = 2;
    }
    
    /* no digits for values under MiB/s unit size */
    if (number < 1024.0 * 1024.0)
    {
        digits = 0;
    }

    /* calculate number and appropriate unit size for display */
    while(number_displayed >= 1024.0 && uidx < (sizeof(unit_names) / sizeof(char*) - 1))
    {
        number_displayed /= 1024.0;
        uidx++;
    }

    /* format number first */
    snprintf(formatstring, BUFSIZ, "%%.%df", digits);
    snprintf(buffer, BUFSIZ, formatstring, number_displayed);
    
    /* get the number of integer characters */
    count = numberOfIntegerChars = ( digits > 0
                        ? ( strstr( buffer, localeinfo->decimal_point ) - buffer )
                        :   strlen( buffer ) );
    
    
    /* check for length */
    if( numberOfIntegerChars / grouping + (int)strlen( buffer ) > stringsize )
    {
        return NULL;
    }
    
    
    /* insert the thousands separator */
    while (*bufptr != 0 && *bufptr != localeinfo->decimal_point[0])
    {
        if (count % grouping == 0 && count != numberOfIntegerChars)
        {
            for (i = 0; i < strlen( localeinfo->thousands_sep ); i++)
            {
                *str++ = localeinfo->thousands_sep[i];
            }
        }
        
        *str++ = *bufptr++;
        count--;
    }
    
    /* Copy the rest of the number */
    while (digits > 0 && *bufptr != 0)
    {
        *str++ = *bufptr++;
    }
    
    /* Add space */
    *str++ = ' ';

    /* terminate with 0 finally */
    *str = 0;
    
    /* Add the unit name */
    g_strlcat(string, _(unit_names[uidx]), stringsize);

    return string;
}
