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


#ifndef __giffilter__
#define __giffilter__

#include <string>
#include "receptor.h"

using namespace std;

/* class CGifFilter
 * This class parses the content of a GIF image and removes 2nd+ frames fron it.
 */
class CGifFilter : public CDataReceptor {

public:
    // Constructor
    CGifFilter(CDataReceptor* out);

    // Destructor
    ~CGifFilter() { }

    // Reset processing state
    void dataReset();

    // receive new data
    void dataFeed(const string& data);

    // process buffer to the end (end of data stream)
    void dataDump();

private:
    // receptor for data output
    CDataReceptor* outReceptor;

    // processing state
    bool doneFirstImage;
    string buffer;
};

#endif
