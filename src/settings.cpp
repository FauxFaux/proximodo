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


#include "settings.h"
#include <sstream>
#include <wx/file.h>
#include <wx/msgdlg.h>
#include "util.h"
#include "const.h"
#include "matcher.h"

using namespace std;


/* The address of the instance
 */
CSettings* CSettings::instance = NULL;


/* Method to obtain the instance address
 */
CSettings& CSettings::ref() {

    if (!instance) {
        instance = new CSettings();
        instance->init();
    }
    return *instance;
}


/* Method to destroy safely the instance (better than delete)
 */
void CSettings::destroy() {

    if (instance) delete instance;
    instance = NULL;
}


/* Constructor
 */
CSettings::CSettings() {

    // Default values
    proxyPort     = "8080";
    useNextProxy  = false;
    nextProxy     = "localhost:8088";
    allowIPRange  = false;
    minIPRange    = 0x00000000;
    maxIPRange    = 0x00000000;
    currentConfig = "default";
    filterIn      = true;
    filterOut     = true;
    filterText    = true;
    filterGif     = true;
    language      = DEFAULT_LANGUAGE;
    showOnStartup = true;
    firstRun      = false;
    modified      = false;
}


/* Destructor
 */
CSettings::~CSettings() {

    // (Nothing to do)
}


/* Load all files.
 */
void CSettings::init() {

    loadSettings();
    loadMessages(DEFAULT_LANGUAGE);
    loadMessages(language);
    loadFilters();
    loadLists();
}


/* Checks if settings have been saved, or asks the user.
 */
void CSettings::save(bool prompt) {

    if (prompt) {
        if (!modified) return;
        int ret = wxMessageBox(getMessage("SETTINGS_NOT_SAVED").c_str(),
                                                APP_NAME, wxYES_NO);
        if (ret == wxNO) return;
    }
    saveSettings();
    saveFilters();
    modified = false;
}


/* Loads everything except messages (we don't want GUI to be partly translated)
 */
void CSettings::load() {

    loadSettings();
    loadFilters();
    loadLists();
    modified = false;
}


/* Read a setting from a text file.
 * If the line is [title]\n only title (trimmed) is set.
 * If the line is label=value\n optionally followed by several _value\n
 * only label (trimmed and uppercased) and value (trimmed if needed) are set.
 * returns true if the line conformed one of those two formats.
 */
bool CSettings::readSetting(wxTextFile& f, string& title,
                            string& label, string& value, bool trimValue) {

    title = label = value = "";
    string line = f.GetLine(f.GetCurrentLine()).c_str();
    f.GoToLine(f.GetCurrentLine() + 1);
    // Check for title
    if (line[0] == '[') {
        unsigned int end = line.find(']', 1);
        if (end == string::npos) return false;
        title = line.substr(1, end-1);
        CUtil::trim(title, " \t");
        return true;
    }
    // Check for label
    unsigned int eq = line.find('=');
    if (eq == string::npos) return false;
    label = line.substr(0, eq);
    CUtil::trim(label, " \t");
    CUtil::upper(label);
    // Get value
    value = line.substr(eq+1);
    // Add next _lines
    while (f.GetCurrentLine() < f.GetLineCount()) {
        line = f.GetLine(f.GetCurrentLine());
        if (line[0] != '_') break;
        value += "\n" + line.substr(1);
        f.GoToLine(f.GetCurrentLine() + 1);
    }
    if (trimValue) CUtil::trim(value);
    return true;
}


/* Add a line to a file
 */
void CSettings::addLine(wxTextFile& f, string s) {

    unsigned int pos1 = 0, pos2;
    while ((pos2 = s.find("\n", pos1)) != string::npos) {
        f.AddLine(((pos1?"_":"")+s.substr(pos1, pos2-pos1)).c_str());
        pos1 = pos2+1;
    }
    f.AddLine(((pos1?"_":"")+s.substr(pos1)).c_str());
}


/* Save settings
 */
