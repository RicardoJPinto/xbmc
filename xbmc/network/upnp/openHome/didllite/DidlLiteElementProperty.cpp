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

#include "DidlLiteElementProperty.h"
#include "network/upnp/openHome/didllite/DidlLiteUtils.h"
#include "network/upnp/openHome/didllite/IDidlLiteElement.h"
#include "utils/StringUtils.h"
#include "utils/XBMCTinyXml.h"
#include "utils/XMLUtils.h"

CDidlLiteElementProperty::CDidlLiteElementProperty(const std::string& name, Type type, void* value)
  : CDidlLiteElementProperty("", name, type, value)
{ }

CDidlLiteElementProperty::CDidlLiteElementProperty(const std::string& ns, const std::string& name, Type type, void* value)
  : m_value(value),
    m_type(type),
    m_isElement(true),
    m_required(false),
    m_multipleValues(false),
    m_valid(false),
    m_valueGenerator(nullptr),
    m_valueGenerated(false)
{
  m_isElement = !DidlLiteUtils::IsAttributeName(name);
  if (m_isElement)
    m_name = DidlLiteUtils::GetElementName(ns, name);
  else
    m_name = DidlLiteUtils::GetAttributeName(name);
}

CDidlLiteElementProperty::~CDidlLiteElementProperty()
{
  /* TODO: cleanup generated elements
  if (m_valueGenerated)
  {
    if (!m_multipleValues)
      delete reinterpret_cast<IDidlLiteElement*>(m_value);
    else
    {
      std::vector<IDidlLiteElement*>* values = reinterpret_cast<std::vector<IDidlLiteElement*>*>(m_value);
      for (const auto& value : *values)
        delete value;
    }
  }
  */

  delete m_valueGenerator;
  m_valueGenerator = nullptr;

  m_value = nullptr;
}

bool CDidlLiteElementProperty::Serialize(TiXmlElement* element) const
{
  if (element == nullptr)
    return false;

  // any required property needs to be set
  if (IsRequired() && !IsValid())
    return false;

  // ignore invalid properties
  if (!IsValid())
    return true;

  // the property is stored as an attribute
  if (IsAttribute())
    return serializeAttribute(element);

  return serializeElement(element);
}

bool CDidlLiteElementProperty::Deserialize(const TiXmlElement* element)
{
  if (element == nullptr)
    return false;

  // reset the valid flag
  m_valid = false;

  // the property is stored as an element
  if (IsElement())
    m_valid = deserializeElement(element);
  // the property is stored as an attribute
  else
    m_valid = deserializeAttribute(element);

  return m_valid;
}

CDidlLiteElementProperty& CDidlLiteElementProperty::SetGenerator(const IDidlLiteElement* valueGenerator)
{
  if (m_type != Type::Element)
    return *this;

  m_valueGenerator = valueGenerator;
  return *this;
}

template<typename TType>
std::vector<const TType*> getValues(const void* value, bool multipleValues)
{
  std::vector<const TType*> values;
  if (!multipleValues)
    values.push_back(reinterpret_cast<const TType*>(value));
  else
  {
    const std::vector<TType>* vecValues = reinterpret_cast<const std::vector<TType>*>(value);
    for (const auto& it : *vecValues)
      values.push_back(&it);
  }

  return values;
}

bool CDidlLiteElementProperty::serializeElement(TiXmlElement* element) const
{
  switch (m_type)
  {
  case Type::Element:
  {
    std::vector<const IDidlLiteElement*> values = getValues<IDidlLiteElement>(m_value, m_multipleValues);
    for (const auto& value : values)
    {
      if (!value->Serialize(element))
        return false;
    }
    break;
  }

  case Type::Boolean:
  {
    std::vector<const bool*> values = getValues<bool>(m_value, m_multipleValues);
    for (const auto& value : values)
      XMLUtils::SetBoolean(element, m_name.c_str(), *value);
    break;
  }

  case Type::String:
  {
    std::vector<const std::string*> values = getValues<std::string>(m_value, m_multipleValues);
    for (const auto& value : values)
      XMLUtils::SetString(element, m_name.c_str(), *value);
    break;
  }

  case Type::Integer:
  {
    std::vector<const int*> values = getValues<int>(m_value, m_multipleValues);
    for (const auto& value : values)
      XMLUtils::SetInt(element, m_name.c_str(), *value);
    break;
  }

  case Type::UnsignedInteger:
  {
    std::vector<const unsigned int*> values = getValues<unsigned int>(m_value, m_multipleValues);
    for (const auto& value : values)
      XMLUtils::SetInt(element, m_name.c_str(), *value);
    break;
  }

  case Type::Number:
  {
    std::vector<const double*> values = getValues<double>(m_value, m_multipleValues);
    for (const auto& value : values)
      XMLUtils::SetFloat(element, m_name.c_str(), static_cast<float>(*value));
    break;
  }

  default:
    return false;
  }

  return true;
}

