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


#include "proxy.h"
#include <wx/url.h>
#include <wx/stream.h>
#include "log.h"
#include "util.h"
#include "settings.h"
#include "events.h"

using namespace std;

enum {
    // id for sockets
    SERVER_ID
};


BEGIN_EVENT_TABLE(CProxy, wxEvtHandler)
  EVT_SOCKET(SERVER_ID, CProxy::OnServerEvent)
END_EVENT_TABLE()


/* The address of the instance
 */
CProxy* CProxy::instance = NULL;


/* Method to obtain the instance address
 */
CProxy& CProxy::ref() {

    if (!instance) instance = new CProxy();
    return *instance;
}


/* Method to destroy safely the instance (better than delete)
 */
void CProxy::destroy() {

    if (instance) delete instance;
    instance = NULL;
}


/* Constructor
 */
CProxy::CProxy() {

    server = NULL;
    accepting = true;
}


/* Destructor
 */
CProxy::~CProxy() {

    // Close all sockets
    closeProxyPort();
    abortConnections();

    // Terminate all threads
    for (vector<CManagerThread*>::iterator it = threads.begin();
                it != threads.end(); it++) {
        (*it)->Delete();
        delete *it;
    }

    // Destroy all managers
    CUtil::deleteVector<CRequestManager>(managers);
}


/* Test a port
 */
bool CProxy::testPort(string port) {

    wxIPV4address addr;
    addr.Service(port.c_str());
    if (server) {
        wxIPV4address local;
        server->GetLocal(local);
        if (addr.Service() == local.Service()) return true;
    }
    wxSocketServer test(addr);
    return test.Ok();
}


/* Test a remote proxy
 */
bool CProxy::testRemoteProxy(string hostport) {

    unsigned int colon = hostport.find(':');
    if (colon == string::npos) return false;

    wxIPV4address addr;
    addr.Hostname(hostport.substr(0, colon).c_str());
    addr.Service(hostport.substr(colon + 1).c_str());
    if (addr.IsLocalHost() && server) {
        wxIPV4address local;
        server->GetLocal(local);
        if (addr.Service() == local.Service()) return false;
    }

    wxSocketClient sock;
    sock.Connect(addr,false);
    sock.WaitOnConnect(1);
    if (!sock.IsConnected()) return false;

    string mess = "GET http://www.google.com/ HTTP/1.1\r\nHost: www.google.com\r\n\r\n";
    sock.Write(mess.c_str(), mess.length());
    char buf[15]; memset(buf, 0, 15); sock.Read(buf, 15); string resp(buf, 15);
    if (resp.find(' ') == string::npos) return false;
    resp.erase(0, resp.find(' ') + 1);
    if (resp.find(' ') == string::npos) return false;
    resp.erase(resp.find(' '));
    stringstream code(resp);
    int num; code >> num;
    return (num >= 100 && num < 400);
}


/* Attempts opening server socket
 */
bool CProxy::openProxyPort() {

    // Create the socket
    if (!server) {

        // Create the address (default host is localhost)
        wxIPV4address addr;
        addr.Service(CSettings::ref().proxyPort.c_str());

        // Create the socket
        server = new wxSocketServer(addr);
    }
    
    // Check if the port is open
    if (!server->Ok()) {
        server->Destroy();
        server = NULL;
        return false;
    }
    
    // Setup the event handler and subscribe to connection events
    server->SetEventHandler(*this, SERVER_ID);
    server->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST);
    server->Notify(TRUE);
    accepting = true;
    wxIPV4address addr;
    server->GetLocal(addr);
    CLog::ref().logProxyEvent(pmEVT_PROXY_TYPE_START, addr);
    return true;
}


/* Closes server socket
 */
void CProxy::closeProxyPort() {

    if (!server) return;
    wxIPV4address addr;
    server->GetLocal(addr);
    CLog::ref().logProxyEvent(pmEVT_PROXY_TYPE_STOP, addr);
    server->Destroy();
    server = NULL;
    accepting = false;
}


/* Accepting status access methods
 */
void CProxy::acceptConnections(bool accept) {
    accepting = accept;
}

bool CProxy::acceptConnections() {
    return accepting;
}


/* Aborts all current connections
 */
void CProxy::abortConnections() {

    for (vector<CRequestManager*>::iterator it = managers.begin();
                it != managers.end(); it++) {
        (*it)->abort();
    }
}


/* Marks all current managers as obsolete
 */
void CProxy::refreshManagers() {

    for (vector<CRequestManager*>::iterator it = managers.begin();
                it != managers.end(); it++) {
        (*it)->valid = false;
    }
}


/* Called by server socket's event manager
 */
void CProxy::OnServerEvent(wxSocketEvent& event) {

    if (event.GetSocketEvent() == wxSOCKET_LOST) {
        // Error: the server socket closed
        closeProxyPort();
        return;
    }

    CSettings& settings = CSettings::ref();

    // Create a socket for the new connection
    wxSocketBase *sock;
    sock = server->Accept(FALSE);
    if (!sock) {
        // Error: couldn't technically accept a new connection
        return;
    }

    // Read the connection properties
    wxIPV4address local, peer;
    sock->GetLocal(local);
    sock->GetPeer(peer);
    unsigned long peerIP = CUtil::fromDotted(local.IPAddress().c_str());

    // Check if connection satisfies all conditions
    if (  !accepting
        || peer.IPAddress() != local.IPAddress()
        && (  !settings.allowIPRange
            || settings.minIPRange > peerIP
            || settings.maxIPRange < peerIP ) ) {
        // Log refused connection
        CLog::ref().logProxyEvent(pmEVT_PROXY_TYPE_REFUSE, peer);
        // Drop connection
        sock->Destroy();
        return;
    }
    
    // Log accepted connection
    CLog::ref().logProxyEvent(pmEVT_PROXY_TYPE_ACCEPT, peer);

    // Replace obsolete & unused managers
    vector<CRequestManager*>::iterator itm = managers.begin();
    while (itm != managers.end()) {
        if ((*itm)->available && !(*itm)->valid) {
            delete *itm;
            managers.erase(itm);
            itm = managers.begin();
        } else {
            itm++;
        }
    }

    // Delete terminated thread objects
    vector<CManagerThread*>::iterator itt = threads.begin();
    while (itt != threads.end()) {
        if (!(*itt)->IsRunning()) {
            (*itt)->Delete();
            delete *itt;
            threads.erase(itt);
            itt = threads.begin();
        } else {
            itt++;
        }
    }

    // Choose a manager to process this connection
    CRequestManager* manager;
    for (itm = managers.begin(); itm!=managers.end()
                                 && !(*itm)->available; itm++);
    if (itm!=managers.end()) {
        // Reuse an existing available manager
        manager = *itm;
        manager->available = false;
    } else {
        // Create and record a new manager
        manager = new CRequestManager();
        managers.push_back(manager);
    }

    // Send the socket to the chosen manager, running in a new thread
    CManagerThread* thread = new CManagerThread(sock, manager);
    if (thread->Create() == wxTHREAD_NO_ERROR) {
        thread->Run();
        threads.push_back(thread);
    } else {
        thread->Delete();
        delete thread;
    }
}

