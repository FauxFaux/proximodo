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


#include "requestmanager.h"
#include <sstream>
#include <map>
#include <wx/thread.h>
#include <wx/filefn.h>
#include <wx/file.h>
#include "util.h"
#include "settings.h"
#include "log.h"
#include "const.h"
#include "matcher.h"

using namespace std;

/* Constructor
 */
CRequestManager::CRequestManager() {

    valid = true;
    available = false;
    browser = NULL;
    website = NULL;
    urlMatcher = NULL;
    urlFilter = NULL;
    chain = this;
    reqNumber = 0;
    cnxNumber = 0;
    GIFfilter = new CGifFilter(this);

    // Retrieve filter descriptions to embed
    CSettings& se = CSettings::ref();
    vector<string> filterNames = se.config[se.currentConfig];
    vector<CFilterDescriptor> filters;
    for (unsigned int i=0; i<filterNames.size(); i++) {
        map<string, CFilterDescriptor>::iterator it;
        it = se.filterbank.find(filterNames[i]);
        if (it != se.filterbank.end()) {
            filters.push_back(it->second);
        }
    }

    // Create header filters
    for (unsigned int i=0; i<filters.size(); i++) {
        try {
            if (filters[i].filterType == CFilterDescriptor::HEADOUT)
                OUTfilters.push_back(new CHeaderFilter(filters[i], *this));

            else if (filters[i].filterType == CFilterDescriptor::HEADIN)
                INfilters.push_back(new CHeaderFilter(filters[i], *this));
        } catch (parsing_exception) {
            // Invalid filters are just ignored
        }
    }

    // Text filters must be created and chained in reverse order;
    for (int i = (int)filters.size()-1; i>=0; i--) {
        try {
            if (filters[i].filterType == CFilterDescriptor::TEXT) {
                CTextFilter* filter = new CTextFilter(*this, filters[i], chain);
                TEXTfilters.push_back(filter);
                chain = filter;
            }
        } catch (parsing_exception) {
            // Invalid filters are just ignored
        }
    }

    // Construct bypass-URL matcher
    urlFilter = new CFilter(*this);
    if (!CSettings::ref().bypass.empty()) {
        try {
            urlMatcher = new CMatcher(url.getUrl(), CSettings::ref().bypass,
                                      *urlFilter);
        } catch (parsing_exception e) {
            // Ignore invalid bypass pattern
        }
    }
}


/* Destructor
 */
CRequestManager::~CRequestManager() {

    destroy();

    if (urlMatcher) delete urlMatcher;
    if (urlFilter)  delete urlFilter;

    CUtil::deleteVector<CTextFilter>(TEXTfilters);
    CUtil::deleteVector<CHeaderFilter>(OUTfilters);
    CUtil::deleteVector<CHeaderFilter>(INfilters);
    delete GIFfilter;
}


/* Main loop with socket I/O and data processing.
 *
 * Note: One could add  && (!wxThread::This() || !wxThread::This()->TestDestroy())
 * to the two while() conditions, but since we only call wxThread::Destroy()
 * on terminated threads or after calling abort(), the condition
 * browser->IsConnected() is enough
 */
void CRequestManager::manage(wxSocketBase* browser) {

    this->browser = browser;
    browser->GetPeer(addr);
    website = new wxSocketClient();

    // Update statistics
    ++CLog::ref().numOpenSockets;
    CLog::ref().logProxyEvent(pmEVT_PROXY_TYPE_NEWSOCK, addr);
    available = false;
    cnxNumber = ++CLog::ref().numConnections;

    // Reset processing states
    recvOutBuf.clear();
    sendOutBuf.clear();
    outStep = STEP_START;
    recvInBuf.clear();
    sendInBuf.clear();
    inStep  = STEP_START;
    previousHost.clear();

    // Reactivate all header filters
    for (vector<CHeaderFilter*>::iterator it =
            OUTfilters.begin(); it != OUTfilters.end(); it++)
        (*it)->active = true;
    for (vector<CHeaderFilter*>::iterator it =
            INfilters.begin(); it != INfilters.end(); it++)
        (*it)->active = true;

    // Main loop, continues as long as the browser is connected
    while (browser->IsConnected()) {

        // Process only outgoing data
        bool rest = true;
        if (receiveOut()) { rest = false; processOut(); }
        if (sendIn())     { rest = false; }

        if (website->IsConnected()) {
            do {
                // Full processing
                bool rest = true;
                if (sendOut())      { rest = false; }
                if (receiveIn())    { rest = false; processIn(); }
                if (sendIn())       { rest = false; }
                if (receiveOut())   { rest = false; processOut(); }
                if (rest) wxThread::Sleep(5); else wxThread::Yield();

            } while (website->IsConnected() && browser->IsConnected());

            // Terminate feeding browser
            receiveIn();
            processIn();
            if (inStep == STEP_RAW || inStep == STEP_CHUNK) {
                endFeeding();
                inStep = STEP_FINISH;
                processIn();
            }
            sendIn();
        }

        if (rest) wxThread::Sleep(50); else wxThread::Yield();
    }

    destroy();
}


/* Close the browser connection, if connected.
 * At next execution loop in the dedicated thread, the
 * sockets will be destroyed and the statistics updated.
 */
void CRequestManager::abort() {

    if (browser && browser->IsConnected())
        browser->Close();
}


