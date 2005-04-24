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


#include "settingsscreen.h"
#include "mainframe.h"
#include "settings.h"
#include "util.h"
#include "const.h"
#include "proxy.h"
#include "controls.h"
#include "matcher.h"
#include "images/btn_add20.xpm"
#include "images/btn_view20.xpm"
#include "images/btn_file20.xpm"
#include "images/btn_trash20.xpm"
#include <wx/sizer.h>
#include <wx/msgdlg.h>
#include <wx/event.h>
#include <wx/filedlg.h>
#include <wx/filename.h>
#include <wx/event.h>
#include <wx/textfile.h>
#include <map>
#include <algorithm>
#include <sstream>

using namespace std;

/* Events
 */
BEGIN_EVENT_TABLE(CSettingsScreen, wxEvtHandler)
    EVT_BUTTON     (ID_APPLYBUTTON,          CSettingsScreen::OnCommand)
    EVT_BUTTON     (ID_REVERTBUTTON,         CSettingsScreen::OnCommand)
    EVT_BUTTON     (ID_NEXTPROXYBUTTON,      CSettingsScreen::OnCommand)
    EVT_BUTTON     (ID_NEWBUTTON,            CSettingsScreen::OnCommand)
    EVT_BUTTON     (ID_OPENBUTTON,           CSettingsScreen::OnCommand)
    EVT_BUTTON     (ID_BROWSERPATHBUTTON,    CSettingsScreen::OnCommand)
    EVT_BUTTON     (ID_CHOOSEBUTTON,         CSettingsScreen::OnCommand)
    EVT_CHECKBOX   (ID_ALLOWIPCHECKBOX,      CSettingsScreen::OnCommand)
    EVT_CHECKBOX   (ID_STARTBROWSERCHECKBOX, CSettingsScreen::OnCommand)
    EVT_CHECKBOX   (ID_SHOWGUICHECKBOX,      CSettingsScreen::OnCommand)
    EVT_CHECKBOX   (ID_USEPROXYCHECKBOX,     CSettingsScreen::OnCommand)
    EVT_COMBOBOX   (ID_LANGUAGEDROPDOWN,     CSettingsScreen::OnCommand)
    EVT_COMBOBOX   (ID_NEXTPROXYDROPDOWN,    CSettingsScreen::OnCommand)
    EVT_COMBOBOX   (ID_LISTNAMEDROPDOWN,     CSettingsScreen::OnCommand)
    EVT_TEXT_ENTER (ID_LISTNAMEDROPDOWN,     CSettingsScreen::OnCommand)
    EVT_TEXT_ENTER (ID_NEXTPROXYDROPDOWN,    CSettingsScreen::OnCommand)
    EVT_TEXT_ENTER (ID_BYPASSTEXT,           CSettingsScreen::OnCommand)
    EVT_TEXT_ENTER (ID_MAXRANGETEXT,         CSettingsScreen::OnCommand)
    EVT_TEXT_ENTER (ID_MINRANGETEXT,         CSettingsScreen::OnCommand)
    EVT_TEXT_ENTER (ID_BROWSERPATHTEXT,      CSettingsScreen::OnCommand)
    EVT_TEXT_ENTER (ID_PORTTEXT,             CSettingsScreen::OnCommand)
    EVT_TEXT_ENTER (ID_LISTFILETEXT,         CSettingsScreen::OnCommand)
END_EVENT_TABLE()


/* Constructor
 */