bool CDidlLiteElementProperty::serializeAttribute(TiXmlElement* element) const
{
  std::string propertyValue;
  // try to serialize the property's value into a string
  if (!serializePropertyValue(m_type, m_value, propertyValue))
    return false;

  // set the attribute
  element->SetAttribute(m_name, propertyValue);

  return true;
}

size_t countElements(const TiXmlElement* propertyElement, const std::string& name)
{
  size_t count = 0;
  while (propertyElement != nullptr)
  {
    ++count;

    propertyElement = propertyElement->NextSiblingElement(name);
  }

  return count;
}

template<typename TType>
void setValues(std::vector<TType> values, void* value, bool multipleValues)
{
  if (!multipleValues)
    *reinterpret_cast<TType*>(value) = values.at(0);
  else
  {
    std::vector<TType>* propertyValues = reinterpret_cast<std::vector<TType>*>(value);
    propertyValues->assign(values.begin(), values.end());
  }
}

bool CDidlLiteElementProperty::deserializeElement(const TiXmlElement* element)
{
  // count all elements matching the property's name
  size_t elementCount = 0;
  const TiXmlElement* propertyElement = element->FirstChildElement(m_name);
  while (propertyElement != nullptr)
  {
    ++elementCount;

    propertyElement = propertyElement->NextSiblingElement(m_name);
  }

  // we couldn't find any matching elements
  if (elementCount == 0)
  {
    // a required property is missing
    if (IsRequired())
      return false;

    // an optional property is not defined
    return true;
  }

  // the property doesn't support multiple values but we found more than one matching element
  if (elementCount > 1 && !m_multipleValues)
    return false;

  // deserialize the attribute value
  switch (m_type)
  {
  case Type::Element:
  {
    // to support multiple or optional values we need a generator
    if (m_valueGenerator == nullptr && (m_multipleValues || m_value == nullptr))
      return false;

    std::vector<IDidlLiteElement*> values;
    if (!m_multipleValues)
    {
      // get a new element using the generator if necessary
      if (m_value == NULL)
      {
        m_value = m_valueGenerator->Clone();
        m_valueGenerated = true;
      }

      values.push_back(reinterpret_cast<IDidlLiteElement*>(m_value));
    }
    else
    {
      for (size_t i = 0; i < elementCount; ++i)
        values.push_back(m_valueGenerator->Clone());
      m_valueGenerated = true;
    }

    bool result = true;
    propertyElement = element->FirstChildElement(m_name);
    for (auto& value : values)
    {
      if (!value->Deserialize(propertyElement))
      {
        result = false;
        break;
      }

      propertyElement = propertyElement->NextSiblingElement(m_name);
    }

    if (!result)
    {
      if (m_valueGenerated)
      {
        // cleanup generated values
        for (auto& value : values)
          delete value;

        if (!m_multipleValues)
          m_value = nullptr;
      }

      return result;
    }

    setValues<IDidlLiteElement*>(values, m_value, m_multipleValues);
    return true;
  }

  case Type::Boolean:
  {
    std::vector<bool> values;
    for (size_t i = 0; i < elementCount; ++i)
    {
      bool value;
      if (!XMLUtils::GetBoolean(element, m_name.c_str(), value))
        return false;

      values.push_back(value);
    }

    setValues<bool>(values, m_value, m_multipleValues);
    return true;
  }

  case Type::String:
  {
    std::vector<std::string> values;
    for (size_t i = 0; i < elementCount; ++i)
    {
      std::string value;
      if (!XMLUtils::GetString(element, m_name.c_str(), value))
        return false;

      values.push_back(value);
    }

    setValues<std::string>(values, m_value, m_multipleValues);
    return true;
  }

  case Type::Integer:
  {
    std::vector<int> values;
    for (size_t i = 0; i < elementCount; ++i)
    {
      int value;
      if (!XMLUtils::GetInt(element, m_name.c_str(), value))
        return false;

      values.push_back(value);
    }

    setValues<int>(values, m_value, m_multipleValues);
    return true;
  }

  case Type::UnsignedInteger:
  {
    std::vector<unsigned int> values;
    for (size_t i = 0; i < elementCount; ++i)
    {
      unsigned int value;
      if (!XMLUtils::GetUInt(element, m_name.c_str(), value))
        return false;

      values.push_back(value);
    }

    setValues<unsigned int>(values, m_value, m_multipleValues);
    return true;
  }

  case Type::Number:
  {
    std::vector<double> values;
    for (size_t i = 0; i < elementCount; ++i)
    {
      double value;
      if (!XMLUtils::GetDouble(element, m_name.c_str(), value))
        return false;

      values.push_back(value);
    }

    setValues<double>(values, m_value, m_multipleValues);
    return true;
  }

  default:
    return false;
  }

  return false;
}

