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


#include "expander.h"
#include <map>
#include <vector>
#include <sstream>
#include <set> 
#include <wx/msgdlg.h>
#include <wx/textdlg.h>
#include <wx/datetime.h>
#include <wx/filefn.h>
#include "url.h"
#include "memory.h"
#include "const.h"
#include "util.h"
#include "settings.h"
#include "matcher.h"
#include "log.h"
#include "logframe.h"

using namespace std;

/* Decode the pattern.
 * In case a conditional command returns false, the decoding is stopped.
 *
 * Note: The filter using expand() MUST unlock the mutex if it is still
 * locked. It cannot be done from inside CExpander (nor CMatcher) because
 * of possible recursive calls.
 */
string CExpander::expand(const string& pattern, CFilter& filter) {

    ostringstream output;       // stream for the return string
    string command;             // decoded command name
    string content;             // decoded command content

    // Iterator to stacked memories
    vector<CMemory>::iterator stackedMem = filter.memoryStack.begin();

    unsigned int size = pattern.size();
    unsigned int index = 0;
    
    while (index<size) {

        // We can drop the pattern directly to the
        // output up to a special character
        unsigned int pos = pattern.find_first_of( "$\\" , index);

        if (pos == string::npos) {
            // No more special chars, drop the pattern to the end
            output << pattern.substr(index);
            index = size;
            continue;
        }
            
        // Send previous characters to output
        output << pattern.substr(index, pos-index);
        index = pos;

        if (pattern[pos] == '\\') {

            // ensure there is something after the backslash
            if (pos == size-1) {
                // the pattern is terminated by \ so do as for a normal character
                output << '\\';
                index++;
                continue;
            }
            
            // this is an escaped code
            char c = pattern[index+1];
            index += 2;
            switch (c) {
            case '#':
                // get one more memorized string from the stack
                if (stackedMem != filter.memoryStack.end()) {
                    if (stackedMem->isByValue()) {
                        output << expand(stackedMem->getValue(), filter);
                    } else {
                        output << stackedMem->getValue();
                    }
                    stackedMem++;
                }
                break;
            case '@':
                // get all remaining memorized strings from the stack
                while (stackedMem != filter.memoryStack.end()) {
                    if (stackedMem->isByValue()) {
                        output << expand(stackedMem->getValue(), filter);
                    } else {
                        output << stackedMem->getValue();
                    }
                    stackedMem++;
                }
                break;
            case 'k':
                if (filter.owner.url.getPath().find(".gif") != string::npos)
                    filter.owner.rdirToHost = "http://file//./html/killed.gif";
                else
                    filter.owner.rdirToHost = "http://file//./html/killed.html";
                break;
            // simple escaped codes
            case 't': output << '\t'; break;
            case 'r': output << '\r'; break;
            case 'n': output << '\n'; break;
            case 'u': output << filter.owner.url.getUrl(); break;
            case 'h': output << filter.owner.url.getHost(); break;
            case 'p': output << filter.owner.url.getPath(); break;
            case 'q': output << filter.owner.url.getQuery(); break;
            case 'a': output << filter.owner.url.getAnchor(); break;
            case 'd': output << "file:///";
                      output << CUtil::replaceAll(wxGetCwd().c_str(), "\\", "/");
                      break;
            default :
                if (CUtil::digit(c)) {
                    // code \0-9 we get the corresponding memorized string
                    CMemory& mem = filter.memoryTable[c - '0'];
                    if (mem.isByValue()) {
                        output << expand(mem.getValue(), filter);
                    } else {
                        output << mem.getValue();
                    }
                } else {
                    // escaped special codes, we drop them to the output
                    output << c; break;
                }
            }

        } else {    // '$'

            // Decode the command name, i.e all following uppercase letters
            command.clear();
            for (pos=index+1; pos<size &&
                        pattern[pos]>='A' && pattern[pos]<='Z'; pos++)
                command += pattern[pos];
                
            // Does it look like a command?
            if (command.size() > 0 && pos < size && pattern[pos] == '(') {

                // Decode content
                content.clear();
                int level = 1;
                unsigned int end = pos + 1;
                while (   end<size
                       && (   level>1
                           || pattern[end]!=')'
                           || pattern[end-1]=='\\' ) ) {
                    if (pattern[end] == '(' && pattern[end-1] != '\\') {
                        level++;
                    } else if (pattern[end] == ')' && pattern[end-1] != '\\') {
                        level--;
                    }
                    end++;
                }
                content = pattern.substr(pos+1, end-pos-1);
                if (end<size) end++;
                index = end;
                
                // PENDING: Commands
                // Each command is accessed in its "else if" block.
                // For each command, the call and use of results can be slightly different.
                // For example, the _result_ of $GET() is parsed before feeding the output.
                // Note that commands that are not CGenerator-dependent (ex. $ESC) should
                // be placed in another class.
                // This parse function will have to be modified to take into account
                // commands with several comma-separated parameters.
                //   output << f_COMMAND(parse(pos+1, pos));
                
                if (command == "GET") {

                    CUtil::trim(content);
                    CUtil::lower(content);
                    output << filter.owner.variables[content];

                } else if (command == "SET") {

                    unsigned int eq = content.find('=');
                    if (eq == string::npos) continue;
                    string name = content.substr(0, eq);
                    string value = content.substr(eq+1);
                    CUtil::trim(name);
                    CUtil::lower(name);
                    if (name[0] == '\\') name.erase(0,1);
                    if (name == "#") {
                        filter.memoryStack.push_back(CMemory(value));
                    } else if (name.size() == 1 && CUtil::digit(name[0])) {
                        filter.memoryTable[name[0]-'0'] = CMemory(value);
                    } else {
                        filter.owner.variables[name] = expand(value, filter);
                    }

                } else if (command == "SETPROXY") {

                    CUtil::trim(content);
                    unsigned int len = content.length();
                    set<string>& proxies = CSettings::ref().proxies;
                    for (set<string>::iterator it = proxies.begin();
                                it != proxies.end(); it++) {
                        if (content == it->substr(0, len)) {
                            filter.owner.contactHost = *it;
                            filter.owner.useSettingsProxy = false;
                            break;
                        }
                    }

                } else if (command == "USEPROXY") {

                    CUtil::trim(content);
                    CUtil::lower(content);
                    if (content == "true") {
                        filter.owner.useSettingsProxy = true;
                    } else if (content == "false") {
                        filter.owner.useSettingsProxy = false;
                    }

                } else if (command == "ALERT") {

                    wxMessageBox(expand(content, filter).c_str(), APP_NAME);

                } else if (command == "CONFIRM") {

                    int answer = wxMessageBox(expand(content, filter).c_str(),
                                                APP_NAME, wxYES_NO);
                    if (answer == wxNO) index = size;

                } else if (command == "ESC") {

                    string value = expand(content, filter);
                    output << CUtil::ESC(value);
                    
                } else if (command == "UESC") {

                    string value = expand(content, filter);
                    output << CUtil::UESC(value);

                } else if (command == "WESC") {

                    string value = expand(content, filter);
                    output << CUtil::WESC(value);

                } else if (command == "STOP") {

                    filter.active = false;

                } else if (command == "JUMP") {

                    filter.owner.jumpToHost = CUtil::trim(content);

                } else if (command == "RDIR") {

                    filter.owner.rdirToHost = CUtil::trim(content);

                } else if (command == "FILTER") {

                    CUtil::trim(content);
                    CUtil::lower(content);
                    if (content == "true") {
                        filter.owner.bypassBody = false;
                        filter.owner.bypassBodyForced = true;
                    } else if (content == "false") {
                        filter.owner.bypassBody = true;
                        filter.owner.bypassBodyForced = true;
                    }

                } else if (command == "FILE") {

                    output << CUtil::getFile(expand(content, filter));

                } else if (command == "LOG") {

                    string log = expand(content, filter);
                    CLog::ref().logFilterEvent(pmEVT_FILTER_TYPE_LOGCOMMAND,
                                filter.owner.reqNumber, filter.title, log);

                } else if (command == "LOCK") {

                    if (!filter.locked) {
                        CLog::ref().filterLock.Lock();
                        filter.locked = true;
                    }

                } else if (command == "UNLOCK") {

                    if (filter.locked) {
                        CLog::ref().filterLock.Unlock();
                        filter.locked = false;
                    }

                } else if (command == "KEYCHK") {

                    if (!CUtil::keyCheck(CUtil::upper(content))) index = size;

                } else if (command == "ADDLST") {

                    unsigned int comma = content.find(',');
                    if (comma != string::npos) {
                        string name = content.substr(0, comma);
                        string value = content.substr(comma + 1);
                        CUtil::trim(name);
                        CSettings::ref().addListLine(name, expand(value, filter));
                    }

                } else if (command == "ADDLSTBOX") {

                    unsigned int comma = content.find(',');
                    if (comma != string::npos) {
                        string name = content.substr(0, comma);
                        CUtil::trim(name);
                        string title = APP_NAME;
                        string value = content.substr(comma + 1);
                        comma = CUtil::findUnescaped(value, ',');
                        if (comma != string::npos) {
                            title = value.substr(0, comma);
                            value = value.substr(comma + 1);
                        }
                        title = expand(title, filter);
                        value = expand(value, filter);
                        string message = CSettings::ref().getMessage(
                                                "ADDLSTBOX_MESSAGE", name);
                        wxTextEntryDialog dlg(NULL, message.c_str(), title.c_str(), value.c_str());
                        if (dlg.ShowModal() == wxID_OK)
                            CSettings::ref().addListLine(name, dlg.GetValue().c_str());
                    }

                } else if (command == "URL") {

                    int end, reached;
                    const string& url = filter.owner.url.getUrl();
                    // Here we use the static matching function
                    bool ret = CMatcher::match(url, content, filter,
                                               0, url.size(), end, reached);
                    if (!ret) index = size;

                } else if (command == "RESP") {

                    int end, reached;
                    string& resp = filter.owner.responseCode;
                    bool ret = CMatcher::match(resp, content, filter,
                                               0, resp.size(), end, reached);
                    if (!ret) index = size;

                } else if (command == "TST") {

                    int end, reached;
                    unsigned int eq = content.find('=');
                    if (eq == string::npos) { eq = 0; content = '='; }
                    string value = content.substr(eq + 1);
                    string name = content.substr(0, eq);
                    CUtil::trim(name);
                    CUtil::lower(name);
                    if (name[0] == '\\') name.erase(0,1);
                    string toMatch;
                    if (name == "#") {
                        if (!filter.memoryStack.empty())
                            toMatch = filter.memoryStack.back().getValue();
                    } else if (name.size() == 1 && CUtil::digit(name[0])) {
                        toMatch = filter.memoryTable[name[0]-'0'].getValue();
                    } else {
                        toMatch = filter.owner.variables[name];
                    }
                    bool ret = CMatcher::match(toMatch, value, filter,
                                               0, toMatch.size(), end, reached);
                    if (!ret || end != (int)toMatch.size()) index = size;

                } else if (command == "IHDR") {

                    int end, reached;
                    unsigned int colon = content.find(':');
                    if (colon == string::npos) { pos=0; content = ":"; }
                    string pattern = content.substr(colon + 1);
                    string name = content.substr(0, colon);
                    CUtil::trim(name);
                    CUtil::lower(name);
                    string value = filter.owner.inHeaders[name];
                    if (!CMatcher::match(value, pattern, filter, 0,
                                         value.size(), end, reached))
                        index = size;

                } else if (command == "OHDR") {

                    int end, reached;
                    unsigned int colon = content.find(':');
                    if (colon == string::npos) { pos=0; content = ":"; }
                    string pattern = content.substr(colon + 1);
                    string name = content.substr(0, colon);
                    CUtil::trim(name);
                    CUtil::lower(name);
                    string value = filter.owner.outHeaders[name];
                    if (!CMatcher::match(value, pattern, filter, 0,
                                         value.size(), end, reached))
                        index = size;

                } else if (command == "DTM") {

                    string day[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
                    stringstream ss;
                    wxDateTime now = wxDateTime::UNow();
                    unsigned int i = 0;
                    while (i < content.size()) {
                        char c = content[i];
                        switch (c) {
                        case 'Y' : ss << now.GetYear(); break;
                        case 'M' : ss << CUtil::pad(now.GetMonth(),2); break;
                        case 'D' : ss << CUtil::pad(now.GetDay(),2); break;
                        case 'H' : ss << CUtil::pad(now.GetHour(),2); break;
                        case 'm' : ss << CUtil::pad(now.GetMinute(),2); break;
                        case 's' : ss << CUtil::pad(now.GetSecond(),2); break;
                        case 't' : ss << CUtil::pad(now.GetMillisecond(),3); break;
                        case 'h' : ss << CUtil::pad(((now.GetHour()+11)%12+1),2); break;
                        case 'a' : ss << (now.GetDay()<12?"am":"pm"); break;
                        case 'c' : ss << filter.owner.reqNumber; break;
                        case 'w' : ss << day[now.GetWeekDay()]; break;
                        case 'T' : content.replace(i, 1, "H:m:s"); continue;
                        case 'U' : content.replace(i, 1, "M/D/Y"); continue;
                        case 'E' : content.replace(i, 1, "D/M/Y"); continue;
                        case 'd' : content.replace(i, 1, "Y-M-D"); continue;
                        case 'I' : ss << now.Format("%a, %d %b %Y %H:%M:%S GMT", wxDateTime::GMT0); break;
                        case '\\' : if (i+1 < content.size()) { ss << content[i+1]; i++; } break;
                        default  : ss << c;
                        }
                        i++;
                    }
                    output << ss.str();

                }

            } else {
                // not a command, consume the $ as a normal character
                output << '$';
                index++;
            }
        }
    }
    // End of pattern, return the decoded string
    return output.str();
}

