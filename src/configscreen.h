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


#ifndef __configscreen__
#define __configscreen__

#include <wx/sizer.h>
#include <wx/event.h>
#include <string>
#include <map>
#include <vector>
#include "controls.h"
#include "windowcontent.h"

using namespace std;

/* This class creates a box sizer to be set as the main frame sizer.
 * It display the filter bank list and an editable configuration.
 */
class CConfigScreen : public CWindowContent {

public:
    CConfigScreen(wxFrame* frame, wxWindow* window);
    ~CConfigScreen();

private:
    // Variable management functions
    void revert(bool confirm);
    void apply(bool confirm);
    bool hasChanged();
    void sortList(int numcol);
    void displayConfig();
    void selectItem(long index);
    string makeNewName(string oldname, string newname);

    // Managed variables
    map<string, vector<string> > config;   // name, list of filter titles

    // GUI variables
    int sortedColumn;
    int sortedDirection;
    string newActiveConfig;
    string editedConfigName;
    vector<string> editedConfig;
    map<string, string> categories;        // title, category
    map<string, string> types;             // title, filter type

    // Controls
	pmComboBox *nameEdit;
	pmListCtrl *allList;
	pmListCtrl *okList;
	pmTextCtrl *commentText;

    // Event handling function
    void OnCommand(wxCommandEvent& event);
    void OnListEvent(wxListEvent& event);

    // IDs
    enum {
        // Controls' ID
        ID_ALLLIST = 1600,
        ID_OKLIST,
        ID_DOWNBUTTON,
        ID_UPBUTTON,
        ID_REMOVEBUTTON,
        ID_ADDBUTTON,
        ID_NAMEEDIT,
        ID_COMMENTTEXT,
        // Menu's ID
        ID_CONFIGNEW,
        ID_CONFIGDUPLICATE,
        ID_CONFIGDELETE,
        ID_CONFIGREVERT,
        ID_CONFIGAPPLY,
        ID_CONFIGADD,
        ID_CONFIGREMOVE,
        ID_CONFIGMOVEUP,
        ID_CONFIGMOVEDOWN
    };

    // Event table
    DECLARE_EVENT_TABLE()
};

#endif

