//------------------------------------------------------------------
//
//this file is part of Proximodo
//Copyright (C) 2004-2005 Antony BOUCHER ( kuruden@users.sourceforge.net )
//                        Paul Rupe      ( prupe@users.sourceforgen.net )
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


#include "logframe.h"
#include <string>
#include <sstream>
#include <wx/settings.h>
#include <wx/colour.h>
#include <wx/icon.h>
#include "images/icon32.xpm"
#include "util.h"

using namespace std;

/* Events
 */
BEGIN_EVENT_TABLE(CLogFrame, wxFrame)
    EVT_CLOSE  (CLogFrame::OnClose)
    EVT_SHOW   (CLogFrame::OnShow)
    EVT_BUTTON (ID_CLEARBUTTON, CLogFrame::OnCommand)
    EVT_BUTTON (ID_STARTBUTTON, CLogFrame::OnCommand)
    EVT_PROXY  (CLogFrame::OnProxyEvent)
    EVT_HTTP   (CLogFrame::OnHttpEvent)
    EVT_FILTER (CLogFrame::OnFilterEvent)
END_EVENT_TABLE()


/* Constructor
 */
CLogFrame::CLogFrame() 
       : wxFrame((wxFrame *)NULL, wxID_ANY,
                 CSettings::ref().getMessage("LOG_WINDOW_TITLE").c_str(),
                 wxDefaultPosition, wxDefaultSize,
                 wxDEFAULT_FRAME_STYLE |
                 wxTAB_TRAVERSAL |
                 wxNO_FULL_REPAINT_ON_RESIZE |
                 wxCLIP_CHILDREN ),
         settings(CSettings::ref()) {

    SetIcon(wxIcon(icon32_xpm));
    CSettings& settings = CSettings::ref();
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

    wxBoxSizer* vertSizer = new wxBoxSizer(wxVERTICAL);
    this->SetSizer(vertSizer);

    logText =  new pmTextCtrl(this, wxID_ANY, "" ,
        wxDefaultPosition, wxSize(500,250),
        wxTE_READONLY | wxTE_RICH |  wxTE_MULTILINE | wxTE_DONTWRAP );
    logText->SetBackgroundColour(wxColour(*wxBLACK));
    logText->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY, *wxBLACK,
        wxFont(8, wxDEFAULT, wxNORMAL, wxBOLD)));
    vertSizer->Add(logText,1,wxGROW | wxALL,5);

    wxBoxSizer* lowerSizer = new wxBoxSizer(wxHORIZONTAL);
    vertSizer->Add(lowerSizer,0,wxALIGN_LEFT | wxALL,0);

    startButton =  new pmButton(this, ID_STARTBUTTON,
        settings.getMessage("LOG_BUTTON_START").c_str() );
    lowerSizer->Add(startButton,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    pmButton* clearButton =  new pmButton(this, ID_CLEARBUTTON,
        settings.getMessage("LOG_BUTTON_CLEAR").c_str() );
    lowerSizer->Add(clearButton,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    wxBoxSizer* ckbxSizer1 = new wxBoxSizer(wxVERTICAL);
    lowerSizer->Add(ckbxSizer1,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    httpCheckbox =  new pmCheckBox(this, wxID_ANY,
        settings.getMessage("LOG_CKB_HTTP").c_str() );
    ckbxSizer1->Add(httpCheckbox,0,wxALIGN_LEFT | wxALL,2);

    postCheckbox =  new pmCheckBox(this, wxID_ANY,
        settings.getMessage("LOG_CKB_POST").c_str() );
    ckbxSizer1->Add(postCheckbox,0,wxALIGN_LEFT | wxALL,2);

    browserCheckbox =  new pmCheckBox(this, wxID_ANY,
        settings.getMessage("LOG_CKB_BROWSER").c_str() );
    ckbxSizer1->Add(browserCheckbox,0,wxALIGN_LEFT | wxALL,2);

    wxBoxSizer* ckbxSizer2 = new wxBoxSizer(wxVERTICAL);
    lowerSizer->Add(ckbxSizer2,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    proxyCheckbox =  new pmCheckBox(this, wxID_ANY,
        settings.getMessage("LOG_CKB_PROXY").c_str() );
    ckbxSizer2->Add(proxyCheckbox,0,wxALIGN_LEFT | wxALL,2);

    filterCheckbox =  new pmCheckBox(this, wxID_ANY,
        settings.getMessage("LOG_CKB_FILTER").c_str() );
    ckbxSizer2->Add(filterCheckbox,0,wxALIGN_LEFT | wxALL,2);

    logCheckbox =  new pmCheckBox(this, wxID_ANY,
        settings.getMessage("LOG_CKB_LOG").c_str() );
    ckbxSizer2->Add(logCheckbox,0,wxALIGN_LEFT | wxALL,2);

    active = false;
    httpCheckbox->SetValue(true);
    browserCheckbox->SetValue(true);
    logCheckbox->SetValue(true);
    filterCheckbox->SetValue(true);

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);
}


/* Destructor
 */
CLogFrame::~CLogFrame() {
}


/* Hide the window when clicking on the [X] button
 */
void CLogFrame::OnClose(wxCloseEvent& event) {

    Hide();
    if (active) {
        startButton->SetLabel(settings.getMessage("LOG_BUTTON_START").c_str());
        active = false;
    }
}


/* Turn on the display when the window appears
 */
void CLogFrame::OnShow(wxShowEvent& event) {

    startButton->SetLabel(settings.getMessage("LOG_BUTTON_STOP").c_str());
    active = true;
}


/* Event handling functions
 */
void CLogFrame::OnCommand(wxCommandEvent& event) {

    switch (event.GetId()) {
    case ID_STARTBUTTON:
        {
            if (active)
                startButton->SetLabel(settings.getMessage("LOG_BUTTON_START").c_str());
            else
                startButton->SetLabel(settings.getMessage("LOG_BUTTON_STOP").c_str());
            active = !active;
            break;
        }
    case ID_CLEARBUTTON:
        logText->Clear(); break;
        
    default:
        event.Skip();
    }
}


/* Log event listeners
 */
void CLogFrame::OnProxyEvent(CProxyEvent& evt) {

    evt.Skip();
    // Exclude unwanted events
    if (!active || !proxyCheckbox->GetValue()) return;
    
    // Proxy events are grey
    logText->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY));

    stringstream port;
    port << evt.addr.Service();
    
    string mess;
    switch (evt.type) {
    case pmEVT_PROXY_TYPE_START:
        mess = "EVT_PROXY_START";   break;
        
    case pmEVT_PROXY_TYPE_STOP:
        mess = "EVT_PROXY_STOP";    break;
        
    case pmEVT_PROXY_TYPE_ACCEPT:
        mess = "EVT_PROXY_ACCEPT";  break;
        
    case pmEVT_PROXY_TYPE_REFUSE:
        mess = "EVT_PROXY_REFUSE";  break;
        
    case pmEVT_PROXY_TYPE_NEWREQ:
        mess = "EVT_PROXY_NEWREQ";  break;
        
    case pmEVT_PROXY_TYPE_ENDREQ:
        mess = "EVT_PROXY_ENDREQ";  break;
        
    case pmEVT_PROXY_TYPE_NEWSOCK:
        mess = "EVT_PROXY_NEWSOCK"; break;
        
    case pmEVT_PROXY_TYPE_ENDSOCK:
        mess = "EVT_PROXY_ENDSOCK"; break;
        
    default:
        return;
    }

    logText->AppendText(settings.getMessage(mess, port.str()).c_str());
    logText->AppendText("\n");
    logText->ShowPosition(logText->GetInsertionPoint());
}

