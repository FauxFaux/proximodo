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
#include <sstream>
#include <algorithm>
#include <wx/msgdlg.h>
#include <wx/icon.h>
#include <wx/sysopt.h>
#include <wx/settings.h>
#include <wx/menu.h>
#include "matcher.h"
#include "settings.h"
#include "const.h"
#include "util.h"
#include "images/icon32.xpm"

using namespace std;

/* Events
 */
BEGIN_EVENT_TABLE(CEditScreen, wxFrame)
    EVT_CLOSE      (CEditScreen::OnClose)
    EVT_CHECKBOX   (ID_MULTICHECKBOX, CEditScreen::OnCommand)
    EVT_COMBOBOX   (ID_TYPECOMBOBOX,  CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_TITLEEDIT,     CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_AUTHOREDIT,    CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_COMMENTEDIT,   CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_VERSIONEDIT,   CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_URLEDIT,       CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_WIDTHEDIT,     CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_PRIORITYEDIT,  CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_BOUNDSEDIT,    CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_HEADEREDIT,    CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_MATCHMEMO,     CEditScreen::OnCommand)
    EVT_TEXT_ENTER (ID_REPLACEMEMO,   CEditScreen::OnCommand)
    EVT_MENU_RANGE (ID_FILTERSENCODE, ID_HELPSYNTAX, CEditScreen::OnCommand)
END_EVENT_TABLE()


/* Constructor
 */