/* Destroy sockets and update statistics
 */
void CRequestManager::destroy() {

    if (!browser) return; // (already done)

    // Destroy sockets
    browser->Close();
    website->Close();
    browser->Destroy();
    website->Destroy();
    browser = NULL;
    website = NULL;

    // If we were processing outgoing or incoming
    // data, we were an active request
    if (outStep != STEP_START || inStep != STEP_START) {
        --CLog::ref().numActiveRequests;
        CLog::ref().logProxyEvent(pmEVT_PROXY_TYPE_ENDREQ, addr);
    }

    // Update statistics
    --CLog::ref().numOpenSockets;
    CLog::ref().logProxyEvent(pmEVT_PROXY_TYPE_ENDSOCK, addr);
    available = true;
}


/* Receive outgoing data from browser
 */
bool CRequestManager::receiveOut() {

    // Read socket by blocks
    char buf[CRM_READSIZE];
    browser->SetFlags(wxSOCKET_NOWAIT);
    browser->Read(buf, CRM_READSIZE);
    int count = browser->LastCount();
    bool ret = count;
    while (count > 0 && browser) {
        recvOutBuf += string(buf, count);
        browser->SetFlags(wxSOCKET_NOWAIT);
        browser->Read(buf, CRM_READSIZE);
        count = browser->LastCount();
    }
    return ret;
}


/* Send outgoing data to website
 */
bool CRequestManager::sendOut() {

    bool ret = false;
    // Send everything to website, if connected
    if (!sendOutBuf.empty() && website->IsConnected()) {
        ret = true;
        website->SetFlags(wxSOCKET_WAITALL);
        website->Write(sendOutBuf.c_str(), sendOutBuf.size());
        sendOutBuf.erase(0, website->LastCount());
        if (website->Error()) {
            // Trouble with website's socket, end of all things
            browser->Close();
        }
    }
    return ret;
}


/* Receive incoming data from website
 */
bool CRequestManager::receiveIn() {

    // Read socket by blocks
    char buf[CRM_READSIZE];
    website->SetFlags(wxSOCKET_NOWAIT);
    website->Read(buf, CRM_READSIZE);
    int count = website->LastCount();
    bool ret = count;
    while (count > 0) {
        recvInBuf += string(buf, count);
        website->SetFlags(wxSOCKET_NOWAIT);
        website->Read(buf, CRM_READSIZE);
        count = website->LastCount();
    }
    return ret;
}


/* Send incoming data to browser
 */
bool CRequestManager::sendIn() {

    bool ret = false;
    // Send everything to browser, if connected
    if (!sendInBuf.empty() && browser->IsConnected()) {
        ret = true;
        browser->SetFlags(wxSOCKET_WAITALL);
        browser->Write(sendInBuf.c_str(), sendInBuf.size());
        sendInBuf.erase(0, browser->LastCount());
        if (browser->Error()) {
            // Trouble with browser's socket, end of all things
            browser->Close();
        }
    }
    return ret;
}


/* Buffer data went trough filter chain, make a chunk
 */
void CRequestManager::dataFeed(const string& data) {

    unsigned int size = data.size();
    if (size && sendContentCoding
        && (useChain || recvContentCoding != sendContentCoding)) {

        // Send a chunk of compressed data
        string tmp;
        compressor.feed(data);
        compressor.read(tmp);
        size = tmp.size();
        if (size)
            sendInBuf += "\r\n" + CUtil::makeHex(size) + "\r\n" + tmp;

    } else if (size) {

        // Send a chunk of uncompressed/unchanged data
        sendInBuf += "\r\n" + CUtil::makeHex(size) + "\r\n" + data;
    }
}


/* Record that the chain emptied itself, finish compression if needed
 */
void CRequestManager::dataDump() {

    if (sendContentCoding
        && (useChain || recvContentCoding != sendContentCoding)) {

        string tmp;
        compressor.dump();
        compressor.read(tmp);
        unsigned int size = tmp.size();
        if (size)
            sendInBuf += "\r\n" + CUtil::makeHex(size) + "\r\n" + tmp;
    }
}


/* Needed for CDataReceptor implementation
 */
void CRequestManager::dataReset() {
    // Nothing to do: we are at the origin of the cascading call
}


/* Determine if a MIME type can go through body filters
 */
bool CRequestManager::canFilter(string& content) {
    unsigned int end = content.find(';');
    if (end == string::npos) end = content.size();
    string type = content.substr(0, end);
    CUtil::lower(type);
    return (type == "text/html" ||
            type == "application/x-javascript" ||
            type == "text/asp");
}


/* Process outgoing data (from browser to website)
 */
