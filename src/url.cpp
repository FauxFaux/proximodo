//------------------------------------------------------------------
//
//this file is part of Proximodo
//Copyright (C) 2004 Antony BOUCHER ( kuruden@users.sourceforge.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//------------------------------------------------------------------
// Modifications: (date, author, description)
//
//------------------------------------------------------------------


#include "url.h"

/* Constructor with parsing
 */
CUrl::CUrl(const string& str) {
    parseUrl(str);
}


/* Parses the URL string and stores the URL and each part in
 * different strings
 */
void CUrl::parseUrl(const string& str) {

    unsigned int pos1 = str.find("://");
    if (pos1 == string::npos) pos1 = 0; else pos1 +=3;
    unsigned int pos2 = str.find_first_of("/?#", pos1);
    if (pos2 == string::npos) pos2 = str.length();
    unsigned int pos3 = str.find_first_of("?#", pos2);
    if (pos3 == string::npos) pos3 = str.length();
    unsigned int pos4 = str.find_first_of("#", pos3);
    if (pos4 == string::npos) pos4 = str.length();

    url       = str;
    protocol  = (pos1 ? str.substr(0, pos1-3) : string("http"));
    fromhost  = str.substr(pos1);
    afterhost = str.substr(pos2);
    host      = str.substr(pos1, pos2 - pos1);
    path      = str.substr(pos2, pos3 - pos2);
    query     = str.substr(pos3, pos4 - pos3);
    anchor    = str.substr(pos4);
    hostport  = host;
    if (hostport.find(":") == string::npos)
        hostport += ':' + protocol;
}

