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


#ifndef __filterowner__
#define __filterowner__

#include <string>
#include <vector>
#include <map>
#include "url.h"

using namespace std;

/* class CFilterOwner
 * This class only contains and implements what is common to
 * filter owners (such as CRequestManager and filter tester window).
 * URL and variables are such common members.
 */
class CFilterOwner {

public:
    CUrl   url;                         // URL of the document to filter

    string responseCode;                // response code from website

    int    reqNumber;                   // number of the request
    int    cnxNumber;                   // number of the connection

    bool   useSettingsProxy;            // can be overridden by $USEPROXY
    string contactHost;                 // can be overridden by $SETPROXY
    string rdirToHost;                  // set by $RDIR
    string jumpToHost;                  // set by $JUMP

    bool   bypassIn;                    // tells if we can filter incoming headers
    bool   bypassOut;                   // tells if we can filter outgoing headers
    bool   bypassBody;                  // will tell if we can filter the body or not
    bool   bypassBodyForced;            // set to true if $FILTER changed bypassBody

    map<string,string> variables;       // variables for $SET and $GET

    map<string,string> outHeaders;      // Outgoing headers
    map<string,string> inHeaders;       // Incoming headers
    
    string fileType;                    // useable by $TYPE

    // Clear Variables
    void clearVariables();

    // Try to find out the type of a piece of data (htm, css, js, vbs, oth)
    static string evaluateType(string data);
    static string evaluateType(const CUrl& url);
};

#endif
