#pragma once
/*
 *      Copyright (C) 2014 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <vector>

#include <OpenHome/OsTypes.h>

namespace OpenHome
{
  class Environment;
}

class COhUPnPContentDirectoryControlPoint;

class COhUPnP
{
public:
  virtual ~COhUPnP();

  static COhUPnP& Get();

  bool Initialize();
  void Uninitialize();

  bool StartServer();
  void StopServer();
  bool IsServerRunning() const;

  bool StartContentDirectoryClient();
  void StopContentDirectoryClient();
  bool IsContentDirectoryClientRunning() const;
  const COhUPnPContentDirectoryControlPoint& GetContentDirectoryClient() const { return *m_contentDirectoryClient; }

  bool StartRenderer();
  void StopRenderer();
  bool IsRendererRunning() const;

  // TODO
private:
  COhUPnP();
  COhUPnP(const COhUPnP&);
  COhUPnP const& operator=(COhUPnP const&);

  void ohNetLogOutput(const char* msg);
  void ohNetFatalErrorHandler(const char* msg);

  bool m_initialised;
  OpenHome::Environment *m_ohEnvironment;
  TIpAddress m_subnet;

  COhUPnPContentDirectoryControlPoint *m_contentDirectoryClient;
};
