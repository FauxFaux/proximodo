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


#ifndef __testframe__
#define __testframe__

#include <wx/frame.h>
#include <wx/sizer.h>
#include "descriptor.h"
#include "controls.h"

/* This class creates the main Proximodo window, with toolbar and status bar.
 * The controls displayed within the window are created and managed by a
 * sizer attributed to the main window.
 */
class CTestFrame : public wxFrame {

public:
    CTestFrame(CFilterDescriptor& d, CTestFrame** p);
    ~CTestFrame();
    

private:
    // Event functions
    void OnClose(wxCloseEvent& event);
    void OnCommand(wxCommandEvent& event);
    
    // Filter to use
    CFilterDescriptor& desc;
    
    // Address that we'll set to NULL when the frame is destroyed
    CTestFrame** pointer;
    
    
    // Controls
	pmTextCtrl *resultMemo;
	pmTextCtrl *testMemo;

    // IDs
    enum {
        // Buttons
        ID_TEST = 1500,
        ID_TESTCLOSE,
        ID_TESTTEXT
    };

    // For event management
    DECLARE_EVENT_TABLE()
};

#endif
