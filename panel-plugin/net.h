/*  XFce 4 - Netload Plugin
 *    Copyright (c) 2003 Bernhard Walle <bernhard.walle@gmx.de>
 *  
 *  Id: $Id: net.h,v 1.3 2003/08/24 20:02:29 bwalle Exp $
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


#ifndef NET_H
#define NET_H

/**
 * Initializes the netload plugin. Used to set up inital values. This function must
 * be called after each change of the network interface.
 * @param   device      The network device, e.g. <code>ippp0</code> for ISDN on Linux.
 */
void init_netload(const char* device);

/**
 * Gets the current netload. You must call init_netload() once before you use this function!
 * @param in        Input load in byte/s.
 * @param out       Output load in byte/s.
 * @param tot       Total load in byte/s.
 */
void get_current_netload(unsigned long *in, unsigned long *out, unsigned long *tot);

/**
 * Should be called to do cleanup work.
 */
void close_netload();

#endif /* NET_H */
