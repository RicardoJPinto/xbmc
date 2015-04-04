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

#include "UPnPImageItem.h"
#include "FileItem.h"
#include "network/upnp/openHome/didllite/DidlLiteUtils.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"
#include "pictures/PictureInfoTag.h"

CUPnPImageItem::CUPnPImageItem()
  : CUPnPImageItem("object.item.imageItem")
{ }

CUPnPImageItem::CUPnPImageItem(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPItem(classType, className),
    m_storageMedium("UNKNOWN")
{
  initializeProperties();
}

CUPnPImageItem::CUPnPImageItem(const CUPnPImageItem& imageItem)
  : CUPnPItem(imageItem),
    m_longDescription(imageItem.m_longDescription),
    m_storageMedium(imageItem.m_storageMedium),
    m_description(imageItem.m_description),
    m_publisher(imageItem.m_publisher),
    m_date(imageItem.m_date),
    m_rights(imageItem.m_rights)
{
  copyElementProperty(m_rating, imageItem.m_rating);

  initializeProperties();
  copyPropertyValidity(&imageItem);
}

CUPnPImageItem::~CUPnPImageItem()
{ }

bool CUPnPImageItem::ToFileItem(CFileItem& item) const
{
  if (!CUPnPItem::ToFileItem(item))
    return false;

  CPictureInfoTag& pictureInfo = *item.GetPictureInfoTag();
  // TODO: m_longDescription
  // TODO: m_storageMedium
  // TODO: m_rating
  // TODO: m_description
  // TODO: m_publisher
  // TODO: m_date
  // TODO: m_rights

  return true;
}

bool CUPnPImageItem::FromFileItem(const CFileItem& item)
{
  if (!CUPnPItem::FromFileItem(item))
    return false;

  // nothing to do if the item doesn't have video info
  if (!item.HasVideoInfoTag())
    return true;

  const CPictureInfoTag& pictureInfo = *item.GetPictureInfoTag();
  // TODO: m_longDescription
  // TODO: m_storageMedium
  // TODO: m_rating
  // TODO: m_description
  // TODO: m_publisher
  // TODO: m_date
  // TODO: m_rights

  return true;
}

void CUPnPImageItem::initializeProperties()
{
  // define all attributes
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "longDescription", &m_longDescription).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageMedium", &m_storageMedium).SetOptional();
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "rating", &m_rating).SetOptional().SupportMultipleValues().SetGenerator(new CUPnPRating());
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "description", &m_description).SetOptional();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "publisher", &m_publisher).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "date", &m_date).SetOptional();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "rights", &m_rights).SetOptional().SupportMultipleValues();
}
