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


#include "textfilter.h"
#include <vector>
#include <map>
#include <sstream>
#include "url.h"
#include "expander.h"
#include "const.h"
#include "log.h"

using namespace std;

/* Constructor
 */
CTextFilter::CTextFilter(CFilterOwner& owner, const CFilterDescriptor& desc,
                         CDataReceptor* next) throw (parsing_exception) :
        CFilter(owner), nextFilter(next) {

    textMatcher = boundsMatcher = urlMatcher = NULL;

    title           =  desc.title;
    windowWidth     =  desc.windowWidth;
    multipleMatches =  desc.multipleMatches;
    replacePattern  =  desc.replacePattern;

    if (!desc.urlPattern.empty()) {
        urlMatcher = new CMatcher(owner.url.getFromHost(), desc.urlPattern, *this);
        // (it can throw a parsing_exception)
    }

    isForStart = isForEnd = false;
    if (desc.matchPattern == "<start>") {
        isForStart = true;
        return;
    } else if (desc.matchPattern == "<end>") {
        isForEnd = true;
        return;
    }
    
    try {
        textMatcher = new CMatcher(buffer, desc.matchPattern, *this);
        textMatcher->mayMatch(okayChars);
    } catch (parsing_exception e) {
        if (urlMatcher) delete urlMatcher;
        throw e;
    }

    if (!desc.boundsPattern.empty()) {
        try {
            boundsMatcher = new CMatcher(buffer, desc.boundsPattern, *this);
            bool tab[256];
            boundsMatcher->mayMatch(tab);
            for (int i=0; i<256; i++) okayChars[i] = okayChars[i] && tab[i];
        } catch (parsing_exception e) {
            if (urlMatcher) delete urlMatcher;
            if (textMatcher) delete textMatcher;
            throw e;
        }
    }
}


/* Desctructor
 */
CTextFilter::~CTextFilter() {

    if(urlMatcher)    delete urlMatcher;
    if(textMatcher)   delete textMatcher;
    if(boundsMatcher) delete boundsMatcher;
}


/* Reset internal state and get ready for a new flow
 */
void CTextFilter::dataReset() {

    active = true;
    isStarted = false;
    
    if (urlMatcher) {
        // The filter will be inactive if the URL does not match
        int end, reached;
        active = urlMatcher->match(0, owner.url.getFromHost().size(), end, reached);
        unlock();
    }
    
    needed = CTF_THRESHOLD2;
    lastEnd = -1;
    buffer.clear();
    clearMemory();

    // Forward reset to following filters
    nextFilter->dataReset();
}


/* Receive data
 */
void CTextFilter::dataFeed(const string& data) {
    process(data, true);
}


/* Process data still in the buffer
 */
void CTextFilter::dataDump() {
    string str = "";
    process(str, false);
    nextFilter->dataDump();
}


/* Process data
 * It puts data in the buffer, and does process it only it the buffer is big
 * enough (unless dumping). Running through it character by character, it tries
 * and match the pattern (or first the bounds, if specified).
 * If if matches, the replacement string is computed and sent to output
 * buffer or replaced in input buffer, depending on 'allowMultiple'.
 * To avoid adding characters one by one to output buffer, it waits for
 * a match or the end before adding them in one <<. The loop continues
 * to the end of buffer, or the filter is inactivated (in which case
 * the rest of buffer is passed), or match() reaches end of buffer
 * (whether it succeeds or fails), whichever comes first. But the later
 * condition does not apply if we are dumping the buffer.
 * The expected buffer size is CTF_THRESHOLD2 most of the time, but if
 * match() reached the end of buffer after it processed more than
 * CTF_THRESHOLD1 characters, the next expected size will be CTF_THRESHOLD2 + 
 * window size (so that next time we try, the buffer is big enough for a full
 * attempt).
 */
void CTextFilter::process(const string& data, bool feeding) {

    // if filter has been set inactive (i.e by $STOP() or
    // mismatched URL), bypass the filter
    if (!active) {
        // buffer should be empty
        if (!buffer.empty()) {
            // buffer has not yet been emptied
            nextFilter->dataFeed(buffer);
            needed = CTF_THRESHOLD2;
            buffer = "";
        }
        // directly send data to next filter
        nextFilter->dataFeed(data);
        return;
    }
    
    // for special <start> and <end> filters
    if (isForStart || isForEnd) {
        if (isForStart && isStarted || isForEnd && feeding) {
            nextFilter->dataFeed(data);
        } else {
            string str = CExpander::expand(replacePattern, *this);
            unlock();
            str += data;
            if (!str.empty()) nextFilter->dataFeed(str);
        }
        isStarted = true;
        return;
    }
    
    // send data to buffer
    buffer += data;
    int size = buffer.size();

    // do we have enough to start scanning?
    if (size < needed && feeding) return;

    // output buffer
    ostringstream output;

    int done = 0;       // index up to which we sent to output
    int start = 0;      // index where we currently look for a match
    
    // scan buffer
    do {
        // if character cannot match, pass it
        if (!okayChars[(unsigned char)buffer[start]]) {
            ++start;
            continue;
        }
        
        bool matched;
        int end, reached;

        // compute up to where we want to match
        int stop = start + windowWidth;
        if (stop > size) stop = size;

        // clear memory
        clearMemory();

        if (boundsMatcher) {
            // let's try and find the bounds first
            matched = boundsMatcher->match(start, stop, end, reached);
            unlock();
            // could have had a different result with more data,
            // we'll wait for it
            if (reached == size && feeding) break;
            // bounds not matching
            if (!matched) {
                ++start;
                continue;
            }
            stop = end;
        }

        // try matching
        matched = textMatcher->match(start, stop, end, reached);
        unlock();
        if (reached == size && feeding) break;

        // pattern not matching (and condition for avoiding infinite loop)
        if (!matched || multipleMatches && end <= lastEnd) {
            ++start;
            continue;
        }

        // send preceding data to output
        output << buffer.substr(done, start-done);
        done = start;

        // compute replacement text
        string str = CExpander::expand(replacePattern, *this);
        unlock();
        
        // log events
        string replaced = buffer.substr(start, end-start);
        CLog::ref().logFilterEvent(pmEVT_FILTER_TYPE_TEXTMATCH,
                                   owner.reqNumber, title, replaced);
        CLog::ref().logFilterEvent(pmEVT_FILTER_TYPE_TEXTREPLACE,
                                   owner.reqNumber, title, str);

        // replace
        if (multipleMatches) {
            buffer.replace(start, end-start, str);
            size = buffer.size();
            lastEnd = start + str.size();
        } else {
            output << str;
            if (start == end)
                ++start;        // avoid infinite loop
            else
                done = start = end;
        }
    } while (start < size && active);

    // If the filter has been deactivated in the loop, the rest of
    // the buffer is considered unmatching
    if (!active) start = size;

    // decide how much data we'll then wait for
    needed = CTF_THRESHOLD2;
    if (size-start > CTF_THRESHOLD1 && feeding) needed += windowWidth;

    // output the last scanned characters
    output << buffer.substr(done, start-done);

    // clear buffer
    buffer.erase(0, start);
    lastEnd -= start;

    // send output to next filter
    string str = output.str();
    if (!str.empty()) nextFilter->dataFeed(str);
}

