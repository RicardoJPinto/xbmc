#pragma once
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

#include <OpenHome/Net/Cpp/CpUpnpOrgContentDirectory1.h>

#include "network/upnp/openHome/didllite/DidlLiteElementFactory.h"
#include "network/upnp/openHome/didllite/objects/classmappers/UPnPClassMapping.h"
#include "network/upnp/openHome/ohNet/ohUPnPControlPoint.h"

class CDidlLiteDocument;
class CFileItem;
class CFileItemList;

class COhUPnPContentDirectoryControlPoint : public IOhUPnPControlPoint<OpenHome::Net::CpProxyUpnpOrgContentDirectory1Cpp>
{
public:
  COhUPnPContentDirectoryControlPoint();
  virtual ~COhUPnPContentDirectoryControlPoint();

  bool BrowseSync(const std::string& uuid, const std::string& objectId, CFileItemList& items,
                  uint32_t start = 0, uint32_t count = 0, const std::string& filter = "", const std::string& sorting = "") const;
  bool BrowseMetadataSync(const std::string& uuid, const std::string& objectId, CFileItem& item, const std::string& filter = "") const;

protected:
  // implementation of IOhUPnPServiceClient
  virtual void onServiceAdded(const UPnPControlPointService& service);
  virtual void onServiceRemoved(const UPnPControlPointService& service);

  bool browseSync(const std::string& uuid, const std::string& objectId, bool metadata, std::string& result, uint32_t& resultCount, uint32_t& resultTotal,
                  uint32_t start, uint32_t count, const std::string& filter, const std::string& sorting) const;

  bool resultToFileItemList(const std::string& result, uint32_t resultCount, uint32_t resultTotal, CFileItemList& items) const;

private:
  CDidlLiteElementFactory m_elementFactory;
  CUPnPClassMapping m_classMapping;
};