void CSettings::saveSettings() {

    wxTextFile f(FILE_SETTINGS);
    if (!f.Create()) {
        f.Open();
        f.Clear();
    }
    
    addLine (f,"[Settings]");
    addLine (f, "ShowOnStartup = " + string(showOnStartup ? "yes" : "no"));
    addLine (f, "Port = " + proxyPort);        // For file readability only
    addLine (f, "UseProxy = " + string(useNextProxy ? "yes" : "no"));
    addLine (f, "CurrentProxy = " + nextProxy);
    addLine (f, "AllowIPRange = " + string(allowIPRange ? "yes" : "no"));
    addLine (f, "MinIP = " + CUtil::toDotted(minIPRange));
    addLine (f, "MaxIP = " + CUtil::toDotted(maxIPRange));
    addLine (f, "FilterIn = " + string(filterIn ? "yes" : "no"));
    addLine (f, "FilterOut = " + string(filterOut ? "yes" : "no"));
    addLine (f, "FilterText = " + string(filterText ? "yes" : "no"));
    addLine (f, "FilterGif = " + string(filterGif ? "yes" : "no"));
    addLine (f, "Language = " + language);
    addLine (f, "CurrentConfig = " + currentConfig);
    addLine (f, "Bypass = " + bypass);
    addLine (f, "");
    addLine (f, "[Proxies]");
    for (set<string>::iterator it = proxies.begin();
                it != proxies.end(); it++) {
        addLine (f, "Proxy = " + *it);
    }
    addLine (f, "");
    addLine (f, "[Lists]");
    for (map<string,string>::iterator it = listNames.begin();
                it != listNames.end(); it++) {
        addLine (f, "List = " + it->first + ", " + it->second);
    }
    for (map<string, vector<string> >::iterator it1 = config.begin();
                it1 != config.end(); it1++) {
        addLine (f, "");
        addLine (f, "[" + it1->first + "]");
        for (vector<string>::iterator it2 = it1->second.begin();
                it2 != it1->second.end(); it2++) {
            addLine (f, "Config = " + it1->first + ", " + *it2);
        }
    }
    
    f.Write();
    f.Close();
}


/* Load settings
 */
void CSettings::loadSettings() {

    config.clear();
    proxies.clear();
    listNames.clear();
    
    wxTextFile f(FILE_SETTINGS);
    if (!f.Open()) return;
    while (f.GetCurrentLine() < f.GetLineCount()) {
        string title, label, value;
        readSetting(f, title, label, value, true);
        
        if      (label == "PORT")
            proxyPort = value;
        else if (label == "SHOWONSTARTUP")
            showOnStartup = (toupper(value[0])=='Y');
        else if (label == "FIRSTRUN")
            firstRun = true;
        else if (label == "USEPROXY")
            useNextProxy = (toupper(value[0])=='Y');
        else if (label == "CURRENTPROXY")
            nextProxy = value;
        else if (label == "ALLOWIPRANGE")
            allowIPRange = (toupper(value[0])=='Y');
        else if (label == "FILTERIN")
            filterIn = (toupper(value[0])=='Y');
        else if (label == "FILTEROUT")
            filterOut = (toupper(value[0])=='Y');
        else if (label == "FILTERTEXT")
            filterText = (toupper(value[0])=='Y');
        else if (label == "FILTERGIF")
            filterGif = (toupper(value[0])=='Y');
        else if (label == "MINIP")
            minIPRange = CUtil::fromDotted(value);
        else if (label == "MAXIP")
            maxIPRange = CUtil::fromDotted(value);
        else if (label == "LANGUAGE")
            language = value;
        else if (label == "PROXY")
            proxies.insert(value);
        else if (label == "BYPASS") {
            if (CMatcher::testPattern(value)) bypass = value;
        }
        else if (label == "CURRENTCONFIG") {
            currentConfig = value;
            config[value];
        }
        else if (label == "LIST") {
            unsigned int comma = value.find(",");
            if (comma == string::npos) continue;
            string name = value.substr(0, comma);
            string path = value.substr(comma + 1);
            CUtil::trim(name);
            CUtil::trim(path);
            if (name.empty() || path.empty()) continue;
            listNames[name] = path;
        }
        else if (label == "CONFIG") {
            unsigned int comma = value.find(",");
            if (comma == string::npos) continue;
            string name = value.substr(0, comma);
            string title = value.substr(comma + 1);
            CUtil::trim(name);
            CUtil::trim(title);
            if (name.empty() || title.empty()) continue;
            config[name].push_back(title);
        }
    }
    f.Close();
}


/* Load localized messages for use in GUI.
 * It overloads messages currently in memory but keeps those that are
 * not present in the file (so that default messages appear for
 * non-translated ones).
 * Note that all message labels are in uppercase.
 */
void CSettings::loadMessages(string language) {

    wxTextFile f((language+".lng").c_str());
    if (!f.Open()) return;
    while (f.GetCurrentLine() < f.GetLineCount()) {
        string title, label, value;
        readSetting(f, title, label, value, false);
        if (!label.empty()) messages[label] = value;
    }
    f.Close();
}


/* This function retrieves a message with the label, and replaces
 * parameters by those given, if needed.
 * Places in message where to insert label is indicated by %1 %2 and %3
 */
string CSettings::getMessage(string name,
                string param1, string param2, string param3, string param4) {

    if (messages.find(name) == messages.end()) return name;
    string mess = messages[name];
    unsigned int pos = 0;
    while ((pos = mess.find("%1", pos)) != string::npos) {
        mess.replace(pos, 2, param1);
        pos += param1.size();
    }
    while ((pos = mess.find("%2", pos)) != string::npos) {
        mess.replace(pos, 2, param2);
        pos += param2.size();
    }
    while ((pos = mess.find("%3", pos)) != string::npos) {
        mess.replace(pos, 2, param3);
        pos += param3.size();
    }
    while ((pos = mess.find("%4", pos)) != string::npos) {
        mess.replace(pos, 2, param4);
        pos += param4.size();
    }
    return mess;
}

