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


#ifndef __nodes__
#define __nodes__

#include <string>
#include <vector>
#include <map>
#include "node.h"
#include "memory.h"
#include "url.h"
#include "matcher.h"
#include "filter.h"

using namespace std;

/* class CNode_Star
 * Try and match any string
 */
class CNode_Star : public CNode {

private:
    int pos;
    bool maxFirst;
    bool tab[256];
    bool useTab;

public:
    CNode_Star(const string& text, int& reached);
    ~CNode_Star() { }
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(STAR)
    void setNextNode(CNode* node, bool eop);
    NODE_MATCH()
};


/* class CNode_MemStar
 * Try and match any string and store the match
 */
class CNode_MemStar : public CNode {

private:
    int pos;
    bool maxFirst;
    bool tab[256];
    bool useTab;
    CMemory* memory;
    vector<CMemory>* stack;
    CMemory backup;

public:
    CNode_MemStar(const string& text, int& reached, CMemory& mem);
    CNode_MemStar(const string& text, int& reached, vector<CMemory>& st);
    ~CNode_MemStar() { }
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(MEMSTAR)
    void setNextNode(CNode* node, bool eop);
    NODE_MATCH()
};


/* class CNode_Space
 * Matches any spaces
 */
class CNode_Space : public CNode {

public:
    CNode_Space(const string& text, int& reached);
    ~CNode_Space() { }
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(SPACE)
    NODE_SETNEXTNODE()
    NODE_MATCH()
};


/* class CNode_Equal
 * Matches an Equal sign surrounded by spaces
 */
class CNode_Equal : public CNode {

public:
    CNode_Equal(const string& text, int& reached);
    ~CNode_Equal() { }
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(EQUAL)
    NODE_SETNEXTNODE()
    NODE_MATCH()
};


/* class CNode_Quote
 * Try and match a " or a '
 */
class CNode_Quote : public CNode {

private:
    char quote;                 // can be ' or "
    char matched;
    CNode_Quote *openingQuote;  // points to next ' in run, if any

public:
    CNode_Quote(const string& text, int& reached, char q);
    ~CNode_Quote() { }
    void setOpeningQuote(CNode_Quote *node);
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(QUOTE)
    NODE_SETNEXTNODE()
    NODE_MATCH()
};


/* class CNode_Char
 * Try and match a single character
 */
class CNode_Char : public CNode {

private:
    char byte;

public:
    CNode_Char(const string& text, int& reached, char c);
    ~CNode_Char() { }
    char getChar();
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(CHAR)
    NODE_SETNEXTNODE()
    NODE_MATCH()
};


/* class CNode_Range
 * Try and match a single character
 */
class CNode_Range : public CNode {

private:
    int min, max;
    bool allow;

public:
    CNode_Range(const string& text, int& reached,
                int min, int max, bool allow=true);
    ~CNode_Range() { }
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(RANGE)
    NODE_SETNEXTNODE()
    NODE_MATCH()
};


/* class CNode_String
 * Try and match a string of characters
 */
class CNode_String : public CNode {

private:
    string str;
    int size;

public:
    CNode_String(const string& text, int& reached, const string& s);
    ~CNode_String() { }
    void append(char c);
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(STRING)
    NODE_SETNEXTNODE()
    NODE_MATCH()
};


/* class CNode_Chars
 * Try and match a single character
 */
class CNode_Chars : public CNode {

private:
    bool byte[256];
    bool allow;

public:
    CNode_Chars(const string& text, int& reached, string c, bool allow=true);
    ~CNode_Chars() { }
    void add(unsigned char c);
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(CHARS)
    NODE_SETNEXTNODE()
    NODE_MATCH()
};


/* class CNode_Empty
 * Empty pattern
 */
class CNode_Empty : public CNode {

private:
    bool ret;

public:
    CNode_Empty(const string& text, int& reached, bool ret=true);
    ~CNode_Empty() { }
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(EMPTY)
    NODE_SETNEXTNODE()
    NODE_MATCH()
};


