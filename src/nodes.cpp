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


#include "nodes.h"
#include <algorithm>
#include <functional>
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include "const.h"
#include "util.h"
#include "settings.h"
#include "expander.h"
#include "log.h"
#include "logframe.h"

using namespace std;

/* this macro updates the CNode::reached value
 */
#define UPDATE_REACHED(n) (reached<n?reached=n:n)


/* class CNode_Star
 * Try and match any string. Corresponds to *
 */
CNode_Star::CNode_Star(const string& text, int& reached) : CNode(text, reached) {
}

int CNode_Star::consume() {

    int ret;
    // * at end of pattern -> try first by consuming everything
    if (maxFirst) {
        if (first) {    // more initialization
            first = false;
            pos = start-1;
            ret = UPDATE_REACHED(stop);
        } else {
            ret = (++pos < stop ? pos : -1);
        }

    // * at middle of pattern -> try positions one by one (use hint if available)
    } else {
        if (first) {    // more initialization
            first = false;
            pos = start-1;
        }
        if (useTab) {
            while (++pos < stop && !tab[(unsigned char)text[pos]]);
        } else {
            ++pos;
        }
        ret = (pos <= stop ? UPDATE_REACHED(pos) : -1);
    }
    return ret;
}

bool CNode_Star::mayMatch(bool* tab) {
    for (int i=0; i<256; i++) tab[i] = true;
    return true;
}

void CNode_Star::setNextNode(CNode* node, bool eop) {
    nextNode = node;
    maxFirst = eop;
    useTab = false;
    if (!maxFirst && nextNode) {
        for (int i=0; i<256; i++) tab[i] = false;
        if (node->mayMatch(tab)) {
            maxFirst = true;
        } else {
            for (int i=0; i<256; i++) {
                if (!tab[i]) {
                    useTab = true;
                    break;
                }
            }
        }
    }
}


/* class CNode_MemStar
 * Try and match any string, and store the match. Corresponds to \0 or \#
 */
CNode_MemStar::CNode_MemStar(const string& text, int& reached,
            CMemory& mem) : CNode(text, reached),
            memory(&mem), stack(NULL) {
}

CNode_MemStar::CNode_MemStar(const string& text, int& reached,
            vector<CMemory>& st) : CNode(text, reached),
            memory(NULL), stack(&st) {
}

int CNode_MemStar::consume() {

    int ret;
    // * at end of pattern -> try first by consuming everything
    if (maxFirst) {
        if (first) {    // more initialization
            pos = start-1;
            ret = UPDATE_REACHED(stop);
        } else {
            ret = (++pos < stop ? pos : -1);
        }

    // * at middle of pattern -> try positions one by one (use hint if available)
    } else {
        if (first) {    // more initialization
            pos = start-1;
        }
        if (useTab) {
            while (++pos < stop && !tab[(unsigned char)text[pos]]);
        } else {
            ++pos;
        }
        ret = (pos <= stop ? UPDATE_REACHED(pos) : -1);
    }
    
    // Now, store/change/remove memory depending on result
    if (ret >= 0) {
        if (first) {
            first = false;
            // Backup memory and replace by a new one, or push new one on stack
            if (memory) {
                backup = *memory;
                (*memory)(&text, start, ret);
            } else {
                stack->push_back(CMemory(&text, start, ret));
            }
        } else {
            // Change stored/pushed memory
            if (memory) {
                (*memory)(&text, start, ret);
            } else {
                stack->back()(&text, start, ret);
            }
        }
    } else {
        if (!first) {
            // Undo backup.
            if (memory) {
                *memory = backup;
            } else {
                stack->pop_back();
            }
        }
    }
    return ret;
}

bool CNode_MemStar::mayMatch(bool* tab) {
    for (int i=0; i<256; i++) tab[i] = true;
    return true;
}

void CNode_MemStar::setNextNode(CNode* node, bool eop) {
    nextNode = node;
    maxFirst = eop;
    useTab = false;
    if (!maxFirst && nextNode) {
        for (int i=0; i<256; i++) tab[i] = false;
        if (node->mayMatch(tab)) {
            maxFirst = true;
        } else {
            for (int i=0; i<256; i++) {
                if (!tab[i]) {
                    useTab = true;
                    break;
                }
            }
        }
    }
}


/* class CNode_Space
 * Matches any spaces.
 */
CNode_Space::CNode_Space(const string& text, int& reached) : CNode(text, reached) {
}

int CNode_Space::consume() {
    retry = false;
    // Rule: space
    while (start < stop && text[start] <= ' ') ++start;
    return UPDATE_REACHED(start);
}

