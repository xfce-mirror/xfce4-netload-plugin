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
 */


#ifndef _NET_H_
#define _NET_H_
#include <gtk/gtk.h>

/**
 * Should be called to initialize.
 */
void init_netload();

/**
 * Gets the current netload. You must call init_netload() once before you use this function!
 * @param device    A device string like "ippp0" or "eth0".
 * @param in        Input load in byte/s.
 * @param out       Output load in byte/s.
 * @param tot       Total load in byte/s.
 */
void get_current_netload(const gchar* device, gint64 *in, gint64 *out, gint64 *tot);

/**
 * Should be called to do cleanup work.
 */
void close_netload();

#endif /* _NET_H_ */
