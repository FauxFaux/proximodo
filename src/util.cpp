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


#include "util.h"
#include <sstream>
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/arrstr.h>
#include <wx/mimetype.h>
#include <wx/clipbrd.h>
#include <wx/list.h>
#include "platform.h"
#include "const.h"

using namespace std;

// Case-insensitive compare
bool CUtil::noCaseEqual(const string& s1, const string& s2) {
    if (s1.size() != s2.size()) return false;
    return equal(s1.begin(), s1.end(), s2.begin(), insensitive_compare());
}

// Returns true if s2 begins with s1
bool CUtil::noCaseBeginsWith(const string& s1, const string& s2) {
    return equal(s1.begin(), s1.end(), s2.begin(), insensitive_compare());
}

// Returns true if s2 contains s1
bool CUtil::noCaseContains(const string& s1, const string& s2) {
    string nc1 = s1, nc2 = s2;
    CUtil::lower(nc1);
    CUtil::lower(nc2);
    return (nc2.find(nc1) != string::npos);
}

// Trim string
string& CUtil::trim(string& s, string list) {
    unsigned int p1 = s.find_first_not_of(list);
    if (p1 == string::npos) return s = "";
    unsigned int p2 = s.find_last_not_of(list);
    return s = s.substr(p1, p2+1-p1);
}

// Decode hexadecimal number at string start
unsigned int CUtil::readHex(const string& s) {
    unsigned int n = 0, h = 0;
    string H("0123456789ABCDEF");
    for (string::const_iterator c = s.begin(); c != s.end()
            && (h = H.find(toupper(*c))) != string::npos; c++)
        n = n*16 + h;
    return n;
}

// Make a hex representation of a number;
string CUtil::makeHex(unsigned int n) {
    stringstream ss;
    ss << uppercase << hex << n;
    return ss.str();
}

// Put string in uppercase
string& CUtil::upper(string& s) {
    for (string::iterator it = s.begin(); it != s.end(); it++) {
        *it = toupper(*it);
    }
    return s;
}

// Put string in lowercase
string& CUtil::lower(string& s) {
    for (string::iterator it = s.begin(); it != s.end(); it++) {
        *it = tolower(*it);
    }
    return s;
}

// Convert a numerical IPV4 address to its dotted representation
string CUtil::toDotted(unsigned long adr) {
    stringstream ss;
    ss << (adr/0x1000000)%0x100 << ".";
    ss << (adr/0x10000)%0x100 << ".";
    ss << (adr/0x100)%0x100 << ".";
    ss << (adr)%0x100;
    return ss.str();
}

// Convert a dotted IPV4 address to its ulong value
unsigned long CUtil::fromDotted(string adr) {
    unsigned long ret = 0, byte = 0;
    for (string::iterator it = adr.begin(); it != adr.end(); it++) {
        if (digit(*it)) {
            byte = byte*10 + *it -'0';
        } else if (*it == '.') {
            ret = (ret + byte)*256;
            byte = 0;
        }
    }
    return ret + byte;
}

// Escape a string
string CUtil::ESC(const string& str) {
    stringstream out;
    static const string ok = "@*-_+./";
    static const string hex = "0123456789ABCDEF";
    for (string::const_iterator c = str.begin(); c != str.end(); c++) {
        if (   *c >= 'A' && *c <= 'Z'
            || *c >= 'a' && *c <= 'z'
            || *c >= '0' && *c <= '9'
            || ok.find_first_of(*c) != string::npos ) {
            out << *c;
        } else {
            out << '%' << hex[*c/16] << hex[*c%16];
        }
    }
    return out.str();
}

// Escape special characters in a string
string CUtil::WESC(const string& str) {
    stringstream out;
    static const string nok = "*+[]()\\\"'|&";
    static const string hex = "0123456789ABCDEF";
    for (string::const_iterator c = str.begin(); c != str.end(); c++) {
        if (nok.find_first_of(*c) != string::npos) out << '\\';
        out << *c;
    }
    return out.str();
}

// Unescape a string
string CUtil::UESC(const string& str) {
    stringstream out;
    static const string hex = "0123456789ABCDEF";
    for (string::const_iterator c = str.begin(); c != str.end(); c++) {
        if (*c == '%' && hex.find(*(c+1)) != string::npos
                      && hex.find(*(c+2)) != string::npos) {
            out << (char)(hex.find(*(c+1))*16+hex.find(*(c+2)));
            c += 2;
        } else {
            out << *c;
        }
    }
    return out.str();
}

// Formats a number
string CUtil::pad(int n, int size) {
    stringstream ss;
    ss << n;
    return string(size-ss.str().size(), '0') + ss.str();
}