bool CNode_Space::mayMatch(bool* tab) {
    tab[' ']  = true;
    tab['\t'] = true;
    tab['\r'] = true;
    tab['\n'] = true;
    return true;
}


/* class CNode_Equal
 * Matches an Equal sign surrounded by spaces.
 */
CNode_Equal::CNode_Equal(const string& text, int& reached) : CNode(text, reached) {
}

int CNode_Equal::consume() {
    retry = false;
    // Rule: =
    while (start < stop && text[start] <= ' ') ++start;
    if    (start < stop && text[start] == '=') {
        ++start;
    } else {
        UPDATE_REACHED(start);
        return -1;
    }
    while (start < stop && text[start] <= ' ') ++start;
    return UPDATE_REACHED(start);
}

bool CNode_Equal::mayMatch(bool* tab) {
    tab['=']  = true;
    tab[' ']  = true;
    tab['\t'] = true;
    tab['\r'] = true;
    tab['\n'] = true;
    return false;
}


/* class CNode_Quote
 * Try and match a " or a '.
 */
CNode_Quote::CNode_Quote(const string& text, int& reached, char q) :
            CNode(text, reached), quote(q), matched(0), openingQuote(NULL) {
}

void CNode_Quote::setOpeningQuote(CNode_Quote *node) {
    openingQuote = node;
}

int CNode_Quote::consume() {
    retry = false;
    if (start >= stop) return -1;
    matched = text[start];
    // Rule: "
    // Rule: '
    return (    matched == '\''
            && (   quote == '\"'
                || !openingQuote
                || openingQuote->matched == '\'' )
            ||  matched == '\"'
            && (   quote == '\"'
                || openingQuote
                && openingQuote->matched == '\"' ) ?
        (++start, UPDATE_REACHED(start)) : -1);
}

bool CNode_Quote::mayMatch(bool* tab) {
    tab['\"'] = true;
    tab['\''] = true;
    return false;
}


/* class CNode_Char
 * Try and match a single character
 */
CNode_Char::CNode_Char(const string& text, int& reached, char c) :
            CNode(text, reached), byte(c) {
}

char CNode_Char::getChar() {
    return byte;
}

int CNode_Char::consume() {
    retry = false;
    // The test is case insensitive
    return (start<stop && tolower(text[start]) == byte ?
            (++start, UPDATE_REACHED(start)) : -1);
}

bool CNode_Char::mayMatch(bool* tab) {
    tab[(unsigned char)byte] = true;
    tab[(unsigned char)toupper(byte)] = true;
    return false;
}


/* class CNode_Range
 * Try and match a single character
 */
CNode_Range::CNode_Range(const string& text, int& reached, int min, int max,
        bool allow) : CNode(text, reached), min(min), max(max), allow(allow) {
}

int CNode_Range::consume() {
    retry = false;
    int index = start;
    int token = text[index];
    // Check if there is a minus sign
    int sign = 1;
    if (token == '-' && start < stop) {
        token = text[++index];
        UPDATE_REACHED(index);
        sign = -1;
    }
    // Check if there is a digit
    if (!CUtil::digit(token) || index>=stop) return -1;
    // Read the number
    int num = 0;
    while (CUtil::digit(token) && index<stop) {
        num = num*10 + token - '0';
        token = text[++index];
    }
    num *= sign;
    UPDATE_REACHED(index);
    // Rule: [#]
    // Check if the number is in the range
    return (allow ^ (num < min || num > max)) ? index : -1;
}

bool CNode_Range::mayMatch(bool* tab) {
    for (int i='0'; i<='9'; i++) tab[i] = true;
    tab['-'] = true;
    return false;
}


/* class CNode_String
 * Try and match a string of characters.
 * Note: s and c must be in lowercase.
 */
CNode_String::CNode_String(const string& text, int& reached, const string& s) :
            CNode(text, reached), str(s) {
    size = s.length();
}

void CNode_String::append(char c) {
    str += c;
    size++;
}

int CNode_String::consume() {
    retry = false;
    int i = 0;
    int max = (size < stop-start ? size : stop-start);
    while (i < max && str[i] == tolower(text[start+i])) i++;
    UPDATE_REACHED(start+i);
    return (i == size ? start+i : -1);
}

bool CNode_String::mayMatch(bool* tab) {
    tab[(unsigned char)str[0]] = true;
    tab[(unsigned char)toupper(str[0])] = true;
    return false;
}


/* class CNode_Chars
 * Try and match a single character.
 */
