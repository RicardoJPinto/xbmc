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

#include "UPnPAlbumContainer.h"
#include "FileItem.h"
#include "network/upnp/openHome/didllite/DidlLiteUtils.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"

CUPnPAlbumContainer::CUPnPAlbumContainer()
  : CUPnPAlbumContainer("object.container.album")
{ }

CUPnPAlbumContainer::CUPnPAlbumContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPContainer(classType, className)
{
  initializeProperties();
}

CUPnPAlbumContainer::CUPnPAlbumContainer(const CUPnPAlbumContainer& albumContainer)
  : CUPnPContainer(albumContainer),
    m_storageMedium(albumContainer.m_storageMedium),
    m_longDescription(albumContainer.m_longDescription),
    m_description(albumContainer.m_description),
    m_publisher(albumContainer.m_publisher),
    m_contributor(albumContainer.m_contributor),
    m_date(albumContainer.m_date),
    m_relation(albumContainer.m_relation),
    m_rights(albumContainer.m_rights)
{
  initializeProperties();
  copyPropertyValidity(&albumContainer);
}

CUPnPAlbumContainer::~CUPnPAlbumContainer()
{ }

bool CUPnPAlbumContainer::ToFileItem(CFileItem& item) const
{
  if (!CUPnPContainer::ToFileItem(item))
    return false;

  item.m_dateTime = DidlLiteUtils::GetDateTime(m_date);

  return true;
}

bool CUPnPAlbumContainer::FromFileItem(const CFileItem& item)
{
  if (!CUPnPContainer::FromFileItem(item))
    return false;

  // TODO

  return true;
}

void CUPnPAlbumContainer::initializeProperties()
{
  // define all attributes
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageMedium", &m_storageMedium).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "longDescription", &m_longDescription).SetOptional();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "description", &m_description).SetOptional();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "publisher", &m_publisher).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "contributor", &m_contributor).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "date", &m_date).SetOptional();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "relation", &m_relation).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "rights", &m_rights).SetOptional().SupportMultipleValues();
}
