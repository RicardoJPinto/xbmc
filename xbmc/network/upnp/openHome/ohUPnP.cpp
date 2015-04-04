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

#include <OpenHome/Net/Core/OhNet.h>
#include <OpenHome/Net/Cpp/CpDeviceUpnp.h>

#include "ohUPnP.h"
#include "network/upnp/openHome/controlpoints/ohUPnPContentDirectoryControlPoint.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"
#include "utils/log.h"

using namespace OpenHome;
using namespace OpenHome::Net;

COhUPnP::COhUPnP()
  : m_initialised(false),
    m_ohEnvironment(NULL),
    m_contentDirectoryClient(NULL)
{ }

COhUPnP::~COhUPnP()
{
  Uninitialize();
}

COhUPnP& COhUPnP::Get()
{
  static COhUPnP s_instance;
  return s_instance;
}

bool COhUPnP::Initialize()
{
  if (m_initialised)
    return true;

  // TODO: put together the configuration for the ohNet UpnpLibrary
  InitialisationParams* initParams = InitialisationParams::Create();
  initParams->SetLogOutput(MakeFunctorMsg(*this, &COhUPnP::ohNetLogOutput));
  initParams->SetFatalErrorHandler(MakeFunctorMsg(*this, &COhUPnP::ohNetFatalErrorHandler));

  // initialise the ohNet UpnpLibrary
  m_ohEnvironment = UpnpLibrary::Initialise(initParams);
  if (m_ohEnvironment == NULL)
  {
    CLog::Log(LOGERROR, "COhUPnP: initializing UPnP failed");
    return false;
  }

  // get a list of available subnets
  std::vector<OpenHome::NetworkAdapter*> *subnets = UpnpLibrary::CreateSubnetList();
  if (subnets->empty())
  {
    CLog::Log(LOGERROR, "COhUPnP: initializing UPnP failed: no subnets available");

    UpnpLibrary::Close();
    m_ohEnvironment = NULL;
    return false;
  }

  m_subnet = subnets->at(0)->Subnet();
  UpnpLibrary::DestroySubnetList(subnets);

  m_initialised = true;
  return true;
}

void COhUPnP::Uninitialize()
{
  if (!m_initialised)
    return;

  StopRenderer();
  StopContentDirectoryClient();
  StopServer();

  // TODO

  UpnpLibrary::Close();
  m_ohEnvironment = NULL;

  m_initialised = false;
}

bool COhUPnP::StartServer()
{
  // TODO

  return false;
}

void COhUPnP::StopServer()
{
  // TODO
}

bool COhUPnP::IsServerRunning() const
{
  // TODO

  return false;
}

bool COhUPnP::StartContentDirectoryClient()
{
  if (IsContentDirectoryClientRunning())
    return true;

  CLog::Log(LOGINFO, "COhUPnP: starting control point stack on subnet %d.%d.%d.%d...", (m_subnet & 0xFF), ((m_subnet >> 8) & 0xFF), ((m_subnet >> 16) & 0xFF), ((m_subnet >> 24) & 0xFF));
  UpnpLibrary::StartCp(m_subnet);

  m_contentDirectoryClient = new COhUPnPContentDirectoryControlPoint();

  return true;
}

void COhUPnP::StopContentDirectoryClient()
{
  if (!IsContentDirectoryClientRunning())
    return;

  // TODO

  CLog::Log(LOGINFO, "COhUPnP: stopping control point stack...", m_subnet);
  delete m_contentDirectoryClient;
  m_contentDirectoryClient = NULL;
}

bool COhUPnP::IsContentDirectoryClientRunning() const
{
  return m_contentDirectoryClient != NULL;
}

bool COhUPnP::StartRenderer()
{
  // TODO

  return false;
}

void COhUPnP::StopRenderer()
{
  // TODO
}

bool COhUPnP::IsRendererRunning() const
{
  // TODO

  return false;
}

void COhUPnP::ohNetLogOutput(const char* msg)
{
  CLog::Log(LOGDEBUG, "[ohNet] %s", msg);
}

void COhUPnP::ohNetFatalErrorHandler(const char* msg)
{
  CLog::Log(LOGFATAL, "[ohNet] %s", msg);
}