CNode_Chars::CNode_Chars(const string& text, int& reached, string s, bool allow) :
            CNode(text, reached), allow(allow){
    // For a fast matching, all allowed (or forbidden) characters are
    // tagged in a table of booleans.
    unsigned int i;
    for (i=0; i<256; i++) byte[i] = !allow;
    for (string::iterator it = s.begin(); it != s.end(); it++) {
        byte[(unsigned char)(*it)] = allow;
    }
}

void CNode_Chars::add(unsigned char c) {
    // We can add characters to the list after the construction
    byte[c] = allow;
}

int CNode_Chars::consume() {
    retry = false;
    return (start < stop && byte[(unsigned char)text[start]] ?
                (++start, UPDATE_REACHED(start)) : -1);
}

bool CNode_Chars::mayMatch(bool* tab) {
    for (int i=0; i<256; i++) if (byte[i]) tab[i] = true;
    return false;
}


/* class CNode_Empty
 * Empty pattern. Always matches (but only once) or never, depending on ret
 */
CNode_Empty::CNode_Empty(const string& text, int& reached, bool ret) :
        CNode(text, reached), ret(ret) {
}

int CNode_Empty::consume() {
    retry = false;
    return ret ? start : -1;
}

bool CNode_Empty::mayMatch(bool* tab) {
    return ret;
}


/* class CNode_Any
 * Matches any character. Corresponds to ?
 */
CNode_Any::CNode_Any(const string& text, int& reached) : CNode(text, reached) {
}

int CNode_Any::consume() {
    retry = false;
    // Rule: ?
    return start < stop ? (++start, UPDATE_REACHED(start)) : -1;
}

bool CNode_Any::mayMatch(bool* tab) {
    for (int i=0; i<256; i++) tab[i] = true;
    return false;
}


/* class CNode_Run
 * Try and match a seauence of nodes.
 */
CNode_Run::CNode_Run(const string& text, int& reached, vector<CNode*> *vect) :
            CNode(text, reached) {
    nodes = vect;
    firstNode = *nodes->begin();
}

CNode_Run::~CNode_Run() {
    CUtil::deleteVector<CNode>(*nodes);
    delete nodes;
}

int CNode_Run::match() {
    if (first) {    // more initialization
        first = false;
        firstNode->reset(start, stop);
    }

    int pos = firstNode->match();
    if (pos >= 0) {
        retry = firstNode->retry;
        consumed = nodes->back()->consumed;
        return pos;
    } else {
        retry = false;
        return consumed = -1;
    }
}

bool CNode_Run::mayMatch(bool* tab) {
    bool ret = true;
    for (unsigned int i=0; ret && i<nodes->size(); i++) {
        ret = (*nodes)[i]->mayMatch(tab);
        if (!ret) break;
    }
    return ret;
}

void CNode_Run::setNextNode(CNode* node, bool eop) {
    nextNode = NULL;
    for (vector<CNode*>::reverse_iterator it = nodes->rbegin();
                it != nodes->rend(); it++) {
        (*it)->setNextNode(node, eop);
        node = *it;
        eop = false;
    }
}


/* class CNode_Or
 * Try and match nodes one after another
 */
CNode_Or::CNode_Or(const string& text, int& reached, vector<CNode*> *vect) :
            CNode(text, reached) {
    nodes = vect;
}

CNode_Or::~CNode_Or() {
    CUtil::deleteVector<CNode>(*nodes);
    delete nodes;
}

int CNode_Or::match() {
    if (first) {    // more initialization
        first = false;
        iter = nodes->begin();
        (*iter)->reset(start, stop);
    }

    // Rule: |
    do {
        // if the current node can still be tried, try it and check result
        int pos;
        if ((*iter)->retry && (pos = (*iter)->match()) >= 0) {
            // the current gave us a (new) match, return it
            retry = (*iter)->retry;
            consumed = (*iter)->consumed;
            return pos;
        // The current node cannot or could not give us a match (any more)
        // Go to the next one
        } else if (++iter != nodes->end()) {
            // Reset it
            (*iter)->reset(start, stop);
        } else {
            // No more nodes to try, return a failure
            retry = false;
            return consumed = -1;
        }
    } while (true);
}

bool CNode_Or::mayMatch(bool* tab) {
    bool ret = false;
    for (unsigned int i=0; i<nodes->size(); i++)
        if ((*nodes)[i]->mayMatch(tab)) ret = true;
    return ret;
}

void CNode_Or::setNextNode(CNode* node, bool eop) {
    nextNode = NULL;
    for (vector<CNode*>::iterator it = nodes->begin();
                it != nodes->end(); it++) {
        (*it)->setNextNode(node, eop);
    }
}


