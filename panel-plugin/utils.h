/*  XFce 4 - Netload Plugin
 *    Copyright (c) 2003 Bernhard Walle <bernhard.walle@gmx.de>
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * -------------------------------------------------------------------------------------------------
 */
#ifndef UTILS_H
#define UTILS_H

/**
 * Formats the number according the current locale with thousands separator. E.g.
 * 1234.5678 is formated in a German locale with digits=2 to 1.234,57. If the size
 * is too small, <code>NULL</code> is returned and the string contains garbage.
 * @param   string      a character array in which the result is stored
 * @param   stringsize  the size of the character array
 * @param   number      the number that should be formated
 * @param   digits      the number of digits after the decimal point
 * @return  the string to allow concatening buffers or <code>null</code>
 */
char* format_with_thousandssep( char* string, int stringsize, double number, int digits );

/**
 * Returns the minimum of the array. The array must contain at least one element.
 * @param   array       the array which must contain at least one element. 
 *                      <code>NULL</code> is not allowed
 * @param   size        the size of the array
 * @return  the minimum
 */
unsigned long min_array( unsigned long array[], int size );

/**
 * Returns the maximum of the array. The array must contain at least one element.
 * @param   array       the array which must contain at least one element. 
 *                      <code>NULL</code> is not allowed
 * @param   size        the size of the array
 * @return  the minimum
 */
unsigned long max_array( unsigned long array[], int size );

#endif /* UTILS_H */
