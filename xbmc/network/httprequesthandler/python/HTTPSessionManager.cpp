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

#include <time.h>

#include "HTTPSessionManager.h"
#include "threads/SingleLock.h"
#include "utils/md5.h"
#include "utils/StringUtils.h"

#define COOKIE_NAME   "xbmcpysid"
#define COOKIE_LENGTH strlen(COOKIE_NAME)

#define SESSION_TIMEOUT 30 * 60  // in seconds

using namespace std;
using namespace PYXBMC;
using namespace XFILE;

CHTTPSessionManager::CHTTPSessionManager()
{ }

CHTTPSessionManager::~CHTTPSessionManager()
{
  CSingleLock lock(m_critSection);
  for (map<string, HTTPSession>::iterator it = m_sessions.begin(); it != m_sessions.end(); it++)
    Py_XDECREF(it->second.session);

  m_sessions.clear();
}

CHTTPSessionManager& CHTTPSessionManager::Get()
{
  static CHTTPSessionManager m_instance;

  return m_instance;
}

Session* CHTTPSessionManager::GetSession(HTTPPythonRequest *request)
{
  if (request == NULL)
    return NULL;

  CStdString sessionId;
  string cookieData;
  pair<multimap<string, string>::const_iterator, multimap<string, string>::const_iterator> range = request->headerValues.equal_range(MHD_HTTP_HEADER_COOKIE);
  for (multimap<string, string>::const_iterator it = range.first; it != range.second; it++)
  {
    cookieData.append(it->second);
    cookieData.push_back(';'); // make sure two different cookies are seperated
  }

  CStdStringArray parts;
  StringUtils::SplitString(cookieData, ";", parts);
  for (unsigned int index = 0; index < parts.size(); index++)
  {
    CStdString data = parts[index];
    data.Trim();
    if (data.compare(0, COOKIE_LENGTH, COOKIE_NAME) != 0)
      continue;

    sessionId = data.substr(COOKIE_LENGTH + 1);
    sessionId.Trim();

    if (sessionId.size() == 32 && sessionId.find_first_not_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") == string::npos)
      break;
  }

  CSingleLock lock(m_critSection);
  if (sessionId.size() == 32 && sessionId.find_first_not_of("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ") == string::npos)
  {
    map<string, HTTPSession>::const_iterator it = m_sessions.find(sessionId);
    // The session has already been loaded before so let's just return it
    if (it != m_sessions.end())
    {
      it->second.session->isNew = false;
      return it->second.session;
    }
  }

  // generate a new sessionid
  // Get the ip address of the requester
  const MHD_ConnectionInfo *ipInfo = MHD_get_connection_info(request->connection, MHD_CONNECTION_INFO_CLIENT_ADDRESS);

  int count = 0;
  do
  {
    CStdString temp;
    temp.Format("%d%d%d%s", time(NULL) * (10000 + count), rand(), rand(), inet_ntoa(ipInfo->client_addr->sin_addr));
    sessionId = XBMC::XBMC_MD5::GetMD5(temp);

    count += 1;
  }
  while (m_sessions.find(sessionId) != m_sessions.end());

  HTTPSession session;
  session.request = request;

  // Get the python session object
  session.session = Session_New(sessionId.c_str(), true);
  if (session.session == NULL)
  {
    PyErr_SetString(PyExc_Exception, "unable to create a new session");
    return NULL;
  }

  // Set the "Set-Cookie" header field
  request->responseHeaders.insert(pair<string, string>(MHD_HTTP_HEADER_SET_COOKIE, COOKIE_NAME "=" + sessionId));

  m_sessions[sessionId] = session;
  return session.session;
}

void CHTTPSessionManager::RemoveSession(const std::string &sessionId, bool invalidate /* = false */)
{
  if (sessionId.empty())
    return;

  CSingleLock lock(m_critSection);
  map<string, HTTPSession>::iterator it = m_sessions.find(sessionId);
  if (it == m_sessions.end())
    return;

  // make sure to delete the cookie
  if (invalidate)
    it->second.request->responseHeaders.insert(pair<string, string>(MHD_HTTP_HEADER_SET_COOKIE, COOKIE_NAME "=" + sessionId + ";Max-Age=0"));

  m_sessions.erase(sessionId);
}