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


#ifndef __descriptor__
#define __descriptor__

#include <string>
#include <map>

using namespace std;

/* class CFilterDescriptor
 * Describes a filter.
 * All filters will be stored in a repository (CSettings), managed 
 * via the GUI. No two filters can have the same title, as the _title_
 * is the key in the repository.
 * When the proxy needs to process an HTTP request,
 * the descriptors which titles are in the current config's list
 * are used to build a chain of filters.
 */
class CFilterDescriptor {

public:
    CFilterDescriptor();

    // The following data is used for organizing/editing filters
    string title;       // Title of the filter
    string version;     // Version number
    string author;      // Name of author
    string comment;     // Comment (such as description of what the filter does)
    string category;    // Example: "Ad Removal" or "My Custom Filters"

    // Type of filter
    enum TYPE { HEADOUT, HEADIN, TEXT };
    TYPE filterType;

    // Data specific to header filters
    string headerName;

    // Data specific to text filters
    bool   multipleMatches;
    int    windowWidth;
    string boundsPattern;

    // Data commom to both
    string urlPattern;
    string matchPattern;
    string replacePattern;
    
    // Default filter number (set to 0 for new/modified filters)
    int defaultFilter;
    
    // Check if all data is valid
    bool isValid() const;
    bool isValid(string& errmsg) const;
    
    // Compare filters
    bool operator ==(const CFilterDescriptor& d) const;

    // Clear all content
    void clear();
    
    // Export to a string
    string exportFilter() const;
    
    // Export a map to a string
    static string exportFilters(const map<string,CFilterDescriptor>& bank);
    
    // Import from a string to a map
    static int importFilters(const string& text,
                    map<string,CFilterDescriptor>& bank);

    // Import Proxomitron filters
    static int importProxomitron(const string& text,
                    map<string,CFilterDescriptor>& bank);
};

#endif
