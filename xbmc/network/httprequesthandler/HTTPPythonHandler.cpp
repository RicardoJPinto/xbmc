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

#include <vector>

#include "HTTPPythonHandler.h"
#include "network/WebServer.h"
#include "HTTPWebinterfaceHandler.h"
#include "python/HTTPPythonInterpreter.h"
#include "addons/IAddon.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "utils/StdString.h"

#define MAX_STRING_POST_SIZE 20000

using namespace std;
using namespace ADDON;

bool CHTTPPythonHandler::CheckHTTPRequest(const HTTPRequest &request)
{
  return URIUtils::GetExtension(request.url).compare(".py") == 0 && (request.method == GET || request.method == POST);
}

int CHTTPPythonHandler::HandleHTTPRequest(const HTTPRequest &request)
{
  // Get the real path of the script and check if it actually exists
  string path;
  AddonPtr addon;
  m_responseCode = CHTTPWebinterfaceHandler::ResolveUrl(request.url, path, addon);
  if (m_responseCode != MHD_HTTP_OK)
  {
    m_responseType = HTTPError;
    if (m_responseCode == MHD_HTTP_FOUND)
      m_responseCode = MHD_HTTP_INTERNAL_SERVER_ERROR;

    return MHD_YES;
  }

  vector<CStdString> args;
  args.push_back(path);

  try
  {
    CHTTPPythonInterpreter pythonInterpreter(request.connection, request.url, request.method, request.version, m_postFields, addon);
    if (!pythonInterpreter.ExecuteFile(path, args))
    {
      m_responseCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
      m_responseType = HTTPError;

      return MHD_YES;
    }

    HTTPPythonRequest* pythonRequest = pythonInterpreter.GetPythonRequest();
    if (pythonRequest == NULL)
    {
      m_responseType = HTTPError;
      m_responseCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
      return MHD_YES;
    }

    m_responseCode = pythonRequest->responseStatus;
    if (m_responseCode < MHD_HTTP_BAD_REQUEST)
    {
      m_responseType = HTTPMemoryDownloadNoFreeCopy;
      m_responseHeaderFields = pythonRequest->responseHeaders;
    }
    else
    {
      m_responseType = HTTPError;
      m_responseHeaderFields = pythonRequest->responseHeadersError;
    }
    m_response = pythonRequest->responseData;

    if (!pythonRequest->responseContentType.empty())
    {
      m_responseHeaderFields.erase("Content-Type");
      m_responseHeaderFields.insert(pair<string, string>(MHD_HTTP_HEADER_CONTENT_TYPE, pythonRequest->responseContentType));
    }
  }
  catch (...)
  {
    m_responseType = HTTPError;
    m_responseCode = MHD_HTTP_INTERNAL_SERVER_ERROR;
    return MHD_YES;
  }

  return MHD_YES;
}

#if (MHD_VERSION >= 0x00040001)
bool CHTTPPythonHandler::appendPostData(const char *data, size_t size)
#else
bool CHTTPPythonHandler::appendPostData(const char *data, unsigned int size)
#endif
{
  if (m_request.size() + size > MAX_STRING_POST_SIZE)
  {
    CLog::Log(LOGERROR, "WebServer: Stopped uploading post since it exceeded size limitations");
    return false;
  }

  m_request.append(data, size);

  return true;
}
