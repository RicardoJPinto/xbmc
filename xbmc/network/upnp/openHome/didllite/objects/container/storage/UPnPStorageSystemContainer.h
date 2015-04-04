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

#include "network/upnp/openHome/didllite/objects/container/UPnPContainer.h"

class CUPnPStorageSystemContainer : public CUPnPContainer
{
public:
  CUPnPStorageSystemContainer();
  CUPnPStorageSystemContainer(const std::string& classType, const std::string& className = "");
  CUPnPStorageSystemContainer(const CUPnPStorageSystemContainer& storageSystemContainer);
  virtual ~CUPnPStorageSystemContainer();

  // implementations of IDidlLiteElement
  virtual IDidlLiteElement* Create() const { return Clone(); }
  virtual IDidlLiteElement* Clone() const { return new CUPnPStorageSystemContainer(*this); }

  // specializations of IFileItemElement
  virtual bool ToFileItem(CFileItem& item) const;
  virtual bool FromFileItem(const CFileItem& item);

protected:
  void initializeProperties();

private:
  int m_storageTotal;
  int m_storageUsed;
  int m_storageFree;
  int m_storageMaxPartition;
  std::string m_storageMedium;
};
