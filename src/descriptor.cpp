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


#include "descriptor.h"
#include <sstream>
#include "matcher.h"
#include "util.h"
#include "settings.h"

using namespace std;

/* Constructor
 */
CFilterDescriptor::CFilterDescriptor() {
    filterType = TEXT;
    multipleMatches = false;
    windowWidth = 256;
    defaultFilter = 0;
}


/* Check if all data is valid
 */
bool CFilterDescriptor::isValid(string& errmsg) const {
    CSettings& settings = CSettings::ref();
    if (title.empty()) {
        errmsg = settings.getMessage("INVALID_FILTER_TITLE");
        return false;
    }
    if (filterType == TEXT) {
        if (matchPattern.empty()) {
            errmsg = settings.getMessage("INVALID_FILTER_MATCHEMPTY");
            return false;
        }
        if (windowWidth <= 0) {
            errmsg = settings.getMessage("INVALID_FILTER_WIDTH");
            return false;
        }
        if (!CMatcher::testPattern(boundsPattern, errmsg)) {
            return false;
        }
        if (!CMatcher::testPattern(urlPattern, errmsg)) {
            return false;
        }
        if (!CMatcher::testPattern(matchPattern, errmsg)) {
            return false;
        }
    } else {
        if (headerName.empty()) {
            errmsg = settings.getMessage("INVALID_FILTER_HEADEREMPTY");
            return false;
        }
        if (!CMatcher::testPattern(urlPattern, errmsg)) {
            return false;
        }
        if (!CMatcher::testPattern(matchPattern, errmsg)) {
            return false;
        }
    }
    return true;
}

bool CFilterDescriptor::isValid() const {
    string tmp;
    return isValid(tmp);
}


/* Clear all content
 */
void CFilterDescriptor::clear() {

    title = version = author = comment = category = headerName = "";
    boundsPattern = urlPattern = matchPattern = replacePattern = "";
    filterType = TEXT;
    multipleMatches = false;
    windowWidth = 256;
    defaultFilter = 0;
}


/* Export to a string.
 */
string CFilterDescriptor::exportFilter() const {

    stringstream out;
    
    out << "[" << title << "]" << endl;
    if (filterType == TEXT) {
        out << "Type=text" << endl;
        out << "Multiple=" << (multipleMatches?"yes":"no") << endl;
        out << "Width=" << windowWidth << endl;
    } else if (filterType == HEADOUT) {
        out << "Type=out" << endl;
    } else {
        out << "Type=in" << endl;
    }
    
    if (!category.empty())       out << "Category=" << category   << endl;
    if (!title.empty())          out << "Title="    << title      << endl;
    if (!version.empty())        out << "Version="  << version    << endl;
    if (!author.empty())         out << "Author="   << author     << endl;
    if (!headerName.empty())     out << "Header="   << headerName << endl;

    if (defaultFilter) out << "DefaultFilter=" << defaultFilter << endl;

    if (!comment.empty())
        out << "Comment=" << CUtil::replaceAll(comment,       "\n","\n_") << endl;
    if (!urlPattern.empty())
        out << "URL="     << CUtil::replaceAll(urlPattern,    "\n","\n_") << endl;
    if (!boundsPattern.empty())
        out << "Bounds="  << CUtil::replaceAll(boundsPattern, "\n","\n_") << endl;
    if (!matchPattern.empty())
        out << "Match="   << CUtil::replaceAll(matchPattern,  "\n","\n_") << endl;
    if (!replacePattern.empty())
        out << "Replace=" << CUtil::replaceAll(replacePattern,"\n","\n_") << endl;
    out << endl;
    return out.str();
}


/* Import filters from a string to a map. 
 * Returns the number of filters imported.
 */