/* class CNode_And
 * Try and match left node then right node (returns max length of both).
 * 'force' is for "&&", limiting right to matching exactly what left matched
 */
CNode_And::CNode_And(const string& text, int& reached, CNode *L, CNode *R,
        bool force) : CNode(text, reached), nodeL(L), nodeR(R), force(force) {
}

CNode_And::~CNode_And() {
    delete nodeL;
    delete nodeR;
}

int CNode_And::match() {
    retry = false;

    // Ask left node for the first match
    nodeL->reset(start, stop);
    int posL = nodeL->match();
    if (posL < 0) return consumed = -1;
    consumed = nodeL->consumed;

    // Ask right node for the first match
    nodeR->reset(start, (force ? consumed : stop));
    int posR = nodeR->match();
    if (posR < 0 || force && posR != consumed) return consumed = -1;
    if (consumed < nodeR->consumed) consumed = nodeR->consumed;
    return (posL > posR ? posL : posR);
}

bool CNode_And::mayMatch(bool* tab) {
    bool tabL[256], tabR[256], retL, retR;
    for (int i=0; i<256; i++) tabL[i] = tabR[i] = false;
    retL = nodeL->mayMatch(tabL);
    retR = nodeR->mayMatch(tabR);
    if (!retL && !retR) {
        for (int i=0; i<256; i++)
            if (tabL[i] && tabR[i]) tab[i] = true;
    } else {
        for (int i=0; i<256; i++)
            if (retR && tabL[i] || retL && tabR[i]) tab[i] = true;
    }
    return retL && retR;
}

void CNode_And::setNextNode(CNode* node, bool eop) {
    nextNode = NULL;
    nodeL->setNextNode(node, eop);
    nodeR->setNextNode((force ? NULL : node), (force ? true : eop));
}


/* class CNode_Repeat
 * Try and match a pattern several times
 */
CNode_Repeat::CNode_Repeat(const string& text, int& reached, CNode *node,
            int rmin, int rmax, bool iter) : CNode(text, reached),
            node(node), rmin(rmin), rmax(rmax), iterate(iter) {
}
CNode_Repeat::~CNode_Repeat() {
    delete node;
}

int CNode_Repeat::consume() {
    if (first) {    // more initialization
        first = false;
        pos = start;
        rcount = -1;
        // rcount is the number of times we matched the pattern to reach
        // position pos. We start with value -1 to indicate ++ we won't try
        // the pattern on the first call of next()
    }

    if (iterate) {
        // Rule: ++
        if (rcount<0) {
            rcount = 0;
            // On first call, we match 0 times (provided zero
            // is within the limits)
            if (rmin <= 0) return start;
        }
        do {
            // Try one more match
            node->reset(pos, stop);
            int pos2 = node->match();
            if (pos2 == pos) { rcount = rmax; break; } // infinite-loop protection
            pos = pos2;
            rcount++;
            // Rule: {,}
        // and if we did not reach the lower limit yet, try again and again
        } while (rcount<rmin && pos>=0);
        // If it did not match, or we reached the upper limit,
        // we don't want to be called again
        if (pos<0 || rcount>=rmax) retry = false;
        return pos;
    } else {
        // Rule: +
        retry = false;  // + is a single match, it matches or it doesn't.
        rcount = 0;
        // Stop when upper limit reached
        while (rcount<rmax) {
            node->reset(pos, stop);
            int pos2 = node->match();
            if (pos2 < 0) break;
            if (pos2 == pos) { rcount = rmax; break; } // infinite-loop protection
            pos = pos2;
            rcount++;
        }
        // Rule: {,}
        // Check for lower limit
        return (rcount>=rmin ? pos : -1);
    }
}

bool CNode_Repeat::mayMatch(bool* tab) {
    bool ret = node->mayMatch(tab);
    return rmin==0 ? true : ret;
}

void CNode_Repeat::setNextNode(CNode* node, bool eop) {
    nextNode = node;
    this->node->setNextNode(NULL, eop);
}


/* class CNode_Memory
 * Try and match something, and if it does, store the position with a CMemory
 */
CNode_Memory::CNode_Memory(const string& text, int& reached, CNode *node,
            CMemory& mem) : CNode(text, reached), node(node),
            memory(&mem), stack(NULL) {
}

CNode_Memory::CNode_Memory(const string& text, int& reached, CNode *node,
            vector<CMemory>& st) : CNode(text, reached), node(node),
            memory(NULL), stack(&st) {
}

