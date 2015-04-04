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
#include <map>
#include <string>
#include <stdint.h>

#include <OpenHome/Net/Core/OhNet.h>
#include <OpenHome/Net/Cpp/CpDeviceUpnp.h>

#include "network/upnp/openHome/ohNet/ohUPnPControlPointDevice.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"
#include "threads/CriticalSection.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

template<class ohUPnPControlPointService>
class IOhUPnPControlPoint
{
public:
  IOhUPnPControlPoint(const std::string &domain, const std::string &serviceName, uint8_t version)
    : m_serviceName(serviceName),
      m_serviceVersion(version),
      m_devices(nullptr)
  {
    if (serviceName.empty())
      return;

    if (version <= 0)
      version = 1;

    CSingleLock lock(m_devicesSection);
    m_devices = new OpenHome::Net::CpDeviceListCppUpnpServiceType(
      domain, serviceName, version,
      OpenHome::Net::MakeFunctorCpDeviceCpp(*this, &IOhUPnPControlPoint::added),
      OpenHome::Net::MakeFunctorCpDeviceCpp(*this, &IOhUPnPControlPoint::removed));
  }

  virtual ~IOhUPnPControlPoint()
  {
    {
      CSingleLock lock(m_servicesSection);
      for (auto& service : m_services)
        delete service.second.service;

      m_services.clear();
    }

    {
      CSingleLock lock(m_devicesSection);
      delete m_devices;
      m_devices = nullptr;
    }
  }

  typedef struct UPnPControlPointService
  {
    COhUPnPControlPointDevice device;
    ohUPnPControlPointService* service;
  } UPnPControlPointService;

  std::vector<UPnPControlPointService> GetControlPointServices() const
  {
    std::vector<COhUPnPControlPointDevice> controlPointServices;
    {
      CSingleLock lock(m_servicesSection);
      for (const auto& service : m_services)
        controlPointServices.push_back(service.second);
    }

    return controlPointServices;
  }

  std::vector<COhUPnPControlPointDevice> GetDevices() const
  {
    std::vector<COhUPnPControlPointDevice> devices;
    {
      CSingleLock lock(m_servicesSection);
      for (const auto& service : m_services)
        devices.push_back(service.second.device);
    }

    return devices;
  }

  bool GetDevice(const std::string &uuid, COhUPnPControlPointDevice &device) const
  {
    if (uuid.empty())
      return false;

    {
      CSingleLock lock(m_servicesSection);
      const auto& service = m_services.find(uuid);
      if (service == m_services.end())
        return false;

      device = service->second.device;
    }
    return true;
  }

  std::vector<ohUPnPControlPointService*> GetServices() const
  {
    std::vector<COhUPnPControlPointDevice> services;
    {
      CSingleLock lock(m_servicesSection);
      for (const auto& service : m_services)
        services.push_back(service.second.service);
    }

    return services;
  }

  bool GetService(const std::string &uuid, ohUPnPControlPointService* &controlPointService) const
  {
    if (uuid.empty())
      return nullptr;

    CSingleLock lock(m_servicesSection);
    const auto& service = m_services.find(uuid);
    if (service == m_services.end())
      return nullptr;

    controlPointService = service->second.service;
    return controlPointService != nullptr;
  }

  ohUPnPControlPointService* GetService(const std::string &uuid) const
  {
    ohUPnPControlPointService* service = nullptr;
    if (!GetService(uuid, service))
      return nullptr;

    return service;
  }

protected:
  /*!
   * \brief Callback for every service that has been discovered
   */
  virtual void onServiceAdded(const UPnPControlPointService &service) = 0;

  /*!
   * \brief Callback for every previously discovered service that disappeared
   */
  virtual void onServiceRemoved(const UPnPControlPointService &service) = 0;

  void refreshServiceList()
  {
    CSingleLock lock(m_devicesSection);
    m_devices->Refresh();
  }

  void added(OpenHome::Net::CpDeviceCpp& device)
  {
    std::string friendlyName;
    device.GetAttribute(UPNP_DEVICE_ATTRIBUTE_FRIENDLY_NAME, friendlyName);
    CLog::Log(LOGDEBUG, "UPnP: service \"%s:%hhu\" detected: %s (%s)",
      m_serviceName.c_str(), m_serviceVersion, friendlyName.c_str(), device.Udn().c_str());

    COhUPnPControlPointDevice upnpDevice(device);
    if (!upnpDevice.IsValid())
    {
      CLog::Log(LOGWARNING, "UPnP: service %s is invalid", friendlyName.c_str());
      return;
    }

    CSingleLock lock(m_servicesSection);
    UPnPControlPointService service = { upnpDevice, new ohUPnPControlPointService(device) };
    m_services.insert(std::make_pair(device.Udn(), service));

    onServiceAdded(service);
  }

  void removed(OpenHome::Net::CpDeviceCpp& device)
  {
    std::string friendlyName;
    device.GetAttribute(UPNP_DEVICE_ATTRIBUTE_FRIENDLY_NAME, friendlyName);
    CLog::Log(LOGDEBUG, "UPnP: service \"%s:%hhu\" removed: %s (%s)",
      m_serviceName.c_str(), m_serviceVersion, friendlyName.c_str(), device.Udn().c_str());

    CSingleLock lock(m_servicesSection);
    auto& service = m_services.find(device.Udn());
    if (service == m_services.end())
      return;

    onServiceRemoved(service->second);

    delete service->second.service;
    m_services.erase(service);
  }

  CCriticalSection m_servicesSection;
  std::map<std::string, UPnPControlPointService> m_services;

private:
  std::string m_serviceName;
  uint8_t m_serviceVersion;

  CCriticalSection m_devicesSection;
  OpenHome::Net::CpDeviceListCppUpnpServiceType *m_devices;
};