void CRequestManager::processOut() {

    // We will exit from a step that has not enough data to complete
    while (1) {

        // Marks the beginning of a request
        if (outStep == STEP_START) {

            // We need something in the buffer
            if (recvOutBuf.empty()) return;
            outStep = STEP_FIRSTLINE;

            // Update active request stat, but if processIn() did it already
            if (inStep == STEP_START) {
                ++CLog::ref().numActiveRequests;
                CLog::ref().logProxyEvent(pmEVT_PROXY_TYPE_NEWREQ, addr);
            }

            // Reset variables relative to a single request/response
            bypassOut = false;
            bypassIn = false;
            bypassBody = false;
            bypassBodyForced = false;
            variables.clear();
            reqNumber = ++CLog::ref().numRequests;
            jumpToHost.clear();
            rdirToHost.clear();
            recvConnectionClose = sendConnectionClose = false;
            recvContentCoding = sendContentCoding = 0;
        }

        // Read first HTTP request line
        else if (outStep == STEP_FIRSTLINE) {

            // Do we have the full first line yet?
            unsigned int crlf = recvOutBuf.find("\r\n");
            if (crlf == string::npos) return;

            // Get it and record it
            requestLine = recvOutBuf.substr(0, crlf+2);
            logRequest = requestLine;
            recvOutBuf.erase(0, crlf+2);

            // Next step will be to read headers
            outStep = STEP_HEADERS;

            // Unless we have Content-Length or Transfer-Encoding headers,
            // we won't expect anything after headers.
            outSize = 0;
            outChunked = false;
            outHeaders.clear();
            // (in case a silly OUT filter wants to read IN headers)
            inHeaders.clear();
            responseCode.clear();
        }

        // Read and process headers, as long as there are any
        else if (outStep == STEP_HEADERS) {

            while (true) {
                // Look for end of line
                unsigned int crlf = recvOutBuf.find("\r\n");

                // Check if we reached the empty line
                if (crlf == 0) {
                    outStep = STEP_DECODE;
                    break;
                }

                // Find the header end
                while (    crlf != string::npos
                        && (   crlf+2 >= recvOutBuf.size()
                            || recvOutBuf[crlf+2] == ' '
                            || recvOutBuf[crlf+2] == '\t')) {
                    crlf = recvOutBuf.find("\r\n", crlf+2);
                }

                // If we don't have it, wait for more
                if (crlf == string::npos) return;

                // Record header
                unsigned int colon = recvOutBuf.find(':');
                if (colon != string::npos) {
                    SHeader header;
                    header.name = recvOutBuf.substr(0, colon);
                    header.content = recvOutBuf.substr(colon+1, crlf-colon-1);
                    CUtil::trim(header.content);
                    outHeaders.push_back(header);

                    // We must find the Content-Length and Transfer-Encoding
                    // headers to know if there is a body
                    if (CUtil::noCaseEqual(header.name, "Transfer-Encoding")) {
                        if (!CUtil::noCaseEqual(header.content.substr(0,8),
                                                "identity"))
                            outChunked = true;
                    } else if (CUtil::noCaseEqual(header.name, "Content-Length")) {
                        stringstream ss(header.content);
                        ss >> outSize;
                    }
                }
                logRequest += recvOutBuf.substr(0, crlf+2);
                recvOutBuf.erase(0, crlf+2);
            }
        }

        // Decode and process headers
        else if (outStep == STEP_DECODE) {

            CLog::ref().logHttpEvent(pmEVT_HTTP_TYPE_RECVOUT, addr,
                                      reqNumber, logRequest);

            // Parse request line to get the URL
            unsigned int p1 = requestLine.find_first_of(" ");
            if (p1 == string::npos) p1 = 0; else ++p1;
            unsigned int p2 = requestLine.find_first_of(" ", p1);
            if (p2 == string::npos) p2 = p1;
            string urlstr = requestLine.substr(p1, p2-p1);
            url.parseUrl(urlstr);
            fileType = evaluateType(url);

            // ... and the host to contact (unless we use a proxy)
            useSettingsProxy = CSettings::ref().useNextProxy;
            contactHost = url.getHostPort();

            // If host is local.ptron, we will not filter the request
            if (CUtil::noCaseEqual("local.ptron", url.getHost())) {
                bypassOut = bypassIn = bypassBody = true;
            }

            // Test URL with bypass-URL matcher, if matches we'll bypass all
            if (urlMatcher) {
                urlFilter->clearMemory();
                int tmp1, tmp2;
                if (urlMatcher->match(0, url.getUrl().size(), tmp1, tmp2)) {
                    bypassOut = bypassIn = bypassBody = true;
                }
            }

            // We'll work on a copy, since we don't want to alter
            // the real headers that $IHDR and $OHDR may access
            vector<SHeader> outHeadersFiltered = outHeaders;

            if (!bypassOut && CSettings::ref().filterOut) {

                // Let URL through URL* OUT filters
                for (vector<CHeaderFilter*>::iterator itf =
                        OUTfilters.begin(); itf != OUTfilters.end(); itf++) {
                    if (CUtil::noCaseBeginsWith("url", (*itf)->getHeaderName())) {
                        // filter works on a copy of the URL, w/o protocol://
                        string test = url.getFromHost();
                        (*itf)->filter(test);
                        CUtil::trim(test);
                        // if filter changed the url, update variables
                        if (!test.empty() && test != url.getFromHost()) {
                            // We won't change contactHost if it has been
                            // set to a proxy address by a $SETPROXY command
                            bool changeHost = (contactHost == url.getHostPort());
                            // update URL
                            url.parseUrl(url.getProtocol() + "://" + test);
                            fileType = evaluateType(url);
                            if (changeHost) contactHost = url.getHostPort();
                            // update request line
                            unsigned int pos1 = requestLine.find(' ');
                            if (pos1 == string::npos) pos1 = 0; else pos1++;
                            unsigned int pos2 = requestLine.find(' ', pos1);
                            if (pos2 == string::npos) pos2 = pos1;
                            requestLine = requestLine.substr(0, pos1) + url.getUrl()
                                        + requestLine.substr(pos2);
                        }
                    }
                }

                // Add missing headers to be processed: some are probably not
                // provided by the browser, but we want to invoke all filters
                // at least once, and thus perhaps _add_ wanted headers.
                set<string> names;
                for (vector<SHeader>::iterator it = outHeadersFiltered.begin();
                        it != outHeadersFiltered.end(); it++) {
                    string name = it->name;
                    names.insert(CUtil::lower(name));
                }
                for (vector<CHeaderFilter*>::iterator it = OUTfilters.begin();
                        it != OUTfilters.end(); it++) {
                    string name = (*it)->getHeaderName();
                    CUtil::lower(name);
                    if (name.substr(0,3) != "url" && names.find(name) == names.end()) {
                        SHeader header = { (*it)->getHeaderName(), string("") };
                        outHeadersFiltered.push_back(header);
                    }
                }

                // Process headers one by one
                for (vector<SHeader>::iterator ith  = outHeadersFiltered.begin();
                                               ith != outHeadersFiltered.end();
                                               ith++) {
                    for (vector<CHeaderFilter*>::iterator itf =
                            OUTfilters.begin(); itf != OUTfilters.end(); itf++) {
                        if (CUtil::noCaseEqual(ith->name, (*itf)->getHeaderName())) {
                            (*itf)->filter(ith->content);
                            break;
                        }
                    }
                }
            }

            // Test for transparent redirection to URL ($RDIR)
            // Note: new URL will not go through URL* OUT filters
            if (!rdirToHost.empty() &&
                !CUtil::noCaseBeginsWith("http://file//", rdirToHost)) {

                // Change URL (for filters)
                url.parseUrl(rdirToHost);
                fileType = evaluateType(url);
                // Change destination
                useSettingsProxy = CSettings::ref().useNextProxy;
                contactHost = url.getHostPort();
                // Change request line
                unsigned int pos1 = requestLine.find(' ');
                if (pos1 == string::npos) pos1 = 0; else pos1++;
                unsigned int pos2 = requestLine.find(' ', pos1);
                if (pos2 == string::npos) pos2 = pos1;
                requestLine = requestLine.substr(0, pos1) + rdirToHost
                            + requestLine.substr(pos2);
                // Change Host header field
                for (vector<SHeader>::iterator it  = outHeaders.begin();
                               it != outHeaders.end(); it++) {
                    if (it->name == "host") {
                        it->content = url.getHost();
                        break;
                    }
                }
                for (vector<SHeader>::iterator it  = outHeadersFiltered.begin();
                               it != outHeadersFiltered.end(); it++) {
                    if (it->name == "host") {
                        it->content = url.getHost();
                        break;
                    }
                }
                rdirToHost.clear();
            }

            // Now we can put everything in the filtered buffer
            sendOutBuf = requestLine;
            for (vector<SHeader>::iterator it = outHeadersFiltered.begin();
                    it != outHeadersFiltered.end(); it++) {
                if (!it->content.empty()) {
                    sendOutBuf += it->name + ": " + it->content + "\r\n";
                }
            }

            // Decide next step
            if (CUtil::noCaseBeginsWith("HEAD ", requestLine)) {
                // HEAD requests have no body, even if a size is provided
                recvOutBuf.erase(0, 2);
                sendOutBuf += "\r\n";
                outStep = STEP_FINISH;
            } else if (outChunked) {
                outStep = STEP_CHUNK;
                logPostData.clear();
            } else if (outSize) {
                // Add the blank line and read raw body
                recvOutBuf.erase(0, 2);
                sendOutBuf += "\r\n";
                outStep = STEP_RAW;
                logPostData.clear();
            } else {
                recvOutBuf.erase(0, 2);
                sendOutBuf += "\r\n";
                outStep = STEP_FINISH;
            }

            // If we haven't connected to this host yet, we do it now.
            // This will allow incoming processing to start. If the
            // connection fails, incoming processing will jump to
            // the finish state, so that we can accept other requests
            // on the same browser socket (persistent connection).
            connectWebsite();
        }

        // Read \r\n HexRawLength * \r\n before raw data
        // or \r\n zero * \r\n\r\n
        else if (outStep == STEP_CHUNK) {

            if (recvOutBuf.substr(0, 2) != "\r\n") return;
            unsigned int crlf = recvOutBuf.find("\r\n", 2);
            if (crlf == string::npos) return;
            outSize = CUtil::readHex(recvOutBuf.substr(2,crlf-2));

            if (outSize > 0) {
                sendOutBuf += recvOutBuf.substr(0, crlf+2);
                recvOutBuf.erase(0, crlf+2);
                outStep = STEP_RAW;
            } else {
                crlf = recvOutBuf.find("\r\n\r\n", 2);
                if (crlf == string::npos) return;
                sendOutBuf += recvOutBuf.substr(0, crlf+4);
                recvOutBuf.erase(0, crlf+4);
                CLog::ref().logHttpEvent(pmEVT_HTTP_TYPE_POSTOUT, addr,
                                         reqNumber, logPostData);
                outStep = STEP_FINISH;
            }

            // We shouldn't send anything to website if it is not expecting
            // data (i.e we obtained a faked response)
            if (inStep == STEP_FINISH) sendOutBuf.clear();
        }

        // The next outSize bytes are left untouched
        else if (outStep == STEP_RAW) {

            int copySize = recvOutBuf.size();
            if (copySize > outSize) copySize = outSize;
            if (!copySize) return;
            string postData = recvOutBuf.substr(0, copySize);
            sendOutBuf += postData;
            logPostData += postData;
            recvOutBuf.erase(0, copySize);
            outSize -= copySize;

            // If we finished reading as much raw data as was expected,
            // continue to next step (finish or next chunk)
            if (!outSize) {
                if (outChunked) {
                    outStep = STEP_CHUNK;
                } else {
                    CLog::ref().logHttpEvent(pmEVT_HTTP_TYPE_POSTOUT, addr,
                                             reqNumber, logPostData);
                    outStep = STEP_FINISH;
                }
            }

            // We shouldn't send anything to website if it is not expecting
            // data (i.e we obtained a faked response)
            if (inStep == STEP_FINISH) sendOutBuf.clear();
        }

        // We'll wait for response completion
        else if (outStep == STEP_FINISH) {

            if (inStep != STEP_FINISH) return;

            outStep = STEP_START;
            inStep = STEP_START;

            // Update active request stat
            --CLog::ref().numActiveRequests;
            CLog::ref().logProxyEvent(pmEVT_PROXY_TYPE_ENDREQ, addr);

            // Disconnect browser if we sent it "Connection:close"
            // If manager has been set invalid, close connection too
            if (sendConnectionClose || !valid) {
                sendIn();
                browser->Close();
            }

            // Disconnect website if it sent us "Connection:close"
            if (recvConnectionClose || browser->IsDisconnected())
                website->Close();
        }
    }
}


