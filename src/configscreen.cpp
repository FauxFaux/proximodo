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


#include "configscreen.h"
#include <set>
#include <list>
#include <algorithm>
#include <wx/msgdlg.h>
#include <wx/menu.h>
#include "mainframe.h"
#include "descriptor.h"
#include "settings.h"
#include "const.h"
#include "util.h"
#include "proxy.h"
#include "testframe.h"
#include "images/btn_up.xpm"
#include "images/btn_down.xpm"
#include "images/btn_left.xpm"
#include "images/btn_right.xpm"

using namespace std;

/* Events
 */
BEGIN_EVENT_TABLE(CConfigScreen, wxEvtHandler)
    EVT_COMBOBOX   (ID_NAMEEDIT,     CConfigScreen::OnCommand)
    EVT_TEXT_ENTER (ID_NAMEEDIT,     CConfigScreen::OnCommand)
    EVT_BUTTON     (ID_DOWNBUTTON,   CConfigScreen::OnCommand)
    EVT_BUTTON     (ID_UPBUTTON,     CConfigScreen::OnCommand)
    EVT_BUTTON     (ID_ADDBUTTON,    CConfigScreen::OnCommand)
    EVT_BUTTON     (ID_REMOVEBUTTON, CConfigScreen::OnCommand)
    EVT_MENU_RANGE (ID_CONFIGNEW, ID_CONFIGMOVEDOWN, CConfigScreen::OnCommand)
    EVT_LIST_COL_CLICK     (ID_ALLLIST, CConfigScreen::OnListEvent)
    EVT_LIST_ITEM_SELECTED (ID_ALLLIST, CConfigScreen::OnListEvent)
    EVT_LIST_ITEM_SELECTED (ID_OKLIST,  CConfigScreen::OnListEvent)
END_EVENT_TABLE()


/* Constructor
 */