// Check if keys are pressed (keys must be in uppercase)
bool CUtil::keyCheck(const string& keys) {
    unsigned int size = keys.size();
    for (unsigned int i = 0; i<size; i++) {
        int key = (int)keys[i];
        int num = 0;
        if (key == '^' && i+1 < size) {
            key = keys[++i];
            switch (key) {
            case 'T' : key = WXK_TAB;     break;
            case 'C' : key = WXK_CONTROL; break;
            case 'A' : key = WXK_MENU;    break;
            case 'S' : key = WXK_SHIFT;   break;
            default : // ^num or ^Fnum
                if (key == 'F') {
                    key = WXK_F1 - 1;
                    ++i;
                } else {
                    key = 0;
                }
                for (; i < size && CUtil::digit(keys[i]); ++i)
                    num = num*10 + keys[i] - '0';
                key += num;
                --i;
            }
        }
        if (!CPlatform::isKeyPressed((wxKeyCode)key)) return false;
    }
    return true;
}

// Finds the first unescaped occurence of a character in a string
unsigned int CUtil::findUnescaped(const string& str, char c) {
    unsigned int pos = str.find(c);
    while (pos != string::npos && pos > 0 && str[pos-1] == '\\')
        pos = str.find(c, pos+1);
    return pos;
}

// Replace all occurences of a string by another
string CUtil::replaceAll(const string& str, string s1, string s2) {
    string ret = str;
    unsigned int pos = ret.find(s1);
    while (pos != string::npos) {
        ret.replace(pos, s1.length(), s2);
        pos = ret.find(s1, pos + s2.length());
    }
    return ret;
}

// Get the content of a binary file
string CUtil::getFile(string filename) {
    trim(filename);
    replaceAll(filename, "\\\\", "/");  // for correctly decoding $FILE()
    filename = CUtil::makePath(filename);
    wxFile file;
    int size;
    if (wxFile::Exists(filename.c_str())
            && file.Open(filename.c_str())
                && (size = file.Length()) > 0) {
        char buf[size];
        size = file.Read(buf, size);
        return string(buf, size);
    }
    return "";
}

// Get MIME type of a file
string CUtil::getMimeType(string filename) {
    unsigned int dot = filename.rfind('.');
    if (dot == string::npos)
        return "application/octet-stream";
    string ext = filename.substr(dot+1);
    wxString mime;
    wxFileType* type = wxTheMimeTypesManager->GetFileTypeFromExtension(ext.c_str());
    if (type == NULL || !type->GetMimeType(&mime))
        return "application/octet-stream";
    return mime.c_str();
}


// Increment a string
string& CUtil::increment(string& str) {
    unsigned int i, j;
    i = str.rfind('(');
    j = str.rfind(')');
    if (i < j && j == str.size()-1 && isUInt(str.substr(i+1, j-i-1))) {
        unsigned int n;
        stringstream ss2, ss(str.substr(i+1, j-i-1));
        ss >> n;
        ss2 << (n+1);
        str = str.substr(0, i+1) + ss2.str() + ")";
    } else {
        str += " (2)";
    }
    return str;
}


// Tell if a string is exclusively composed of digits
bool CUtil::isUInt(string s) {
    if (s.empty()) return false;
    for (string::iterator it = s.begin(); it != s.end(); it++) {
        if (!digit(*it)) return false;
    }
    return true;
}


// Set content of clipboard
void CUtil::setClipboard(const string& str) {
    if (wxTheClipboard->Open()) {
        wxTheClipboard->SetData(new wxTextDataObject(str.c_str()));
        wxTheClipboard->Close();
    }
}


// Get content of clipboard
string CUtil::getClipboard() {
    wxTextDataObject data;
    if (wxTheClipboard->Open()) {
        if (wxTheClipboard->IsSupported( wxDF_TEXT )) {
            wxTheClipboard->GetData( data );
        }
        wxTheClipboard->Close();
    }
    return data.GetText().c_str();
}


