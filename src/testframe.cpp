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

#include "testframe.h"
#include <string>
#include <sstream>
#include <wx/sysopt.h>
#include <wx/settings.h>
#include <wx/event.h>
#include <wx/msgdlg.h>
#include <wx/icon.h>
#include "settings.h"
#include "filterowner.h"
#include "matcher.h"
#include "expander.h"
#include "filter.h"
#include "const.h"
#include "log.h"
#include "images/icon32.xpm"

using namespace std;

/* Event table
 */
BEGIN_EVENT_TABLE(CTestFrame, wxFrame)
    EVT_CLOSE(CTestFrame::OnClose)
    EVT_TEXT  (ID_TESTTEXT, CTestFrame::OnCommand)
    EVT_BUTTON(ID_TEST,     CTestFrame::OnCommand)
END_EVENT_TABLE()


/* Constructor
 */
CTestFrame::CTestFrame(CFilterDescriptor* desc)
       : wxFrame((wxFrame *)NULL, wxID_ANY,
                 CSettings::ref().getMessage("TEST_WINDOW_TITLE").c_str(),
                 wxDefaultPosition, wxDefaultSize,
                 wxDEFAULT_FRAME_STYLE |
                 wxTAB_TRAVERSAL |
                 wxNO_FULL_REPAINT_ON_RESIZE |
                 wxCLIP_CHILDREN ),
         current(desc) {

    SetIcon(wxIcon(icon32_xpm));
    CSettings& settings = CSettings::ref();
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(mainSizer);

    testMemo =  new pmTextCtrl(this, ID_TESTTEXT, CLog::ref().testString.c_str(),
        wxPoint(5,5),wxSize(250,60)  ,
        wxTE_MULTILINE);
    mainSizer->Add(testMemo,1,wxGROW | wxALL,5);

    resultMemo =  new pmTextCtrl(this, wxID_ANY, "" ,
        wxPoint(12,75),wxSize(250,60)  ,
        wxTE_MULTILINE | wxTE_READONLY);
    mainSizer->Add(resultMemo,1,wxGROW | wxALL,5);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    mainSizer->Add(buttonSizer,0,wxALIGN_CENTER_HORIZONTAL | wxALL,0);

    pmButton* testButton =  new pmButton(this, ID_TEST,
        settings.getMessage("TEST_WINDOW_TEST_BUTTON").c_str(),
        wxPoint(0,0),wxSize(75,25) );
    buttonSizer->Add(testButton,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
}


/* Destructor
 */
CTestFrame::~CTestFrame() {
}


/* Change current filter
 */
void CTestFrame::setCurrent(CFilterDescriptor* desc) {
    current = desc;
}


/* Hide the window when clicking on the [X] button
 * The window is destroyed by the config screen on its own destruction
 */
void CTestFrame::OnClose(wxCloseEvent& event) {
    Hide();
}


/* Screen containing proxy settings and others
 */
void CTestFrame::OnCommand(wxCommandEvent& event) {
    switch (event.GetId()) {
        case ID_TESTTEXT: {
            CLog::ref().testString = testMemo->GetValue().c_str();
            break;
        }
        case ID_TEST: {
            string errmsg;
            if (!current->isValid(errmsg)) {
                wxMessageBox(errmsg.c_str(), APP_NAME);
                return;
            }
            CFilterOwner owner;
            string url = "http://www.host.org/path/page.html?query=true#anchor";
            owner.url.parseUrl(url);
            CFilterOwner::setHeader(owner.inHeaders, "Host", "www.host.org");
            owner.cnxNumber = 1;
            CFilter filter(owner);
            string text = testMemo->GetValue().c_str();
            stringstream result;
            int end, reached;
            CMatcher matcher(text, current->matchPattern, filter);

            // Test of a text filter.
            // We don't take URL pattern into account, otherwise the process
            // is the same as in CTextFilter (without incomplete buffer
            // considerations)
            if (current->filterType == CFilterDescriptor::TEXT) {

                // for special <start> and <end> filters
                if (current->matchPattern == "<start>" || current->matchPattern == "<end>") {
                    string str = CExpander::expand(current->replacePattern, filter);
                    filter.unlock();
                    if (current->matchPattern == "<start>") {
                        str += text;
                    } else {
                        str = text + str;
                    }
                    resultMemo->SetValue(str.c_str());
                    return;
                }

                bool found = false;
                bool okayChars[256];
                matcher.mayMatch(okayChars);
                CMatcher boundsMatcher(text, current->boundsPattern, filter);
                if (!current->boundsPattern.empty()) {
                    bool tab[256];
                    boundsMatcher.mayMatch(tab);
                    for (int i=0; i<256; i++) okayChars[i] = okayChars[i] && tab[i];
                }
                int size = text.size();
                int done = 0;
                int start = 0;
                int lastEnd = -1;
                while (start < size && !filter.bypassed && !filter.killed) {
                    if (!okayChars[(unsigned char)text[start]]) {
                        ++start;
                        continue;
                    }
                    bool matched;
                    int end, reached;
                    int stop = start + current->windowWidth;
                    if (stop > size) stop = size;
                    filter.clearMemory();
                    if (!current->boundsPattern.empty()) {
                        matched = boundsMatcher.match(start, stop, end, reached);
                        filter.unlock();
                        if (!matched) {
                            ++start;
                            continue;
                        }
                        stop = end;
                    }
                    matched = matcher.match(start, stop, end, reached);
                    filter.unlock();
                    if (!matched
                            || !current->boundsPattern.empty() && end != stop
                            || current->multipleMatches && end <= lastEnd) {
                        ++start;
                        continue;
                    }
                    found = true;
                    result << text.substr(done, start-done);
                    done = start;
                    string str = CExpander::expand(current->replacePattern, filter);
                    filter.unlock();
                    if (current->multipleMatches) {
                        text.replace(start, end-start, str);
                        size = text.size();
                        lastEnd = start + str.size();
                    } else {
                        result << str;
                        if (start == end)
                            ++start;
                        else
                            done = start = end;
                    }
                }
                if (!filter.killed) {
                    if (filter.bypassed) start = size;
                    result << text.substr(done, start-done);
                }
                if (!found) result.str(CSettings::ref().getMessage("TEST_NO_MATCH"));

            // Test of a header filter. Much simpler...
            } else {

                if ((!text.empty() || current->matchPattern.empty())
                    && matcher.match(0, text.size(), end, reached)) {

                    result << CExpander::expand(current->replacePattern, filter);
                } else {
                    result << CSettings::ref().getMessage("TEST_NO_MATCH");
                }
                filter.unlock();
            }
            resultMemo->SetValue(result.str().c_str());
            break;
        }
    }
}

