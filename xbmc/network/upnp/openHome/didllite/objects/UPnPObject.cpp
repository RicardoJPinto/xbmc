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

#include "UPnPObject.h"
#include "FileItem.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"
#include "utils/StringUtils.h"

CUPnPObject::CUPnPObject(const std::string& elementName, const std::string& classType, const std::string& className /* = "" */)
  : IFileItemElement(elementName),
    m_class(classType, className),
    m_writeStatus("UNKNOWN")
{
  initializeProperties();
}

CUPnPObject::CUPnPObject(const CUPnPObject& obj)
  : IFileItemElement(obj),
    m_id(obj.m_id),
    m_parentID(obj.m_parentID),
    m_restricted(obj.m_restricted),
    m_class(obj.m_class),
    m_title(obj.m_title),
    m_creator(obj.m_creator),
    m_writeStatus(obj.m_writeStatus)

{
  initializeProperties();
}

CUPnPObject::~CUPnPObject()
{
  for (const auto& res : m_res)
    delete res;
  m_res.clear();
}

bool CUPnPObject::ToFileItem(CFileItem& item) const
{
  if (m_id.empty())
    return false;

  item.Reset();

  item.SetPath(m_id);
  // TODO: m_parentID
  item.SetLabel(m_title);
  item.SetLabelPreformated(true);
  item.m_strTitle = m_title;
  // TODO: m_res

  item.m_bIsFolder = StringUtils::StartsWith(m_class.GetType(), "object.container");

  return true;
}

bool CUPnPObject::FromFileItem(const CFileItem& item)
{
  if (item.GetPath().empty())
    return false;

  m_id = item.GetPath();
  // TODO: m_parentID
  m_title = item.GetLabel();
  // TODO: m_res

  return true;
}

std::vector<const CUPnPResource*> CUPnPObject::GetResources() const
{
  std::vector<const CUPnPResource*> resources;
  for (const auto& res : m_res)
    resources.push_back(res);

  return resources;
}

void CUPnPObject::initializeProperties()
{
  // define all attributes
  addStringProperty("@id", &m_id).AsAttribute().SetRequired();
  addStringProperty("@parentID", &m_parentID).AsAttribute().SetRequired();
  addBooleanProperty("@restricted", &m_restricted).AsAttribute().SetRequired();
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "class", &m_class).SetRequired();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "title", &m_title).SetRequired();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "creator", &m_creator).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "writeStatus", &m_writeStatus).SetOptional();
  addElementProperty("res", &m_res).SetOptional().SupportMultipleValues().SetGenerator(new CUPnPResource());

  // set the validity of certain attributes
  setPropertyValid(UPNP_DIDL_DC_NAMESPACE, "creator");
}