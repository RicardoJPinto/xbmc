/*
 *      Copyright (C) 2014 Team XBMC
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
#include <algorithm>

#include <OpenHome/Net/Cpp/CpDevice.h>

#include "ohUPnPControlPointDevice.h"
#include "URL.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/XMLUtils.h"

#define UPNP_DEVICE_ELEMENT_ROOT              "root"
#define UPNP_DEVICE_ELEMENT_DEVICE            "device"
#define UPNP_DEVICE_ELEMENT_DEVICE_TYPE       "deviceType"
#define UPNP_DEVICE_ELEMENT_FRIENDLY_NAME     "friendlyName"
#define UPNP_DEVICE_ELEMENT_MANUFACTURER      "manufacturer"
#define UPNP_DEVICE_ELEMENT_MANUFACTURER_URL  "manufacturerURL"
#define UPNP_DEVICE_ELEMENT_MODEL_DESCRIPTION "modelDescription"
#define UPNP_DEVICE_ELEMENT_MODEL_NAME        "modelName"
#define UPNP_DEVICE_ELEMENT_MODEL_NUMBER      "modelNumber"
#define UPNP_DEVICE_ELEMENT_MODEL_URL         "modelURL"
#define UPNP_DEVICE_ELEMENT_SERIAL_NUMBER     "serialNumber"
#define UPNP_DEVICE_ELEMENT_UDN               "UDN"
#define UPNP_DEVICE_ELEMENT_UPC               "UPC"

#define UPNP_DEVICE_ELEMENT_ICON_LIST         "iconList"
#define UPNP_DEVICE_ELEMENT_ICON              "icon"
#define UPNP_DEVICE_ELEMENT_ICON_URL          "url"
#define UPNP_DEVICE_ELEMENT_ICON_MIMETYPE     "mimetype"
#define UPNP_DEVICE_ELEMENT_ICON_WIDTH        "width"
#define UPNP_DEVICE_ELEMENT_ICON_HEIGHT       "height"
#define UPNP_DEVICE_ELEMENT_ICON_DEPTH        "depth"

#define UPNP_DEVICE_ELEMENT_SERVICE_LIST      "serviceList"
#define UPNP_DEVICE_ELEMENT_DEVICE_LIST       "deviceList"
#define UPNP_DEVICE_ELEMENT_PRESENTATION_URL  "presentationURL"

using namespace OpenHome;
using namespace OpenHome::Net;

COhUPnPControlPointDevice::COhUPnPControlPointDevice()
  : m_valid(false)
{ }

COhUPnPControlPointDevice::COhUPnPControlPointDevice(CpDeviceCpp& device)
  : m_valid(false)
{
  m_uuid = device.Udn();
  if (m_uuid.empty())
    return;

  std::string descriptionUrl;
  if (!device.GetAttribute(UPNP_DEVICE_ATTRIBUTE_LOCATION, descriptionUrl) || descriptionUrl.empty())
    return;

  std::string descriptionXml;
  if (!device.GetAttribute(UPNP_DEVICE_ATTRIBUTE_DEVICE_XML, descriptionXml) || descriptionXml.empty())
    return;

  m_valid = setDescription(descriptionUrl, descriptionXml);
}

COhUPnPControlPointDevice::~COhUPnPControlPointDevice()
{ }

std::string COhUPnPControlPointDevice::GetIconUrl() const
{
  if (!m_valid || m_icons.empty())
    return "";

  return URIUtils::AddFileToFolder(m_baseUrl, m_icons.begin()->GetPath());
}

std::string COhUPnPControlPointDevice::GetIconUrl(const std::string &mimetype) const
{
  if (!m_valid || m_icons.empty())
    return "";

  if (mimetype.empty())
    return GetIconUrl();

  for (std::vector<CUPnPIcon>::const_iterator icon = m_icons.begin(); icon != m_icons.end(); ++icon)
  {
    if (StringUtils::EqualsNoCase(icon->GetMimeType(), mimetype))
      return URIUtils::AddFileToFolder(m_baseUrl, icon->GetPath());
  }

  return "";
}

bool COhUPnPControlPointDevice::setDescription(const std::string &descriptionUrl, const std::string &descriptionXml)
{
  if (descriptionUrl.empty() || descriptionXml.empty())
    return false;

  m_descriptionUrl = descriptionUrl;
  if (!setBaseUrl(m_descriptionUrl))
    return false;

  return deserialize(descriptionXml);
}

bool COhUPnPControlPointDevice::deserialize(const std::string &deviceDescriptionXml)
{
  if (deviceDescriptionXml.empty())
    return false;

  CXBMCTinyXML doc;
  if (!doc.Parse(deviceDescriptionXml) || doc.RootElement() == NULL)
    return false;

  TiXmlElement *rootElement = doc.RootElement();
  if (!StringUtils::EqualsNoCase(rootElement->ValueStr(), UPNP_DEVICE_ELEMENT_ROOT))
    return false;

  TiXmlNode *deviceNode = rootElement->FirstChild(UPNP_DEVICE_ELEMENT_DEVICE);
  if (deviceNode == NULL)
    return false;

  if (!XMLUtils::GetString(deviceNode, UPNP_DEVICE_ELEMENT_DEVICE_TYPE, m_deviceTypeString) ||
      !parseDeviceType(m_deviceTypeString, m_deviceType, m_deviceTypeVersion))
    return false;

  if (!XMLUtils::GetString(deviceNode, UPNP_DEVICE_ELEMENT_FRIENDLY_NAME, m_friendlyName) ||
      m_friendlyName.empty())
    return false;

  if (!XMLUtils::GetString(deviceNode, UPNP_DEVICE_ELEMENT_MANUFACTURER, m_manufacturer) ||
      m_manufacturer.empty())
    return false;

  XMLUtils::GetString(deviceNode, UPNP_DEVICE_ELEMENT_MANUFACTURER_URL, m_manufacturerUrl);
  XMLUtils::GetString(deviceNode, UPNP_DEVICE_ELEMENT_MODEL_DESCRIPTION, m_modelDescription);

  if (!XMLUtils::GetString(deviceNode, UPNP_DEVICE_ELEMENT_MODEL_NAME, m_modelName) ||
      m_modelName.empty())
    return false;

  XMLUtils::GetString(deviceNode, UPNP_DEVICE_ELEMENT_MODEL_NUMBER, m_modelNumber);
  XMLUtils::GetString(deviceNode, UPNP_DEVICE_ELEMENT_MODEL_URL, m_modelUrl);
  XMLUtils::GetString(deviceNode, UPNP_DEVICE_ELEMENT_SERIAL_NUMBER, m_serialNumber);
  XMLUtils::GetString(deviceNode, UPNP_DEVICE_ELEMENT_UPC, m_universalProductCode);
  XMLUtils::GetString(deviceNode, UPNP_DEVICE_ELEMENT_PRESENTATION_URL, m_presentationUrl);

  // parse <iconList>
  TiXmlNode *iconListNode = deviceNode->FirstChild(UPNP_DEVICE_ELEMENT_ICON_LIST);
  if (iconListNode != NULL)
  {
    TiXmlNode *iconNode = iconListNode->FirstChild(UPNP_DEVICE_ELEMENT_ICON);
    while (iconNode != NULL)
    {
      std::string url, mimetype;
      uint32_t width, height, depth;
      if (XMLUtils::GetString(iconNode, UPNP_DEVICE_ELEMENT_ICON_URL, url) && !url.empty() &&
          XMLUtils::GetString(iconNode, UPNP_DEVICE_ELEMENT_ICON_MIMETYPE, mimetype) && !mimetype.empty() &&
          XMLUtils::GetUInt(iconNode, UPNP_DEVICE_ELEMENT_ICON_WIDTH, width) &&
          XMLUtils::GetUInt(iconNode, UPNP_DEVICE_ELEMENT_ICON_HEIGHT, height) &&
          XMLUtils::GetUInt(iconNode, UPNP_DEVICE_ELEMENT_ICON_DEPTH, depth))
        m_icons.push_back(CUPnPIcon(url, mimetype, width, height, depth));

      iconNode = iconNode->NextSibling();
    }

    // sort by desencing icon quality
    std::sort(m_icons.begin(), m_icons.end());
    std::reverse(m_icons.begin(), m_icons.end());
  }

  // TODO: serviceList
  // TODO: deviceList

  return true;
}

bool COhUPnPControlPointDevice::setBaseUrl(const std::string &descriptionUrl)
{
  if (descriptionUrl.empty())
    return false;

  CURL url(descriptionUrl);
  // remove any options
  url.SetOptions("");
  // remove the filename of the description XML
  url.SetFileName(URIUtils::GetDirectory(url.GetFileName()));

  m_baseUrl = url.Get();

  return !m_baseUrl.empty();
}

bool COhUPnPControlPointDevice::parseDeviceType(const std::string &deviceTypeString, std::string &deviceType, uint8_t &version)
{
  // the deviceType value must be of the form "urn:<domain-name>:device:<deviceType>:<version>"
  if (!StringUtils::StartsWith(deviceTypeString, "urn:"))
    return false;

  std::string tmp = StringUtils::Mid(deviceTypeString, 4);
  size_t pos = tmp.find(':');
  if (pos == std::string::npos)
    return false;

  // we don't care for the domain-name part
  tmp = StringUtils::Mid(tmp, pos + 1);

  // make sure the next part is "device:"
  if (!StringUtils::StartsWith(tmp, "device:"))
    return false;
  tmp = StringUtils::Mid(tmp, 7);

  // now split the deviceType and the version
  pos = tmp.find(":");
  if (pos == std::string::npos)
    return false;

  // extract the deviceType
  deviceType = StringUtils::Left(tmp, pos);
  tmp = StringUtils::Mid(tmp, pos + 1);

  char *endptr = NULL;
  version = static_cast<uint8_t>(strtoul(tmp.c_str(), &endptr, 0));
  if (endptr != NULL && *endptr != '\0')
    return false;

  return true;
}
