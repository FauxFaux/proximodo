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


#include "headerfilter.h"
#include <vector>
#include <map>
#include "url.h"
#include "expander.h"
#include "util.h"
#include "log.h"

using namespace std;

/* Constructor
 */
CHeaderFilter::CHeaderFilter(const CFilterDescriptor& desc, CFilterOwner& owner)
                throw (parsing_exception) : CFilter(owner) {

    headerName = desc.headerName;
    CUtil::lower(headerName);
    title = desc.title;
    urlMatcher = textMatcher = NULL;
    replacePattern = desc.replacePattern;
    if (!desc.urlPattern.empty()) {
        urlMatcher = new CMatcher(content, desc.urlPattern, *this);
    }
    if (!desc.matchPattern.empty()) {
        try {
            textMatcher = new CMatcher(content, desc.matchPattern, *this);
        } catch (parsing_exception e) {
            if (urlMatcher) delete urlMatcher;
            throw e;
        }
    }
}


/* Destructor
 */
CHeaderFilter::~CHeaderFilter() {

    if (urlMatcher) delete urlMatcher;
    if (textMatcher) delete textMatcher;
}


/* Transform header according to filter description
 */
bool CHeaderFilter::filter(string& content) {

    if (!active || content.empty() && textMatcher && textMatcher->isStar())
        return false;

    // clear memory
    clearMemory();

    // check if URL matches
    if (urlMatcher) {
        int end, reached;
        bool ret;
        this->content = owner.url.getFromHost();
        ret = urlMatcher->match(0, content.size(), end, reached);
        unlock();
        if (!ret) return false;
    }

    // Check if content matches
    if (textMatcher) {
        int end, reached;
        bool ret;
        this->content = content;
        ret = textMatcher->match(0, content.size(), end, reached);
        unlock();
        if (!ret) return false;
    }

    // Log match event
    CLog::ref().logFilterEvent(pmEVT_FILTER_TYPE_HEADERMATCH,
                               owner.reqNumber, title, content);
                               
    // Compute new header content
    content = CExpander::expand(replacePattern, *this);
    unlock();
    CUtil::trim(content);

    // Ensure the header is still standard compliant
    // PENDING: I only do the minimum amount of tests here...
    unsigned int index = 0;
    while ((index = content.find_first_of("\r\n", index)) != string::npos) {
        // Multiligne must be formatted as CR LF SPACE
        int pos = index+1;
        while (string(" \t\r\n").find(content[pos])) ++pos;
        content.replace(index, pos-index, "\r\n ");
        index += 3;
    }

    // Log replace event
    CLog::ref().logFilterEvent(pmEVT_FILTER_TYPE_HEADERREPLACE,
                               owner.reqNumber, title, content);

    return true;
}

