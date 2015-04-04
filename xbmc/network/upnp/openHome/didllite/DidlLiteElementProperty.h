#pragma once
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

#include <string>

class IDidlLiteElement;
class TiXmlElement;

class CDidlLiteElementProperty
{
public:
  enum class Type {
    Element,
    String,
    Boolean,
    Integer,
    UnsignedInteger,
    Number
  };

  CDidlLiteElementProperty(const std::string& name, Type type, void* value);
  CDidlLiteElementProperty(const std::string& ns, const std::string& name, Type type, void* value);
  virtual ~CDidlLiteElementProperty();

  bool Serialize(TiXmlElement* element) const;
  bool Deserialize(const TiXmlElement* element);

  CDidlLiteElementProperty& AsAttribute() { m_isElement = false; return *this; }
  CDidlLiteElementProperty& SetOptional(bool optional = true) { m_required = !optional; return *this; }
  CDidlLiteElementProperty& SetRequired(bool required = true) { m_required = required; return *this; }
  CDidlLiteElementProperty& SupportMultipleValues() { m_multipleValues = true; return *this; }
  CDidlLiteElementProperty& SetGenerator(const IDidlLiteElement* valueGenerator);
  CDidlLiteElementProperty& SetValid(bool valid = true) { m_valid = valid; return *this; }
  CDidlLiteElementProperty& SetInvalid(bool invalid = true) { m_valid = !invalid; return *this; }

  const std::string& GetName() const { return m_name; }

  const void* GetValue() const { return m_value; }
  void* GetValue() { return m_value; }
  template<typename TType> const TType* GetValue() const { return reinterpret_cast<TType*>(m_value); }
  template<typename TType> TType* GetValue() { return reinterpret_cast<TType*>(m_value); }

  Type GetType() const { return m_type; }
  bool IsElement() const { return m_isElement; }
  bool IsAttribute() const { return !m_isElement; }
  bool IsRequired() const { return m_required; }
  bool IsOptional() const { return !m_required; }
  bool SupportsMultipleValues() const { return m_multipleValues; }
  bool IsValid() const { return m_valid; }

private:
  bool serializeElement(TiXmlElement* element) const;
  bool serializeAttribute(TiXmlElement* element) const;
  bool deserializeElement(const TiXmlElement* element);
  bool deserializeAttribute(const TiXmlElement* element);

  static bool serializePropertyValue(Type type, const void* propertyValue, std::string& value);
  static bool deserializePropertyValue(Type type, const std::string& value, void* propertyValue);

  std::string m_name;
  void* m_value;
  Type m_type;

  bool m_isElement;
  bool m_required;
  bool m_multipleValues;
  bool m_valid;
  const IDidlLiteElement* m_valueGenerator;
  bool m_valueGenerated;
};