CNode_Memory::~CNode_Memory() {
    delete node;
}

int CNode_Memory::consume() {
    if (first) {    // more initialization
        node->reset(start, stop);
    }

    // Try and match the embedded node
    int pos;
    if (node->retry && (pos = node->match()) >= 0) {
        if (first) {
            first = false;
            // Backup memory and replace by a new one, or push new one on stack
            if (memory) {
                backup = *memory;
                (*memory)(&text, start, pos);
            } else {
                stack->push_back(CMemory(&text, start, pos));
            }
        } else {
            // Change stored/pushed memory
            if (memory) {
                (*memory)(&text, start, pos);
            } else {
                stack->back()(&text, start, pos);
            }
        }
        return pos;
    } else {
        if (!first) {
            // Undo backup.
            if (memory) {
                *memory = backup;
            } else {
                stack->pop_back();
            }
        }
        return -1;
    }
}

bool CNode_Memory::mayMatch(bool* tab) {
    return node->mayMatch(tab);
}

void CNode_Memory::setNextNode(CNode* node, bool eop) {
    nextNode = node;
    this->node->setNextNode(NULL, eop);
}


/* class CNode_Negate
 * Try and match a pattern. If it does, failure (-1), else success (start)
 */
CNode_Negate::CNode_Negate(const string& text, int& reached, CNode *node) :
            CNode(text, reached), node(node) {
}

CNode_Negate::~CNode_Negate() {
    delete node;
}

int CNode_Negate::consume() {
    retry = false;
    node->reset(start, stop);
    // The negation node does not consume text
    return node->match() < 0 ? start : -1;
}

bool CNode_Negate::mayMatch(bool* tab) {
    return true;
}

void CNode_Negate::setNextNode(CNode* node, bool eop) {
    nextNode = node;
    this->node->setNextNode(NULL, false);
}


/* class CNode_AV
 * Try and match an html parameter
 */
CNode_AV::CNode_AV(const string& text, int& reached, CNode *node, bool isAVQ) :
            CNode(text, reached), node(node), isAVQ(isAVQ) {
}

CNode_AV::~CNode_AV() {
    delete node;
}

int CNode_AV::consume() {
    retry = false;

    // find parameter limits
    bool consumeQuote = false;
    int begin, end;
    begin = end = start;
    if (start<stop) {
        char token = text[start];
        if (token == '\'' || token == '\"') {
            // We'll try and match a quoted parameter. Look for closing quote.
            end++;
            if (!isAVQ) begin++; // AV: the matching will start after the quote
            while (end<stop && text[end]!=token) end++;
            if (end<stop) {
                if (isAVQ)
                    end++; // AVQ: the matching will include the closing quote
                else
                    // AV: if we match the interior, we will
                    // consume the closing quote
                    consumeQuote = true;
            }
        } else {
            // Parameter without quote (single word), look for its end
            while (end<stop && text[end] > ' ' && text[end] != '>') end++;
            if (end == begin) return -1;
        }
    }
    
    // test parameter value
    node ->reset(begin, end);
    if (node->match() == end) {
        if (consumeQuote) {
            end++;
            UPDATE_REACHED(end);
        }
        return end;
    }
    return -1;
}

bool CNode_AV::mayMatch(bool* tab) {
    tab['\''] = true;
    tab['\"'] = true;
    return node->mayMatch(tab);
}

void CNode_AV::setNextNode(CNode* node, bool eop) {
    nextNode = node;
    this->node->setNextNode(NULL, true);
}


/* class CNode_Url
 * Try and match a part of URL
 */
CNode_Url::CNode_Url(const string& text, int& reached, const CUrl& url, char token) :
            CNode(text, reached), url(url), token(token) {
}

int CNode_Url::consume() {
    retry = false;
    const string * str = NULL;
    switch (token) {
        case 'u': str = &(url.getUrl());     break;
        case 'h': str = &(url.getHost());    break;
        case 'p': str = &(url.getPath());    break;
        case 'q': str = &(url.getQuery());   break;
        case 'a': str = &(url.getAnchor());  break;
    }
    int size = str->length();
    if (!size) return -1;
    int i = 0;
    int max = (size < stop-start ? size : stop-start);
    while (i < max && (*str)[i] == tolower(text[start+i])) i++;
    UPDATE_REACHED(start+i);
    return (i == size ? start+i : -1);
}

bool CNode_Url::mayMatch(bool* tab) {
    // At construction time, we don't have the URL address
    // (it will be set/changed later, or when recyling the filter)
    // so we must allow everything.
    for (int i=0; i<256; i++) tab[i] = true;
    return false;
}