CSettingsScreen::CSettingsScreen(wxFrame* frame) : CWindowContent(frame) {

    wxBoxSizer* leftBox = new wxBoxSizer(wxVERTICAL);
    Add(leftBox,1,wxGROW | wxALL,0);

    wxBoxSizer* rightBox = new wxBoxSizer(wxVERTICAL);
    Add(rightBox,1,wxGROW | wxALL,0);

    // Filtering options

    wxStaticBox* filterStaticBox_StaticBoxObj = new wxStaticBox(frame,wxID_ANY,
        settings.getMessage("LB_SETTINGS_FILTERING").c_str());
    pmStaticBoxSizer* filterStaticBox = new pmStaticBoxSizer(
        filterStaticBox_StaticBoxObj,wxHORIZONTAL);
    leftBox->Add(filterStaticBox,0,wxGROW | wxALL,5);

    wxBoxSizer* filterBox = new wxBoxSizer(wxVERTICAL);
    filterStaticBox->Add(filterBox,1,wxALIGN_TOP | wxALL,0);

    wxBoxSizer* bypassBox = new wxBoxSizer(wxHORIZONTAL);
    filterBox->Add(bypassBox,0,wxGROW | wxALL,0);

    pmStaticText* bypassLabel =  new pmStaticText(frame, wxID_ANY ,
        settings.getMessage("LB_SETTINGS_BYPASS").c_str());
    bypassBox->Add(bypassLabel,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    bypassText =  new pmTextCtrl(frame, ID_BYPASSTEXT,
        "" , wxDefaultPosition, wxDefaultSize);
    bypassText->SetHelpText(settings.getMessage("SETTINGS_BYPASS_TIP").c_str());
    bypassBox->Add(bypassText,1,wxALIGN_CENTER_VERTICAL | wxALL,5);

    // GUI options

    wxStaticBox* guiStaticBox_StaticBoxObj = new wxStaticBox(frame,wxID_ANY,
        settings.getMessage("LB_SETTINGS_GUI").c_str());
    pmStaticBoxSizer* guiStaticBox = new pmStaticBoxSizer(
        guiStaticBox_StaticBoxObj,wxHORIZONTAL);
    rightBox->Add(guiStaticBox,0,wxGROW | wxALL,5);

    wxBoxSizer* guiBox = new wxBoxSizer(wxVERTICAL);
    guiStaticBox->Add(guiBox,1,wxALIGN_TOP | wxALL,0);

    wxBoxSizer* languageBox = new wxBoxSizer(wxHORIZONTAL);
    guiBox->Add(languageBox,0,wxGROW | wxALL,0);

    pmStaticText* languageLabel =  new pmStaticText(frame, wxID_ANY ,
        settings.getMessage("LB_SETTINGS_LANGUAGE").c_str());
    languageBox->Add(languageLabel,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    wxArrayString choices;
    languageDropDown =  new pmComboBox(frame, ID_LANGUAGEDROPDOWN ,
        "" , wxDefaultPosition, wxDefaultSize, choices,
        wxCB_DROPDOWN | wxCB_READONLY | wxCB_SORT );
    languageDropDown->SetHelpText(settings.getMessage("SETTINGS_LANGUAGE_TIP").c_str());
    languageBox->Add(languageDropDown,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    showGuiCheckbox =  new pmCheckBox(frame, ID_SHOWGUICHECKBOX,
        settings.getMessage("LB_SETTINGS_SHOWGUI").c_str());
    showGuiCheckbox->SetHelpText(settings.getMessage("SETTINGS_SHOWGUI_TIP").c_str());
    guiBox->Add(showGuiCheckbox,0,wxALIGN_LEFT | wxALL,5);

    startBrowserCheckbox =  new pmCheckBox(frame, ID_STARTBROWSERCHECKBOX,
        settings.getMessage("LB_SETTINGS_STARTBROWSER").c_str());
    startBrowserCheckbox->SetHelpText(settings.getMessage("SETTINGS_STARTBROWSER_TIP").c_str());
    guiBox->Add(startBrowserCheckbox,0,wxALIGN_LEFT | wxALL,5);

    wxBoxSizer* browserPathBox = new wxBoxSizer(wxHORIZONTAL);
    guiBox->Add(browserPathBox,0,wxGROW | wxALL,0);

    browserPathText =  new pmTextCtrl(frame, ID_BROWSERPATHTEXT,
        "" , wxDefaultPosition, wxDefaultSize);
    browserPathText->SetHelpText(settings.getMessage("SETTINGS_BROWSERPATH_TIP").c_str());
    browserPathBox->Add(browserPathText,1,wxALIGN_CENTER_VERTICAL | wxALL,5);

    pmBitmapButton* browserButton =  new pmBitmapButton(frame, ID_BROWSERPATHBUTTON,
        wxBitmap(btn_file20_xpm) );
    browserButton->SetHelpText(settings.getMessage("SETTINGS_BROWSERCHOOSE_TIP").c_str());
    browserPathBox->Add(browserButton,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    // Proxy options

    wxStaticBox* proxyStaticBox_StaticBoxObj = new wxStaticBox(frame,wxID_ANY,
        settings.getMessage("LB_SETTINGS_PROXY").c_str());
    pmStaticBoxSizer* proxyStaticBox = new pmStaticBoxSizer(
        proxyStaticBox_StaticBoxObj,wxHORIZONTAL);
    leftBox->Add(proxyStaticBox,1,wxGROW | wxALL,5);

    wxBoxSizer* proxyBox = new wxBoxSizer(wxVERTICAL);
    proxyStaticBox->Add(proxyBox,1,wxALIGN_TOP | wxALL,0);

    wxBoxSizer* portBox = new wxBoxSizer(wxHORIZONTAL);
    proxyBox->Add(portBox,0,wxALIGN_LEFT | wxALL,0);

    pmStaticText* portLabel =  new pmStaticText(frame, wxID_ANY ,
        settings.getMessage("LB_SETTINGS_PORT").c_str());
    portBox->Add(portLabel,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    portText =  new pmTextCtrl(frame, ID_PORTTEXT,
        "" , wxDefaultPosition, wxSize(50, 21));
    portText->SetHelpText(settings.getMessage("SETTINGS_PORT_TIP").c_str());
    portBox->Add(portText,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    useProxyCheckbox =  new pmCheckBox(frame, ID_USEPROXYCHECKBOX,
        settings.getMessage("LB_SETTINGS_USEPROXY").c_str());
    useProxyCheckbox->SetHelpText(settings.getMessage("SETTINGS_USEPROXY_TIP").c_str());
    proxyBox->Add(useProxyCheckbox,0,wxALIGN_LEFT | wxALL,5);

    wxBoxSizer* nextProxyBox = new wxBoxSizer(wxHORIZONTAL);
    proxyBox->Add(nextProxyBox,0,wxGROW | wxALL,0);

    pmStaticText* proxyLabel =  new pmStaticText(frame, wxID_ANY ,
        settings.getMessage("LB_SETTINGS_NEXTPROXY").c_str());
    nextProxyBox->Add(proxyLabel,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    nextProxyDropdown =  new pmComboBox(frame, ID_NEXTPROXYDROPDOWN ,
        "" , wxDefaultPosition, wxDefaultSize,
        choices, wxCB_DROPDOWN | wxCB_SORT  );
    nextProxyDropdown->SetHelpText(settings.getMessage("SETTINGS_NEXTPROXY_TIP").c_str());
    nextProxyBox->Add(nextProxyDropdown,1,wxALIGN_CENTER_VERTICAL | wxALL,5);

    pmBitmapButton* nextProxyButton =  new pmBitmapButton(frame, ID_NEXTPROXYBUTTON,
        wxBitmap(btn_trash20_xpm) );
    nextProxyButton->SetHelpText(settings.getMessage("SETTINGS_REMOVEPROXY_TIP").c_str());
    nextProxyBox->Add(nextProxyButton,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    allowIPCheckbox =  new pmCheckBox(frame, ID_ALLOWIPCHECKBOX,
        settings.getMessage("LB_SETTINGS_ALLOWIP").c_str());
    allowIPCheckbox->SetHelpText(settings.getMessage("SETTINGS_ALLOWIP_TIP").c_str());
    proxyBox->Add(allowIPCheckbox,0,wxALIGN_LEFT | wxALL,5);

    wxFlexGridSizer* rangeBox = new wxFlexGridSizer(2,2,5,5);
    proxyBox->Add(rangeBox,0,wxALIGN_LEFT | wxALL,5);

    pmStaticText* minRangeLabel =  new pmStaticText(frame, wxID_ANY ,
        settings.getMessage("LB_SETTINGS_MINRANGE").c_str());
    rangeBox->Add(minRangeLabel,0,wxALIGN_LEFT |
        wxALIGN_CENTER_VERTICAL| wxALL,0);

    minRangeText =  new pmTextCtrl(frame, ID_MINRANGETEXT,
        "" , wxDefaultPosition, wxDefaultSize);
    minRangeText->SetHelpText(settings.getMessage("SETTINGS_MINRANGE_TIP").c_str());
    rangeBox->Add(minRangeText,0,wxALIGN_CENTER_HORIZONTAL |
        wxALIGN_CENTER_VERTICAL | wxALL,0);

    pmStaticText* maxRangeLabel =  new pmStaticText(frame, wxID_ANY ,
        settings.getMessage("LB_SETTINGS_MAXRANGE").c_str());
    rangeBox->Add(maxRangeLabel,0,wxALIGN_LEFT |
        wxALIGN_CENTER_VERTICAL| wxALL,0);

    maxRangeText =  new pmTextCtrl(frame, ID_MAXRANGETEXT,
        "" , wxDefaultPosition, wxDefaultSize);
    maxRangeText->SetHelpText(settings.getMessage("SETTINGS_MAXRANGE_TIP").c_str());
    rangeBox->Add(maxRangeText,0,wxALIGN_CENTER_HORIZONTAL |
        wxALIGN_CENTER_VERTICAL | wxALL,0);

    // List options

    wxStaticBox* listStaticBox_StaticBoxObj = new wxStaticBox(frame,wxID_ANY,
        settings.getMessage("LB_SETTINGS_LIST").c_str());
    pmStaticBoxSizer* listStaticBox = new pmStaticBoxSizer(
        listStaticBox_StaticBoxObj,wxHORIZONTAL);
    rightBox->Add(listStaticBox,1,wxGROW | wxALL,5);

    wxBoxSizer* listBox = new wxBoxSizer(wxVERTICAL);
    listStaticBox->Add(listBox,1,wxALIGN_TOP | wxALL,0);

    wxBoxSizer* listNameBox = new wxBoxSizer(wxHORIZONTAL);
    listBox->Add(listNameBox,0,wxGROW | wxALL,0);

    pmStaticText* listNameLabel =  new pmStaticText(frame, wxID_ANY ,
        settings.getMessage("LB_SETTINGS_LISTNAME").c_str());
    listNameBox->Add(listNameLabel,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    listDropDown =  new pmComboBox(frame, ID_LISTNAMEDROPDOWN ,
        "" , wxDefaultPosition, wxSize(70, 21),
        choices, wxCB_DROPDOWN | wxCB_SORT  );
    listDropDown->SetHelpText(settings.getMessage("SETTINGS_LISTNAME_TIP").c_str());
    listNameBox->Add(listDropDown,1,wxALIGN_CENTER_VERTICAL | wxALL,5);

    pmBitmapButton* newButton =  new pmBitmapButton(frame, ID_NEWBUTTON,
        wxBitmap(btn_add20_xpm) );
    newButton->SetHelpText(settings.getMessage("SETTINGS_NEWLIST_TIP").c_str());
    listNameBox->Add(newButton,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    pmBitmapButton* openButton =  new pmBitmapButton(frame, ID_OPENBUTTON,
        wxBitmap(btn_view20_xpm) );
    openButton->SetHelpText(settings.getMessage("SETTINGS_OPENLIST_TIP").c_str());
    listNameBox->Add(openButton,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    wxBoxSizer* listFileBox = new wxBoxSizer(wxHORIZONTAL);
    listBox->Add(listFileBox,0,wxGROW | wxALL,0);

    pmStaticText* listFileLabel =  new pmStaticText(frame, wxID_ANY ,
        settings.getMessage("LB_SETTINGS_LISTFILE").c_str());
    listFileBox->Add(listFileLabel,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    listFileText =  new pmTextCtrl(frame, ID_LISTFILETEXT,
        "" , wxDefaultPosition, wxDefaultSize);
    listFileText->SetHelpText(settings.getMessage("SETTINGS_LISTFILE_TIP").c_str());
    listFileBox->Add(listFileText,1,wxALIGN_CENTER_VERTICAL | wxALL,5);

    pmBitmapButton* chooseButton =  new pmBitmapButton(frame, ID_CHOOSEBUTTON,
        wxBitmap(btn_file20_xpm) );
    chooseButton->SetHelpText(settings.getMessage("SETTINGS_FILECHOOSE_TIP").c_str());
    listFileBox->Add(chooseButton,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    // More buttons

    wxBoxSizer* buttonBox = new wxBoxSizer(wxHORIZONTAL);
    rightBox->Add(buttonBox,0,wxALIGN_CENTER_HORIZONTAL | wxALL,0);

    pmButton* applyButton =  new pmButton(frame, ID_APPLYBUTTON,
        settings.getMessage("LB_SETTINGS_APPLY").c_str());
    applyButton->SetHelpText(settings.getMessage("SETTINGS_APPLY_TIP").c_str());
    buttonBox->Add(applyButton,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    pmButton* revertButton =  new pmButton(frame, ID_REVERTBUTTON,
        settings.getMessage("LB_SETTINGS_REVERT").c_str());
    revertButton->SetHelpText(settings.getMessage("SETTINGS_REVERT_TIP").c_str());
    buttonBox->Add(revertButton,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    // Ready

    makeSizer();
}


/* Destructor
 */
CSettingsScreen::~CSettingsScreen() {

    apply(true);
}


/* Test if local variables differ from settings
 */
bool CSettingsScreen::hasChanged() {

    commitText();
    return (language      != settings.language      ||
            proxyPort     != settings.proxyPort     ||
            useNextProxy  != settings.useNextProxy  ||
            nextProxy     != settings.nextProxy     ||
            allowIPRange  != settings.allowIPRange  ||
            minIPRange    != settings.minIPRange    ||
            maxIPRange    != settings.maxIPRange    ||
            bypass        != settings.bypass        ||
            proxies       != settings.proxies       ||
            showOnStartup != settings.showOnStartup ||
            startBrowser  != settings.startBrowser  ||
            browserPath   != settings.browserPath   ||
            listNames     != settings.listNames     );
}


/* (Re)loads settings into local variables
 */
void CSettingsScreen::revert(bool confirm) {

    // Ask user before proceeding
    if (confirm && hasChanged()) {
        int ret = wxMessageBox(settings.getMessage("REVERT_SETTINGS").c_str(),
                                                APP_NAME, wxYES_NO);
        if (ret == wxNO) return;
    }

    // Load variables
    language      = settings.language      ;
    proxyPort     = settings.proxyPort     ;
    useNextProxy  = settings.useNextProxy  ;
    nextProxy     = settings.nextProxy     ;
    allowIPRange  = settings.allowIPRange  ;
    minIPRange    = settings.minIPRange    ;
    maxIPRange    = settings.maxIPRange    ;
    bypass        = settings.bypass        ;
    proxies       = settings.proxies       ;
    showOnStartup = settings.showOnStartup ;
    startBrowser  = settings.startBrowser  ;
    browserPath   = settings.browserPath   ;
    listNames     = settings.listNames     ;

    // Populate controls
    allowIPCheckbox->SetValue(allowIPRange);
    startBrowserCheckbox->SetValue(startBrowser);
    showGuiCheckbox->SetValue(showOnStartup);
    useProxyCheckbox->SetValue(useNextProxy);
    bypassText->SetValue(bypass.c_str());
    browserPathText->SetValue(browserPath.c_str());
    maxRangeText->SetValue(CUtil::toDotted(maxIPRange).c_str());
    minRangeText->SetValue(CUtil::toDotted(minIPRange).c_str());
    portText->SetValue(proxyPort.c_str());

    // Populate proxy list
    nextProxyDropdown->Clear();
    for (set<string>::iterator it = proxies.begin();
            it != proxies.end(); it++) {
        nextProxyDropdown->Append(it->c_str());
    }
    nextProxyDropdown->SetValue(nextProxy.c_str());

    // Populate language list
    languageDropDown->Clear();
    wxString f = wxFindFirstFile("*.lng");
    while ( !f.IsEmpty() ) {
        languageDropDown->Append(f.SubString(2, f.Length() - 5));
        f = wxFindNextFile();
    }
    languageDropDown->SetValue(language.c_str());
    
    // Populate lists list
    listDropDown->Clear();
    for (map<string,string>::iterator it = listNames.begin();
            it != listNames.end(); it++) {
        listDropDown->Append(it->first.c_str());
    }
    currentListName.clear();
    listDropDown->SetValue("");
    listFileText->SetValue("");

    allowIPCheckbox->SetFocus();
}


/* Copies local variables to settings. The proxy is also restarted.
 */
void CSettingsScreen::apply(bool confirm) {

    if (!hasChanged()) return;
    if (confirm) {
        // Ask user before proceeding
        int ret = wxMessageBox(settings.getMessage("APPLY_SETTINGS").c_str(),
                               APP_NAME, wxYES_NO);
        if (ret == wxNO) return;
    }
    // Stop proxy
    CProxy::ref().closeProxyPort();
    CProxy::ref().refreshManagers();
    // Apply variables
    settings.useNextProxy  = useNextProxy  ;
    settings.language      = language      ;
    settings.proxyPort     = proxyPort     ;
    settings.nextProxy     = nextProxy     ;
    settings.allowIPRange  = allowIPRange  ;
    settings.minIPRange    = minIPRange    ;
    settings.maxIPRange    = maxIPRange    ;
    settings.bypass        = bypass        ;
    settings.proxies       = proxies       ;
    settings.showOnStartup = showOnStartup ;
    settings.startBrowser  = startBrowser  ;
    settings.browserPath   = browserPath   ;
    settings.listNames     = listNames     ;
    settings.modified      = true;
    // Remove lists with no filename
    map<string,string>::iterator it = settings.listNames.begin();
    while (it != settings.listNames.end()) {
        if (it->second.empty()) {
            settings.listNames.erase(it);
            it = settings.listNames.begin();
        } else it++;
    }
    // Reload lists
    settings.loadLists();
    // Restart proxy
    CProxy::ref().openProxyPort();
}


/* Event handling function
 */
void CSettingsScreen::OnCommand(wxCommandEvent& event) {
    string value;
    
    switch (event.GetId()) {
    case ID_ALLOWIPCHECKBOX:
        allowIPRange = allowIPCheckbox->GetValue(); break;

    case ID_USEPROXYCHECKBOX:
        if (nextProxyDropdown->GetValue().IsEmpty())
            useProxyCheckbox->SetValue(false);
        useNextProxy = useProxyCheckbox->GetValue(); break;

    case ID_SHOWGUICHECKBOX:
        showOnStartup = showGuiCheckbox->GetValue(); break;

    case ID_STARTBROWSERCHECKBOX:
        startBrowser = startBrowserCheckbox->GetValue(); break;

    case ID_APPLYBUTTON:
        settings.proxyPort.clear(); // (to force apply, in case list files were edited)
        apply(false); break;

    case ID_REVERTBUTTON:
        revert(true); break;

    case ID_LANGUAGEDROPDOWN:
        language = languageDropDown->GetValue(); break;
        
    case ID_BROWSERPATHTEXT:
        browserPath = browserPathText->GetValue().c_str(); break;

    case ID_BYPASSTEXT:
        {
            string newValue = bypassText->GetValue().c_str();
            string errmsg;
            if (CMatcher::testPattern(newValue, errmsg)) {
                bypass = newValue;
            } else {
                wxMessageBox(errmsg.c_str(), APP_NAME);
            }
            break;
        }
    case ID_MAXRANGETEXT:
        {
            maxIPRange = CUtil::fromDotted(maxRangeText->GetValue().c_str());
            maxRangeText->SetValue(CUtil::toDotted(maxIPRange).c_str());
            break;
        }
    case ID_MINRANGETEXT:
        {
            minIPRange = CUtil::fromDotted(minRangeText->GetValue().c_str());
            minRangeText->SetValue(CUtil::toDotted(minIPRange).c_str());
            break;
        }
    case ID_PORTTEXT: 
        {
            string newValue = portText->GetValue().c_str();
            stringstream ss(newValue);
            unsigned short num = 0;
            ss >> num;
            if (num > 0) {
                proxyPort = newValue;
                if (!CProxy::ref().testPort(proxyPort)) {
                    wxMessageBox(settings.getMessage("PORT_UNAVAILABLE").c_str(),
                                 APP_NAME);
                }
            }
            portText->SetValue(proxyPort.c_str());
            break;
        }
    case ID_NEXTPROXYBUTTON:
        {
            set<string>::iterator it = proxies.find(nextProxy);
            if (it != proxies.end())
                proxies.erase(it);
            int pos = nextProxyDropdown->FindString(nextProxy.c_str());
            if (pos != wxNOT_FOUND)
                nextProxyDropdown->Delete(pos);
            if (proxies.empty()) {
                nextProxy = "";
            } else {
                nextProxy = *(proxies.begin());
            }
            nextProxyDropdown->SetValue(nextProxy.c_str());
            break;
        }
    case ID_NEXTPROXYDROPDOWN:
        {
            nextProxy = nextProxyDropdown->GetValue();
            CUtil::trim(nextProxy);
            if (!nextProxy.empty()) {
                if (nextProxy.find(':') == string::npos) {
                    nextProxy += ":8080";
                    nextProxyDropdown->SetValue(nextProxy.c_str());
                }
                proxies.insert(nextProxy);
                if (nextProxyDropdown->FindString(nextProxy.c_str()) == wxNOT_FOUND) {
                    nextProxyDropdown->Append(nextProxy.c_str());
                    if (!CProxy::ref().testRemoteProxy(nextProxy)) {
                        wxMessageBox(settings.getMessage("BAD_REMOTE_PROXY").c_str(),
                                     APP_NAME);
                    }
                }
            } else {
                useProxyCheckbox->SetValue(useNextProxy = false);
            }
            break;
        }
    case ID_NEWBUTTON:
        {
            currentListName.clear();
            listDropDown->SetValue("");
            listFileText->SetValue("");
            listDropDown->SetFocus();
            break;
        }
    case ID_OPENBUTTON:
        {
            if (!currentListName.empty())
                CUtil::openNotepad(listNames[currentListName]);
            break;
        }
    case ID_LISTNAMEDROPDOWN:
        {
            if (event.GetEventType() == wxEVT_COMMAND_TEXT_ENTER) {

                string newListName = listDropDown->GetValue().c_str();
                CUtil::trim(newListName);
                if (newListName != currentListName) {
                    if (!currentListName.empty()) {
                        // Delete/Rename list
                        listNames.erase(currentListName);
                        listDropDown->Delete(
                            listDropDown->FindString(currentListName.c_str()));
                    }
                    if (!newListName.empty()) {
                        // Create/Rename list
                        listNames[newListName] = listFileText->GetValue();
                        CUtil::trim(listNames[newListName]);
                        listDropDown->Append(newListName.c_str());
                    } else {
                        listFileText->SetValue("");
                    }
                    currentListName = newListName;
                    listDropDown->SetValue(currentListName.c_str());
                }
            } else if (event.GetEventType() == wxEVT_COMMAND_COMBOBOX_SELECTED) {

                currentListName = event.GetString();
                listFileText->SetValue(listNames[currentListName].c_str());
            }
            break;
        }
    case ID_CHOOSEBUTTON:
        {
            wxFileDialog fd(frame,
                  settings.getMessage("OPEN_LISTFILE_QUESTION").c_str(),
                  "Lists", // This is the directory name
                  "", "Text files (*.txt)|*.txt|All files (*.*)|*.*", wxOPEN);
            if (fd.ShowModal() != wxID_OK) break;
            listFileText->SetValue(fd.GetPath());
            // No break here, we'll do following tests
        }
    case ID_LISTFILETEXT:
        {
            wxFileName fn(listFileText->GetValue());

            fn.MakeAbsolute();
            string fullpath = fn.GetFullPath().c_str();
            
            fn.MakeRelativeTo(wxFileName::GetCwd());
            if (fullpath.size() < fn.GetFullPath().Length())
                fn.MakeAbsolute();
            string path = fn.GetFullPath().c_str();

            if (!currentListName.empty())
                listNames[currentListName] = path;
            listFileText->SetValue(path.c_str());
            
            if (!path.empty() && !fn.FileExists()) {
                int answer = wxMessageBox(
                              settings.getMessage("ASK_CREATE_NEWLIST").c_str(),
                              APP_NAME, wxYES_NO);
                if (answer == wxYES) {
                    wxTextFile f(path.c_str());
                    f.Create();
                    f.Close();
                }
            }
            break;
        }
    case ID_BROWSERPATHBUTTON:
        {
            wxFileDialog fd(frame,
                  settings.getMessage("OPEN_BROWSER_QUESTION").c_str(),
                  ".", // This is the directory name
                  "", "All files (*.*)|*.*", wxOPEN);
            if (fd.ShowModal() != wxID_OK) break;
            wxString path = fd.GetPath();
            if (path.Find(' ') >= 0) path = "\"" + path + "\"";
            browserPathText->SetValue(path);
            browserPath = path.c_str();
            break;
        }
    default:
        event.Skip();
    }
}