CEditScreen::CEditScreen(CFilterDescriptor* desc) :
        wxFrame((wxFrame *)NULL, wxID_ANY,
                CSettings::ref().getMessage("EDIT_WINDOW_TITLE").c_str(),
                wxDefaultPosition, wxDefaultSize,
                wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL |
                wxNO_FULL_REPAINT_ON_RESIZE | wxCLIP_CHILDREN) {

    SetIcon(wxIcon(icon32_xpm));
    CSettings& settings = CSettings::ref();
    SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));

    // Controls creation
    wxBoxSizer* editSizer = new wxBoxSizer(wxVERTICAL);
    SetSizer(editSizer);

    wxFlexGridSizer* descSizer = new wxFlexGridSizer(5,2,5,5);
    descSizer->AddGrowableCol(1);
    editSizer->Add(descSizer,0,wxGROW | wxALL,5);

    pmStaticText* titleLabel =  new pmStaticText(this, wxID_ANY ,
        settings.getMessage("LB_EDIT_TITLE").c_str()  );
    descSizer->Add(titleLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    titleEdit =  new pmTextCtrl(this, ID_TITLEEDIT, "" ,
        wxDefaultPosition, wxDefaultSize);
    titleEdit->SetHelpText(settings.getMessage("EDIT_TITLE_TIP").c_str());
    descSizer->Add(titleEdit,1,wxGROW | wxALIGN_CENTER_VERTICAL | wxALL,0);

    pmStaticText* authorLabel =  new pmStaticText(this, wxID_ANY ,
        settings.getMessage("LB_EDIT_AUTHOR").c_str()  );
    descSizer->Add(authorLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    wxBoxSizer* authorSizer = new wxBoxSizer(wxHORIZONTAL);
    descSizer->Add(authorSizer,0,wxGROW | wxALL,0);

    authorEdit =  new pmTextCtrl(this, ID_AUTHOREDIT, "" ,
        wxDefaultPosition, wxDefaultSize );
    authorEdit->SetHelpText(settings.getMessage("EDIT_AUTHOR_TIP").c_str());
    authorSizer->Add(authorEdit,1,wxGROW | wxALIGN_CENTER_VERTICAL | wxALL,0);

    pmStaticText* versionLabel =  new pmStaticText(this, wxID_ANY ,
        settings.getMessage("LB_EDIT_VERSION").c_str()  );
    authorSizer->Add(versionLabel,0,wxALIGN_CENTER_VERTICAL | wxLEFT,10);

    versionEdit =  new pmTextCtrl(this, ID_VERSIONEDIT, "" ,
        wxDefaultPosition, wxSize(60,21) );
    versionEdit->SetHelpText(settings.getMessage("EDIT_VERSION_TIP").c_str());
    authorSizer->Add(versionEdit,0,wxALIGN_CENTER_VERTICAL | wxLEFT,5);

    pmStaticText* commentLabel =  new pmStaticText(this, wxID_ANY ,
        settings.getMessage("LB_EDIT_COMMENT").c_str()  );
    descSizer->Add(commentLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    commentEdit =  new pmTextCtrl(this, ID_COMMENTEDIT, "" ,
        wxDefaultPosition, wxSize(50,45), wxTE_MULTILINE );
    commentEdit->SetHelpText(settings.getMessage("EDIT_COMMENT_TIP").c_str());
    descSizer->Add(commentEdit,1,wxGROW | wxALIGN_CENTER_VERTICAL | wxALL,0);

    pmStaticText* priorityLabel =  new pmStaticText(this, wxID_ANY ,
        settings.getMessage("LB_EDIT_PRIORITY").c_str()  );
    descSizer->Add(priorityLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    wxBoxSizer* prioritySizer = new wxBoxSizer(wxHORIZONTAL);
    descSizer->Add(prioritySizer,0,wxGROW | wxALL,0);

    priorityEdit =  new pmTextCtrl(this, ID_PRIORITYEDIT, "" ,
        wxDefaultPosition, wxSize(40,21) );
    priorityEdit->SetHelpText(settings.getMessage("EDIT_PRIORITY_TIP").c_str());
    prioritySizer->Add(priorityEdit,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    widthLabel =  new pmStaticText(this, wxID_ANY ,
        settings.getMessage("LB_EDIT_WIDTH").c_str()  );
    prioritySizer->Add(widthLabel,0,wxALIGN_CENTER_VERTICAL | wxLEFT,10);

    widthEdit =  new pmTextCtrl(this, ID_WIDTHEDIT, "" ,
        wxDefaultPosition, wxSize(40,21) );
    widthEdit->SetHelpText(settings.getMessage("EDIT_WIDTH_TIP").c_str());
    prioritySizer->Add(widthEdit,0,wxALIGN_CENTER_VERTICAL | wxLEFT,5);

    multiCheckBox =  new pmCheckBox(this, ID_MULTICHECKBOX,
        settings.getMessage("LB_EDIT_MULTIMATCH").c_str(),
        wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT  );
    multiCheckBox->SetHelpText(settings.getMessage("EDIT_MULTIMATCH_TIP").c_str());
    prioritySizer->Add(multiCheckBox,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT,10);

    pmStaticText* typeLabel =  new pmStaticText(this, wxID_ANY ,
        settings.getMessage("LB_EDIT_TYPE").c_str()  );
    descSizer->Add(typeLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    wxBoxSizer* typeSizer = new wxBoxSizer(wxHORIZONTAL);
    descSizer->Add(typeSizer,0,wxGROW | wxALL,0);

    wxArrayString choices;
    typeComboBox =  new pmComboBox(this, ID_TYPECOMBOBOX , "" ,
        wxDefaultPosition, wxSize(120,21),
        choices, wxCB_DROPDOWN | wxCB_READONLY );
    typeComboBox->SetHelpText(settings.getMessage("EDIT_TYPE_TIP").c_str());
    typeComboBox->Append(settings.getMessage("LB_EDIT_TYPEOUT").c_str());
    typeComboBox->Append(settings.getMessage("LB_EDIT_TYPEIN").c_str());
    typeComboBox->Append(settings.getMessage("LB_EDIT_TYPETEXT").c_str());
    typeSizer->Add(typeComboBox,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    headerLabel =  new pmStaticText(this, wxID_ANY ,
        settings.getMessage("LB_EDIT_HEADERNAME").c_str()  );
    typeSizer->Add(headerLabel,0,wxALIGN_CENTER_VERTICAL | wxLEFT, 10);

    headerEdit =  new pmTextCtrl(this, ID_HEADEREDIT, "" ,
        wxDefaultPosition, wxSize(150,21) );
    headerEdit->SetHelpText(settings.getMessage("EDIT_HEADERNAME_TIP").c_str());
    headerEdit->SetFont(wxFont(10, wxSWISS ,wxNORMAL,wxNORMAL,FALSE,_T("Courier")));
    typeSizer->Add(headerEdit,0,wxALIGN_CENTER_VERTICAL | wxLEFT, 5);

    wxFlexGridSizer* boundsSizer = new wxFlexGridSizer(2,2,5,5);
    boundsSizer->AddGrowableCol(1);
    editSizer->Add(boundsSizer,0,wxGROW | wxALL,5);

    boundsLabel =  new pmStaticText(this, wxID_ANY ,
        settings.getMessage("LB_EDIT_BOUNDS").c_str()  );
    boundsSizer->Add(boundsLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    boundsEdit =  new pmTextCtrl(this, ID_BOUNDSEDIT, "" ,
        wxDefaultPosition, wxDefaultSize );
    boundsEdit->SetHelpText(settings.getMessage("EDIT_BOUNDS_TIP").c_str());
    boundsEdit->SetFont(wxFont(10, wxSWISS ,wxNORMAL,wxNORMAL,FALSE,_T("Courier")));
    boundsSizer->Add(boundsEdit,1, wxALIGN_CENTER_VERTICAL | wxGROW | wxALL,0);

    pmStaticText* urlLabel =  new pmStaticText(this, wxID_ANY ,
        settings.getMessage("LB_EDIT_URL").c_str()  );
    boundsSizer->Add(urlLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxALL,0);

    urlEdit =  new pmTextCtrl(this, ID_URLEDIT, "" ,
        wxDefaultPosition, wxDefaultSize );
    urlEdit->SetHelpText(settings.getMessage("EDIT_URL_TIP").c_str());
    urlEdit->SetFont(wxFont(10, wxSWISS ,wxNORMAL,wxNORMAL,FALSE,_T("Courier")));
    boundsSizer->Add(urlEdit,1, wxALIGN_CENTER_VERTICAL | wxGROW | wxALL,0);

    pmStaticText* matchLabel =  new pmStaticText(this, wxID_ANY ,
        settings.getMessage("LB_EDIT_MATCH").c_str()  );
    editSizer->Add(matchLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT,5);

    matchMemo =  new pmTextCtrl(this, ID_MATCHMEMO, "" ,
        wxDefaultPosition, wxSize(200,55)  ,
        wxTE_MULTILINE | wxTE_DONTWRAP | wxTE_PROCESS_TAB);
    matchMemo->SetHelpText(settings.getMessage("EDIT_MATCH_TIP").c_str());
    matchMemo->SetFont(wxFont(10, wxSWISS ,wxNORMAL,wxNORMAL,FALSE,_T("Courier")));
    editSizer->Add(matchMemo,3,wxGROW | wxALL,5);

    pmStaticText* replaceLabel =  new pmStaticText(this, wxID_ANY ,
        settings.getMessage("LB_EDIT_REPLACE").c_str()  );
    editSizer->Add(replaceLabel,0,wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL | wxLEFT,5);

    replaceMemo =  new pmTextCtrl(this, ID_REPLACEMEMO, "" ,
        wxDefaultPosition, wxSize(200,55)  ,
        wxTE_MULTILINE | wxTE_DONTWRAP | wxTE_PROCESS_TAB);
    replaceMemo->SetHelpText(settings.getMessage("EDIT_REPLACE_TIP").c_str());
    replaceMemo->SetFont(wxFont(10, wxSWISS ,wxNORMAL,wxNORMAL,FALSE,_T("Courier")));
    editSizer->Add(replaceMemo,2,wxGROW | wxALL,5);

    // Menu creation
    wxMenuBar* menubar = new wxMenuBar();
    wxMenu* menu = new wxMenu();
    menu->Append(ID_FILTERSTEST,
        settings.getMessage("MENU_FILTERSTEST").c_str(),
        settings.getMessage("MENU_FILTERSTEST_TIP").c_str());
    menu->Append(ID_FILTERSENCODE,
        settings.getMessage("MENU_FILTERSENCODE").c_str(),
        settings.getMessage("MENU_FILTERSENCODE_TIP").c_str());
    menu->Append(ID_FILTERSDECODE,
        settings.getMessage("MENU_FILTERSDECODE").c_str(),
        settings.getMessage("MENU_FILTERSDECODE_TIP").c_str());
    menubar->Append(menu,
        settings.getMessage("MENU_FILTERS").c_str());
    menu = new wxMenu();
    menu->Append(ID_HELPFILTERS,
        settings.getMessage("MENU_HELPFILTERS").c_str(),
        settings.getMessage("MENU_HELPFILTERS_TIP").c_str());
    menu->Append(ID_HELPSYNTAX,
        settings.getMessage("MENU_HELPSYNTAX").c_str(),
        settings.getMessage("MENU_HELPSYNTAX_TIP").c_str());
    menubar->Append(menu,
        settings.getMessage("MENU_HELP").c_str());
    SetMenuBar(menubar);

    GetSizer()->Fit(this);
    GetSizer()->SetSizeHints(this);

    current = desc;
    testWindow = new CTestFrame(current);
}


/* Destructor
 */
CEditScreen::~CEditScreen() {

    delete testWindow;
}


/* Hide the window when clicking on the [X] button
 * The window is destroyed by the config screen on its own destruction
 */
void CEditScreen::OnClose(wxCloseEvent& event) {
    testWindow->Hide();
    Hide();
}


/* Display filter properties in controls
 */
void CEditScreen::setCurrent(CFilterDescriptor* desc) {

    current = desc;
    testWindow->setCurrent(current);
    
    stringstream ss;
    ss << current->windowWidth;
    widthEdit->SetValue(ss.str().c_str());
    ss.str("");
    ss << current->priority;
    priorityEdit->SetValue(ss.str().c_str());
    
    multiCheckBox->SetValue(current->multipleMatches);
    authorEdit   ->SetValue(current->author        .c_str());
    boundsEdit   ->SetValue(current->boundsPattern .c_str());
    commentEdit  ->SetValue(current->comment       .c_str());
    headerEdit   ->SetValue(current->headerName    .c_str());
    titleEdit    ->SetValue(current->title         .c_str());
    urlEdit      ->SetValue(current->urlPattern    .c_str());
    versionEdit  ->SetValue(current->version       .c_str());
    matchMemo    ->SetValue(current->matchPattern  .c_str());
    replaceMemo  ->SetValue(current->replacePattern.c_str());
    
    if (current->filterType == CFilterDescriptor::HEADOUT) {
        typeComboBox->SetValue(typeComboBox->GetString(0));
    } else if (current->filterType == CFilterDescriptor::HEADIN) {
        typeComboBox->SetValue(typeComboBox->GetString(1));
    } else {
        typeComboBox->SetValue(typeComboBox->GetString(2));
    }
    enableFields();
}


/* Enables fields wrt. filter type
 */
void CEditScreen::enableFields() {

    bool canEdit = (current->id != -1);
    wxWindowList& children = GetChildren();
    for (wxWindowListNode* child = children.GetFirst(); child; child = child->GetNext())
        child->GetData()->Enable(canEdit);
    if (!canEdit) return;

    bool isText = (current->filterType == CFilterDescriptor::TEXT);
    boundsEdit->Enable(isText);
    widthEdit->Enable(isText);
    multiCheckBox->Enable(isText);
    headerEdit->Enable(!isText);
    boundsLabel->Enable(isText);
    widthLabel->Enable(isText);
    headerLabel->Enable(!isText);
}


/* Fields events handling function
 */
void CEditScreen::OnCommand(wxCommandEvent& event) {

    switch (event.GetId()) {
    case ID_MULTICHECKBOX:
        {
            current->multipleMatches = multiCheckBox->GetValue();
            break;
        }
    case ID_AUTHOREDIT:
        {
            string value = authorEdit->GetValue().c_str();
            current->author = CUtil::trim(value);
            break;
        }
    case ID_VERSIONEDIT:
        {
            string value = versionEdit->GetValue().c_str();
            current->version = CUtil::trim(value);
            break;
        }
    case ID_HEADEREDIT:
        {
            string value = headerEdit->GetValue().c_str();
            current->headerName = CUtil::trim(value);
            current->testValidity();
            break;
        }
    case ID_COMMENTEDIT:
        {
            string value = commentEdit->GetValue().c_str();
            CUtil::trim(value);
            if (value != current->comment)
                current->defaultFilter = 0;
            current->comment = value;
            break;
        }
    case ID_TITLEEDIT:
        {
            string value = titleEdit->GetValue().c_str();
            CUtil::trim(value);
            if (value != current->title)
                current->defaultFilter = 0;
            current->title = value;
            current->testValidity();
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
            current->windowWidth = n;
            current->testValidity();
            break;
        }
    case ID_PRIORITYEDIT:
        {
            stringstream ss(priorityEdit->GetValue().c_str());
            int n = 0;
            ss >> n;
            if (n < 1) {
                n = 256;
                priorityEdit->SetValue("256");
            }
            current->priority = n;
            break;
        }
    case ID_TYPECOMBOBOX:
        {
            string value = typeComboBox->GetValue().c_str();
            int row = typeComboBox->FindString(value.c_str());
            if (row == 0) {
                current->filterType = CFilterDescriptor::HEADOUT;
            } else if (row == 1) {
                current->filterType = CFilterDescriptor::HEADIN;
            } else {
                current->filterType = CFilterDescriptor::TEXT;
            }
            enableFields();
            break;
        }
    case ID_URLEDIT:
        {
            string value = urlEdit->GetValue().c_str();
            if (value != current->urlPattern) {
                current->urlPattern = value;
                string errmsg;
                if (!CMatcher::testPattern(current->urlPattern, errmsg))
                    wxMessageBox(errmsg.c_str(), APP_NAME);
            }
            current->testValidity();
            break;
        }
    case ID_BOUNDSEDIT:
        {
            string value = boundsEdit->GetValue().c_str();
            if (value != current->boundsPattern) {
                current->boundsPattern = value;
                string errmsg;
                if (!CMatcher::testPattern(current->boundsPattern, errmsg))
                    wxMessageBox(errmsg.c_str(), APP_NAME);
            }
            current->testValidity();
            break;
        }
    case ID_MATCHMEMO:
        {
            string value = matchMemo->GetValue().c_str();
            if (value != current->matchPattern) {
                current->matchPattern = value;
                string errmsg;
                if (!CMatcher::testPattern(current->matchPattern, errmsg))
                    wxMessageBox(errmsg.c_str(), APP_NAME);
            }
            current->testValidity();
            break;
        }
    case ID_REPLACEMEMO:
        {
            string value = replaceMemo->GetValue().c_str();
            current->replacePattern = value;
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
    case ID_HELPFILTERS:
        {
            CUtil::openBrowser(CSettings::ref().getMessage("HELP_PAGE_FILTERS"));
            break;
        }
    case ID_HELPSYNTAX:
        {
            CUtil::openBrowser(CSettings::ref().getMessage("HELP_PAGE_SYNTAX"));
            break;
        }
    case ID_FILTERSTEST:
        {
            testWindow->Show();
            testWindow->Raise();
            break;
        }
    default:
        event.Skip();
    }
}