/* class CNode_List
 * Try and match nodes one after another, in CSettings::lists order.
 * Corresponds to $LST() command.
 */
CNode_List::CNode_List(const string& text, int& reached, string name, CMatcher& matcher) :
            CNode(text, reached), matcher(matcher), name(name) {
}

CNode_List::~CNode_List() {
    CUtil::deleteMap<string,CNode>(nodes);
}

int CNode_List::consume() {
    retry = false;
    deque<string>& list = CSettings::ref().lists[name];
    for (deque<string>::iterator it = list.begin(); it != list.end(); it++) {

        CNode* node;
        if (nodes.find(*it) == nodes.end()) { // we have to build this node
            int cur = 0;
            node = nodes[*it] = matcher.expr(*it, cur, it->size());
            node->setNextNode(NULL, isEnd);
        } else {
            node = nodes[*it];
        }
        node->reset(start, stop);
        if (node->match() >= 0) return node->consumed;
    }
    return -1;
}

bool CNode_List::mayMatch(bool* tab) {
    for (int i=0; i<256; i++) tab[i] = true; // The list may evolve unpredictably
    return true;
}

void CNode_List::setNextNode(CNode* node, bool eop) {
    nextNode = node;
    isEnd = eop;
}


/* class CNode_Command
 * Executes a command that does not consume anything.
 */
CNode_Command::CNode_Command(const string& text, int& reached, CMD_ID num,
        string name, string content, CFilter& filter) :
        CNode(text, reached), num(num), name(name), content(content),
        filter(filter), owner(filter.owner) {
    matcher = NULL;
    // For some commands we'll build a CMatcher (content is a pattern)
    if (num >= 100) {
        matcher = new CMatcher(toMatch, content, filter);
    }
}

CNode_Command::~CNode_Command() {
    if (matcher) delete matcher;
}

