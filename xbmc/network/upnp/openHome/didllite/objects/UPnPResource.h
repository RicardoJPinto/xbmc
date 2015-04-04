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

#include "network/upnp/openHome/didllite/IDidlLiteElement.h"

class CUPnPResource : public IDidlLiteElement
{
public:
  CUPnPResource();
  explicit CUPnPResource(const std::string &resourceUri);
  CUPnPResource(const CUPnPResource& upnpResource);
  virtual ~CUPnPResource();

  // implementations of IDidlLiteElement
  virtual IDidlLiteElement* Create() const { return new CUPnPResource(); }
  virtual IDidlLiteElement* Clone() const { return new CUPnPResource(*this); }

  const std::string& GetResourceUri() const { return m_resourceUri; }
  const std::string& GetResourceId() const { return m_resourceId; }

protected:
  // implementations of IDidlLiteElement
  virtual bool serialize(TiXmlElement* element) const;
  virtual bool deserialize(const TiXmlElement* element);

  void initializeProperties();

private:
  std::string m_resourceUri;
  std::string m_resourceId;
  std::string m_resourceProtocolInfo;
  std::string m_resourceImportUri;
  unsigned int m_resourceSize;
  std::string m_resourceDuration;
  std::string m_resourceProtection;
  unsigned int m_resourceBitrate;
  unsigned int m_resourceBitsPerSample;
  unsigned int m_resourceSampleFrequency;
  unsigned int m_resourceNrAudioChannels;
  std::string m_resourceResolution;
  unsigned int m_resourceColorDepth;
  std::string m_resourceTspec;
  std::string m_resourceAllowedUse;
  std::string m_resourceValidityStart;
  std::string m_resourceValidityEnd;
  std::string m_resourceRemainingTime;
  std::string m_resourceUsageInfo;
  std::string m_resourceRightsInfoURI;
  std::string m_resourceContentInfoURI;
  std::string m_resourceRecordQuality;
  std::string m_resourceDaylightSaving;
  std::string m_resourceFramerate;
};