/* class CNode_Any
 * Matches any character
 */
class CNode_Any : public CNode {

public:
    CNode_Any(const string& text, int& reached);
    ~CNode_Any() { }
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(ANY)
    NODE_SETNEXTNODE()
    NODE_MATCH()
};


/* class CNode_Run
 * Try and match a concatenation
 */
class CNode_Run : public CNode {

private:
    vector<CNode*> *nodes;
    CNode* firstNode;

public:
    CNode_Run(const string& text, int& reached, vector<CNode*> *vect);
    ~CNode_Run();
    bool mayMatch(bool* tab);
    NODE_ID(RUN)
    void setNextNode(CNode* node, bool eop);
    int match();
};


/* class CNode_Or
 * Try and match nodes one after another
 */
class CNode_Or : public CNode {

private:
    vector<CNode*> *nodes;
    vector<CNode*>::iterator iter;

public:
    CNode_Or(const string& text, int& reached, vector<CNode*> *vect);
    ~CNode_Or();
    bool mayMatch(bool* tab);
    NODE_ID(OR)
    void setNextNode(CNode* node, bool eop);
    int match();
};


/* class CNode_And
 * Try and match left node then right node (returns max length of both)
 * Stop is for "&&", limiting right to matching exactly what left matched
 */
class CNode_And : public CNode {

private:
    CNode *nodeL, *nodeR;
    bool force;

public:
    CNode_And(const string& text, int& reached,
                CNode *L, CNode *R, bool force=false);
    ~CNode_And();
    bool mayMatch(bool* tab);
    NODE_ID(AND)
    void setNextNode(CNode* node, bool eop);
    int match();
};


/* class CNode_Repeat
 * Try and match a pattern several times
 */
class CNode_Repeat : public CNode {

private:
    CNode *node;
    int rmin, rmax, rcount, pos;
    bool iterate;

public:
    CNode_Repeat(const string& text, int& reached, CNode *node,
                int rmin, int rmax, bool iter=false);
    ~CNode_Repeat();
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(REPEAT)
    void setNextNode(CNode* node, bool eop);
    NODE_MATCH()
};


/* class CNode_Memory
 * Try and match something, store the match if success (in table or on stack)
 */
class CNode_Memory : public CNode {

private:
    CNode* node;
    CMemory* memory;
    vector<CMemory>* stack;
    CMemory backup;

public:
    CNode_Memory(const string& text, int& reached, CNode *node, CMemory& mem);
    CNode_Memory(const string& text, int& reached, CNode *node, vector<CMemory>& st);
    ~CNode_Memory();
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(MEMORY)
    void setNextNode(CNode* node, bool eop);
    NODE_MATCH()
};


/* class CNode_Negate
 * Try and match something, if it does it returns -1, else start
 */
class CNode_Negate : public CNode {

private:
    CNode* node;

public:
    CNode_Negate(const string& text, int& reached, CNode *node);
    ~CNode_Negate();
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(NEGATE)
    void setNextNode(CNode* node, bool eop);
    NODE_MATCH()
};


/* class CNode_AV
 * Try and match an html parameter
 */
class CNode_AV : public CNode {

private:
    CNode* node;
    bool isAVQ;

public:
    CNode_AV(const string& text, int& reached, CNode *node, bool isAVQ);
    ~CNode_AV();
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(AV)
    void setNextNode(CNode* node, bool eop);
    NODE_MATCH()
};


/* class CNode_Url
 * Try and match a part of URL
 */
class CNode_Url : public CNode {

private:
    const CUrl& url;
    char token;

public:
    CNode_Url(const string& text, int& reached, const CUrl& url, char token);
    ~CNode_Url() { }
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(URL)
    NODE_SETNEXTNODE()
    NODE_MATCH()
};