CConfigScreen::CConfigScreen(wxFrame* frame, wxWindow* window) :
                                CWindowContent(frame, window, wxVERTICAL) {

    wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
    Add(topSizer, 1, wxGROW | wxALL,0);
    
    commentText =  new pmTextCtrl(window, ID_COMMENTTEXT, "" ,
        wxDefaultPosition, wxSize(100,60), wxTE_MULTILINE | wxTE_READONLY );
    commentText->SetHelpText(settings.getMessage("CONFIG_COMMENT_TIP").c_str());
    Add(commentText, 0,wxGROW | wxALIGN_CENTER_VERTICAL | wxALL,5);

    allList =  new pmListCtrl(window, ID_ALLLIST,
        wxDefaultPosition, wxSize(250,250),
        wxLC_REPORT);
    allList->SetHelpText(settings.getMessage("CONFIG_ALLLIST_TIP").c_str());
    topSizer->Add(allList,1,wxGROW | wxALL,5);

    wxBoxSizer* buttonSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(buttonSizer,0,wxALIGN_CENTER_VERTICAL | wxALL,0);

    pmBitmapButton* addButton =  new pmBitmapButton(window, ID_ADDBUTTON,
        wxBitmap(btn_right_xpm) );
    addButton->SetHelpText(settings.getMessage("CONFIG_ADD_TIP").c_str());
    buttonSizer->Add(addButton,0,wxALIGN_CENTER_HORIZONTAL | wxALL,5);

    pmBitmapButton* removeButton =  new pmBitmapButton(window, ID_REMOVEBUTTON,
        wxBitmap(btn_left_xpm) );
    removeButton->SetHelpText(settings.getMessage("CONFIG_REMOVE_TIP").c_str());
    buttonSizer->Add(removeButton,0,wxALIGN_CENTER_HORIZONTAL | wxALL,5);

    pmBitmapButton* upButton =  new pmBitmapButton(window, ID_UPBUTTON,
        wxBitmap(btn_up_xpm) );
    upButton->SetHelpText(settings.getMessage("CONFIG_UP_TIP").c_str());
    buttonSizer->Add(upButton,0,wxALIGN_CENTER_HORIZONTAL | wxALL,5);

    pmBitmapButton* downButton =  new pmBitmapButton(window, ID_DOWNBUTTON,
        wxBitmap(btn_down_xpm) );
    downButton->SetHelpText(settings.getMessage("CONFIG_DOWN_TIP").c_str());
    buttonSizer->Add(downButton,0,wxALIGN_CENTER_HORIZONTAL | wxALL,5);

    wxBoxSizer* rightSizer = new wxBoxSizer(wxVERTICAL);
    topSizer->Add(rightSizer,1,wxGROW | wxALL,0);

    wxBoxSizer* nameSizer = new wxBoxSizer(wxHORIZONTAL);
    rightSizer->Add(nameSizer,0,wxGROW | wxALL,0);

    pmStaticText* nameLabel =  new pmStaticText(window, wxID_ANY ,
        settings.getMessage("LB_CONFIG_NAME").c_str(),
        wxDefaultPosition, wxDefaultSize  );
    nameSizer->Add(nameLabel,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    wxArrayString choices;
    nameEdit =  new pmComboBox(window, ID_NAMEEDIT , "",
        wxDefaultPosition, wxDefaultSize, choices,
        wxCB_DROPDOWN | wxCB_SORT );
    nameEdit->SetHelpText(settings.getMessage("CONFIG_NAME_TIP").c_str());
    nameSizer->Add(nameEdit,1,wxALIGN_CENTER_VERTICAL | wxALL,5);

    okList =  new pmListCtrl(window, ID_OKLIST,
        wxDefaultPosition, wxSize(250,250)  ,
        wxLC_REPORT );
    okList->SetHelpText(settings.getMessage("CONFIG_OKLIST_TIP").c_str());
    rightSizer->Add(okList,1,wxGROW | wxALL,5);

    // Create columns
    wxListItem column;
    column.SetText(settings.getMessage("LB_CONFIG_COL1").c_str());
    column.SetWidth(180);
    allList->InsertColumn(0, column);
    okList->InsertColumn(0, column);
    column.SetText(settings.getMessage("LB_CONFIG_COL2").c_str());
    column.SetWidth(100);
    allList->InsertColumn(1, column);
    okList->InsertColumn(1, column);
    column.SetText(settings.getMessage("LB_CONFIG_COL3").c_str());
    column.SetWidth(50);
    allList->InsertColumn(2, column);
    okList->InsertColumn(2, column);

    // Menu
    wxMenu* menuConfig = new wxMenu();
    menuConfig->Append(ID_CONFIGADD,
        settings.getMessage("MENU_CONFIGADD").c_str(),
        settings.getMessage("MENU_CONFIGADD_TIP").c_str());
    menuConfig->Append(ID_CONFIGREMOVE,
        settings.getMessage("MENU_CONFIGREMOVE").c_str(),
        settings.getMessage("MENU_CONFIGREMOVE_TIP").c_str());
    menuConfig->Append(ID_CONFIGMOVEUP,
        settings.getMessage("MENU_CONFIGMOVEUP").c_str(),
        settings.getMessage("MENU_CONFIGMOVEUP_TIP").c_str());
    menuConfig->Append(ID_CONFIGMOVEDOWN,
        settings.getMessage("MENU_CONFIGMOVEDOWN").c_str(),
        settings.getMessage("MENU_CONFIGMOVEDOWN_TIP").c_str());
    menuConfig->AppendSeparator();
    menuConfig->Append(ID_CONFIGNEW,
        settings.getMessage("MENU_CONFIGNEW").c_str(),
        settings.getMessage("MENU_CONFIGNEW_TIP").c_str());
    menuConfig->Append(ID_CONFIGDUPLICATE,
        settings.getMessage("MENU_CONFIGDUPLICATE").c_str(),
        settings.getMessage("MENU_CONFIGDUPLICATE_TIP").c_str());
    menuConfig->Append(ID_CONFIGDELETE,
        settings.getMessage("MENU_CONFIGDELETE").c_str(),
        settings.getMessage("MENU_CONFIGDELETE_TIP").c_str());
    menuConfig->AppendSeparator();
    menuConfig->Append(ID_CONFIGAPPLY,
        settings.getMessage("MENU_CONFIGAPPLY").c_str(),
        settings.getMessage("MENU_CONFIGAPPLY_TIP").c_str());
    menuConfig->Append(ID_CONFIGREVERT,
        settings.getMessage("MENU_CONFIGREVERT").c_str(),
        settings.getMessage("MENU_CONFIGREVERT_TIP").c_str());
    frame->GetMenuBar()->Insert(1, menuConfig,
        settings.getMessage("MENU_CONFIG").c_str());

    sortedColumn = -1;
    makeSizer();
}


/* Destructor
 */
CConfigScreen::~CConfigScreen() {

    if (hasChanged()) apply(true);
    frame->GetMenuBar()->Remove(1);
}


/* Test if local variables differ from settings
 */
bool CConfigScreen::hasChanged() {

    commitText();
    return (config != settings.config);
}


/* Copies local variables to settings. The proxy is also restarted.
 */
void CConfigScreen::apply(bool confirm) {

    if (!hasChanged()) return;
    // Ask user before proceeding
    if (confirm) {
        int ret = wxMessageBox(settings.getMessage("APPLY_CONFIG").c_str(),
                               APP_NAME, wxYES_NO);
        if (ret == wxNO) return;
    }
    // Stop proxy
    CProxy::ref().closeProxyPort();
    CProxy::ref().refreshManagers();
    // Apply variables
    settings.config = config;
    settings.currentConfig = newActiveConfig;
    settings.modified = true;
    // Restart proxy
    CProxy::ref().openProxyPort();
}


/* (Re)loads settings into local variables
 */
void CConfigScreen::revert(bool confirm) {

    // Ask user before proceeding
    if (confirm && hasChanged()) {
        int ret = wxMessageBox(settings.getMessage("REVERT_CONFIG").c_str(),
                               APP_NAME, wxYES_NO);
        if (ret == wxNO) return;
    }

    // Load variables
    config = settings.config;
    editedConfigName = newActiveConfig = settings.currentConfig;
    editedConfig = config[editedConfigName];
    
    // Populate filter list
    allList->Hide();
    allList->DeleteAllItems();
    categories.clear();
    types.clear();
    for (map<string,CFilterDescriptor>::iterator it = settings.filterbank.begin();
                it != settings.filterbank.end(); it++) {

        categories[it->first] = it->second.category;
        if (it->second.filterType == CFilterDescriptor::HEADOUT)
            types[it->first] = settings.getMessage("CONFIG_TYPE_OUT");
        else if (it->second.filterType == CFilterDescriptor::HEADIN)
            types[it->first] = settings.getMessage("CONFIG_TYPE_IN");
        else
            types[it->first] = settings.getMessage("CONFIG_TYPE_TEXT");
        allList->InsertItem(0, it->first.c_str());
        allList->SetItem(0, 1, it->second.category.c_str());
        allList->SetItem(0, 2, types[it->first].c_str());
    }
    sortedColumn = -1;
    sortList(0);
    allList->Show();
    
    // Populate combobox's list
    nameEdit->Clear();
    for (map<string,vector<string> >::iterator it = config.begin();
                it != config.end(); it++)
        nameEdit->Append(it->first.c_str());

    // Display edited configuration
    displayConfig();

    nameEdit->SetFocus();
}


/* Display configuration content
 */
void CConfigScreen::displayConfig() {

    long count = 0;
    okList->DeleteAllItems();
    for (vector<string>::iterator it = editedConfig.begin();
                it != editedConfig.end(); it++) {

        okList->InsertItem(count, (*it).c_str());
        okList->SetItem(count, 1, categories[*it].c_str());
        okList->SetItem(count, 2, types[*it].c_str());
        count++;
    }
    okList->EnsureVisible(0);
    nameEdit->SetValue(editedConfigName.c_str());
}


/* Event handling functions
 */
void CConfigScreen::OnCommand(wxCommandEvent& event) {

    switch (event.GetId()) {
    case ID_CONFIGMOVEUP:
    case ID_UPBUTTON:
        {
            // Find first item not selected
            long index = 0, size = okList->GetItemCount();
            while ( index < size &&
                    (wxLIST_STATE_SELECTED & okList->GetItemState(index,-1)) )
                index++;
            // Then each selected item will be moved 1 up
            okList->Hide();
            while ( index < size ) {
                if (wxLIST_STATE_SELECTED & okList->GetItemState(index,-1)) {
                    string name = okList->GetItemText(index - 1).c_str();
                    editedConfig.erase(editedConfig.begin()+index - 1);
                    editedConfig.insert(editedConfig.begin()+index, name);
                    okList->DeleteItem(index - 1);
                    okList->InsertItem(index, name.c_str());
                    okList->SetItem(index, 1, categories[name].c_str());
                    okList->SetItem(index, 2, types[name].c_str());
                }
                index++;
            }
            okList->Show();
            config[editedConfigName] = editedConfig;
            break;
        }
    case ID_CONFIGMOVEDOWN:
    case ID_DOWNBUTTON:
        {
            // Find last item not selected
            long size = okList->GetItemCount();
            long index = size-1;
            while ( index >= 0 &&
                    (wxLIST_STATE_SELECTED & okList->GetItemState(index,-1)) )
                index--;
            // Then each selected item will be moved 1 down
            okList->Hide();
            while ( index >=0 ) {
                if (wxLIST_STATE_SELECTED & okList->GetItemState(index,-1)) {
                    string name = okList->GetItemText(index + 1).c_str();
                    editedConfig.erase(editedConfig.begin()+index + 1);
                    editedConfig.insert(editedConfig.begin()+index, name);
                    okList->DeleteItem(index + 1);
                    okList->InsertItem(index, name.c_str());
                    okList->SetItem(index, 1, categories[name].c_str());
                    okList->SetItem(index, 2, types[name].c_str());
                }
                index--;
            }
            okList->Show();
            config[editedConfigName] = editedConfig;
            break;
        }
    case ID_CONFIGADD:
    case ID_ADDBUTTON:
        {
            // Find insertion point (first selected item, or end of list)
            long ins = 0, size = okList->GetItemCount(), item = -1;
            while ( ins < size &&
                    !(wxLIST_STATE_SELECTED & okList->GetItemState(ins,-1)) )
                ins++;
            // Inserted selected items
            okList->Hide();
            while (true) {
                item = allList->GetNextItem(item, wxLIST_NEXT_ALL,
                                             wxLIST_STATE_SELECTED);
                if (item == -1) break;
                string name = allList->GetItemText(item).c_str();
                editedConfig.insert(editedConfig.begin()+ins, name);
                okList->InsertItem(ins, name.c_str());
                okList->SetItem(ins, 1, categories[name].c_str());
                okList->SetItem(ins, 2, types[name].c_str());
                okList->EnsureVisible(ins);
                ins++;
            }
            okList->Show();
            config[editedConfigName] = editedConfig;
            break;
        }
    case ID_CONFIGREMOVE:
    case ID_REMOVEBUTTON:
        {
            long item = -1, first = -1;
            okList->Hide();
            while (true) {
                item = okList->GetNextItem(item, wxLIST_NEXT_ALL,
                                             wxLIST_STATE_SELECTED);
                if (item == -1) break;
                if (first == -1) first = item;
                editedConfig.erase(editedConfig.begin()+item);
                okList->DeleteItem(item--);
            }
            if (first == okList->GetItemCount())
                first = okList->GetItemCount() - 1;
            selectItem(first);
            okList->Show();
            config[editedConfigName] = editedConfig;
            break;
        }
    case ID_NAMEEDIT:
        {
            if (event.GetEventType() == wxEVT_COMMAND_TEXT_ENTER) {

                string newConfigName = nameEdit->GetValue().c_str();
                CUtil::trim(newConfigName);
                if (newConfigName != editedConfigName) {
                    // The user renamed the configuration
                    if (newConfigName.empty())
                        newConfigName = settings.getMessage("CONFIG_NEW_NAME");
                    newConfigName = makeNewName(editedConfigName, newConfigName);
                    if (editedConfigName == newActiveConfig)
                        newActiveConfig = newConfigName;
                    config.erase(editedConfigName);
                    config[newConfigName] = editedConfig;
                    nameEdit->SetValue(newConfigName.c_str());
                    nameEdit->Append(newConfigName.c_str());
                    nameEdit->Delete(nameEdit->FindString(editedConfigName.c_str()));
                    editedConfigName = newConfigName;
                }
            } else if (event.GetEventType() == wxEVT_COMMAND_COMBOBOX_SELECTED) {

                // The user changed current configuration
                editedConfigName = event.GetString();
                editedConfig = config[editedConfigName];
                displayConfig();
            }
            break;
        }
    case ID_CONFIGNEW:
        {
            commitText();
            editedConfigName = settings.getMessage("CONFIG_NEW_NAME");
            editedConfigName = makeNewName("", editedConfigName);
            editedConfig.clear();
            config[editedConfigName];
            nameEdit->Append(editedConfigName.c_str());
            displayConfig();
            break;
        }
    case ID_CONFIGDUPLICATE:
        {
            commitText();
            editedConfigName = makeNewName("", editedConfigName);
            config[editedConfigName] = editedConfig;
            nameEdit->Append(editedConfigName.c_str());
            displayConfig();
            break;
        }
    case ID_CONFIGDELETE:
        {
            commitText();
            config.erase(editedConfigName);
            nameEdit->Delete(nameEdit->FindString(editedConfigName.c_str()));
            if (config.empty()) {
                string newName = settings.getMessage("CONFIG_NEW_NAME");
                config[newName];
                nameEdit->Append(newName.c_str());
            }
            if (editedConfigName == newActiveConfig)
                newActiveConfig = config.begin()->first;
            editedConfigName = config.begin()->first;
            editedConfig = config.begin()->second;
            displayConfig();
            break;
        }
    case ID_CONFIGREVERT:
        {
            commitText();
            revert(true);
            break;
        }
    case ID_CONFIGAPPLY:
        {
            commitText();
            apply(false);
            break;
        }
    default:
        event.Skip();
    }
}

void CConfigScreen::OnListEvent(wxListEvent& event) {

    switch (event.GetId()) {
    case ID_ALLLIST:
        {
            if (event.GetEventType() == wxEVT_COMMAND_LIST_ITEM_SELECTED) {
                long index = event.GetIndex();
                if (index < 0) return;
                string title = allList->GetItemText(index).c_str();
                if (title.empty()) return;
                commentText->SetValue(settings.filterbank[title].comment.c_str());
            } else {
                sortList(event.m_col);
            }
            break;
        }
    case ID_OKLIST:
        {
            long index = event.GetIndex();
            if (index < 0) return;
            string title = okList->GetItemText(index).c_str();
            if (title.empty()) return;
            commentText->SetValue(settings.filterbank[title].comment.c_str());
            break;
        }
        
    }
}


/* Function for sorting the list control
 */
void CConfigScreen::sortList(int numcol) {

    sortedDirection = (numcol == sortedColumn ? -sortedDirection : 1);
    sortedColumn = numcol;

    list<pair<string,string> > items;
    for (map<string,string>::iterator it = categories.begin();
                    it != categories.end(); it++) {
        pair<string,string> item;
        item.first = (numcol == 0 ? it->first :
                      numcol == 1 ? categories[it->first] : types[it->first]);
        CUtil::lower(item.first);
        CUtil::lower(item.second = it->first);
        items.push_back(item);
    }
    items.sort();
    int counter = 0;
    for (list<pair<string,string> >::iterator it = items.begin();
                    it != items.end(); it++) {
        int index = allList->FindItem(-1, it->second.c_str());
        allList->SetItemData(index, counter += sortedDirection);
    }
    allList->SortItems(CUtil::sortFunction, 0);
}


/* Select an item in the configuration list
 */
void CConfigScreen::selectItem(long index) {

    for (int i=0; i<okList->GetItemCount(); i++)
        okList->SetItemState(i, 0, wxLIST_STATE_SELECTED);
    if (index < 0 || index > okList->GetItemCount()) return;
    okList->EnsureVisible(index);
    SetEvtHandlerEnabled(false);
    okList->SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    SetEvtHandlerEnabled(true);
}


/* Increments a name if it is already in the config name list
 */
string CConfigScreen::makeNewName(string oldname, string newname) {

    set<string> names;
    for (map<string, vector<string> >::iterator it = config.begin();
                it != config.end(); it++) {
        if (it->first != oldname) {
            names.insert(it->first);
        }
    }
    while (names.find(newname) != names.end())
        CUtil::increment(newname);
    return newname;
}
