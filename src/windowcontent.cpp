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


#include "windowcontent.h"
#include "util.h"

/* Constructor
 */
CWindowContent::CWindowContent(wxFrame* frame, wxWindow* window, int orient) :
                    wxBoxSizer(orient), window(window), frame(frame),
                    settings(CSettings::ref()) {

    window->SetSizer(this);
}


/* At the end of inherited class instanciation, this function fills controls
 * and resizes the window.
 */
void CWindowContent::makeSizer() {

    revert(false);
    frame->PushEventHandler(this);  // (to get menu events from menubar)
}


/* Destructor
 */
CWindowContent::~CWindowContent() {

    frame->RemoveEventHandler(this);
    DeleteWindows();
}


/* This function moves focus, in order to trigger kill_focus events on
 * custom text controls. This triggering must happen to prevent losing
 * the text just typed in, e.g before applying settings.
 */
void CWindowContent::commitText() {

    wxWindow* focused = wxWindow::FindFocus();
    if (!focused) return;
    wxWindow temp(window,wxID_ANY, wxDefaultPosition, wxSize(0,0));
    temp.SetFocus();
    focused->SetFocus();
}