int CNode_Command::consume() {
    if (first) {    // more initialization
        first = false;
        done = false;
    }

    int tmp, end, reached;

    switch (num) {

    case CMD_TSTSHARP:
        retry = false;
        if (filter.memoryStack.empty()) return -1;
        toMatch = filter.memoryStack.back().getValue();
        return (!toMatch.empty()
                && matcher->match(0, toMatch.size(), end, reached)
                && end == (int)toMatch.size() ? start : -1);

    case CMD_TSTEXPAND:
        retry = false;
        toMatch = CExpander::expand(name, filter);
        return (!toMatch.empty()
                && matcher->match(0, toMatch.size(), end, reached)
                && end == (int)toMatch.size() ? start : -1);

    case CMD_TSTDIGIT:
        retry = false;
        toMatch = filter.memoryTable[name[0]-'0'].getValue();
        return (!toMatch.empty()
                && matcher->match(0, toMatch.size(), end, reached)
                && end == (int)toMatch.size() ? start : -1);

    case CMD_TSTVAR:
        retry = false;
        toMatch = owner.variables[name];
        return (!toMatch.empty()
                && matcher->match(0, toMatch.size(), end, reached)
                && end == (int)toMatch.size() ? start : -1);

    case CMD_URL:
        retry = false;
        toMatch = owner.url.getUrl();
        return (matcher->match(0, toMatch.size(), end, reached) ? start : -1);

    case CMD_IHDR:
        retry = false;
        toMatch = CFilterOwner::getHeader(owner.inHeaders, name);
        return (matcher->match(0, toMatch.size(), end, reached) ? start : -1);

    case CMD_OHDR:
        retry = false;
        toMatch = CFilterOwner::getHeader(owner.outHeaders, name);
        return (matcher->match(0, toMatch.size(), end, reached) ? start : -1);

    case CMD_RESP:
        retry = false;
        toMatch = owner.responseCode;
        return (matcher->match(0, toMatch.size(), end, reached) ? start : -1);

    case CMD_SETSHARP:
        if (!done) {
            filter.memoryStack.push_back(CMemory(content));
            done = true;
            return start;
        } else {
            filter.memoryStack.pop_back();
            retry = false;
            return -1;
        }

    case CMD_SETDIGIT:
        filter.memoryTable[name[0]-'0'] = CMemory(content);
        break;

    case CMD_SETVAR:
        owner.variables[name] = CExpander::expand(content, filter);
        break;

    case CMD_KEYCHK:
        retry = false;
        return CUtil::keyCheck(content) ? start : -1;

    case CMD_KILL:
        // \k acts as a command (it changes variables and does not consume)
        // so it is processed by CNode_Command.
        filter.killed = true;
        break;

    case CMD_ADDLST:
        CSettings::ref().addListLine(name, CExpander::expand(content, filter));
        break;
        
    case CMD_ADDLSTBOX:
        {
            string title = APP_NAME;
            string value = content;
            unsigned int comma = CUtil::findUnescaped(value, ',');
            if (comma != string::npos) {
                title = value.substr(0, comma);
                value = value.substr(comma + 1);
            }
            title = CExpander::expand(title, filter);
            value = CExpander::expand(value, filter);
            string message = CSettings::ref().getMessage(
                                    "ADDLSTBOX_MESSAGE", name);
            wxTextEntryDialog dlg(NULL, message.c_str(), title.c_str(), value.c_str());
            if (dlg.ShowModal() == wxID_OK)
                CSettings::ref().addListLine(name, dlg.GetValue().c_str());
        }
        break;
    
    case CMD_ALERT:
        wxMessageBox(CExpander::expand(content, filter).c_str(), APP_NAME);
        break;

    case CMD_CONFIRM:
        retry = false;
        tmp = wxMessageBox(CExpander::expand(content, filter).c_str(),
                            APP_NAME, wxYES_NO);
        return (tmp == wxYES ? start : -1);

    case CMD_TYPE:
        retry = false;
        return (content == owner.fileType ? start : -1);

    case CMD_STOP:
        filter.bypassed = true;
        break;

    case CMD_USEPROXY:
        owner.useSettingsProxy = (content[0]=='t');
        break;

    case CMD_SETPROXY:
        for (set<string>::iterator it = CSettings::ref().proxies.begin();
                    it != CSettings::ref().proxies.end(); it++) {
            if (CUtil::noCaseBeginsWith(content, *it)) {
                owner.contactHost = *it;
                owner.useSettingsProxy = false;
                break;
            }
        }
        break;

    case CMD_LOG:
        {
            string log = CExpander::expand(content, filter);
            CLog::ref().logFilterEvent(pmEVT_FILTER_TYPE_LOGCOMMAND,
                        owner.reqNumber, filter.title, log);
        }
        break;

    case CMD_JUMP:
        owner.rdirToHost = CExpander::expand(content, filter);
        CUtil::trim(owner.rdirToHost);
        owner.rdirMode = 0;
        break;

    case CMD_RDIR:
        owner.rdirToHost = CExpander::expand(content, filter);
        CUtil::trim(owner.rdirToHost);
        owner.rdirMode = 1;
        break;

    case CMD_FILTER:
        owner.bypassBody = (content[0]!='t');
        owner.bypassBodyForced = true;
        break;

    case CMD_LOCK:
        if (!filter.locked) {
            CLog::ref().filterLock.Lock();
            filter.locked = true;
        }
        break;

    case CMD_UNLOCK:
        if (filter.locked) {
            CLog::ref().filterLock.Unlock();
            filter.locked = false;
        }
        break;
    
    } // end of switch

    // Default behaviour is match once
    retry = false;
    return start;
}

bool CNode_Command::mayMatch(bool* tab) {
    return true;
}


/* class CNode_Cnx
 * Matches depending on connection number.
 */
CNode_Cnx::CNode_Cnx(const string& text, int& reached, int x, int y, int z, int& num) :
        CNode(text, reached), x(x), y(y), z(z), num(num) {
}

int CNode_Cnx::consume() {
    retry = false;
    return (((num-1)/z)%y) == x-1 ? start : -1;
}

bool CNode_Cnx::mayMatch(bool* tab) {
    return true;
}


/* class CNode_Nest
 * Matches nested tags, with optional content.
 */
CNode_Nest::CNode_Nest(const string& text, int& reached, CNode* left,
        CNode* middle, CNode* right, bool inest) : CNode(text, reached),
        left(left), middle(middle), right(right), inest(inest) {
    int i;
    for (i=0; i<256; i++) tabL[i] = false;
    if (left->mayMatch(tabL)) for (i=0; i<256; i++) tabL[i] = true;
    for (i=0; i<256; i++) tabR[i] = false;
    if (right->mayMatch(tabR)) for (i=0; i<256; i++) tabR[i] = true;
}

CNode_Nest::~CNode_Nest() {
    if (left)   delete left;
    if (middle) delete middle;
    if (right)  delete right;
}

