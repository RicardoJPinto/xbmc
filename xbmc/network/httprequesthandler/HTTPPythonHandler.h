#pragma once
/*
 *      Copyright (C) 2011 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "IHTTPRequestHandler.h"

class CHTTPPythonHandler : public IHTTPRequestHandler
{
public:
  CHTTPPythonHandler() { };
  
  virtual IHTTPRequestHandler* GetInstance() { return new CHTTPPythonHandler(); }
  virtual bool CheckHTTPRequest(const HTTPRequest &request);
  virtual int HandleHTTPRequest(const HTTPRequest &request);

  virtual void* GetHTTPResponseData() const { return (void *)m_response.c_str(); };
  virtual size_t GetHTTPResonseDataLength() const { return m_response.size(); }

  virtual int GetPriority() const { return 2; }

protected:
#if (MHD_VERSION >= 0x00040001)
  virtual bool appendPostData(const char *data, size_t size);
#else
  virtual bool appendPostData(const char *data, unsigned int size);
#endif

private:
  std::string m_request;
  std::string m_response;
};