void CLogFrame::OnHttpEvent(CHttpEvent& evt) {

    evt.Skip();
    // Exclude unwanted events
    if (   !active
        || !httpCheckbox->GetValue()
        || !postCheckbox->GetValue()
           && evt.type == pmEVT_HTTP_TYPE_POSTOUT
        || browserCheckbox->GetValue()
           && (   evt.type == pmEVT_HTTP_TYPE_SENDOUT
               || evt.type == pmEVT_HTTP_TYPE_RECVIN)
        || !browserCheckbox->GetValue()
           && (   evt.type == pmEVT_HTTP_TYPE_RECVOUT
               || evt.type == pmEVT_HTTP_TYPE_SENDIN)   )
        return;

    // Outgoing HTTP events are red, Incoming are green
    if (evt.type == pmEVT_HTTP_TYPE_RECVIN || evt.type == pmEVT_HTTP_TYPE_SENDIN)
        logText->SetDefaultStyle(wxTextAttr(*wxGREEN));
    else
        logText->SetDefaultStyle(wxTextAttr(*wxRED));

    stringstream port;
    port << evt.addr.Service();
    stringstream req;
    req << evt.req;

    string mess;
    switch (evt.type) {
    case pmEVT_HTTP_TYPE_RECVOUT:
        mess = "EVT_HTTP_RECVOUT";  break;

    case pmEVT_HTTP_TYPE_SENDOUT:
        mess = "EVT_HTTP_SENDOUT";  break;

    case pmEVT_HTTP_TYPE_RECVIN:
        mess = "EVT_HTTP_RECVIN";   break;

    case pmEVT_HTTP_TYPE_SENDIN:
        mess = "EVT_HTTP_SENDIN";   break;

    case pmEVT_HTTP_TYPE_POSTOUT:
        mess = "EVT_HTTP_POSTOUT";  break;

    default:
        return;
    }

    logText->AppendText(settings.getMessage(mess, req.str(), port.str(),
                                            evt.text).c_str());
    logText->AppendText("\n");
    logText->ShowPosition(logText->GetInsertionPoint());
}

