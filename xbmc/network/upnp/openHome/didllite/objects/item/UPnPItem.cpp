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

#include "UPnPItem.h"
#include "FileItem.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"

CUPnPItem::CUPnPItem()
  : CUPnPItem("object.item")
{ }

CUPnPItem::CUPnPItem(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPObject("item", classType, className),
    m_objectUpdateID(0)
{
  initializeProperties();
}

CUPnPItem::CUPnPItem(const CUPnPItem& item)
  : CUPnPObject(item),
    m_objectUpdateID(item.m_objectUpdateID),
    m_refID(item.m_refID),
    m_bookmarkIDs(item.m_bookmarkIDs)
{
  initializeProperties();
  copyPropertyValidity(&item);
}

CUPnPItem::~CUPnPItem()
{ }

bool CUPnPItem::ToFileItem(CFileItem& item) const
{
  if (!CUPnPObject::ToFileItem(item))
    return false;

  if (!m_refID.empty())
    item.SetPath(m_refID);

  return true;
}

void CUPnPItem::initializeProperties()
{
  // define all attributes
  addUnsignedIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "objectUpdateID", &m_objectUpdateID).SetOptional();
  addStringProperty("@refID", &m_refID).AsAttribute().SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "bookmarkID", &m_bookmarkIDs).SetOptional().SupportMultipleValues();
}