// MIME BASE64 encoder/decoder
string CUtil::encodeBASE64(const string& str) {
    string base64("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
    stringstream out;
    int size = (int)str.size();
    for (int i=0; i<size; i+=3) {
        unsigned long code = (unsigned long)str[i] << 16;
        if (i+1 < size) code += (unsigned long)str[i+1] << 8;
        if (i+2 < size) code += (unsigned long)str[i+2];
        out << base64[code >> 18];
        out << base64[(code >> 12) & 0x3F];
        out << (i+1 < size ? base64[(code >> 6) & 0x3F] : '=');
        out << (i+2 < size ? base64[code & 0x3F] : '=');
    }
    return out.str();
}

string CUtil::decodeBASE64(const string& str) {
    string base64("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
    stringstream out;
    int size = (int)str.size();
    int step = 0;
    unsigned long code = 0;
    for (int i=0; i<size; i++) {
        unsigned char c = str[i];
        if (c == '=') break;
        unsigned long v = base64.find(str[i]);
        if (v == string::npos) continue;
        code = (code << 6) + v;
        step += 6;
        if (step >= 8) {
            step -= 8;
            out << (unsigned char)((code >> step) & 0xFF);
        }
    }
    return out.str();
}


// Launch default browser (for a Proximodo help page)
// If path is empty or omitted, just opens the browser to its 
// default home page
void CUtil::openBrowser(const string& path) {

    // Get file type
    wxFileType* type = wxTheMimeTypesManager->GetFileTypeFromMimeType("text/html");
    if (!type) return;

    // Get path string
    wxString pathString;
    // Get command to open given path, if provided
    if (!path.empty()) {
        wxFileName fn(CUtil::makePath(path).c_str());
        fn.MakeAbsolute();
        pathString = fn.GetFullPath();
    }
    
    // Get command
    wxString command;
    if (!type->GetOpenCommand(&command, pathString)) return;
    
    // Remove parameters from command line if path was empty
    if (path.empty())
        command = getExeName(command.c_str()).c_str();
    
    // Execute command
    wxExecute(command, wxEXEC_ASYNC);
}


// Extract the executable name from a command line
string CUtil::getExeName(const string& cmd) {

    string name = cmd;
    if (name.find("WX_DDE#") == 0) {
        name.erase(0, 7);
        name.erase(name.find('#'));
    }
    if (name.find('\"') == 0) {
        name.erase(name.find('\"',1) + 1);
    }
    else if (name.find(' ') != string::npos) {
        name.erase(name.find(' '));
    }
    return name;
}


// Launch default text editor (for a list file)
void CUtil::openNotepad(const string& path) {

    wxFileType* type = wxTheMimeTypesManager->GetFileTypeFromMimeType("text/plain");
    if (!type) return;
    wxFileName fn(CUtil::makePath(path).c_str());
    fn.MakeAbsolute();
    wxString command;
    if (!type->GetOpenCommand(&command, fn.GetFullPath())) return;
    wxExecute(command, wxEXEC_ASYNC);
}


// Function for sorting a list control
int wxCALLBACK CUtil::sortFunction(long item1, long item2, long sortData) {
    return item1 - item2;
}


// Function to show and raise a top-level window
void CUtil::show(wxTopLevelWindow* window) {

        window->Show();
        window->Maximize(false);
        window->Raise();
}


// Converts / to the platform's path separator
string CUtil::makePath(const string& str) {

    unsigned int len = str.length();
    if (!len) return str;
    stringstream sep;
    sep << wxFileName::GetPathSeparator();
    return replaceAll(str, "/", sep.str());
}


// Converts platform's path separators to /
string CUtil::unmakePath(const string& str) {

    unsigned int len = str.length();
    if (!len) return str;
    stringstream sep;
    sep << wxFileName::GetPathSeparator();
    return replaceAll(str, sep.str(), "/");
}


// Quote a string if it contains quotes or special characters
string CUtil::quote(string str, string codes) {

    trim(str);
    if (   str.find("\"") == string::npos
        && str.find_first_of(codes) == string::npos )
        return str;
    else
        return "\"" + replaceAll(str, "\"", "\"\"") + "\"";
}


// Reverse of quote()
string CUtil::unquote(string str) {

    trim(str);
    unsigned int size = str.length();
    if (size < 2 || str[0] != '\"' || str[size-1] != '\"') return str;
    str = replaceAll(str.substr(1, size-2), "\"\"", "\"");
    return trim(str);
}


// Find the end of the possibly-quoted substring and get the unquoted value
int CUtil::getQuoted(const string& str, string& out, int start, char token) {
    // use start=-1 for first attempt, then start, stop when start<0

    if (start < -1) {
        out.clear();
        return start;
    }
    start++;
    unsigned int quote = str.find('\"', start);
    unsigned int stop = str.find(token, start);
    if (quote < stop) {
        unsigned int size = str.length();
        stop = str.find('\"', quote+1);
        while (stop != string::npos && stop+1 < size && str[stop+1] == '\"')
            stop = str.find('\"', stop+2);
        if (stop != string::npos) stop++;
        out = unquote(str.substr(start, stop-start));
    } else {
        trim(out = str.substr(start, stop-start));
    }
    return stop == string::npos ? -2 : (int)stop;
}


// Locates the next end-of-line (can be CRLF or LF)
bool CUtil::endOfLine(const string& str, unsigned int start,
                      unsigned int& pos, unsigned int& len, int nbr) {

    while (true) {
        unsigned int index = str.find(LF, start);
        if (index == string::npos) return false;
        if (start < index && str[index-1] == CR) {
            len = 2; pos = index - 1;
        } else {
            len = 1; pos = index;
        }
        if (nbr <= 1) {
            return true;
        } else {
            int remain = nbr-1;
            string::const_iterator it = str.begin() + (index + 1);
            while (remain && it != str.end()) {
                if (*it == LF) {
                    len++; it++; remain--;
                } else if (*it == CR) {
                    len++; it++;
                } else {
                    break;
                }
            }
            if (remain) {
                start = pos+len;
            } else {
                return true;
            }
        }
    }
}

