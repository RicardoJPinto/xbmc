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

#include <Python.h>

#include "HTTPPythonInterpreter.h"
#include "interfaces/python/XBPython.h"
#include "interfaces/python/xbmcmodule/HttpRequest.h"
#include "network/WebServer.h"
#include "utils/log.h"
#include "utils/StdString.h"
#include "utils/URIUtils.h"

extern "C" {
  void InitHttpModule(void);
  void DeinitHttpModule(void);
}

using namespace std;

CHTTPPythonInterpreter::CHTTPPythonInterpreter(struct MHD_Connection *connection, const std::string &url, HTTPMethod method,
                                               const std::string version, const std::map<std::string, std::string> &postValues,
                                               ADDON::AddonPtr addon /* = ADDON::AddonPtr() */)
  : CPythonInterpreter(addon)
{
  if (connection == NULL)
    throw 1;

  multimap<string, string> headerValues;
  CWebServer::GetRequestHeaderValues(connection, MHD_HEADER_KIND, headerValues);
  map<string, string> getValues;
  CWebServer::GetRequestHeaderValues(connection, MHD_GET_ARGUMENT_KIND, getValues);

  m_pythonRequest = new HTTPPythonRequest();
  m_pythonRequest->connection = connection;
  m_pythonRequest->url = url;
  m_pythonRequest->file = URIUtils::GetFileName(url);
  m_pythonRequest->method = method;
  m_pythonRequest->version = version;
  m_pythonRequest->headerValues = headerValues;
  m_pythonRequest->getValues = getValues;
  m_pythonRequest->postValues = postValues;
  m_pythonRequest->responseStatus = 200;
}

CHTTPPythonInterpreter::~CHTTPPythonInterpreter()
{
	if (m_pythonRequest != NULL)
		delete m_pythonRequest;
}

#define RUNSCRIPT \
        "" \
        "import xbmc\n" \
        "import xbmchttp\n" \
        "" \
        "class httpout:\n" \
        "\tdef write(self, data):\n" \
        "\t\treq.write(data)\n" \
        "\tdef close(self):\n" \
        "\t\tpass\n" \
        "\tdef flush(self):\n" \
        "\t\tpass\n" \
        "" \
        "class xbmcout:\n" \
        "\tdef __init__(self, loglevel=xbmc.LOGNOTICE):\n" \
        "\t\tself.ll=loglevel\n" \
        "\tdef write(self, data):\n" \
        "\t\txbmc.log(data,self.ll)\n" \
        "\tdef close(self):\n" \
        "\t\txbmc.log('.')\n" \
        "\tdef flush(self):\n" \
        "\t\txbmc.log('.')\n" \
        "" \
        "import sys\n" \
        "sys.stdout = httpout()\n" \
        "sys.stderr = xbmcout(xbmc.LOGERROR)\n" \
        "" \
        "xbmc.log('-->HTTP Python Interpreter Initialized<--', xbmc.LOGNOTICE)\n" \
        ""

#define RUNSCRIPT_COMPLIANT \
  RUNSCRIPT_PRAMBLE RUNSCRIPT_POSTSCRIPT

void CHTTPPythonInterpreter::onInitialization()
{
  XBPython::InitializeModules();
  InitHttpModule();
  
  // redirecting default output to debug console
  if (PyRun_SimpleString(RUNSCRIPT) == -1)
    CLog::Log(LOGFATAL, "HTTPPythonInterpreter: initialization error");
}

void CHTTPPythonInterpreter::onPythonModuleInitialization(void* moduleDict)
{
  CPythonInterpreter::onPythonModuleInitialization(moduleDict);

  if (m_pythonRequest == NULL)
    return;

  PyObject *httpRequest = PYXBMC::HttpRequest_fromHTTPPythonRequest(m_pythonRequest);
  if (httpRequest == NULL)
    return;

  // set the global HttpRequest object for this request
  PyDict_SetItem((PyObject *)moduleDict, PyString_FromString("req"), httpRequest);
}

void CHTTPPythonInterpreter::onSuccess()
{
  // Nothing to do for now
}

void CHTTPPythonInterpreter::onAbort()
{
  m_pythonRequest->responseStatus = 500; // MHD_HTTP_INTERNAL_SERVER_ERROR
}

void CHTTPPythonInterpreter::onError(const std::string &type, const std::string &value, const std::string &traceback)
{
  m_pythonRequest->responseStatus = 200; // MHD_HTTP_OK

	CStdString output = type + ": " + value + "\n" + traceback;
	output.Replace("<", "&lt;");
	output.Replace(">", "&gt;");
  output.Replace(" ", "&nbsp;");
  output.Replace("\n", "\n<br />");
  output = "<b>" + output;
  output.insert(output.find('\n'), "</b>");
  
  m_pythonRequest->responseData = "<html><head><title>XBMC: python error</title></head><body>" + output + "</body></html>";
}

void CHTTPPythonInterpreter::onDeinitialization()
{
  XBPython::DeinitializeModules();
  DeinitHttpModule();
}
