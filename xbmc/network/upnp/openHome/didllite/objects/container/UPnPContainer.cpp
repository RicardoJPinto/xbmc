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

#include "UPnPContainer.h"
#include "FileItem.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"

CUPnPContainer::CUPnPClassDefinition::CUPnPClassDefinition(const std::string& classType /* = "" */ , const std::string& className /* = "" */ , bool includeDerived /* = false */)
  : CUPnPClass(classType, className),
    m_includeDerived(includeDerived)
{
  initializeProperties();
}

CUPnPContainer::CUPnPClassDefinition::CUPnPClassDefinition(const CUPnPClassDefinition& classDefinition)
  : CUPnPClass(classDefinition),
    m_includeDerived(classDefinition.m_includeDerived)
{
  initializeProperties();
  copyPropertyValidity(&classDefinition);
}

CUPnPContainer::CUPnPClassDefinition::~CUPnPClassDefinition()
{ }

void CUPnPContainer::CUPnPClassDefinition::initializeProperties()
{
  addBooleanProperty("@includeDerived", &m_includeDerived).AsAttribute().SetOptional().SetValid();
}

CUPnPContainer::CUPnPContainer()
  : CUPnPContainer("object.container")
{ }

CUPnPContainer::CUPnPContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPObject("container", classType, className),
    m_objectUpdateID(0),
    m_containerUpdateID(0),
    m_totalDeletedChildCount(0),
    m_childCount(0),
    m_childContainerCount(0),
    m_searchable(false),
    m_neverPlayable(true)
{
  initializeProperties();
}

CUPnPContainer::CUPnPContainer(const CUPnPContainer& container)
  : CUPnPObject(container),
    m_objectUpdateID(container.m_objectUpdateID),
    m_containerUpdateID(container.m_containerUpdateID),
    m_totalDeletedChildCount(container.m_totalDeletedChildCount),
    m_childCount(container.m_childCount),
    m_childContainerCount(container.m_childContainerCount),
    m_searchable(container.m_searchable),
    m_neverPlayable(container.m_neverPlayable)
{
  initializeProperties();
  copyPropertyValidity(&container);
}

CUPnPContainer::~CUPnPContainer()
{ }

bool CUPnPContainer::ToFileItem(CFileItem& item) const
{
  if (!CUPnPObject::ToFileItem(item))
    return false;

  item.m_bIsFolder = true;

  return true;
}

bool CUPnPContainer::FromFileItem(const CFileItem& item)
{
  if (!CUPnPObject::FromFileItem(item))
    return false;

  // TODO

  return true;
}

void CUPnPContainer::initializeProperties()
{
  // define all attributes
  addUnsignedIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "objectUpdateID", &m_objectUpdateID).SetOptional();
  addUnsignedIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "containerUpdateID", &m_containerUpdateID).SetOptional();
  addUnsignedIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "totalDeletedChildCount", &m_totalDeletedChildCount).SetOptional();
  addUnsignedIntegerProperty("@childCount", &m_childCount).AsAttribute().SetOptional();
  addUnsignedIntegerProperty("@childContainerCount", &m_childContainerCount).AsAttribute().SetOptional();
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "createClass", &m_createClass).SetOptional().SupportMultipleValues();
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "searchClass", &m_createClass).SetOptional().SupportMultipleValues();
  addBooleanProperty("@searchable", &m_searchable).AsAttribute().SetOptional();
  addBooleanProperty("@neverPlayable", &m_neverPlayable).AsAttribute().SetOptional();
}