/* Process incoming data (from website to browser)
 */
void CRequestManager::processIn() {

    // We will exit from a step that has not enough data to complete
    while (1) {

        // Marks the beginning of a request
        if (inStep == STEP_START) {

            // We need something in the buffer
            if (recvInBuf.empty()) return;
            inStep = STEP_FIRSTLINE;
            // Update active request stat, but if processOut() did it already
            if (outStep == STEP_START) {
                ++CLog::ref().numActiveRequests;
                CLog::ref().logProxyEvent(pmEVT_PROXY_TYPE_NEWREQ, addr);
            }
        }

        // Read first HTTP response line
        else if (inStep == STEP_FIRSTLINE) {

            // Do we have the full first line yet?
            unsigned int crlf = recvInBuf.find("\r\n");
            if (crlf == string::npos) return;

            // Get it and remove it from receive-in buffer
            // we don't place it immediately on send-in buffer,
            // as we may be willing to fake it (cf $REDIR)
            responseLine = recvInBuf.substr(0, crlf+2);
            logResponse = responseLine;
            recvInBuf.erase(0, crlf+2);

            // Parse it
            unsigned int p1 = responseLine.find_first_of(" ", 0);
            if (p1 == string::npos) p1 = 0; else p1++;
            responseCode = responseLine.substr(p1);
            CUtil::trim(responseCode);

            // Next step will be to read headers
            inStep = STEP_HEADERS;

            // Unless we have Content-Length or Transfer-Encoding headers,
            // we won't expect anything after headers.
            inSize = 0;
            inChunked = false;
            inHeaders.clear();
            responseCode.clear();
            recvConnectionClose = sendConnectionClose = false;
            recvContentCoding = sendContentCoding = 0;
            useGifFilter = false;
        }

        // Read and process headers, as long as there are any
        else if (inStep == STEP_HEADERS) {

            while (true) {
                // Look for end of line
                unsigned int crlf =recvInBuf.find("\r\n");

                // Check if we reached the empty line
                if (crlf == 0) {
                    inStep = STEP_DECODE;
                    break;
                }

                // Find the header end
                while (    crlf != string::npos
                        && (   crlf+2 >= recvInBuf.size()
                            || recvInBuf[crlf+2] == ' '
                            || recvInBuf[crlf+2] == '\t')) {
                    crlf = recvInBuf.find("\r\n", crlf+2);
                }

                // If we don't have it, wait for more
                if (crlf == string::npos) return;

                // Record header
                unsigned int colon = recvInBuf.find(':');
                if (colon != string::npos) {
                    SHeader header;
                    header.name = recvInBuf.substr(0, colon);
                    header.content = recvInBuf.substr(colon+1, crlf-colon-1);
                    CUtil::trim(header.content);
                    inHeaders.push_back(header);

                    // We must find the Content-Length and Transfer-Encoding
                    // headers to know body length
                    if (CUtil::noCaseEqual(header.name, "Transfer-Encoding")) {
                        if (!CUtil::noCaseContains("identity", header.content))
                            inChunked = true;
                    } else if (CUtil::noCaseEqual(header.name, "Content-Length")) {
                        stringstream ss(header.content);
                        ss >> inSize;
                    } else if (CUtil::noCaseEqual(header.name, "Content-Type")) {
                        // In case size is not given, we'll read body until
                        // connection closes
                        if (!inSize) inSize = BIG_NUMBER;
                        // Check for filterable MIME types
                        if (!canFilter(header.content) && !bypassBodyForced) {
                            bypassBody = true;
                        }
                        // Check for GIF to freeze
                        if (CUtil::noCaseContains("image/gif", header.content)) {
                            useGifFilter = CSettings::ref().filterGif;
                        }
                    } else if (CUtil::noCaseEqual(header.name, "Connection")) {
                        if (CUtil::noCaseContains("close", header.content))
                            recvConnectionClose = true;
                    } else if (CUtil::noCaseEqual(header.name, "Content-Encoding")) {
                        if (CUtil::noCaseContains("gzip", header.content)) {
                            recvContentCoding = 1;
                            decompressor.reset(false, true);
                        } else if (CUtil::noCaseContains("deflate", header.content)) {
                            recvContentCoding = 2;
                            decompressor.reset(false, false);
                        } else if (CUtil::noCaseContains("compress", header.content)) {
                            bypassBody = true;
                        }
                    }
                }
                logResponse += recvInBuf.substr(0, crlf+2);
                recvInBuf.erase(0, crlf+2);
            }
        }

        // Decode and process headers
        else if (inStep == STEP_DECODE) {

            CLog::ref().logHttpEvent(pmEVT_HTTP_TYPE_RECVIN, addr,
                                      reqNumber, logResponse);

            // We'll work on a copy, since we don't want to alter
            // the real headers that $IHDR and $OHDR may access
            vector<SHeader> inHeadersFiltered = inHeaders;

            if (!bypassIn && CSettings::ref().filterIn) {

                // Add missing headers to be processed (some
                // are probably not provided by the browser)
                set<string> names;
                for (vector<SHeader>::iterator it = inHeadersFiltered.begin();
                        it != inHeadersFiltered.end(); it++) {
                    string name = it->name;
                    names.insert(CUtil::lower(name));
                }
                for (vector<CHeaderFilter*>::iterator it = INfilters.begin();
                        it != INfilters.end(); it++) {
                    string name = (*it)->getHeaderName();
                    if (names.find(CUtil::lower(name)) == names.end()) {
                        SHeader header = { (*it)->getHeaderName(), string("") };
                        inHeadersFiltered.push_back(header);
                    }
                }

                // Process headers one by one
                for (vector<SHeader>::iterator ith  = inHeadersFiltered.begin();
                                               ith != inHeadersFiltered.end();
                                               ith++) {
                    for (vector<CHeaderFilter*>::iterator itf =
                            INfilters.begin(); itf != INfilters.end(); itf++) {
                        if (CUtil::noCaseEqual(ith->name, (*itf)->getHeaderName())) {
                            (*itf)->filter(ith->content);
                            break;
                        }
                    }
                }
            }

            // Test for non-transparent redirection ($JUMP)
            if (!jumpToHost.empty()) {
                sendInBuf =
                    "HTTP/1.1 302 Found\r\n"
                    "Location: " + jumpToHost + "\r\n"
                    "\r\n";
                inStep = STEP_FINISH;
                CLog::ref().logHttpEvent(pmEVT_HTTP_TYPE_SENDIN, addr,
                                         reqNumber, sendInBuf);
                // We have to disconnect from website, to avoid downloading
                // the genuine document
                jumpToHost.clear();
                website->Close();
                continue;
            }

            // Test for "local.ptron" host
            if (CUtil::noCaseBeginsWith("http://local.ptron", rdirToHost)) {
                rdirToHost = "http://file//./html" + CUrl(rdirToHost).getPath();
            }

            // redirection to file: we fake the whole response then exit
            if (CUtil::noCaseBeginsWith("http://file//", rdirToHost)) {
                string filename = rdirToHost.substr(13);
                if (wxFile::Exists(filename.c_str())) {
                    fakeResponse("200 OK", filename);
                } else {
                    fakeResponse("404 Not Found", "./html/error.html", true,
                             wxGetCwd().c_str(),
                             CSettings::ref().getMessage("404_NOT_FOUND"),
                             filename);
                }
                rdirToHost.clear();
                website->Close();
                continue;
            }

            // Test for transparent redirection ($RDIR)
            // Note: will only work for GET requests. New URL will not
            // go through URL* OUT filters
            if (!rdirToHost.empty() &&
                        CUtil::noCaseBeginsWith("GET ", requestLine)) {
                // Disconnect (to avoid downloading genuine document)
                website->Close();
                receiveIn();
                recvInBuf.clear();
                // Reset incoming variables
                inStep = STEP_START;
                bypassBody = bypassBodyForced = false;
                // Redefine URL (for filters)
                url.parseUrl(rdirToHost);
                fileType = evaluateType(url);
                // Define host to contact (possibly through proxy)
                useSettingsProxy = CSettings::ref().useNextProxy;
                contactHost = url.getHostPort();
                // Fake request
                sendOutBuf =
                    "GET " + rdirToHost + " HTTP/1.1\r\n"
                    "Host: " + url.getHost() + "\r\n"
                    "\r\n";
                rdirToHost.clear();
                CLog::ref().logHttpEvent(pmEVT_HTTP_TYPE_SENDOUT, addr,
                                         reqNumber, sendOutBuf);
                // Reconnect
                connectWebsite();
                // Send request
                sendOut();
                continue;
            }

            if (inChunked || inSize) {
                // Our output will always be chunked: filtering can
                // change body size. So let's force this header.
                for (vector<SHeader>::iterator ith  = inHeadersFiltered.begin();
                                               ith != inHeadersFiltered.end();
                                               ith++) {
                    if (CUtil::noCaseEqual(ith->name, "Transfer-Encoding")) {
                        inHeadersFiltered.erase(ith);
                        break;
                    }
                }
                SHeader header = { string("Transfer-Encoding"), string("chunked") };
                inHeadersFiltered.push_back(header);
            }

            // Now we can put everything in the filtered buffer
            sendInBuf = responseLine;
            for (vector<SHeader>::iterator it = inHeadersFiltered.begin();
                    it != inHeadersFiltered.end(); it++) {
                if (!it->content.empty()) {
                    sendInBuf += it->name + ": " + it->content + "\r\n";

                    if (CUtil::noCaseEqual(it->name, "Connection")) {
                        if (CUtil::noCaseContains("close", it->content))
                            sendConnectionClose = true;
                    } else if (CUtil::noCaseEqual(it->name, "Content-Encoding")) {
                        if (CUtil::noCaseContains("gzip", it->content)) {
                            sendContentCoding = 1;
                            compressor.reset(true, true);
                        } else if (CUtil::noCaseContains("deflate", it->content)) {
                            sendContentCoding = 2;
                            compressor.reset(true, false);
                        }
                    }
                }
            }
            CLog::ref().logHttpEvent(pmEVT_HTTP_TYPE_SENDIN, addr,
                                     reqNumber, sendInBuf);

            // Tell text filters to see whether they should work on it
            useChain = (!bypassBody && CSettings::ref().filterText);
            if (useChain) chain->dataReset();
            if (useGifFilter) GIFfilter->dataReset();

            // File type will be reevaluated using first block of data
            fileType.clear();

            if (inChunked) {
                // Start reading chunks
                inStep = STEP_CHUNK;
            } else if (inSize) {
                // Add the blank line and read raw body
                recvInBuf.erase(0, 2);
                sendInBuf += "\r\n";
                inStep = STEP_RAW;
            } else {
                // No body to read
                recvInBuf.erase(0, 2);
                sendInBuf += "\r\n";
                inStep = STEP_FINISH;
            }
        }

        // Read \r\n HexRawLength * \r\n before raw data
        // or \r\n zero * \r\n\r\n
        else if (inStep == STEP_CHUNK) {

            if (recvInBuf.substr(0, 2) != "\r\n") return;
            unsigned int crlf = recvInBuf.find("\r\n", 2);
            if (crlf == string::npos) return;
            inSize = CUtil::readHex(recvInBuf.substr(2,crlf-2));

            if (inSize > 0) {
                recvInBuf.erase(0, crlf+2);
                inStep = STEP_RAW;
            } else {
                crlf = recvInBuf.find("\r\n\r\n", 2);
                if (crlf == string::npos) return;
                recvInBuf.erase(0, crlf+4);

                endFeeding();
                inStep = STEP_FINISH;
            }
        }

        // The next inSize bytes are left untouched
        else if (inStep == STEP_RAW) {

            int copySize = recvInBuf.size();
            if (copySize > inSize) copySize = inSize;
            if (!copySize) return;

            string data = recvInBuf.substr(0, copySize);
            recvInBuf.erase(0, copySize);
            inSize -= copySize;

            // We must decompress compressed data,
            // unless bypassed body with same coding
            if (recvContentCoding && (useChain ||
                        recvContentCoding != sendContentCoding)) {
                decompressor.feed(data);
                decompressor.read(data);
            }

            if (useChain) {
                // For the first block of data to filter, we'll evaluate
                // the content type
                if (fileType.empty()) fileType = evaluateType(data);
                // provide filter chain with raw unfiltered data
                chain->dataFeed(data);
            } else if (useGifFilter) {
                // Freeze GIF
                GIFfilter->dataFeed(data);
            } else {
                // In case we bypass the body, we directly send data to
                // the end of chain (the request manager itself)
                dataFeed(data);
            }

            // If we finished reading as much raw data as was expected,
            // continue to next step (finish or next chunk)
            if (!inSize) {
                if (inChunked) {
                    inStep = STEP_CHUNK;
                } else {
                    endFeeding();
                    inStep = STEP_FINISH;
                }
            }
        }

        // A few things have to be done before we go back to start state...
        else if (inStep == STEP_FINISH) {

            if (outStep != STEP_FINISH && outStep != STEP_START) return;
            outStep = STEP_START;
            inStep = STEP_START;

            // Update active request stat
            --CLog::ref().numActiveRequests;
            CLog::ref().logProxyEvent(pmEVT_PROXY_TYPE_ENDREQ, addr);

            // Disconnect browser if we sent it "Connection:close"
            // If manager has been set invalid, close connection too
            if (sendConnectionClose || !valid) {
                sendIn();
                browser->Close();
            }

            // Disconnect website if it sent us "Connection:close"
            if (recvConnectionClose || browser->IsDisconnected())
                website->Close();
        }
    }
}