bool CDidlLiteElementProperty::deserializeAttribute(const TiXmlElement* element)
{
  // try to retrieve the value of the attribute
  const std::string* attributeValue = element->Attribute(m_name);
  if (attributeValue == nullptr)
  {
    // a required attribute is missing
    if (IsRequired())
      return false;

    // an optional attribute is not defined
    return true;
  }

  // deserialize the property value
  return deserializePropertyValue(m_type, *attributeValue, m_value);
}

bool CDidlLiteElementProperty::serializePropertyValue(Type type, const void* propertyValue, std::string& value)
{
  switch (type)
  {
  case Type::String:
    value = *reinterpret_cast<const std::string*>(propertyValue);
    break;

  case Type::Boolean:
    value = *reinterpret_cast<const bool*>(propertyValue) ? "1" : "0";
    break;

  case Type::Integer:
    value = StringUtils::Format("%d", *reinterpret_cast<const int*>(propertyValue));
    break;

  case Type::UnsignedInteger:
    value = StringUtils::Format("%u", *reinterpret_cast<const unsigned int*>(propertyValue));
    break;

  case Type::Number:
    value = StringUtils::Format("%.2f", *reinterpret_cast<const double*>(propertyValue));
    break;

  default:
    return false;
  }

  return true;
}

bool CDidlLiteElementProperty::deserializePropertyValue(Type type, const std::string& value, void* propertyValue)
{
  switch (type)
  {
  case Type::String:
    *reinterpret_cast<std::string*>(propertyValue) = value;
    break;

  case Type::Boolean:
    if (value.compare("0") == 0)
      *reinterpret_cast<bool*>(propertyValue) = false;
    else if (value.compare("1") == 0)
      *reinterpret_cast<bool*>(propertyValue) = true;
    else if (StringUtils::EqualsNoCase(value, "false"))
      *reinterpret_cast<bool*>(propertyValue) = false;
    else if (StringUtils::EqualsNoCase(value, "true"))
      *reinterpret_cast<bool*>(propertyValue) = true;
    else
      return false;
    break;

  case Type::Integer:
  {
    char* end = nullptr;
    *reinterpret_cast<int*>(propertyValue) = static_cast<int>(strtol(value.c_str(), &end, 0));
    if (end != nullptr && *end != '\0')
      return false;
    break;
  }

  case Type::UnsignedInteger:
  {
    char* end = nullptr;
    *reinterpret_cast<unsigned int*>(propertyValue) = static_cast<unsigned int>(strtoul(value.c_str(), &end, 0));
    if (end != nullptr && *end != '\0')
      return false;
    break;
  }

  case Type::Number:
  {
    char* end = nullptr;
    *reinterpret_cast<double*>(propertyValue) = static_cast<double>(strtod(value.c_str(), &end));
    if (end != nullptr && *end != '\0')
      return false;
    break;
  }

  default:
    return false;
  }

  return true;
}
