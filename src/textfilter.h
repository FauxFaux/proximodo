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


#ifndef __textfilter__
#define __textfilter__

#include <string>
#include "filter.h"
#include "descriptor.h"
#include "receptor.h"
#include "matcher.h"

using namespace std;

/* class CTextFilter
 * this class buffers incoming data, and when there is enough of it
 * scans it byte after byte looking for a match. The scan only starts
 * if there are 32 bytes available, or, if it previously stopped
 * because of a match on less that window sixe bytes, window size + 32.
 * The filter does not need to call CMatcher::match() if the first byte
 * is not in the list of bytes that may start a match.
 * The scan continues until match() consumes the rest of the buffer being
 * smaller than window size.
 * CTextFilter are chained up, using virtual class CReceptor.
 * Unmatched data, or replaced data, is added to a ostringstream before
 * the filter gives it to the next CTextFilter.
 */
class CTextFilter : public CDataReceptor, public CFilter {

public:
    // Constructor
    // It needs a reference to the filter descriptor
    // I'll later replace CUrl& by reference to the request manager,
    // to be able to interact with it (e.g read/write $SET variables)
    CTextFilter(CFilterOwner& owner, const CFilterDescriptor& desc,
                CDataReceptor* next) throw (parsing_exception);

    // Destructor
    ~CTextFilter();
    
    // reset filter for new dataflow
    void dataReset();

    // receive new data
    void dataFeed(const string& data);

    // process buffer to the end (end of data stream)
    void dataDump();

private:
    // Next filter in the chain (can be a filter or the request manager)
    CDataReceptor* nextFilter;
    
    // input buffer
    string buffer;
    
    // for multiple-matching, end of last inserted text in buffer
    int lastEnd;
    
    // If we match, shall we replace the matched text in the buffer
    // instead of sending it to the output stream?
    bool multipleMatches;
    
    // Window size
    int windowWidth;
    
    // URL matcher
    CMatcher* urlMatcher;

    // Bounds matcher, if needed
    CMatcher* boundsMatcher;
    
    // Text matcher
    CMatcher* textMatcher;
    
    // Replace pattern
    string replacePattern;
    
    // For special <start> and <end> match patterns
    bool isForStart;
    bool isForEnd;
    bool isStarted;
    
    // How much data do we wait for?
    int needed;
    
    // Characters that may match
    bool okayChars[256];
    
    // Main data processing function
    void process(const string& data, bool feeding);
};

#endif
