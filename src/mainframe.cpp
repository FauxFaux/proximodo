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


#include "mainframe.h"

#include "settings.h"
#include "proxy.h"
#include "log.h"
#include "util.h"
#include "const.h"

#include "welcomescreen.h"
#include "settingsscreen.h"
#include "configscreen.h"
#include "logframe.h"
#include "trayicon.h"

#include "images/btn_settings.xpm"
#include "images/btn_log.xpm"
#include "images/btn_config.xpm"
#include "images/btn_help.xpm"
#include "images/btn_monitor.xpm"
#include "images/btn_quit.xpm"
#include "images/icon32.xpm"

#include <wx/sysopt.h>
#include <wx/menu.h>
#include <wx/icon.h>
#include <wx/statusbr.h>
#include <wx/settings.h>
#include <wx/msgdlg.h>
#include <wx/app.h>
#include <wx/toolbar.h>
#include <sstream>

/* Event table
 */
BEGIN_EVENT_TABLE(CMainFrame, wxFrame)
    EVT_ICONIZE   (CMainFrame::OnIconize)
    EVT_PROXY     (CMainFrame::OnProxyEvent)
    EVT_STATUS    (CMainFrame::OnStatusEvent)
    EVT_TOOL_RANGE(ID_SETTINGS,      ID_QUIT,       CMainFrame::OnCommand)
    EVT_MENU_RANGE(ID_TOOLSSETTINGS, ID_HELPSYNTAX, CMainFrame::OnCommand)
END_EVENT_TABLE()


/* Saved window position
 */
int CMainFrame::savedX = BIG_NUMBER,
    CMainFrame::savedY = 0;


/* Constructor
 */
CMainFrame::CMainFrame(const wxPoint& position)
       : wxFrame((wxFrame *)NULL, wxID_ANY, APP_NAME,
                 position, wxDefaultSize,
                 wxDEFAULT_FRAME_STYLE |
                 wxTAB_TRAVERSAL |
                 wxCLIP_CHILDREN ) {

    SetIcon(wxIcon(icon32_xpm));
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    CSettings& settings = CSettings::ref();

    // Toolbar
    wxToolBar* toolbar = CreateToolBar(wxTB_TEXT | wxTB_FLAT);
    toolbar->AddTool(ID_MONITOR,
        settings.getMessage("BUTTON_MONITOR").c_str(), btn_monitor_xpm);
    toolbar->SetToolLongHelp(ID_MONITOR,
        settings.getMessage("BUTTON_MONITOR_TIP").c_str());
    toolbar->AddTool(ID_LOG,
        settings.getMessage("BUTTON_LOG").c_str(), btn_log_xpm);
    toolbar->SetToolLongHelp(ID_LOG,
        settings.getMessage("BUTTON_LOG_TIP").c_str());
    toolbar->AddTool(ID_CONFIG,
        settings.getMessage("BUTTON_CONFIG").c_str(), btn_config_xpm);
    toolbar->SetToolLongHelp(ID_CONFIG,
        settings.getMessage("BUTTON_CONFIG_TIP").c_str());
    toolbar->AddTool(ID_SETTINGS,
        settings.getMessage("BUTTON_SETTINGS").c_str(), btn_settings_xpm);
    toolbar->SetToolLongHelp(ID_SETTINGS,
        settings.getMessage("BUTTON_SETTINGS_TIP").c_str());
    toolbar->AddTool(ID_HELP,
        settings.getMessage("BUTTON_HELP").c_str(), btn_help_xpm);
    toolbar->SetToolLongHelp(ID_HELP,
        settings.getMessage("BUTTON_HELP_TIP").c_str());
    toolbar->AddTool(ID_QUIT,
        settings.getMessage("BUTTON_QUIT").c_str(), btn_quit_xpm);
    toolbar->SetToolLongHelp(ID_QUIT,
        settings.getMessage("BUTTON_QUIT_TIP").c_str());
    toolbar->SetToolBitmapSize(wxSize(48, 40));
    toolbar->Realize();

    // Status bar
    statusbar = CreateStatusBar(3);
    int widths[3] = { -2, 60, -1 };
    statusbar->SetStatusWidths(3, widths);
    CLog::ref().proxyListeners.insert(this);
    updateStatusBar();

    // Menu bar
    wxMenuBar* menubar = new wxMenuBar();
    wxMenu* menuTools = new wxMenu();
    menuTools->Append(ID_TOOLSMONITOR,
        settings.getMessage("MENU_TOOLSMONITOR").c_str(),
        settings.getMessage("MENU_TOOLSMONITOR_TIP").c_str());
    menuTools->Append(ID_TOOLSSETTINGS,
        settings.getMessage("MENU_TOOLSSETTINGS").c_str(),
        settings.getMessage("MENU_TOOLSSETTINGS_TIP").c_str());
    menuTools->Append(ID_TOOLSCONFIG,
        settings.getMessage("MENU_TOOLSCONFIG").c_str(),
        settings.getMessage("MENU_TOOLSCONFIG_TIP").c_str());
    menuTools->Append(ID_TOOLSLOG,
        settings.getMessage("MENU_TOOLSLOG").c_str(),
        settings.getMessage("MENU_TOOLSLOG_TIP").c_str());
    menuTools->AppendSeparator();
    menuTools->Append(ID_TOOLSSAVE,
        settings.getMessage("MENU_TOOLSSAVE").c_str(),
        settings.getMessage("MENU_TOOLSSAVE_TIP").c_str());
    menuTools->Append(ID_TOOLSRELOAD,
        settings.getMessage("MENU_TOOLSRELOAD").c_str(),
        settings.getMessage("MENU_TOOLSRELOAD_TIP").c_str());
    menuTools->AppendSeparator();
    menuTools->Append(ID_TOOLSICONIZE,
        settings.getMessage("MENU_TOOLSICONIZE").c_str(),
        settings.getMessage("MENU_TOOLSICONIZE_TIP").c_str());
    menuTools->Append(ID_TOOLSQUIT,
        settings.getMessage("MENU_TOOLSQUIT").c_str(),
        settings.getMessage("MENU_TOOLSQUIT_TIP").c_str());
    menubar->Append(menuTools,
        settings.getMessage("MENU_TOOLS").c_str());
    wxMenu* menuHelp = new wxMenu();
    menuHelp->Append(ID_HELPCONTENT,
        settings.getMessage("MENU_HELPCONTENT").c_str(),
        settings.getMessage("MENU_HELPCONTENT_TIP").c_str());
    menuHelp->Append(ID_HELPSYNTAX,
        settings.getMessage("MENU_HELPSYNTAX").c_str(),
        settings.getMessage("MENU_HELPSYNTAX_TIP").c_str());
    menuHelp->AppendSeparator();
    menuHelp->Append(ID_HELPABOUT,
        settings.getMessage("MENU_HELPABOUT").c_str(),
        settings.getMessage("MENU_HELPABOUT_TIP").c_str());
    menuHelp->Append(ID_HELPLICENSE,
        settings.getMessage("MENU_HELPLICENSE").c_str(),
        settings.getMessage("MENU_HELPLICENSE_TIP").c_str());
    menubar->Append(menuHelp,
        settings.getMessage("MENU_HELP").c_str());
    SetMenuBar(menubar);

    // Display welcome screen
    content = new CWelcomeScreen(this);
    
    // Reload window position
    if (savedX == BIG_NUMBER) {
        Centre(wxBOTH | wxCENTRE_ON_SCREEN);
    } else {
        Move(savedX, savedY);
    }
}