/* Connect to the website or to the proxy.
 * If connection fails, a custom HTTP response is sent to the browser
 * and incoming processing continues to STEP_FINISH, as if the website
 * terminated responding.
 * This functions waits for the connection to succeed or fail.
 */
void CRequestManager::connectWebsite() {

    // Test for "local.ptron" host
    if (CUtil::noCaseEqual("local.ptron", url.getHost())) {
        rdirToHost = "http://file//./html" + url.getPath();
    } else if (CUtil::noCaseBeginsWith("http://local.ptron", rdirToHost)) {
        rdirToHost = "http://file//./html" + CUrl(rdirToHost).getPath();
    }

    // Test for non-transparent redirection ($JUMP)
    if (!jumpToHost.empty()) {
        // We'll keep browser's socket, for persistent connections, and
        // continue processing outgoing data (which will not be moved to
        // send buffer).
        sendOutBuf.clear();
        inStep = STEP_FINISH;
        sendInBuf =
            "HTTP/1.1 302 Found\r\n"
            "Location: " + jumpToHost + "\r\n"
            "\r\n";
        CLog::ref().logHttpEvent(pmEVT_HTTP_TYPE_SENDIN, addr,
                                 reqNumber, sendInBuf);
        jumpToHost.clear();
        return;
    }

    // Test for transparent redirection __to file__ ($RDIR)
    if (CUtil::noCaseBeginsWith("http://file//", rdirToHost)) {
        string filename = rdirToHost.substr(13);
        if (wxFile::Exists(filename.c_str())) {
            fakeResponse("200 OK", filename);
        } else {
            fakeResponse("404 Not Found", "./html/error.html", true,
                     wxGetCwd().c_str(),
                     CSettings::ref().getMessage("404_NOT_FOUND"),
                     filename);
        }
        rdirToHost.clear();
        return;
    }

    // If we must contact the host via the settings' proxy,
    // we now override the contactHost
    if (useSettingsProxy && !CSettings::ref().nextProxy.empty())
        contactHost = CSettings::ref().nextProxy;

    // Unless we are already connected to this host, we try and connect now
    if (previousHost != contactHost || website->IsDisconnected()) {

        // Disconnect from previous host
        website->Close();
        previousHost = contactHost;

        // The host string is composed of host and port
        unsigned int colon = contactHost.find(':');
        if (colon == string::npos) {    // (this should never happen)
            colon = contactHost.size();
            contactHost += ":80";
        }

        // Check the host (Hostname() asks the DNS)
        wxIPV4address host;
        host.Service(contactHost.substr(colon+1).c_str());
        if (!host.Hostname(contactHost.substr(0, colon).c_str())) {
            // The host address is invalid (or unknown by DNS)
            // so we won't try a connection.
            fakeResponse("404 Not Found", "./html/error.html", true,
                         wxGetCwd().c_str(),
                         CSettings::ref().getMessage("404_NOT_FOUND"),
                         contactHost);
            return;
        }

        // Connect
        website->Connect(host, FALSE);
        while (!website->WaitOnConnect(1,0) && browser->IsConnected());
        if (website->IsDisconnected()) {
            // Connection failed, warn the browser
            fakeResponse("503 Service Unavailable", "./html/error.html", true,
                         wxGetCwd().c_str(),
                         CSettings::ref().getMessage("503_UNAVAILABLE"),
                         contactHost);
            return;
        }
    }

    // At this point only, we can log what is meant to be sent to website
    CLog::ref().logHttpEvent(pmEVT_HTTP_TYPE_SENDOUT, addr,
                             reqNumber, sendOutBuf);
}