int CNode_Nest::consume() {
    retry = false;
    int endL = start, startR, pos, level;
    if (!inest) {
        if (!tabL[(int)text[start]]) return -1;
        left->reset(start, stop);
        endL = left->match();
        if (endL < 0) return -1;
    }
    startR = pos = endL;
    level = 1;
    while (pos < stop && level > 0) {
        if (tabR[(int)text[pos]]) {
            right->reset(pos, stop);
            int end = right->match();
            if (end > 0) {
                level--;
                startR = pos;
                pos = end;
                continue;
            }
        }
        if (tabL[(int)text[pos]]) {
            left->reset(pos, stop);
            int end = left->match();
            if (end > 0) {
                level++;
                pos = end;
                continue;
            }
        }
        pos++;
    }
    if (level > 0) return -1;
    if (middle) {
        middle->reset(endL, startR);
        int end = middle->match();
        if (end != startR) return -1;
    }
    return inest ? startR : pos;
}

bool CNode_Nest::mayMatch(bool* tab) {
    bool ret = true;
    if (ret && !inest) ret = left->mayMatch(tab);
    if (ret && middle) ret = middle->mayMatch(tab);
    if (ret && !inest) ret = right->mayMatch(tab);
    return ret;
}

void CNode_Nest::setNextNode(CNode* node, bool eop) {
    nextNode = node;
    left->setNextNode(NULL, false);
    right->setNextNode(NULL, eop);
    if (middle) middle->setNextNode(NULL, true);
}


/* class CNode_Test
 * Try and match a string of characters.
 */
CNode_Test::CNode_Test(const string& text, int& reached, string name,
        CFilter& filter) : CNode(text, reached), name(name), filter(filter) {
}

int CNode_Test::consume() {
    retry = false;
    string str;     // variable content, that will be compared to text
    if (name == "#") {
        if (!filter.memoryStack.empty())
            str = filter.memoryStack.back().getValue();
    } else if (name.size() == 1 && CUtil::digit(name[0])) {
        str = filter.memoryTable[name[0]-'0'].getValue();
    } else if (name[0] == '(' && name[name.size()-1] == ')') {
        str = CExpander::expand(name.substr(1, name.size()-2), filter);
    } else {
        str = filter.owner.variables[name];
    }
    int size = str.size();
    if (!size) return -1;
    int max = (size < stop-start ? size : stop-start);
    int i = 0;
    while (i < max && tolower(str[i]) == tolower(text[start+i])) i++;
    UPDATE_REACHED(start+i);
    return (i == size ? start+i : -1);
}

bool CNode_Test::mayMatch(bool* tab) {
    // We don't know at construction time what the string to match is.
    // So we must allow everything.
    for (int i=0; i<256; i++) tab[i] = true;
    return true;
}


/* class CNode_Ask
 * Automates the insertion of an item in a list based on user choice.
 */
CNode_Ask::CNode_Ask(const string& text, int& reached, CFilter& filter,
        string allowName, string denyName, string question, string item,
        string pattern) : CNode(text, reached), filter(filter),
        allowName(allowName), denyName(denyName), question(question),
        item(item), pattern(pattern) {

    allowMatcher = new CMatcher(toMatch, "$LST(" + allowName + ")", filter);
    try {
        denyMatcher = new CMatcher(toMatch, "$LST(" + denyName + ")", filter);
    } catch (parsing_exception e) {
        delete allowMatcher;
        throw e;
    }
}

CNode_Ask::~CNode_Ask() {
    delete allowMatcher;
    delete denyMatcher;
}

int CNode_Ask::consume() {
    retry = false;
    // We lock so that 2 documents doing the same test at the same time
    // won't ask the question twice. No need unlocking afterwards.
    if (!filter.locked) {
        CLog::ref().filterLock.Lock();
        filter.locked = true;
    }
    int end, reached;
    toMatch = CExpander::expand(pattern, filter);
    // If the pattern is found in Allow list, we return a non-match
    // (so that the filter does not operate)
    if (allowMatcher->match(0, toMatch.size(), end, reached)) return -1;
    // On the contrary, if found in Deny list, we return a match
    // (to continue filtering)
    if (denyMatcher->match(0, toMatch.size(), end, reached)) return start;
    // Now we'll ask the user what they want to do with it
    int tmp = wxMessageBox(CExpander::expand(question, filter).c_str(),
                           APP_NAME, wxYES_NO);
    // Then add the item to one list.
    if (tmp == wxYES) {
        CSettings::ref().addListLine(allowName, CExpander::expand(item, filter));
        return -1;
    } else {
        CSettings::ref().addListLine(denyName, CExpander::expand(item, filter));
        return start;
    }
}

bool CNode_Ask::mayMatch(bool* tab) {
    return true;
}


