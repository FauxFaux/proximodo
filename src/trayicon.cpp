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


#include "trayicon.h"
#include <wx/menu.h>
#include <vector>
#include <map>
#include <string>
#include "images/icon32.xpm"
#include "settings.h"
#include "log.h"
#include "proxy.h"
#include "mainframe.h"
#include "util.h"
#include "const.h"

using namespace std;

BEGIN_EVENT_TABLE(CTrayIcon, wxTaskBarIcon)
    EVT_TASKBAR_LEFT_DOWN (CTrayIcon::OnClick)
    EVT_MENU_RANGE (ID_OPEN, ID_OPEN+200, CTrayIcon::OnCommand)
END_EVENT_TABLE()


/* Constructor
 */
CTrayIcon::CTrayIcon(CMainFrame* mainFrame) :
            wxTaskBarIcon(), mainFrame(mainFrame) {

    SetIcon(wxIcon(icon32_xpm), APP_NAME);
}


/* Destructor
 */
CTrayIcon::~CTrayIcon() {
}


/* Opens the main frame (the user left click on the tray icon)
 */
void CTrayIcon::OnClick(wxTaskBarIconEvent& event) {

    CUtil::show(mainFrame);
}


/* Processes events from menu items
 */
void CTrayIcon::OnCommand(wxCommandEvent& event) {

    CSettings& settings = CSettings::ref();

    switch (event.GetId()) {
    case ID_OPEN:
        CUtil::show(mainFrame);
        break;

    case ID_LOG:
        CUtil::show(CLog::ref().logFrame);
        break;

    case ID_EXIT:
        mainFrame->Destroy();
        break;

    case ID_FILTERNONE:
        settings.filterIn = false;
        settings.filterOut = false;
        settings.filterText = false;
        settings.filterGif = false;
        break;
        
    case ID_FILTERALL:
        settings.filterIn = true;
        settings.filterOut = true;
        settings.filterText = true;
        settings.filterGif = true;
        break;

    case ID_FILTERIN:
        settings.filterIn = !settings.filterIn;
        break;

    case ID_FILTEROUT:
        settings.filterOut = !settings.filterOut;
        break;

    case ID_FILTERTEXT:
        settings.filterText = !settings.filterText;
        break;

    case ID_FILTERGIF:
        settings.filterGif = !settings.filterGif;
        break;

    case ID_USEPROXY:
        settings.useNextProxy = !settings.useNextProxy;
        CProxy::ref().refreshManagers();
        break;

    default: // (a configuration)
        {
            int num = event.GetId() - ID_CONFIG;
            if (num < 0 || num >= (int)settings.config.size()) break;
            map<string,vector<string> >::iterator it = settings.config.begin();
            while (num--) it++;
            string name = it->first;
            if (name == settings.currentConfig) break;
            settings.currentConfig = name;
            CProxy::ref().refreshManagers();
        }
    }
}


/* Creates the popup menu
 */
wxMenu* CTrayIcon::CreatePopupMenu() {

    CSettings& settings = CSettings::ref();

    wxMenu* menuConfig = new wxMenu();
    menuConfig->Append(wxID_ANY, settings.currentConfig.c_str());
    menuConfig->AppendSeparator();
    int num = ID_CONFIG;
    for (map<string,vector<string> >::iterator it = settings.config.begin();
                it != settings.config.end(); it++) {
        if (it->first != settings.currentConfig)
            menuConfig->Append(num, it->first.c_str());
        num++;
    }

    wxMenu* menuFilter = new wxMenu();
    menuFilter->Append(ID_FILTERALL,
        settings.getMessage("TRAY_FILTERALL").c_str());
    menuFilter->Append(ID_FILTERNONE,
        settings.getMessage("TRAY_FILTERNONE").c_str());
    menuFilter->AppendSeparator();
    menuFilter->AppendCheckItem(ID_FILTERIN,
        settings.getMessage("TRAY_FILTERIN").c_str());
    menuFilter->AppendCheckItem(ID_FILTEROUT,
        settings.getMessage("TRAY_FILTEROUT").c_str());
    menuFilter->AppendCheckItem(ID_FILTERTEXT,
        settings.getMessage("TRAY_FILTERTEXT").c_str());
    menuFilter->AppendCheckItem(ID_FILTERGIF,
        settings.getMessage("TRAY_FILTERGIF").c_str());
    if (settings.filterIn)   menuFilter->Check(ID_FILTERIN, true);
    if (settings.filterOut)  menuFilter->Check(ID_FILTEROUT, true);
    if (settings.filterText) menuFilter->Check(ID_FILTERTEXT, true);
    if (settings.filterGif)  menuFilter->Check(ID_FILTERGIF, true);

    wxMenu* menu = new wxMenu();
    menu->Append(wxID_ANY,
        settings.getMessage("TRAY_FILTER").c_str(), menuFilter);
    menu->Append(wxID_ANY,
        settings.getMessage("TRAY_CONFIG").c_str(), menuConfig);
    if (settings.useNextProxy || !settings.nextProxy.empty()) {
        menu->AppendCheckItem(ID_USEPROXY,
            settings.getMessage("TRAY_USEPROXY").c_str());
        if (settings.useNextProxy) menu->Check(ID_USEPROXY, true);
    }
    menu->Append(ID_EXIT,
        settings.getMessage("TRAY_EXIT").c_str());
    menu->Append(ID_LOG,
        settings.getMessage("TRAY_LOG").c_str());
    menu->Append(ID_OPEN,
        settings.getMessage("TRAY_OPEN").c_str());
    return menu;
}

