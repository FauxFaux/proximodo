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


#include "welcomescreen.h"
#include <wx/sizer.h>
#include <wx/bitmap.h>
#include <wx/statbmp.h>
#include "images/logo240.xpm"

using namespace std;

CWelcomeScreen::CWelcomeScreen(wxFrame* frame, wxWindow* window) :
                                CWindowContent(frame, window) {

    wxBoxSizer* content = new wxBoxSizer(wxVERTICAL);
    
    wxStaticBitmap* bitmap = new wxStaticBitmap(window, wxID_ANY,
       wxBitmap(logo240_xpm) );
    content->Add(bitmap, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT, 50 );
    
    Add(0,0,1);
    Add(content, 0, wxALIGN_CENTER | wxALL, 50 );
    Add(0,0,1);

    makeSizer();
}

CWelcomeScreen::~CWelcomeScreen() {
}

