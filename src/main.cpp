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


#include <wx/app.h>
#include <wx/snglinst.h>
#include "proxy.h"
#include "const.h"
#include "settings.h"
#include "mainframe.h"
#include "util.h"

class ProximodoApp : public wxApp {

public:
    virtual bool OnInit();
    virtual int OnExit();
    ~ProximodoApp();

private:
    wxSingleInstanceChecker* sic;
};


IMPLEMENT_APP(ProximodoApp)


bool ProximodoApp::OnInit(){

    // Check for single application instance
    const wxString name = APP_NAME;
    sic = new wxSingleInstanceChecker(name);
    if (sic->IsAnotherRunning()) return false;

    // Start proxy server
    wxSocketBase::Initialize();
    CProxy::ref().openProxyPort();
    
    // Open welcome html page on first run
    if (CSettings::ref().firstRun) {
        CSettings::ref().save(); // (get rid of firstRun)
        CUtil::openBrowser(CSettings::ref().getMessage("HELP_PAGE_WELCOME"));
    }

    // Create main frame
    CMainFrame* mf = new CMainFrame(wxDefaultPosition);
    if (CSettings::ref().showOnStartup) mf->Show();
    
    return true;
}


int ProximodoApp::OnExit() {

    delete sic;
    return 0;
}


ProximodoApp::~ProximodoApp() {

    CSettings::destroy();
}
