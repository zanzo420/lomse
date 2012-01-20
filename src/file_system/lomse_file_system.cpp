//---------------------------------------------------------------------------------------
//  This file is part of the Lomse library.
//  Copyright (c) 2010-2011 Lomse project
//
//  Lomse is free software; you can redistribute it and/or modify it under the
//  terms of the GNU General Public License as published by the Free Software Foundation,
//  either version 3 of the License, or (at your option) any later version.
//
//  Lomse is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
//  PARTICULAR PURPOSE.  See the GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License along
//  with Lomse; if not, see <http://www.gnu.org/licenses/>.
//
//  For any comment, suggestion or feature request, please contact the manager of
//  the project at cecilios@users.sourceforge.net
//
//---------------------------------------------------------------------------------------

#include "lomse_file_system.h"

#include "lomse_zip_stream.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace lomse
{

//=======================================================================================
// DocLocator implementation
//=======================================================================================
DocLocator::DocLocator(const string& locator)
    : m_fullLocator(locator)
    , m_protocol(k_unknown)
    , m_innerProtocol(k_none)
    , m_path("")
    , m_innerPath("")
    , m_innerFile("")
    , m_fValid(false)
{
    split_locator(locator);
    extract_file();
}

//---------------------------------------------------------------------------------------
void DocLocator::split_locator(const string& locator)
{
    //protocol
    m_protocol = k_file;
    int pathStart = 0;
    int colon = int( locator.find(':') );
    if (colon >= 0)
    {
        if (locator.substr(0, colon) == "file")
            pathStart = colon+1;
        else if (locator.substr(0, colon) == "string")
        {
            m_protocol = k_string;
            m_fValid = true;
            return;
        }
    }

    //inner protocol & path
    int sharp = int( locator.find('#') );
    if (sharp >= 0)
    {
        m_path = locator.substr(pathStart, sharp - pathStart);
        int iMax = int( locator.length() );
        int i = sharp+1;
        for (; i < iMax && locator[i] != ':'; ++i);
        if (i == iMax)
        {
            m_fValid = false;
            return;
        }
        string proto =  locator.substr(sharp+1, i - sharp - 1);
        m_innerProtocol = (proto == "zip" ? k_zip : k_unknown);
        if (i < iMax - 2)
            m_innerPath = locator.substr(i+1);
    }
    else
    {
        //path
        if (pathStart > 0)
            m_path = locator.substr(pathStart);
        else
            m_path = locator;
    }

    m_fValid = true;
}

//---------------------------------------------------------------------------------------
void DocLocator::extract_file()
{
    if (m_innerPath.empty())
        return;

    int iMax = int( m_innerPath.length() );
    int i = iMax-1;
    for (; i >=0 && m_innerPath[i] != '/'; --i);
    if (i >= 0)
        m_innerFile = m_innerPath.substr(i+1);
}

//---------------------------------------------------------------------------------------
string DocLocator::get_protocol_string()
{
    switch (m_protocol)
    {
        case k_file:        return "";
        case k_string:      return "string:";
        default:
            return "";
    }
}

//---------------------------------------------------------------------------------------
string DocLocator::get_locator_string()
{
    string loc = get_protocol_string() + get_path();
    if (m_innerProtocol == k_zip)
        return loc + "#zip:" + get_inner_path();
    else
        return loc;
}



//=======================================================================================
// LmbDocLocator implementation
//=======================================================================================
string LmbDocLocator::get_locator_for_image(const string& imagename)
{
    if (m_innerProtocol != k_zip)
    {
        //support for tests (win & Linux): images in the same folder than lms file

        //remove lms file from path
        int iMax = int( m_path.length() );
        int i = iMax-1;
        for (; i >=0 && !(m_path[i] == '/' || m_path[i] == '\\'); --i);
        if (i >= 0)
            return m_path.substr(0, i+1) + imagename;

    }
    //normal behaviour: zip assumed
    return get_protocol_string() + get_path() + "#zip:" + imagename;
}


//=======================================================================================
// FileSystem implementation
//=======================================================================================
InputStream* FileSystem::open_input_stream(const string& filelocator)
{
    //factory method to create InputStream objects

    DocLocator loc(filelocator);
    switch( loc.get_protocol() )
    {
        case DocLocator::k_file:
        {
            switch( loc.get_inner_protocol() )
            {
                case DocLocator::k_none:
                    return LOMSE_NEW LocalInputStream(filelocator);
                case DocLocator::k_zip:
                    return LOMSE_NEW ZipInputStream(filelocator);
                default:
                    throw("Invalid file locator protocol");
            }
        }

        default:
            throw("Invalid file locator protocol");
    }
    return NULL;    //compiler happy
}


//=======================================================================================
// LocalInputStream implementation
//=======================================================================================
LocalInputStream::LocalInputStream(const std::string& filelocator)
    : InputStream()
    , m_file(filelocator.c_str(), ios::in | ios::binary)
{
    if(!m_file.is_open())
    {
        stringstream s;
        s << "File not found: \"" << filelocator << "\"";
        throw std::invalid_argument(s.str());
    }
}

//---------------------------------------------------------------------------------------
char LocalInputStream::get_char()
{
    return m_file.get();
}

//---------------------------------------------------------------------------------------
void LocalInputStream::unget()
{
    m_file.unget();
}

//---------------------------------------------------------------------------------------
bool LocalInputStream::is_open()
{
    return m_file.is_open();
}

//---------------------------------------------------------------------------------------
bool LocalInputStream::eof()
{
    return m_file.eof();
}

//---------------------------------------------------------------------------------------
int LocalInputStream::read (unsigned char* pDestBuffer, int nBytesToRead)
{
    //reads the specified number of bytes from the stream into the dest. buffer.
    //Invoker should allocate a buffer large enough, as this method does not
    //do any checks
    //Returns the actual number of bytes that were read. It might be lower than the
    //requested number of bites if the end of stream is reached.

    m_file.read((char*)pDestBuffer, nBytesToRead);
    return m_file.gcount();
}


}  //namespace lomse
