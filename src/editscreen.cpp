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


#include "editscreen.h"
#include <set>
#include <list>
#include <algorithm>
#include <wx/msgdlg.h>
#include <wx/menu.h>
#include "settings.h"
#include "const.h"
#include "util.h"
#include "proxy.h"

using namespace std;

/* Events
 */
BEGIN_EVENT_TABLE(CEditScreen, wxEvtHandler)
    EVT_CHECKBOX   (ID_MULTICHECKBOX, CEditScreen::OnCommand)
    EVT_COMBOBOX   (ID_TYPECOMBOBOX,  CEditScreen::OnCommand)
    EVT_COMBOBOX   (ID_CATEGCOMBOBOX, CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_CATEGCOMBOBOX, CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_TITLEEDIT,     CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_AUTHOREDIT,    CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_COMMENTEDIT,   CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_VERSIONEDIT,   CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_URLEDIT,       CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_WIDTHEDIT,     CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_BOUNDSEDIT,    CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_HEADEREDIT,    CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_MATCHMEMO,     CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_REPLACEMEMO,   CEditScreen::OnCommand)

    EVT_MENU_RANGE (ID_FILTERSNEW, ID_FILTERSAPPLY, CEditScreen::OnCommand)

    EVT_LIST_ITEM_SELECTED (ID_LISTCTRL, CEditScreen::OnListEvent)
    EVT_LIST_COL_CLICK     (ID_LISTCTRL, CEditScreen::OnListEvent)
END_EVENT_TABLE()


/* Constructor
 */
