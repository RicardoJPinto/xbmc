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

#include <map>
#include <string>

#include "interfaces/python/PythonInterpreter.h"
#include "network/httprequesthandler/IHTTPRequestHandler.h"

typedef struct HTTPPythonRequest
{
  struct MHD_Connection *connection;
  std::string url;
  std::string file;
  HTTPMethod method;
  std::string version;
  std::multimap<std::string, std::string> headerValues;
  std::map<std::string, std::string> getValues;
  std::map<std::string, std::string> postValues;

  int responseStatus;
  std::string responseContentType;
  std::string responseData;
  std::multimap<std::string, std::string> responseHeaders;
  std::multimap<std::string, std::string> responseHeadersError;
} HTTPPythonRequest;

class CHTTPPythonInterpreter : public CPythonInterpreter
{
public:
  CHTTPPythonInterpreter(struct MHD_Connection *connection, const std::string &url, HTTPMethod method,
                         const std::string version, const std::map<std::string, std::string> &postValues,
                         ADDON::AddonPtr addon = ADDON::AddonPtr());
  virtual ~CHTTPPythonInterpreter();

  HTTPPythonRequest* GetPythonRequest() { return m_pythonRequest; }

protected:
  virtual void onInitialization();
  virtual void onPythonModuleInitialization(void* moduleDict);
  virtual void onSuccess();
  virtual void onAbort();
  virtual void onError(const std::string &type, const std::string &value, const std::string &traceback);
  virtual void onDeinitialization();

private:
  HTTPPythonRequest *m_pythonRequest;
};
