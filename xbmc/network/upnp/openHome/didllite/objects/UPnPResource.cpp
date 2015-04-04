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

#include "UPnPResource.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"
#include "utils/XBMCTinyXml.h"

CUPnPResource::CUPnPResource()
  : CUPnPResource("")
{ }

CUPnPResource::CUPnPResource(const std::string& resourceUri)
  : IDidlLiteElement("res"),
    m_resourceUri(resourceUri),
    m_resourceSize(0),
    m_resourceBitrate(0),
    m_resourceBitsPerSample(0),
    m_resourceSampleFrequency(0),
    m_resourceNrAudioChannels(0),
    m_resourceColorDepth(0)
{
  initializeProperties();
}

CUPnPResource::CUPnPResource(const CUPnPResource& upnpResource)
  : IDidlLiteElement(upnpResource),
    m_resourceUri(upnpResource.m_resourceUri),
    m_resourceId(upnpResource.m_resourceId),
    m_resourceProtocolInfo(upnpResource.m_resourceProtocolInfo),
    m_resourceImportUri(upnpResource.m_resourceImportUri),
    m_resourceSize(upnpResource.m_resourceSize),
    m_resourceDuration(upnpResource.m_resourceDuration),
    m_resourceProtection(upnpResource.m_resourceProtection),
    m_resourceBitrate(upnpResource.m_resourceBitrate),
    m_resourceBitsPerSample(upnpResource.m_resourceBitsPerSample),
    m_resourceSampleFrequency(upnpResource.m_resourceSampleFrequency),
    m_resourceNrAudioChannels(upnpResource.m_resourceNrAudioChannels),
    m_resourceResolution(upnpResource.m_resourceResolution),
    m_resourceColorDepth(upnpResource.m_resourceColorDepth),
    m_resourceTspec(upnpResource.m_resourceTspec),
    m_resourceAllowedUse(upnpResource.m_resourceAllowedUse),
    m_resourceValidityStart(upnpResource.m_resourceValidityStart),
    m_resourceValidityEnd(upnpResource.m_resourceValidityEnd),
    m_resourceRemainingTime(upnpResource.m_resourceRemainingTime),
    m_resourceUsageInfo(upnpResource.m_resourceUsageInfo),
    m_resourceRightsInfoURI(upnpResource.m_resourceRightsInfoURI),
    m_resourceContentInfoURI(upnpResource.m_resourceContentInfoURI),
    m_resourceRecordQuality(upnpResource.m_resourceRecordQuality),
    m_resourceDaylightSaving(upnpResource.m_resourceDaylightSaving),
    m_resourceFramerate(upnpResource.m_resourceFramerate)
{
  initializeProperties();
  copyPropertyValidity(&upnpResource);
}

CUPnPResource::~CUPnPResource()
{ }

bool CUPnPResource::serialize(TiXmlElement* element) const
{
  TiXmlText uri(m_resourceUri);
  element->InsertEndChild(uri);

  return true;
}

bool CUPnPResource::deserialize(const TiXmlElement* element)
{
  if (element->FirstChild() == nullptr)
    return false;

  m_resourceUri = element->FirstChild()->ValueStr();
  if (m_resourceUri.empty())
    return false;

  return true;
}

void CUPnPResource::initializeProperties()
{
  addStringProperty("@id", &m_resourceId).AsAttribute().SetOptional();
  addStringProperty("@protocolInfo", &m_resourceProtocolInfo).AsAttribute().SetOptional();
  addStringProperty("@importUri", &m_resourceImportUri).AsAttribute().SetOptional();
  addUnsignedIntegerProperty("@size", &m_resourceSize).AsAttribute().SetOptional();
  addStringProperty("@duration", &m_resourceDuration).AsAttribute().SetOptional();
  addStringProperty("@protection", &m_resourceProtection).AsAttribute().SetOptional();
  addUnsignedIntegerProperty("@bitrate", &m_resourceBitrate).AsAttribute().SetOptional();
  addUnsignedIntegerProperty("@bitsPerSample", &m_resourceBitsPerSample).AsAttribute().SetOptional();
  addUnsignedIntegerProperty("@sampleFrequency", &m_resourceSampleFrequency).AsAttribute().SetOptional();
  addUnsignedIntegerProperty("@nrAudioChannels", &m_resourceNrAudioChannels).AsAttribute().SetOptional();
  addStringProperty("@resolution", &m_resourceResolution).AsAttribute().SetOptional();
  addUnsignedIntegerProperty("@colorDepth", &m_resourceColorDepth).AsAttribute().SetOptional();
  addStringProperty("@tspec", &m_resourceTspec).AsAttribute().SetOptional();
  addStringProperty("@allowedUse", &m_resourceAllowedUse).AsAttribute().SetOptional();
  addStringProperty("@validityStart", &m_resourceValidityStart).AsAttribute().SetOptional();
  addStringProperty("@validityEnd", &m_resourceValidityEnd).AsAttribute().SetOptional();
  addStringProperty("@remainingTime", &m_resourceRemainingTime).AsAttribute().SetOptional();
  addStringProperty("@usageInfo", &m_resourceUsageInfo).AsAttribute().SetOptional();
  addStringProperty("@rightsInfoURI", &m_resourceRightsInfoURI).AsAttribute().SetOptional();
  addStringProperty("@contentInfoURI", &m_resourceContentInfoURI).AsAttribute().SetOptional();
  addStringProperty("@recordQuality", &m_resourceRecordQuality).AsAttribute().SetOptional();
  addStringProperty("@daylightSaving", &m_resourceDaylightSaving).AsAttribute().SetOptional();
  addStringProperty("@framerate", &m_resourceFramerate).AsAttribute().SetOptional();
}
