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


#ifndef __node__
#define __node__

#include <string>

using namespace std;

/* class CNode
 * Generic node in the search tree
 *
 * The tree is composed of leaf nodes, that actually read and test the data,
 * and branch nodes that organize them. Examples of leaf nodes: test a
 * character, test a quote, test an arbitrary number of characters. Examples of
 * branch nodes: 'or', 'and', 'sequentially', 'a number of times'.
 *
 * The tree is generated at the construction of a CMatcher, with respect to a
 * given pattern. When CMatcher is used to test data, it resets the root of the
 * tree (function reset()) then asks it the end position of the match (function
 * next()).
 *
 * Each node resets its children and asks the length of their next match, or
 * tests the data, wrt its mode of operation. Most leaf nodes will match or
 * will not (in this case next() returns -1); those nodes need to be invoked
 * only once, so the retry bool will immediately be set to false to tell the
 * parent node that it should not be asked again (but if reset). The node that
 * tests '*' may match for several lengths, in that case retry will be set to
 * false only once all lengths have been returned. Some branch nodes can match
 * several times (e.g 'or' can match with one alternative or another). So their
 * retry bool will be set to false after they returned all their possible
 * matching lengths. The memory nodes encapsulate a node with an added
 * functionality of storing the match (or undoing it).
 */
class CNode {

public:
    // Virtual destructor.
    // The constructor is protected, as only subclasses must be instanciated.
    virtual ~CNode() { }

    // Immediately after constructing the whole node tree, one must call
    // this function (with default parameters) on the root node. The nodes
    // then link together horizontally, so that each node knows which node
    // will scan the text after itself. This is needed for correct processing
    // of "&" and "&&", which try right pattern on the first left match that
    // _do_ let remaining pattern match.
    virtual void setNextNode(CNode* node = NULL, bool eop = true) =0;

    // The mayMatch() function updates a table of expected characters
    // at the beginning of the text to match. It is called just after
    // the construction of the whole tree, to provide the search engine
    // with a fast way to know if a call to CMatcher::match() is of any
    // use. If !tab[text[start]] the engine knows there cannot be any
    // match at start, _even_ if the buffer is smaller than the window
    // size.
    // The function returns true if the node may consume nothing yet match.
    virtual bool mayMatch(bool* tab) =0;

    // Reset function.
    // This non-virtual function updates start/stop/retry/first variables
    // to prepare matching at the current position in the text.
    // Subclasses that need more initialisation will test 'first' at
    // beginning of match().
    inline void reset(int i, int j) { start = i; stop = j; retry = first = true; }

    // Match function.
    // Returns the end position of text matched by the whole chain of nodes
    // (wrt. nextNode) up to the last one (the one having nextNode==NULL).
    // The implementation must set 'consumed' to the end position match by
    // the pattern represented by the node itself.
    // Subclasses that use the NODE_MATCH() macro should have a
    // int consume() function that provides the next match length.
    // Nodes that can match several times (e.g "*" and "|") will return
    // those lengths at repeated consume() calls.
    virtual int match() =0;

    // This bool indicates if match() can be called again. One should
    // not call match (thus saving a function call) if retry==false
    bool retry;

    // If match() returned a value != -1, this variable contains the
    // position corresponding to the end of what just the node itself matched.
    int consumed;

    // Enumeration for id()
    enum type { AND,      ANY,      ASK,      AV,       CHAR,     CHARS,
                CNX,      COMMAND,  EMPTY,    EQUAL,    LIST,     MEMORY,
                MEMSTAR,  NEGATE,   NEST,     OR,       QUOTE,    RANGE,
                REPEAT,   RUN,      SPACE,    STAR,     STRING,   TEST,
                URL     };

    // Function to compare node types without RTTI
    virtual type id() =0;

protected:
    const string& text;
    int&   reached;
    int    start;
    int    stop;
    bool   first;
    CNode* nextNode;

    // The text string used as a buffer is known when we construct the
    // CMatcher for it. So we'll record it in nodes only once, when
    // building the tree.
    // The 'reached' variable will be set by match() to the end position of
    // scanned text. It is different to match() return value, in that in can
    // equals the end of text buffer while match() failed and returned -1.
    // Test it to know if buffer size may have changed the outcome.
    CNode(const string& text, int& reached) : text(text), reached(reached) { }
};


/* The following macros are simple implementations that can be used for most
 * subclasses.
 */

#define NODE_ID(ID) \
type id() {         \
    return ID;      \
}

#define NODE_SETNEXTNODE()                \
void setNextNode(CNode* node, bool eop) { \
    nextNode = node;                      \
}

#define NODE_MATCH()                               \
int match() {                                      \
    while (retry && (consumed = consume()) >= 0) { \
        if (!nextNode) return consumed;            \
        nextNode->reset(consumed, stop);           \
        int pos = nextNode->match();               \
        if (pos >= 0) return pos;                  \
    }                                              \
    retry = false;                                 \
    return consumed = -1;                          \
}

#endif

