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

#include "UPnPClass.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"
#include "utils/XBMCTinyXml.h"

CUPnPClass::CUPnPClass(const std::string& classType /* = "" */, const std::string& className /* = "" */)
  : IDidlLiteElement(UPNP_DIDL_UPNP_NAMESPACE, "class"),
    m_classType(classType),
    m_className(className)
{
  initializeProperties();
}

CUPnPClass::CUPnPClass(const CUPnPClass& upnpClass)
  : IDidlLiteElement(upnpClass),
    m_classType(upnpClass.m_classType),
    m_className(upnpClass.m_className)
{
  initializeProperties();
  copyPropertyValidity(&upnpClass);
}

CUPnPClass::~CUPnPClass()
{ }

bool CUPnPClass::serialize(TiXmlElement* element) const
{
  TiXmlText type(m_classType);
  element->InsertEndChild(type);

  return true;
}

bool CUPnPClass::deserialize(const TiXmlElement* element)
{
  if (element->FirstChild() == nullptr)
    return false;

  std::string type = element->FirstChild()->ValueStr();
  if (type.empty())
    return false;

  if (!m_classType.empty() && m_classType.compare(type) != 0)
    return false;

  m_classType = type;
  return true;
}

void CUPnPClass::initializeProperties()
{
  addStringProperty("@name", &m_className).AsAttribute().SetOptional();
  if (!m_className.empty())
    setPropertyValid("@name");
}
