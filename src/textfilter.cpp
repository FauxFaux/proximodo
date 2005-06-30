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
#include "url.h"
#include "expander.h"
#include "const.h"
#include "log.h"
#include "descriptor.h"
#include "filterowner.h"
#include "matcher.h"
#include <vector>
#include <map>
#include <sstream>

using namespace std;

/* Constructor
 */
CTextFilter::CTextFilter(CFilterOwner& owner, const CFilterDescriptor& desc,
                         CDataReceptor* next) :
        CFilter(owner), nextFilter(next) {

    textMatcher = boundsMatcher = urlMatcher = NULL;

    title           =  desc.title;
    windowWidth     =  desc.windowWidth;
    multipleMatches =  desc.multipleMatches;
    replacePattern  =  desc.replacePattern;

    if (!desc.urlPattern.empty()) {
        urlMatcher = new CMatcher(desc.urlPattern, *this);
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
        textMatcher = new CMatcher(desc.matchPattern, *this);
        textMatcher->mayMatch(okayChars);
    } catch (parsing_exception e) {
        if (urlMatcher) delete urlMatcher;
        throw e;
    }

    if (!desc.boundsPattern.empty()) {
        try {
            boundsMatcher = new CMatcher(desc.boundsPattern, *this);
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

    bypassed = false;
    
    if (urlMatcher) {
        // The filter will be inactive if the URL does not match
        const char *tStart, *tStop, *tEnd, *tReached;
        tStart = owner.url.getFromHost().c_str();
        tStop = tStart + owner.url.getFromHost().size();
        bypassed = !urlMatcher->match(tStart, tStop, tEnd, tReached);
        unlock();
    }
    
    killed = false;
    isStarted = false;
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
    if (killed) return;
    process(data, true);
}


/* Process data still in the buffer
 */
void CTextFilter::dataDump() {
    if (killed) return;
    string str = "";
    process(str, false);
    nextFilter->dataDump();
    killed = true;
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

    // if filter has been deactivated (i.e by $STOP() or
    // mismatched URL), forward the text unfiltered
    if (bypassed) {
        // buffer should be empty
        if (!buffer.empty()) {
            // buffer has not yet been emptied
            nextFilter->dataFeed(buffer);
            buffer.clear();
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
    
    // add data to buffer
    buffer += data;
    int size = buffer.size();

    // do we have enough to start scanning?
    if (size < needed && feeding) return;

    // output buffer
    ostringstream output;

    const char* bufHead = buffer.c_str();
    const char* bufTail = bufHead + size;
    const char* index   = bufHead; // index where we currently look for a match
    const char* done    = bufHead; // index up to which we sent to output
    
    // scan buffer
    while ((index<bufTail || index==bufTail && !feeding) && !killed && !bypassed) {

        // pass characters that cannot match
        if (index<bufTail && !okayChars[(unsigned char)(*index)]) {
            ++index;
            continue;
        }
        
        bool matched;
        const char *end, *reached;
        
        // compute up to where we want to match
        const char* stop = index + windowWidth;
        if (stop > bufTail) stop = bufTail;

        // clear memory
        clearMemory();

        if (boundsMatcher) {
            // let's try and find the bounds first
            matched = boundsMatcher->match(index, stop, end, reached);
            unlock();
            // could have had a different result with more data,
            // we'll wait for it
            if (reached == bufTail && feeding) break;
            // bounds not matching
            if (!matched) {
                ++index;
                continue;
            }
            stop = end;
        }

        // try matching
        matched = textMatcher->match(index, stop, end, reached);
        unlock();
        if (reached == bufTail && feeding) break;

        // pattern not matching (and condition for avoiding infinite loop)
        if (!matched
                || boundsMatcher   && end != stop
                || multipleMatches && end <= bufHead+lastEnd) {
            ++index;
            continue;
        }

        // send preceding data to output
        output << string(done, (size_t)(index-done));
        done = index;

        // compute replacement text
        string str = CExpander::expand(replacePattern, *this);
        unlock();
        
        // log events
        string replaced(index, (size_t)(end-index));
        CLog::ref().logFilterEvent(pmEVT_FILTER_TYPE_TEXTMATCH,
                                   owner.reqNumber, title, replaced);
        CLog::ref().logFilterEvent(pmEVT_FILTER_TYPE_TEXTREPLACE,
                                   owner.reqNumber, title, str);

        // replace
        if (multipleMatches) {
            int pos = index - bufHead;
            int len = end - index;
            buffer.replace(pos, len, str);
            size    = buffer.size();
            bufHead = buffer.c_str();
            bufTail = bufHead + size;
            index   = bufHead + pos;
            lastEnd = pos + str.size();
        } else {
            output << str;
            if (end == index)
                ++index;        // avoid infinite loop
            else
                done = index = end;
        }
    }
    
    // Set index back to size if it went over (end of file)
    if (index > bufTail) index = bufTail;
    
    // If the filter has been killed at some point,
    // send the processed text then dump next filters
    if (killed) {
        string str = output.str();
        nextFilter->dataFeed(str);
        nextFilter->dataDump();
        buffer.clear();
        return;
    }

    // If the filter has been deactivated in the loop, the rest of
    // the buffer is considered unmatching
    if (bypassed) index = bufTail;

    // decide how much data we'll then wait for
    needed = CTF_THRESHOLD2;
    if (bufTail-index > CTF_THRESHOLD1 && feeding) needed += windowWidth;

    // output the last scanned characters
    output << string(done, (size_t)(index-done));

    // clear buffer
    lastEnd -= (size_t)(index-bufHead);
    buffer.erase(0, (size_t)(index-bufHead));

    // send output to next filter
    string str = output.str();
    if (!str.empty()) nextFilter->dataFeed(str);
}