int CFilterDescriptor::importFilters(const string& text,
                            map<string,CFilterDescriptor>& bank) {

    // Process text: lines will be terminated by \n (even from Mac text files)
    // and multiline values will have \r for inner newlines. The text will end
    // by a [] line so that we don't have to process anything after the loop.
    string str = text + "\n[]\n";
    str = CUtil::replaceAll(str, "\r\n", "\n");
    str = CUtil::replaceAll(str, "\r",   "\n");
    str = CUtil::replaceAll(str, "\n_",  "\r");
    
    CFilterDescriptor d;
    unsigned int i = 0, max = str.size(), count = 0;
    while (i < max) {
        unsigned int j = str.find("\n", i);
        string line = str.substr(i, j - i);
        unsigned int eq = line.find('=');
        if (!line.empty() && line[0] == '[') {
            if (d.isValid()) {
                while (bank.find(d.title) != bank.end())
                    CUtil::increment(d.title);
                bank[d.title] = d;
                count++;
            }
            d.clear();
        } else if (eq != string::npos) {
            string label = line.substr(0, eq);
            CUtil::trim(label);
            CUtil::upper(label);
            string value = line.substr(eq + 1);
            value = CUtil::replaceAll(value, "\r", "\n");
            if (label == "TYPE") {
                CUtil::trim(value);
                char c = toupper(value[0]);
                d.filterType = (c == 'T' ? TEXT : c == 'I' ? HEADIN : HEADOUT);
            } else if (label == "MULTIPLE") {
                CUtil::trim(value);
                d.multipleMatches = (toupper(value[0]) == 'Y');
            } else if (label == "WIDTH") {
                stringstream ss;
                ss << CUtil::trim(value);
                ss >> d.windowWidth;
            }
            else if (label == "DEFAULTFILTER" ) {
                stringstream ss;
                ss << CUtil::trim(value);
                ss >> d.defaultFilter;
            }
            else if (label == "TITLE"   ) CUtil::trim(d.title = value);
            else if (label == "VERSION" ) CUtil::trim(d.version = value);
            else if (label == "AUTHOR"  ) CUtil::trim(d.author = value);
            else if (label == "COMMENT" ) CUtil::trim(d.comment = value);
            else if (label == "CATEGORY") CUtil::trim(d.category = value);
            else if (label == "HEADER"  ) CUtil::trim(d.headerName = value);
            else if (label == "URL"     ) d.urlPattern = value;
            else if (label == "BOUNDS"  ) d.boundsPattern = value;
            else if (label == "MATCH"   ) d.matchPattern = value;
            else if (label == "REPLACE" ) d.replacePattern = value;
        }
        i = j + 1;
    }
    return count;
}


/* Export a map to a string
 */
string CFilterDescriptor::exportFilters(const map<string,CFilterDescriptor>& bank) {

    stringstream out;
    for (map<string,CFilterDescriptor>::const_iterator it = bank.begin();
                it != bank.end(); it++) {
        out << it->second.exportFilter();
    }
    return out.str();
}


/* Import Proxomitron filters from a string to a map.
 * Returns the number of filters imported.
 */
int CFilterDescriptor::importProxomitron(const string& text,
                            map<string,CFilterDescriptor>& bank) {

    // Process text: lines will be terminated by \n (even from Mac text files)
    // and multiline values will have \r for inner newlines. The text will end
    // by a [] line so that we don't have to process anything after the loop.
    string str = text + "\n\n";
    str = CUtil::replaceAll(str, "\r\n", "\n");
    str = CUtil::replaceAll(str, "\r",   "\n");
    str = CUtil::replaceAll(str, "\"\n        \"",  "\r");
    str = CUtil::replaceAll(str, "\"\n          \"",  "\r");

    CFilterDescriptor d;
    unsigned int i = 0, max = str.size(), count = 0;
    while (i < max) {
        unsigned int j = str.find("\n", i);
        string line = str.substr(i, j - i);
        unsigned int eq = line.find('=');
        if (line.empty()) {
            d.category = CSettings::ref().getMessage("PROXOMITRON_CATEGORY");
            if (d.isValid()) {
                while (bank.find(d.title) != bank.end())
                    CUtil::increment(d.title);
                bank[d.title] = d;
                count++;
            }
            d.clear();
        } else if (eq != string::npos) {
            string label = line.substr(0, eq);
            CUtil::trim(label);
            CUtil::upper(label);
            string value = line.substr(eq + 1);
            CUtil::trim(value);
            CUtil::trim(value, "\"");
            value = CUtil::replaceAll(value, "\r", "\n");

            if (label == "KEY") {
                unsigned int colon = value.find(':');
                if (colon != string::npos) {
                    d.headerName = value.substr(0, colon);
                    d.title = value.substr(colon + 1);
                    CUtil::trim(d.headerName);
                    CUtil::trim(d.title);
                    CUtil::lower(value);
                    unsigned int i = value.find('(');
                    d.filterType = ((i != string::npos &&
                        value.find("out", i) != string::npos) ? HEADOUT : HEADIN);
                }
            }
            else if (label == "NAME"   ) {
                d.title = CUtil::trim(value);
                d.filterType = TEXT;
            }
            else if (label == "MULTI") {
                d.multipleMatches = (value[0] == 'T');
            }
            else if (label == "LIMIT") {
                stringstream ss;
                ss << value;
                ss >> d.windowWidth;
            }
            else if (label == "URL"     ) d.urlPattern = value;
            else if (label == "BOUNDS"  ) d.boundsPattern = value;
            else if (label == "MATCH"   ) d.matchPattern = value;
            else if (label == "REPLACE" ) d.replacePattern = value;
        }
        i = j + 1;
    }
    return count;
}


/* Compare two filter descriptions
 */
bool CFilterDescriptor::operator ==(const CFilterDescriptor& d) const {
    return (
        title == d.title &&
        version == d.version &&
        author == d.author &&
        comment == d.comment &&
        category == d.category &&
        filterType == d.filterType &&
        headerName == d.headerName &&
        multipleMatches == d.multipleMatches &&
        windowWidth == d.windowWidth &&
        boundsPattern == d.boundsPattern &&
        urlPattern == d.urlPattern &&
        matchPattern == d.matchPattern &&
        replacePattern == d.replacePattern &&
        defaultFilter == d.defaultFilter );
}

