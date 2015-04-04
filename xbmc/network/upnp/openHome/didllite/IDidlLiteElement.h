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

#include <map>
#include <string>
#include <vector>

#include "network/upnp/openHome/didllite/DidlLiteElementProperty.h"

class TiXmlElement;

class IDidlLiteElement
{
public:
  virtual ~IDidlLiteElement();

  /*!
   * \brief Creates a new DIDL-Lite element instance.
   */
  virtual IDidlLiteElement* Create() const = 0;

  /*!
  * \brief Clones the DIDL-Lite element instance into a new DIDL-Lite element instance.
  */
  virtual IDidlLiteElement* Clone() const = 0;

  /*!
  * \brief Serializes the DIDL-Lite element into the given parent XML element.
  *
  * \param[out] element Parent XML element to serialize the DIDL-Lite element into
  * \return True if the serialization was successful otherwise false
  */
  virtual bool Serialize(TiXmlElement* element) const;

  /*!
  * \brief Deserializes the DIDL-Lite element from the given parent XML element.
  *
  * \param[in] element Parent XML element to deserialize the DIDL-Lite element from
  * \return True if the deserialization was successful otherwise false
  */
  virtual bool Deserialize(const TiXmlElement* element);

  const std::string& GetElementNamespace() const { return m_namespace; }
  const std::string& GetElementName() const { return m_name; }

protected:
  IDidlLiteElement();
  IDidlLiteElement(const std::string& name);
  IDidlLiteElement(const std::string& ns, const std::string& name);
  IDidlLiteElement(const IDidlLiteElement& element);

  /*!
  * \brief Serializes the DIDL-Lite element into the given XML element.
  *
  * \param[out] element XML element to serialize the DIDL-Lite element into
  * \return True if the serialization was successful otherwise false
  */
  virtual bool serialize(TiXmlElement* element) const { return true; }

  /*!
  * \brief Deserializes the DIDL-Lite element from the given XML element.
  *
  * \param[in] element XML element to deserialize the DIDL-Lite element from
  * \return True if the deserialization was successful otherwise false
  */
  virtual bool deserialize(const TiXmlElement* element) { return true; }

  CDidlLiteElementProperty& addElementProperty(const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addElementProperty(const std::string& ns, const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addStringProperty(const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addStringProperty(const std::string& ns, const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addBooleanProperty(const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addBooleanProperty(const std::string& ns, const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addIntegerProperty(const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addIntegerProperty(const std::string& ns, const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addUnsignedIntegerProperty(const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addUnsignedIntegerProperty(const std::string& ns, const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addNumberProperty(const std::string& name, void* propertyValue);
  CDidlLiteElementProperty& addNumberProperty(const std::string& ns, const std::string& name, void* propertyValue);

  void setPropertyValid(const std::string& name);
  void setPropertyValid(const std::string& ns, const std::string& name);
  void setPropertyValidity(const std::string& name, bool valid);
  void setPropertyValidity(const std::string& ns, const std::string& name, bool valid);

  void copyPropertyValidity(const IDidlLiteElement* otherElement);

  template<class TDidlLiteElement> static void copyElementProperty(TDidlLiteElement*& dstElement, const TDidlLiteElement* srcElement)
  {
    if (dstElement != NULL)
    {
      delete dstElement;
      dstElement = NULL;
    }

    if (srcElement == NULL)
      return;

    dstElement = srcElement->Clone();
  }

  template<class TDidlLiteElement> static void copyElementProperty(std::vector<TDidlLiteElement*>& dstElements, const std::vector<TDidlLiteElement*>& srcElements)
  {
    // first clean up the destination
    if (!dstElements.empty())
    {
      for (const auto& elem : dstElements)
        delete elem;

      dstElements.clear();
    }

    for (const auto& elem : srcElements)
      dstElements.push_back(reinterpret_cast<TDidlLiteElement*>(elem->Clone()));
  }

private:
  CDidlLiteElementProperty& addProperty(const std::string& ns, const std::string& name, CDidlLiteElementProperty::Type type, void* propertyValue);

  std::string m_namespace;
  std::string m_name;

  std::map<std::string, CDidlLiteElementProperty> m_properties;
};