string CSettings::getMessage(string name, int number) {

    stringstream ss;
    ss << number;
    return getMessage(name, ss.str());
}


/* Load all files of patterns.
 */
void CSettings::loadLists() {

    lists.clear();
    for (map<string,string>::iterator it = listNames.begin();
                it != listNames.end(); it++) {
        loadList(it->first);
    }
}


/* Load a file of patterns.
 * Blank lines, lines starting with # and invalid patterns are ignored.
 */
void CSettings::loadList(string name) {

    wxTextFile f(listNames[name].c_str());
    vector<string> patterns;
    if (f.Open()) {
        f.AddLine("");  // (so that we don't need post-loop processing)
        string pattern;
        while (f.GetCurrentLine() < f.GetLineCount()) {
            string line = f.GetLine(f.GetCurrentLine()).c_str();
            f.GoToLine(f.GetCurrentLine() + 1);

            if (!line.empty() && (line[0] == ' ' || line[0] == '\t')) {
                pattern += line;
            } else {
                CUtil::trim(pattern);
                if (!pattern.empty() && CMatcher::testPattern(pattern))
                    patterns.push_back(pattern);
                if (line.empty() || line[0] == '#') {
                    pattern.clear();
                } else {
                    pattern = line;
                }
            }
        }
        f.Close();
    }
    lists[name] = patterns;
}


/* Add a line to a list.
 * The insertion is done in memory (if valid pattern) and on HDD.
 */
void CSettings::addListLine(string name, string line) {

    // We don't add if the list does not exist or the line is empty
    if (listNames.find(name) == listNames.end()) return;
    if (CUtil::trim(line).empty()) return;
    
    // multiline patterns are converted to single line, to avoid
    // the danger of having the 2nd+ line not beginning by a space
    line = CUtil::replaceAll(line, "\r", "");
    line = CUtil::replaceAll(line, "\n", "");

    // Append line to file
    wxTextFile f(listNames[name].c_str());
    if (!f.Open()) f.Create();
    f.AddLine(line.c_str());
    f.Write();
    f.Close();
    
    // If line is a valid pattern, keep it in memory
    if (line[0] != '#' && CMatcher::testPattern(line))
        lists[name].push_back(line);
}


/* Save filters
 */
void CSettings::saveFilters() {

    wxFile f(FILE_FILTERS, wxFile::write);
    if (f.IsOpened()) {
        f.Write(CFilterDescriptor::exportFilters(filterbank).c_str());
    }
}


/* Load filters
 */
void CSettings::loadFilters() {

    // Load file content into memory
    stringstream text;
    wxFile f(FILE_FILTERS);
    if (f.IsOpened()) {
        while (!f.Eof()) {
            char buf[1024];
            unsigned int n = f.Read(buf, 1024);
            text << string(buf, n);
        }
    }

    // Decode filters into a temporary filter bank
    map<string, CFilterDescriptor> bank;
    CFilterDescriptor::importFilters(text.str(), bank);

    // Transfer them one by one into settings' filter bank
    filterbank.clear();
    for (map<string, CFilterDescriptor>::iterator it = bank.begin();
                it != bank.end(); it++) {

        // Change titles, comments and categories of (unmodified) default filters
        // according to language file
        string oldTitle = it->second.title;
        if (it->second.defaultFilter) {
            stringstream prefix;
            prefix << "FILTER_" << it->second.defaultFilter << "_";
            string mess;
            mess = getMessage(prefix.str() + "TITLE");
            if (mess != prefix.str() + "TITLE")
                it->second.title = mess;
            mess = getMessage(prefix.str() + "CATEGORY");
            if (mess != prefix.str() + "CATEGORY")
                it->second.category = mess;
            mess = getMessage(prefix.str() + "COMMENT");
            if (mess != prefix.str() + "COMMENT")
                it->second.comment = mess;
        }

        // Ensure new titles do not cause overrides
        while (filterbank.find(it->second.title) != filterbank.end())
            CUtil::increment(it->second.title);

        // Insert into filterbank
        filterbank[it->second.title] = it->second;

        // Update title in configurations
        if (oldTitle != it->second.title) {
            for (map<string, vector<string> >::iterator itm = config.begin();
                    itm != config.end(); itm++) {
                for (vector<string>::iterator itv = itm->second.begin();
                        itv != itm->second.end(); itv++) {
                    if (*itv == oldTitle) *itv = it->second.title;
                }
            }
        }
    }
}

