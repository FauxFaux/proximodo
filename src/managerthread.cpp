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


#include "managerthread.h"

/* Constructor
 */
CManagerThread::CManagerThread(wxSocketBase* sock, CRequestManager* manager) :
        wxThread(wxTHREAD_JOINABLE), sock(sock), manager(manager) {
}


/* Thread code. It provides the given manager with the browser socket,
 * and waits for it being available again. Then the thread ends.
 * We need to wait because manage() returns immediately, socket events
 * triggering the data processing.
 */
wxThread::ExitCode CManagerThread::Entry() {
    manager->manage(sock);
    return 0;
}