/* class CNode_List
 * Try and match nodes one after another. Nodes are looked for in the
 * order they appear in CSettings::lists . They are constructed when
 * first needed, and only destroyed when CNode_List is destroyed.
 */
class CNode_List : public CNode {

private:
    CMatcher& matcher;                  // matcher that will build nodes
    string name;                        // patterns list name
    map<string, CNode*> nodes;          // bank of built nodes
    bool isEnd;

public:
    CNode_List(const string& text, int& reached, string name, CMatcher& matcher);
    ~CNode_List();
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(LIST)
    void setNextNode(CNode* node, bool eop);
    NODE_MATCH()
};


/* class CNode_Command
 * Execute a command. Commands that do not consume anything (whether they
 * always match or not) don't have a dedicated CNode class. They are
 * gathered here.
 */
enum CMD_ID {
    CMD_SETSHARP = 1,
    CMD_SETDIGIT,  CMD_SETVAR,   CMD_ALERT,  CMD_CONFIRM, CMD_STOP,
    CMD_USEPROXY,  CMD_SETPROXY, CMD_LOG,    CMD_JUMP,    CMD_RDIR,
    CMD_FILTER,    CMD_LOCK,     CMD_UNLOCK, CMD_KEYCHK,  CMD_ADDLST,
    CMD_ADDLSTBOX, CMD_TYPE,     CMD_KILL,
    
    CMD_TSTSHARP = 100, // From this point, commands need a CMatcher for content
    CMD_TSTDIGIT, CMD_TSTVAR, CMD_URL,
    CMD_IHDR,     CMD_OHDR,   CMD_RESP,
    CMD_TSTEXPAND
};

class CNode_Command : public CNode {

private:
    CMD_ID num;
    string name;
    string content;
    CFilter& filter;
    CFilterOwner& owner;
    string toMatch;
    CMatcher* matcher;
    bool done;

public:
    CNode_Command(const string& text, int& reached,
                  CMD_ID num, string name, string content, CFilter& filter);
    ~CNode_Command();
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(COMMAND)
    NODE_SETNEXTNODE()
    NODE_MATCH()
};


/* class CNode_Cnx
 * Matches depending on connection number.
 */
class CNode_Cnx : public CNode {

private:
    int x, y, z;
    int& num;

public:
    CNode_Cnx(const string& text, int& reached, int x, int y, int z, int& num);
    ~CNode_Cnx() { }
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(CNX)
    NODE_SETNEXTNODE()
    NODE_MATCH()
};


/* class CNode_Nest
 * Matches nested tags, with optional content.
 */
class CNode_Nest : public CNode {

private:
    CNode *left, *middle, *right;
    bool tabL[256], tabR[256];
    bool inest;

public:
    CNode_Nest(const string& text, int& reached,
                CNode* left, CNode* middle, CNode* right, bool inest);
    ~CNode_Nest();
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(NEST)
    void setNextNode(CNode* node, bool eop);
    NODE_MATCH()
};


/* class CNode_Test
 * Try and match a variable content. Commands $TST(var) and $TST(\?)
 */
class CNode_Test : public CNode {

private:
    string name;
    CFilter& filter;

public:
    CNode_Test(const string& text, int& reached, string name, CFilter& filter);
    ~CNode_Test() { }
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(TEST)
    NODE_SETNEXTNODE()
    NODE_MATCH()
};


/* class CNode_Ask
 * Automates the insertion of an item in a list based on user choice.
 */
class CNode_Ask : public CNode {

private:
    CFilter& filter;
    const string allowName, denyName, question, item, pattern;
    CMatcher *allowMatcher, *denyMatcher;
    string toMatch; // for matchers

public:
    CNode_Ask(const string& text, int& reached, CFilter& filter,
                string allowName, string denyName, string question,
                string item, string pattern);
    ~CNode_Ask();
    int consume();
    bool mayMatch(bool* tab);
    NODE_ID(ASK)
    NODE_SETNEXTNODE()
    NODE_MATCH()
};

#endif