/* Destructor
 */
CMainFrame::~CMainFrame() {

    // Save window position
    GetPosition(&savedX, &savedY);
    
    CLog::ref().proxyListeners.erase(this);
    SetSizer(content = NULL);
    CLog::ref().mainFrame = NULL;
}


/* Closes window when clicking on the [_] button (same as [X])
 * Only the tray icon remains.
 */
void CMainFrame::OnIconize(wxIconizeEvent& event) {

    if (event.Iconized()) Close();
}


/* Event handling for updating status bar's text
 */
void CMainFrame::OnStatusEvent(CStatusEvent& event) {

    if (event.field < 0 && event.field > 2) return;
    if (event.text == statusbar->GetStatusText(event.field).c_str()) return;
    statusbar->SetStatusText(event.text.c_str(), event.field);
}


/* Event handling for updating status bar's statistics
 */
void CMainFrame::OnProxyEvent(CProxyEvent& event) {

    updateStatusBar();
}


/* Update status bar's statistics
 */
void CMainFrame::updateStatusBar() {

    int num = CLog::ref().numActiveRequests;
    string text = (num>1 ? "STAT_ACTIVE_REQS" : "STAT_ACTIVE_REQ");
    stringstream ss; ss << num << "/" << CLog::ref().numOpenSockets;
    text = CSettings::ref().getMessage(text, ss.str());
    statusbar->SetStatusText(text.c_str(), 2);
}


/* Process menu events and toolbar events
 */
void CMainFrame::OnCommand(wxCommandEvent& event) {

    CSettings& settings = CSettings::ref();
    
    switch (event.GetId()) {

    case ID_MONITOR:
    case ID_TOOLSMONITOR:
        content = new CWelcomeScreen(this); break;

    case ID_SETTINGS:
    case ID_TOOLSSETTINGS:
        content = new CSettingsScreen(this); break;

    case ID_CONFIG:
    case ID_TOOLSCONFIG:
        content = new CConfigScreen(this); break;

    case ID_LOG:
    case ID_TOOLSLOG:
        if (!CLog::ref().logFrame)
            CLog::ref().logFrame = new CLogFrame();
        CUtil::show(CLog::ref().logFrame);
        break;

    case ID_TOOLSSAVE:
        if (content) content->apply(false);
        settings.save();
        break;

    case ID_TOOLSRELOAD:
        if (content) content->apply(false);
        settings.save(true);
        // Stop proxy
        CProxy::ref().closeProxyPort();
        CProxy::ref().refreshManagers();
        // Load settings and filters
        settings.load();
        if (content) content->revert(false);
        // Restart proxy
        CProxy::ref().openProxyPort();
        break;

    case ID_QUIT:
    case ID_TOOLSQUIT:
        if (CLog::ref().trayIcon) delete CLog::ref().trayIcon;
        if (CLog::ref().logFrame) CLog::ref().logFrame->Destroy();
        if (CLog::ref().mainFrame) CLog::ref().mainFrame->Destroy();
        break;

    case ID_TOOLSICONIZE:
        Close();
        break;

    case ID_HELP:
    case ID_HELPCONTENT:
        CUtil::openBrowser(settings.getMessage("HELP_PAGE_CONTENT"));
        break;

    case ID_HELPSYNTAX:
        CUtil::openBrowser(settings.getMessage("HELP_PAGE_SYNTAX"));
        break;

    case ID_HELPLICENSE:
        CUtil::openBrowser(settings.getMessage("HELP_PAGE_LICENSE"));
        break;

    case ID_HELPABOUT:
        wxMessageBox(settings.getMessage("HELP_ABOUT_TEXT", APP_VERSION).c_str(),
                     APP_NAME);
        break;

    default:
        event.Skip();
    }
}

