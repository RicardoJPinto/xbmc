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

#include "IDidlLiteElement.h"
#include "network/upnp/openHome/didllite/DidlLiteUtils.h"
#include "utils/StringUtils.h"
#include "utils/XBMCTinyXml.h"
#include "utils/XMLUtils.h"

IDidlLiteElement::IDidlLiteElement()
  : IDidlLiteElement("")
{ }

IDidlLiteElement::IDidlLiteElement(const std::string& name)
  : IDidlLiteElement("", name)
{ }

IDidlLiteElement::IDidlLiteElement(const std::string& ns, const std::string& name)
  : m_namespace(ns),
    m_name(name)
{ }

IDidlLiteElement::IDidlLiteElement(const IDidlLiteElement& element)
  : m_namespace(element.m_namespace),
    m_name(element.m_name)
{ }

IDidlLiteElement::~IDidlLiteElement()
{ }

bool IDidlLiteElement::Serialize(TiXmlElement* element) const
{
  if (element == nullptr)
    return false;

  // create the element
  std::string elementName = DidlLiteUtils::GetElementName(m_namespace, m_name);
  TiXmlElement newElement(elementName);

  // add all specified attributes
  for (const auto& attr : m_properties)
  {
    if (!attr.second.Serialize(&newElement))
      return false;
  }

  // perform any custom serialization
  if (!serialize(&newElement))
    return false;

  // insert the element at the end
  return element->InsertEndChild(newElement) != nullptr;
}

bool IDidlLiteElement::Deserialize(const TiXmlElement* element)
{
  if (element == nullptr)
    return false;

  std::string elementName = element->ValueStr();
  std::vector<std::string> nameParts = StringUtils::Split(elementName, ':');
  if (nameParts.size() > 2)
    return false;

  std::string name;
  std::string ns;
  if (nameParts.size() == 1)
    name = nameParts.at(0);
  else
  {
    ns = nameParts.at(0);
    name = nameParts.at(1);
  }

  if (name.empty())
    return false;

  // check if the deserialized namespace and name match the predefined ones
  if (!m_name.empty())
  {
    if (m_namespace.empty() != ns.empty() ||
       (!m_namespace.empty() && m_namespace.compare(ns) != 0))
      return false;

    if (m_name.compare(name) != 0)
      return false;
  }
  else
  {
    m_namespace = ns;
    m_name = name;
  }

  // deserialize the specified attributes
  for (auto& attr : m_properties)
  {
    if (!attr.second.Deserialize(element))
      return false;
  }

  // perform any custom deserialization
  return deserialize(element);
}

CDidlLiteElementProperty& IDidlLiteElement::addElementProperty(const std::string& name, void* propertyValue)
{
  return addElementProperty("", name, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addElementProperty(const std::string& ns, const std::string& name, void* propertyValue)
{
  return addProperty(ns, name, CDidlLiteElementProperty::Type::Element, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addStringProperty(const std::string& name, void* propertyValue)
{
  return addStringProperty("", name, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addStringProperty(const std::string& ns, const std::string& name, void* propertyValue)
{
  return addProperty(ns, name, CDidlLiteElementProperty::Type::String, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addBooleanProperty(const std::string& name, void* propertyValue)
{
  return addBooleanProperty("", name, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addBooleanProperty(const std::string& ns, const std::string& name, void* propertyValue)
{
  return addProperty(ns, name, CDidlLiteElementProperty::Type::Boolean, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addIntegerProperty(const std::string& name, void* propertyValue)
{
  return addIntegerProperty("", name, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addIntegerProperty(const std::string& ns, const std::string& name, void* propertyValue)
{
  return addProperty(ns, name, CDidlLiteElementProperty::Type::Integer, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addUnsignedIntegerProperty(const std::string& name, void* propertyValue)
{
  return addUnsignedIntegerProperty("", name, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addUnsignedIntegerProperty(const std::string& ns, const std::string& name, void* propertyValue)
{
  return addProperty(ns, name, CDidlLiteElementProperty::Type::UnsignedInteger, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addNumberProperty(const std::string& name, void* propertyValue)
{
  return addNumberProperty("", name, propertyValue);
}

CDidlLiteElementProperty& IDidlLiteElement::addNumberProperty(const std::string& ns, const std::string& name, void* propertyValue)
{
  return addProperty(ns, name, CDidlLiteElementProperty::Type::Number, propertyValue);
}

void IDidlLiteElement::setPropertyValid(const std::string& name)
{
  setPropertyValidity(name, true);
}

void IDidlLiteElement::setPropertyValid(const std::string& ns, const std::string& name)
{
  setPropertyValidity(ns, name, true);
}

void IDidlLiteElement::setPropertyValidity(const std::string& name, bool valid)
{
  setPropertyValidity("", name, valid);
}

void IDidlLiteElement::setPropertyValidity(const std::string& ns, const std::string& name, bool valid)
{
  std::string attributeName = DidlLiteUtils::GetElementName(ns, name);

  auto& attribute = m_properties.find(attributeName);
  if (attribute == m_properties.end())
    return;

  attribute->second.SetValid(valid);
}

void IDidlLiteElement::copyPropertyValidity(const IDidlLiteElement* otherElement)
{
  if (otherElement == nullptr)
    return;

  for (const auto& otherProp : otherElement->m_properties)
    setPropertyValidity(otherProp.first, otherProp.second.IsValid());
}

CDidlLiteElementProperty& IDidlLiteElement::addProperty(const std::string& ns, const std::string& name, CDidlLiteElementProperty::Type type, void* propertyValue)
{
  std::string propertyName = DidlLiteUtils::GetElementName(ns, name);

  // create the property
  CDidlLiteElementProperty elementProperty(ns, name, type, propertyValue);

  // add it to the properties map
  return m_properties.insert(std::make_pair(propertyName, elementProperty)).first->second;
}
