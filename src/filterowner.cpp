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


#include "filterowner.h"
#include "util.h"

/* Evaluate the type of data by checking a few characteristic features.
 * Currently the tests are really minimum. A better way, if improvement
 * is needed, would be to count features from each type and choose the
 * most represented one.
 */
string CFilterOwner::evaluateType(string data) {

    CUtil::lower(data);

    if (data.find("<html")) return "htm";

    if (data.find("dim ") ||
        data.find("\n' ")    ) return "vbs";

    if (data.find("var ") ||
        data.find("\n/*")    ) return "js";

    if (data.find("a:link") ||
        data.find("font-size:") ) return "css";

    return "oth";
}

string CFilterOwner::evaluateType(const CUrl& url) {

    string path = url.getPath();
    unsigned int dot = path.rfind('.');
    if (dot == string::npos) return "oth";
    path = path.substr(dot + 1);
    CUtil::lower(path);
    if (path.find("htm") != string::npos ||
        path == "asp" || path == "php" || path == "jsp" ) return "htm";
    if (path == "vbs" || path == "css" || path == "js") return path;
    return "oth";
}