/* Generate an fake response (headers + body). The body comes from a
 * file in the local directory system.
 * The function accepts replacement strings for the content of the
 * file, where tokens are %%1%%, %%2%% and %%3%%.
 */
void CRequestManager::fakeResponse(string code, string filename, bool replace,
                                   string str1, string str2, string str3) {
    string content = CUtil::getFile(filename);
    if (replace) {
        content = CUtil::replaceAll(content, "%%1%%", str1);
        content = CUtil::replaceAll(content, "%%2%%", str2);
        content = CUtil::replaceAll(content, "%%3%%", str3);
    }
    stringstream ss;
    ss << content.length();
    sendOutBuf.clear();
    inStep = STEP_FINISH;
    sendInBuf =
        "HTTP/1.1 " + code + "\r\n"
        "Content-Type: " + CUtil::getMimeType(filename) + "\r\n"
        "Content-Length: " + ss.str() + "\r\n"
        "\r\n" + content;
    string log = sendInBuf.substr(0, sendInBuf.find("\r\n\r\n")+2);
    CLog::ref().logHttpEvent(pmEVT_HTTP_TYPE_SENDIN, addr,
                             reqNumber, log);
}


/* Send the last deflated bytes to the filter chain then dump the chain
 * and append last empty chunk
 */
void CRequestManager::endFeeding() {

    if (recvContentCoding && (useChain ||
                recvContentCoding != sendContentCoding)) {
        string data;
        decompressor.dump();
        decompressor.read(data);
        if (useChain) {
            chain->dataFeed(data);
        } else if (useGifFilter) {
            GIFfilter->dataFeed(data);
        } else {
            dataFeed(data);
        }
    }
    if (useChain) {
        chain->dataDump();
    } else if (useGifFilter) {
        GIFfilter->dataDump();
    } else {
        dataDump();
    }
    // Write last chunk (size 0)
    sendInBuf += "\r\n0\r\n\r\n";
}

