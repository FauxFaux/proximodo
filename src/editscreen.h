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


#ifndef __editscreen__
#define __editscreen__

#include <wx/sizer.h>
#include <wx/event.h>
#include <string>
#include <map>
#include "controls.h"
#include "descriptor.h"
#include "testframe.h"
#include "windowcontent.h"

using namespace std;

/* This class creates a box sizer to be set as the main frame sizer.
 * It display the filter bank list and fields to edit a filter.
 */
class CEditScreen : public CWindowContent {

public:
    CEditScreen(wxFrame* frame, wxWindow* window);
    ~CEditScreen();
    
private:
    // Non-modal test window
    CTestFrame* testFrame;

    // Variable management functions
    void revert(bool confirm);
    void apply(bool confirm);
    bool hasChanged();
    void updateDesc();
    void updateBank();
    void sortList(int numcol);
    void selectItem(long index);
    void rebuildList();

    // Managed variables
    map<string,CFilterDescriptor> bank;

    // GUI variables
    int sortedColumn;
    int sortedDirection;
    CFilterDescriptor current;               // edited filter
    string title;                            // original title of edited filter
    map<string, vector<string> > newConfig;  // to remove/rename filters in configs

    // Controls
    pmCheckBox *multiCheckBox;
    pmComboBox *categComboBox;
    pmComboBox *typeComboBox;
    pmStaticText *boundsLabel;
    pmStaticText *headerLabel;
    pmStaticText *widthLabel;
    pmTextCtrl *authorEdit;
    pmTextCtrl *boundsEdit;
    pmTextCtrl *commentEdit;
    pmTextCtrl *headerEdit;
    pmTextCtrl *titleEdit;
    pmTextCtrl *urlEdit;
    pmTextCtrl *versionEdit;
    pmTextCtrl *widthEdit;
    pmTextCtrl *matchMemo;
    pmTextCtrl *replaceMemo;
    pmListCtrl *listCtrl;

    // Event handling function
    void OnCommand(wxCommandEvent& event);
    void OnListEvent(wxListEvent& event);

    // IDs
    enum {
        // Controls' ID
        ID_TITLEEDIT = 1400,
        ID_AUTHOREDIT,
        ID_COMMENTEDIT,
        ID_VERSIONEDIT,
        ID_URLEDIT,
        ID_WIDTHEDIT,
        ID_BOUNDSEDIT,
        ID_HEADEREDIT,
        ID_MATCHMEMO,
        ID_REPLACEMEMO,
        ID_MULTICHECKBOX,
        ID_CATEGCOMBOBOX,
        ID_TYPECOMBOBOX,
        ID_LISTCTRL,
        // Menu's ID
        ID_FILTERSNEW,
        ID_FILTERSDUPLICATE,
        ID_FILTERSTEST,
        ID_FILTERSREVERT,
        ID_FILTERSEXPORT,
        ID_FILTERSIMPORT,
        ID_FILTERSPROXOMITRON,
        ID_FILTERSDELETE,
        ID_FILTERSENCODE,
        ID_FILTERSDECODE,
        ID_FILTERSAPPLY
    };

    // Event table
    DECLARE_EVENT_TABLE()
};

#endif

