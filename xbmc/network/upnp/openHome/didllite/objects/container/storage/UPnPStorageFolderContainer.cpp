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

#include "UPnPStorageFolderContainer.h"
#include "FileItem.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"

CUPnPStorageFolderContainer::CUPnPStorageFolderContainer()
  : CUPnPStorageFolderContainer("object.container.storageFolder")
{ }

CUPnPStorageFolderContainer::CUPnPStorageFolderContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPContainer(classType, className),
    m_storageUsed(0)
{
  initializeProperties();
}

CUPnPStorageFolderContainer::CUPnPStorageFolderContainer(const CUPnPStorageFolderContainer& storageFolderContainer)
  : CUPnPContainer(storageFolderContainer),
    m_storageUsed(storageFolderContainer.m_storageUsed)
{
  initializeProperties();
  copyPropertyValidity(&storageFolderContainer);
}

CUPnPStorageFolderContainer::~CUPnPStorageFolderContainer()
{ }

bool CUPnPStorageFolderContainer::ToFileItem(CFileItem& item) const
{
  if (!CUPnPContainer::ToFileItem(item))
    return false;

  // TODO

  return true;
}

bool CUPnPStorageFolderContainer::FromFileItem(const CFileItem& item)
{
  if (!CUPnPContainer::FromFileItem(item))
    return false;

  // TODO

  return true;
}

void CUPnPStorageFolderContainer::initializeProperties()
{
  // define all attributes
  addIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageUsed", &m_storageUsed).SetOptional();
}
