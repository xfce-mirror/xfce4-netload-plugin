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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <locale.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <glib/gi18n.h>

#include "utils.h"

#ifndef BUFSIZ
#define BUFSIZ 512
#endif

#include "utils.h"
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
char* format_byte_humanreadable(char* string, int stringsize, double number, int digits, gboolean as_bits, gboolean ps)
{
    char* str = string;
    char buffer[BUFSIZ], formatstring[BUFSIZ];
    char* bufptr = buffer;
    char* unit_names[] = { N_("B"), N_("KiB"), N_("MiB"), N_("GiB"), N_("TiB") };
    char* unit_names_bits[] = { N_("b"), N_("Kb"), N_("Mb"), N_("Gb"), N_("Tb") };
    unsigned int uidx = 0;
    double number_displayed = 0;
    double thousand_divider = as_bits ? 1000 : 1024;
    unsigned int i;
    int numberOfIntegerChars, count;
    struct lconv* localeinfo = localeconv();
    int grouping = (int)localeinfo->grouping[0] == 0 ? INT_MAX : (int)localeinfo->grouping[0];
    
    /* Start with kilo and adapt to bits values*/
    uidx = 1;
    number_displayed = number / thousand_divider;
    if (as_bits)
    {
        number_displayed *= 8;
    }
    
    /* sensible value for digits */
    if (digits < 0 || digits >= 10)
    {
        digits = 2;
    }

    /* calculate number and appropriate unit size for display */
    while(number_displayed >= thousand_divider && uidx < (sizeof(unit_names) / sizeof(unit_names[0]) - 1))
    {
        number_displayed /= thousand_divider;
        uidx++;
    }

    /* format number first */
    snprintf(formatstring, BUFSIZ, "%%.%df", digits);
    snprintf(buffer, BUFSIZ, formatstring, number_displayed);
    
    /* get the number of integer characters */
    count = numberOfIntegerChars = ( digits > 0
                        ? (unsigned int)( strstr( buffer, localeinfo->decimal_point ) - buffer )
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
    g_strlcat(string, as_bits ? _(unit_names_bits[uidx]) : _(unit_names[uidx]), stringsize);

    /* Per second? */
    g_strlcat(string, ps ? N_("ps") : N_(""), stringsize);

    return string;
}