void CLogFrame::OnFilterEvent(CFilterEvent& evt) {

    evt.Skip();
    
    if (evt.type == pmEVT_FILTER_TYPE_LOGCOMMAND) {
    
        // If a Log event starts with ! the window must appear and show it
        if (evt.text.length() > 0 && evt.text[0] == '!') {

            evt.text.erase(0, 1);
            logCheckbox->SetValue(true);
            startButton->SetLabel(settings.getMessage("LOG_BUTTON_STOP").c_str());
            active = true;
            CUtil::show(this);
        }

        // Exclude unwanted events
        if (!active || !logCheckbox->GetValue() || evt.text.length() < 2) return;

        stringstream req;
        req << evt.req;

        // The first line is yellow
        logText->SetDefaultStyle(wxTextAttr(wxColour(255,255,0)));
        logText->AppendText(settings.getMessage("EVT_FILTER_LOG", req.str(),
                                                evt.title).c_str());
        logText->AppendText("\n");
        
        // The second line's color depends on the first character
        switch (evt.text[0]) {
            case 'R': logText->SetDefaultStyle(*wxRED);   break;
            case 'W': logText->SetDefaultStyle(*wxWHITE); break;
            case 'B': logText->SetDefaultStyle(*wxBLUE);  break;
            case 'G': logText->SetDefaultStyle(*wxGREEN); break;
            case 'C': logText->SetDefaultStyle(*wxCYAN);  break;
            case 'w': logText->SetDefaultStyle(*wxLIGHT_GREY); break;
            case 'Y': logText->SetDefaultStyle(wxTextAttr(wxColour(255,255,0))); break;
            case 'V': logText->SetDefaultStyle(wxTextAttr(wxColour(255,0,255))); break;
        }
        logText->AppendText(evt.text.substr(1).c_str());
        logText->AppendText("\n");
        logText->ShowPosition(logText->GetInsertionPoint());
    
    } else {

        // Exclude unwanted events
        if (!active || !filterCheckbox->GetValue()) return;

        // Filter events are yellow
        logText->SetDefaultStyle(wxTextAttr(wxColour(255,255,0)));

        stringstream req;
        req << evt.req;

        string mess;
        switch (evt.type) {
        case pmEVT_FILTER_TYPE_HEADERMATCH:
            mess = "EVT_FILTER_HMATCH";    break;

        case pmEVT_FILTER_TYPE_HEADERREPLACE:
            mess = "EVT_FILTER_HREPLACE";  break;

        case pmEVT_FILTER_TYPE_TEXTMATCH:
            mess = "EVT_FILTER_TMATCH";    break;

        case pmEVT_FILTER_TYPE_TEXTREPLACE:
            mess = "EVT_FILTER_TREPLACE";  break;

        default:
            return;
        }

        logText->AppendText(settings.getMessage(mess, req.str(), evt.title,
                                                evt.text).c_str());
        logText->AppendText("\n");
        logText->ShowPosition(logText->GetInsertionPoint());
    }
}