CEditScreen::CEditScreen(wxFrame* frame, wxWindow* window) :
                            CWindowContent(frame, window) {

    listCtrl =  new pmListCtrl(window, ID_LISTCTRL,
        wxDefaultPosition, wxSize(265,100),
        wxLC_REPORT );
    listCtrl->SetHelpText(settings.getMessage("EDIT_FILTERLIST_TIP").c_str());
    wxListItem column;
    column.SetText(settings.getMessage("LB_EDIT_COL1").c_str());
    column.SetWidth(180);
    listCtrl->InsertColumn(0, column);
    column.SetText(settings.getMessage("LB_EDIT_COL2").c_str());
    column.SetWidth(100);
    listCtrl->InsertColumn(1, column);
    Add(listCtrl,1,wxGROW | wxALL,5);

    wxBoxSizer* editSizer = new wxBoxSizer(wxVERTICAL);
    Add(editSizer,2,wxGROW | wxALL,0);

    wxFlexGridSizer* descSizer = new wxFlexGridSizer(5,2,5,5);
    descSizer->AddGrowableCol(1);
    editSizer->Add(descSizer,0,wxGROW | wxALL,5);

    pmStaticText* titleLabel =  new pmStaticText(window, wxID_ANY ,
        settings.getMessage("LB_EDIT_TITLE").c_str()  );
    descSizer->Add(titleLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    titleEdit =  new pmTextCtrl(window, ID_TITLEEDIT, "" ,
        wxDefaultPosition, wxDefaultSize);
    titleEdit->SetHelpText(settings.getMessage("EDIT_TITLE_TIP").c_str());
    descSizer->Add(titleEdit,1,wxGROW | wxALIGN_CENTER_VERTICAL | wxALL,0);

    pmStaticText* categLabel =  new pmStaticText(window, wxID_ANY ,
        settings.getMessage("LB_EDIT_CATEGORY").c_str()  );
    descSizer->Add(categLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    wxArrayString choices;
    categComboBox =  new pmComboBox(window, ID_CATEGCOMBOBOX, "" ,
        wxDefaultPosition, wxDefaultSize, choices,
        wxCB_DROPDOWN | wxCB_SORT);
    categComboBox->SetHelpText(settings.getMessage("EDIT_CATEGORY_TIP").c_str());
    descSizer->Add(categComboBox,1,wxGROW | wxALIGN_CENTER_VERTICAL | wxALL,0);

    pmStaticText* commentLabel =  new pmStaticText(window, wxID_ANY ,
        settings.getMessage("LB_EDIT_COMMENT").c_str()  );
    descSizer->Add(commentLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    commentEdit =  new pmTextCtrl(window, ID_COMMENTEDIT, "" ,
        wxDefaultPosition, wxSize(50,45), wxTE_MULTILINE );
    commentEdit->SetHelpText(settings.getMessage("EDIT_COMMENT_TIP").c_str());
    descSizer->Add(commentEdit,1,wxGROW | wxALIGN_CENTER_VERTICAL | wxALL,0);

    pmStaticText* authorLabel =  new pmStaticText(window, wxID_ANY ,
        settings.getMessage("LB_EDIT_AUTHOR").c_str()  );
    descSizer->Add(authorLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    authorEdit =  new pmTextCtrl(window, ID_AUTHOREDIT, "" ,
        wxDefaultPosition, wxDefaultSize );
    authorEdit->SetHelpText(settings.getMessage("EDIT_AUTHOR_TIP").c_str());
    descSizer->Add(authorEdit,1,wxGROW | wxALIGN_CENTER_VERTICAL | wxALL,0);

    pmStaticText* versionLabel =  new pmStaticText(window, wxID_ANY ,
        settings.getMessage("LB_EDIT_VERSION").c_str()  );
    descSizer->Add(versionLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    versionEdit =  new pmTextCtrl(window, ID_VERSIONEDIT, "" ,
        wxDefaultPosition, wxSize(80,21) );
    versionEdit->SetHelpText(settings.getMessage("EDIT_VERSION_TIP").c_str());
    descSizer->Add(versionEdit,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    pmStaticText* typeLabel =  new pmStaticText(window, wxID_ANY ,
        settings.getMessage("LB_EDIT_TYPE").c_str()  );
    descSizer->Add(typeLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    typeComboBox =  new pmComboBox(window, ID_TYPECOMBOBOX , "" ,
        wxDefaultPosition, wxSize(120,21),
        choices, wxCB_DROPDOWN | wxCB_READONLY );
    typeComboBox->SetHelpText(settings.getMessage("EDIT_TYPE_TIP").c_str());
    typeComboBox->Append(settings.getMessage("LB_EDIT_TYPEOUT").c_str());
    typeComboBox->Append(settings.getMessage("LB_EDIT_TYPEIN").c_str());
    typeComboBox->Append(settings.getMessage("LB_EDIT_TYPETEXT").c_str());
    descSizer->Add(typeComboBox,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    wxBoxSizer* headerSizer = new wxBoxSizer(wxHORIZONTAL);
    editSizer->Add(headerSizer,0,wxGROW | wxALL,0);

    headerLabel =  new pmStaticText(window, wxID_ANY ,
        settings.getMessage("LB_EDIT_HEADERNAME").c_str()  );
    headerSizer->Add(headerLabel,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    headerEdit =  new pmTextCtrl(window, ID_HEADEREDIT, "" ,
        wxDefaultPosition, wxSize(150,21) );
    headerEdit->SetHelpText(settings.getMessage("EDIT_HEADERNAME_TIP").c_str());
    headerEdit->SetFont(wxFont(10, wxSWISS ,wxNORMAL,wxNORMAL,FALSE,_T("Courier")));
    headerSizer->Add(headerEdit,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    wxBoxSizer* textSizer = new wxBoxSizer(wxVERTICAL);
    editSizer->Add(textSizer,0,wxGROW | wxALL,0);

    wxBoxSizer* widthSizer = new wxBoxSizer(wxHORIZONTAL);
    textSizer->Add(widthSizer,0,wxGROW | wxALL,0);

    widthLabel =  new pmStaticText(window, wxID_ANY ,
        settings.getMessage("LB_EDIT_WIDTH").c_str()  );
    widthSizer->Add(widthLabel,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    widthEdit =  new pmTextCtrl(window, ID_WIDTHEDIT, "" ,
        wxDefaultPosition, wxSize(80,21) );
    widthEdit->SetHelpText(settings.getMessage("EDIT_WIDTH_TIP").c_str());
    widthSizer->Add(widthEdit,0,wxALIGN_CENTER_VERTICAL | wxALL,5);

    multiCheckBox =  new pmCheckBox(window, ID_MULTICHECKBOX,
        settings.getMessage("LB_EDIT_MULTIMATCH").c_str()  );
    multiCheckBox->SetHelpText(settings.getMessage("EDIT_MULTIMATCH_TIP").c_str());
    widthSizer->Add(multiCheckBox,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,5);

    boundsLabel =  new pmStaticText(window, wxID_ANY ,
        settings.getMessage("LB_EDIT_BOUNDS").c_str()  );
    textSizer->Add(boundsLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    boundsEdit =  new pmTextCtrl(window, ID_BOUNDSEDIT, "" ,
        wxDefaultPosition, wxDefaultSize );
    boundsEdit->SetHelpText(settings.getMessage("EDIT_BOUNDS_TIP").c_str());
    boundsEdit->SetFont(wxFont(10, wxSWISS ,wxNORMAL,wxNORMAL,FALSE,_T("Courier")));
    textSizer->Add(boundsEdit,1,wxGROW | wxALL,5);

    pmStaticText* urlLabel =  new pmStaticText(window, wxID_ANY ,
        settings.getMessage("LB_EDIT_URL").c_str()  );
    editSizer->Add(urlLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    urlEdit =  new pmTextCtrl(window, ID_URLEDIT, "" ,
        wxDefaultPosition, wxDefaultSize );
    urlEdit->SetHelpText(settings.getMessage("EDIT_URL_TIP").c_str());
    urlEdit->SetFont(wxFont(10, wxSWISS ,wxNORMAL,wxNORMAL,FALSE,_T("Courier")));
    editSizer->Add(urlEdit,0,wxGROW | wxALL,5);

    pmStaticText* matchLabel =  new pmStaticText(window, wxID_ANY ,
        settings.getMessage("LB_EDIT_MATCH").c_str()  );
    editSizer->Add(matchLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    matchMemo =  new pmTextCtrl(window, ID_MATCHMEMO, "" ,
        wxDefaultPosition, wxSize(350,89)  ,
        wxTE_MULTILINE | wxTE_DONTWRAP | wxTE_PROCESS_TAB);
    matchMemo->SetHelpText(settings.getMessage("EDIT_MATCH_TIP").c_str());
    matchMemo->SetFont(wxFont(10, wxSWISS ,wxNORMAL,wxNORMAL,FALSE,_T("Courier")));
    editSizer->Add(matchMemo,3,wxGROW | wxALL,5);

    pmStaticText* replaceLabel =  new pmStaticText(window, wxID_ANY ,
        settings.getMessage("LB_EDIT_REPLACE").c_str()  );
    editSizer->Add(replaceLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    replaceMemo =  new pmTextCtrl(window, ID_REPLACEMEMO, "" ,
        wxDefaultPosition, wxSize(350,89)  ,
        wxTE_MULTILINE | wxTE_DONTWRAP | wxTE_PROCESS_TAB);
    replaceMemo->SetHelpText(settings.getMessage("EDIT_REPLACE_TIP").c_str());
    replaceMemo->SetFont(wxFont(10, wxSWISS ,wxNORMAL,wxNORMAL,FALSE,_T("Courier")));
    editSizer->Add(replaceMemo,2,wxGROW | wxALL,5);

    // Menu
    wxMenu* menuFilters = new wxMenu();
    menuFilters->Append(ID_FILTERSNEW,
        settings.getMessage("MENU_FILTERSNEW").c_str(),
        settings.getMessage("MENU_FILTERSNEW_TIP").c_str());
    menuFilters->Append(ID_FILTERSDUPLICATE,
        settings.getMessage("MENU_FILTERSDUPLICATE").c_str(),
        settings.getMessage("MENU_FILTERSDUPLICATE_TIP").c_str());
    menuFilters->Append(ID_FILTERSDELETE,
        settings.getMessage("MENU_FILTERSDELETE").c_str(),
        settings.getMessage("MENU_FILTERSDELETE_TIP").c_str());
    menuFilters->Append(ID_FILTERSTEST,
        settings.getMessage("MENU_FILTERSTEST").c_str(),
        settings.getMessage("MENU_FILTERSTEST_TIP").c_str());
    menuFilters->AppendSeparator();
    menuFilters->Append(ID_FILTERSENCODE,
        settings.getMessage("MENU_FILTERSENCODE").c_str(),
        settings.getMessage("MENU_FILTERSENCODE_TIP").c_str());
    menuFilters->Append(ID_FILTERSDECODE,
        settings.getMessage("MENU_FILTERSDECODE").c_str(),
        settings.getMessage("MENU_FILTERSDECODE_TIP").c_str());
    menuFilters->AppendSeparator();
    menuFilters->Append(ID_FILTERSIMPORT,
        settings.getMessage("MENU_FILTERSIMPORT").c_str(),
        settings.getMessage("MENU_FILTERSIMPORT_TIP").c_str());
    menuFilters->Append(ID_FILTERSPROXOMITRON,
        settings.getMessage("MENU_FILTERSPROXOMITRON").c_str(),
        settings.getMessage("MENU_FILTERSPROXOMITRON_TIP").c_str());
    menuFilters->Append(ID_FILTERSEXPORT,
        settings.getMessage("MENU_FILTERSEXPORT").c_str(),
        settings.getMessage("MENU_FILTERSEXPORT_TIP").c_str());
    menuFilters->AppendSeparator();
    menuFilters->Append(ID_FILTERSAPPLY,
        settings.getMessage("MENU_FILTERSAPPLY").c_str(),
        settings.getMessage("MENU_FILTERSAPPLY_TIP").c_str());
    menuFilters->Append(ID_FILTERSREVERT,
        settings.getMessage("MENU_FILTERSREVERT").c_str(),
        settings.getMessage("MENU_FILTERSREVERT_TIP").c_str());
    frame->GetMenuBar()->Insert(1, menuFilters,
        settings.getMessage("MENU_FILTERS").c_str());

    testFrame = NULL;
    sortedColumn = -1;
    makeSizer();
}


/* Destructor
 */
CEditScreen::~CEditScreen() {

    if (hasChanged()) apply(true);
    frame->GetMenuBar()->Remove(1);
    if (testFrame) testFrame->Destroy();
}


/* Test if local variables differ from settings
 */
bool CEditScreen::hasChanged() {

    commitText();
    return (bank != settings.filterbank);
}


/* Copies local variables to settings. The proxy is also restarted.
 */
void CEditScreen::apply(bool confirm) {

    if (!hasChanged()) return;
    // Ask user before proceeding
    if (confirm) {
        int ret = wxMessageBox(settings.getMessage("APPLY_EDITFILTERS").c_str(),
                               APP_NAME, wxYES_NO);
        if (ret == wxNO) return;
    }
    // Stop proxy
    CProxy::ref().closeProxyPort();
    CProxy::ref().refreshManagers();
    // Apply variables
    settings.filterbank = bank;
    settings.config = newConfig;
    settings.modified = true;
    // Restart proxy
    CProxy::ref().openProxyPort();
}


/* (Re)loads settings into local variables
 */
void CEditScreen::revert(bool confirm) {

    // Ask user before proceeding
    if (confirm && hasChanged()) {
        int ret = wxMessageBox(settings.getMessage("REVERT_EDITFILTERS").c_str(),
                               APP_NAME, wxYES_NO);
        if (ret == wxNO) return;
    }

    // Load variables
    listCtrl->SetFocus();
    bank = settings.filterbank;
    newConfig = settings.config;

    // Populate list
    rebuildList();

    // Clear filter properties
    current.clear();
    title.clear();
    updateDesc();
}


/* Update list control from bank
 */
void CEditScreen::rebuildList() {
    listCtrl->Hide();
    listCtrl->DeleteAllItems();
    for (map<string,CFilterDescriptor>::iterator it = bank.begin();
            it != bank.end(); it++) {
        listCtrl->InsertItem(0, it->first.c_str());
        listCtrl->SetItem(0, 1, it->second.category.c_str());
    }
    sortedColumn = -1;
    sortList(0);
    listCtrl->Show();
}

/* Display filter properties
 */
void CEditScreen::updateDesc() {

    stringstream ss;
    ss << current.windowWidth;
    widthEdit->SetValue(ss.str().c_str());
    multiCheckBox->SetValue(current.multipleMatches);
    authorEdit->SetValue(current.author.c_str());
    boundsEdit->SetValue(current.boundsPattern.c_str());
    commentEdit->SetValue(current.comment.c_str());
    headerEdit->SetValue(current.headerName.c_str());
    titleEdit->SetValue(current.title.c_str());
    urlEdit->SetValue(current.urlPattern.c_str());
    versionEdit->SetValue(current.version.c_str());
    matchMemo->SetValue(current.matchPattern.c_str());
    replaceMemo->SetValue(current.replacePattern.c_str());
    if (current.filterType == CFilterDescriptor::HEADOUT) {
        typeComboBox->SetValue(typeComboBox->GetString(0));
    } else if (current.filterType == CFilterDescriptor::HEADIN) {
        typeComboBox->SetValue(typeComboBox->GetString(1));
    } else {
        typeComboBox->SetValue(typeComboBox->GetString(2));
    }
    bool isText = (current.filterType == CFilterDescriptor::TEXT);
    boundsEdit->Enable(isText);
    widthEdit->Enable(isText);
    multiCheckBox->Enable(isText);
    headerEdit->Enable(!isText);
    boundsLabel->Enable(isText);
    widthLabel->Enable(isText);
    headerLabel->Enable(!isText);
    categComboBox->Clear();
    set<string> categs;
    for (map<string,CFilterDescriptor>::iterator it = bank.begin();
                it != bank.end(); it++)
        categs.insert(it->second.category);
    for (set<string>::iterator it = categs.begin(); it != categs.end(); it++)
        categComboBox->Append(it->c_str());
    categComboBox->SetValue(current.category.c_str());
}


/* Updates the bank and the list. Called whenever the current filter is modified.
 */
void CEditScreen::updateBank() {

    // If the edited filter is new, we insert an empty filter in the bank
    // in order to come down to the general case of filter modification
    if (title.empty()) {
        CFilterDescriptor desc;
        bank[desc.title] = desc;
        int index = listCtrl->GetItemCount();
        listCtrl->InsertItem(index, desc.title.c_str());
        listCtrl->SetItem(index, 1, desc.category.c_str());
        selectItem(index);
    }
    // The new title should not be empty
    if (current.title.empty()) {
        current.title = settings.getMessage("DEFAULT_FILTER_NAME");
        titleEdit->SetValue(current.title.c_str());
    }
    // The title should not be already used by another filter
    if (current.title != title) {
        set<string> titles;
        for (map<string,CFilterDescriptor>::iterator it = bank.begin();
                    it != bank.end(); it++)
            titles.insert(it->second.title);
        titles.erase(titles.find(title));
        while (titles.find(current.title) != titles.end())
            CUtil::increment(current.title);
        titleEdit->SetValue(current.title.c_str());
    }
    // Update list
    if (title != current.title || bank[title].category != current.category) {
        long index = listCtrl->FindItem(-1, title.c_str());
        listCtrl->SetItem(index, 0, current.title.c_str());
        listCtrl->SetItem(index, 1, current.category.c_str());
    }
    // Update bank
    if (title != current.title) bank.erase(bank.find(title));
    bank[current.title] = current;
    
    // Update config
    for (map<string, vector<string> >::iterator itm = newConfig.begin();
                            itm != newConfig.end(); itm++) {
        for (vector<string>::iterator itv = itm->second.begin();
                                itv != itm->second.end(); itv++) {
            if (*itv == title) *itv = current.title;
        }
    }
    
    title = current.title;
}


/* Event handling functions
 */
void CEditScreen::OnCommand(wxCommandEvent& event) {

    switch (event.GetId()) {
    case ID_MULTICHECKBOX:
        {
            current.multipleMatches = multiCheckBox->GetValue();
            updateBank();
            break;
        }
    case ID_AUTHOREDIT:
        {
            string value = authorEdit->GetValue().c_str();
            CUtil::trim(value);
            if (value != current.author) {
                current.author = value;
                updateBank();
            }
            break;
        }
    case ID_COMMENTEDIT:
        {
            string value = commentEdit->GetValue().c_str();
            CUtil::trim(value);
            if (value != current.comment) {
                current.defaultFilter = 0;
                current.comment = value;
                updateBank();
            }
            break;
        }
    case ID_VERSIONEDIT:
        {
            string value = versionEdit->GetValue().c_str();
            CUtil::trim(value);
            if (value != current.version) {
                current.version = value;
                updateBank();
            }
            break;
        }
    case ID_WIDTHEDIT:
        {
            stringstream ss(widthEdit->GetValue().c_str());
            int n = 0;
            ss >> n;
            if (n < 1) {
                n = 256;
                widthEdit->SetValue("256");
            }
            if (n != current.windowWidth) {
                current.windowWidth = n;
                updateBank();
            }
            break;
        }
    case ID_HEADEREDIT:
        {
            string value = headerEdit->GetValue().c_str();
            CUtil::trim(value);
            if (value != current.headerName) {
                current.headerName = value;
                updateBank();
            }
            break;
        }
    case ID_TITLEEDIT:
        {
            string value = titleEdit->GetValue().c_str();
            CUtil::trim(value);
            if (value != current.title) {
                current.defaultFilter = 0;
                current.title = value;
                updateBank();
            }
            break;
        }
    case ID_CATEGCOMBOBOX:
        {
            string value = categComboBox->GetValue().c_str();
            CUtil::trim(value);
            if (value != current.category) {
                current.defaultFilter = 0;
                current.category = value;
                updateBank();
            }
            break;
        }
    case ID_TYPECOMBOBOX:
        {
            string value = typeComboBox->GetValue().c_str();
            int row = typeComboBox->FindString(value.c_str());
            CFilterDescriptor::TYPE n = (row == 0 ? CFilterDescriptor::HEADOUT :
                                         row == 1 ? CFilterDescriptor::HEADIN :
                                                    CFilterDescriptor::TEXT );
            if (n != current.filterType) {
                current.filterType = n;
                updateBank();
                updateDesc();
            }
            break;
        }
    case ID_URLEDIT:
        {
            string value = urlEdit->GetValue().c_str();
            if (value != current.urlPattern) {
                current.urlPattern = value;
                updateBank();
                string errmsg;
                if (!CMatcher::testPattern(current.urlPattern, errmsg))
                    wxMessageBox(errmsg.c_str(), APP_NAME);
            }
            break;
        }
    case ID_BOUNDSEDIT:
        {
            string value = boundsEdit->GetValue().c_str();
            if (value != current.boundsPattern) {
                current.boundsPattern = value;
                updateBank();
                string errmsg;
                if (!CMatcher::testPattern(current.boundsPattern, errmsg))
                    wxMessageBox(errmsg.c_str(), APP_NAME);
            }
            break;
        }
    case ID_MATCHMEMO:
        {
            string value = matchMemo->GetValue().c_str();
            if (value != current.matchPattern) {
                current.matchPattern = value;
                updateBank();
                string errmsg;
                if (!CMatcher::testPattern(current.matchPattern, errmsg))
                    wxMessageBox(errmsg.c_str(), APP_NAME);
            }
            break;
        }
    case ID_REPLACEMEMO:
        {
            string value = replaceMemo->GetValue().c_str();
            if (value != current.replacePattern) {
                current.replacePattern = value;
                updateBank();
            }
            break;
        }
    case ID_FILTERSNEW:
        {
            commitText();
            current.clear();
            title.clear();
            updateDesc();
            titleEdit->SetFocus();
            break;
        }
    case ID_FILTERSDUPLICATE:
        {
            commitText();
            title.clear();
            CUtil::increment(current.title);
            updateBank();
            updateDesc();
            titleEdit->SetFocus();
            break;
        }
    case ID_FILTERSEXPORT:
        {
            commitText();
            stringstream out;
            long item = -1;
            while (true) {
                item = listCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                             wxLIST_STATE_SELECTED);
                if ( item == -1 ) break;
                out << bank[listCtrl->GetItemText(item).c_str()].exportFilter();
            }
            CUtil::setClipboard(out.str());
            break;
        }
    case ID_FILTERSIMPORT:
        {
            commitText();
            string str = CUtil::getClipboard();
            if (!str.empty()) {
                CFilterDescriptor::importFilters(str, bank);
                rebuildList();
            }
            break;
        }
    case ID_FILTERSPROXOMITRON:
        {
            commitText();
            string str = CUtil::getClipboard();
            if (!str.empty()) {
                CFilterDescriptor::importProxomitron(str, bank);
                rebuildList();
            }
            break;
        }
    case ID_FILTERSDELETE:
        {
            commitText();
            stringstream out(CUtil::getClipboard());
            long firstItem = -1;
            long item = -1;
            listCtrl->Hide();
            while (true) {
                item = listCtrl->GetNextItem(item, wxLIST_NEXT_ALL,
                                             wxLIST_STATE_SELECTED);
                if ( item == -1 ) break;
                if (firstItem < 0) firstItem = item;
                string name = listCtrl->GetItemText(item).c_str();
                out << bank[name].exportFilter();
                bank.erase(name);
                listCtrl->DeleteItem(item--);
                for (map<string, vector<string> >::iterator itm = newConfig.begin();
                                        itm != newConfig.end(); itm++) {
                    vector<string>::iterator itv = itm->second.begin();
                    while (itv != itm->second.end()) {
                        if (*itv == name) {
                            itm->second.erase(itv);
                            itv = itm->second.begin();
                        } else {
                            itv++;
                        }
                    }
                }
            }
            listCtrl->Show();
            CUtil::setClipboard(out.str());
            current.clear();
            title.clear();
            updateDesc();
            selectItem(firstItem);
            break;
        }
    case ID_FILTERSREVERT:
        {
            commitText();
            revert(true);
            break;
        }
    case ID_FILTERSAPPLY:
        {
            commitText();
            apply(false);
            break;
        }
    case ID_FILTERSENCODE:
        {
            CUtil::setClipboard(CUtil::encodeBASE64(CUtil::getClipboard()));
            break;
        }
    case ID_FILTERSDECODE:
        {
            CUtil::setClipboard(CUtil::decodeBASE64(CUtil::getClipboard()));
            break;
        }
    case ID_FILTERSTEST:
        {
            if (!testFrame) testFrame = new CTestFrame(current, &testFrame);
            CUtil::show(testFrame);
            break;
        }
    default:
        event.Skip();
    }
}

void CEditScreen::OnListEvent(wxListEvent& event) {
    WXTYPE evType = event.GetEventType();
    if (evType == wxEVT_COMMAND_LIST_ITEM_SELECTED) {
        long index = listCtrl->FindItem(-1, event.GetText().c_str());
        commitText();
        if (!(wxLIST_STATE_SELECTED
                & listCtrl->GetItemState(index, wxLIST_STATE_SELECTED))) {
            selectItem(index);
        }
        current = bank[listCtrl->GetItemText(index).c_str()];
        title = current.title;
        updateDesc();
    } else if (evType == wxEVT_COMMAND_LIST_COL_CLICK) {
        commitText();
        sortList(event.m_col);
    } else {
        event.Skip();
    }
}


/* Function for sorting the list control
 */
void CEditScreen::sortList(int numcol) {

    sortedDirection = (numcol == sortedColumn ? -sortedDirection : 1);
    sortedColumn = numcol;

    if (numcol == 0) {

        list<string> items;
        for (map<string,CFilterDescriptor>::iterator it = bank.begin();
                        it != bank.end(); it++) {
            string item = it->first;
            items.push_back(CUtil::lower(item));
        }
        items.sort();
        int counter = 0;
        for (list<string>::iterator it = items.begin();
                        it != items.end(); it++) {
            int index = listCtrl->FindItem(-1, it->c_str());
            listCtrl->SetItemData(index, counter += sortedDirection);
        }
        listCtrl->SortItems(CUtil::sortFunction, 0);

    } else {

        list<pair<string,string> > items;
        for (map<string,CFilterDescriptor>::iterator it = bank.begin();
                        it != bank.end(); it++) {
            pair<string,string> item;
            CUtil::lower(item.second = it->second.title);
            CUtil::lower(item.first = it->second.category);
            items.push_back(item);
        }
        items.sort();
        int counter = 0;
        for (list<pair<string,string> >::iterator it = items.begin();
                        it != items.end(); it++) {
            int index = listCtrl->FindItem(-1, it->second.c_str());
            listCtrl->SetItemData(index, counter += sortedDirection);
        }
        listCtrl->SortItems(CUtil::sortFunction, 0);
    }
}


/* Select an item in the list control
 */
void CEditScreen::selectItem(long index) {

    for (int i=0; i<listCtrl->GetItemCount(); i++)
        listCtrl->SetItemState(i, 0, wxLIST_STATE_SELECTED);
    if (index < 0 || index > listCtrl->GetItemCount()) return;
    listCtrl->EnsureVisible(index);
    SetEvtHandlerEnabled(false);
    listCtrl->SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
    SetEvtHandlerEnabled(true);
}


