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


#ifndef __settingsscreen__
#define __settingsscreen__

#include <string>
#include <set>
#include "controls.h"
#include "windowcontent.h"

using namespace std;

/* This class creates a box sizer to be set as the main frame sizer.
 * It display all Proximodo settings, except filters and configurations.
 */
class CSettingsScreen : public CWindowContent {

public:
    CSettingsScreen(wxFrame* frame, wxWindow* window);
    ~CSettingsScreen();

private:
    // Variable management functions
    void revert(bool confirm);
    void apply(bool confirm);
    bool hasChanged();

    // Managed variables
    string         language;
    string         currentConfig;
    string         proxyPort;
    bool           useNextProxy;
    string         nextProxy;
    set<string>    proxies;
    bool           allowIPRange;
    unsigned long  minIPRange;
    unsigned long  maxIPRange;
    string         bypass;
    bool           filterIn;
    bool           filterOut;
    bool           filterText;
    bool           filterGif;
    bool           showOnStartup;
    map<string,string> listNames;
    
    // GUI variables
    string currentListName;

    // Controls
    pmCheckBox *allowIPCheckbox;
    pmCheckBox *showCheckbox;
    pmCheckBox *filterInCheckbox;
    pmCheckBox *filterOutCheckbox;
    pmCheckBox *filterTextCheckbox;
    pmCheckBox *filterGifCheckbox;
    pmCheckBox *useProxyCheckbox;
    pmComboBox *configDropdown;
    pmComboBox *languageDropDown;
    pmComboBox *nextProxyDropdown;
    pmComboBox *listDropDown;
    pmTextCtrl *listFileText;
    pmTextCtrl *bypassText;
    pmTextCtrl *maxRangeText;
    pmTextCtrl *minRangeText;
    pmTextCtrl *portText;

    // Event handling function
    void OnCommand(wxCommandEvent& event);

    // IDs
    enum {
        ID_ALLOWIPCHECKBOX = 1300,
        ID_FILTERINCHECKBOX,
        ID_FILTEROUTCHECKBOX,
        ID_SHOWCHECKBOX,
        ID_FILTERTEXTCHECKBOX,
        ID_FILTERGIFCHECKBOX,
        ID_USEPROXYCHECKBOX,
        ID_APPLYBUTTON,
        ID_REVERTBUTTON,
        ID_NEXTPROXYBUTTON,
        ID_BYPASSTEXT,
        ID_MAXRANGETEXT,
        ID_MINRANGETEXT,
        ID_PORTTEXT,
        ID_CONFIGDROPDOWN,
        ID_LANGUAGEDROPDOWN,
        ID_NEXTPROXYDROPDOWN,
        ID_LISTNAMEDROPDOWN,
        ID_LISTFILETEXT,
        ID_NEWBUTTON,
        ID_OPENBUTTON,
        ID_CHOOSEBUTTON
    };

    // Event table
    DECLARE_EVENT_TABLE()
};

#endif

