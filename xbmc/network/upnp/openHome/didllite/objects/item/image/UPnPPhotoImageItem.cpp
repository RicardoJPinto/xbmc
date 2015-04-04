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

#include "UPnPPhotoImageItem.h"
#include "FileItem.h"
#include "network/upnp/openHome/didllite/DidlLiteUtils.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"
#include "pictures/PictureInfoTag.h"

CUPnPPhotoImageItem::CUPnPPhotoImageItem()
  : CUPnPPhotoImageItem("object.item.imageItem.photo")
{ }

CUPnPPhotoImageItem::CUPnPPhotoImageItem(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPImageItem(classType, className)
{
  initializeProperties();
}

CUPnPPhotoImageItem::CUPnPPhotoImageItem(const CUPnPPhotoImageItem& photoImageItem)
  : CUPnPImageItem(photoImageItem),
    m_album(photoImageItem.m_album)
{
  initializeProperties();
  copyPropertyValidity(&photoImageItem);
}

CUPnPPhotoImageItem::~CUPnPPhotoImageItem()
{ }

bool CUPnPPhotoImageItem::ToFileItem(CFileItem& item) const
{
  if (!CUPnPImageItem::ToFileItem(item))
    return false;

  CPictureInfoTag& pictureInfo = *item.GetPictureInfoTag();
  // TODO: m_album

  return true;
}

bool CUPnPPhotoImageItem::FromFileItem(const CFileItem& item)
{
  if (!CUPnPImageItem::FromFileItem(item))
    return false;

  // nothing to do if the item doesn't have video info
  if (!item.HasVideoInfoTag())
    return true;

  const CPictureInfoTag& pictureInfo = *item.GetPictureInfoTag();
  // TODO: m_album

  return true;
}

void CUPnPPhotoImageItem::initializeProperties()
{
  // define all attributes
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "album", &m_album).SetOptional().SupportMultipleValues();
}
