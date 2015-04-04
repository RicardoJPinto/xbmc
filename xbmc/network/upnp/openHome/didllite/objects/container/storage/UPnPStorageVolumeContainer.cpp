/*
 *      Copyright (C) 2015 Team XBMC
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

#include "UPnPStorageVolumeContainer.h"
#include "FileItem.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"

CUPnPStorageVolumeContainer::CUPnPStorageVolumeContainer()
  : CUPnPStorageVolumeContainer("object.container.storageVolume")
{ }

CUPnPStorageVolumeContainer::CUPnPStorageVolumeContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPContainer(classType, className),
    m_storageTotal(0),
    m_storageUsed(0),
    m_storageFree(0)
{
  initializeProperties();
}

CUPnPStorageVolumeContainer::CUPnPStorageVolumeContainer(const CUPnPStorageVolumeContainer& storageVolumeContainer)
  : CUPnPContainer(storageVolumeContainer),
    m_storageTotal(storageVolumeContainer.m_storageTotal),
    m_storageUsed(storageVolumeContainer.m_storageUsed),
    m_storageFree(storageVolumeContainer.m_storageFree),
    m_storageMedium(storageVolumeContainer.m_storageMedium)
{
  initializeProperties();
  copyPropertyValidity(&storageVolumeContainer);
}

CUPnPStorageVolumeContainer::~CUPnPStorageVolumeContainer()
{ }

bool CUPnPStorageVolumeContainer::ToFileItem(CFileItem& item) const
{
  if (!CUPnPContainer::ToFileItem(item))
    return false;

  // TODO

  return true;
}

bool CUPnPStorageVolumeContainer::FromFileItem(const CFileItem& item)
{
  if (!CUPnPContainer::FromFileItem(item))
    return false;

  // TODO

  return true;
}

void CUPnPStorageVolumeContainer::initializeProperties()
{
  // define all attributes
  addIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageTotal", &m_storageTotal).SetOptional();
  addIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageUsed", &m_storageUsed).SetOptional();
  addIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageFree", &m_storageFree).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageMedium", &m_storageMedium).SetOptional();
}
