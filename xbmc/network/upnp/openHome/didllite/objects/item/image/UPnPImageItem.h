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

#include "network/upnp/openHome/didllite/objects/UPnPRating.h"
#include "network/upnp/openHome/didllite/objects/item/UPnPItem.h"

class CUPnPImageItem : public CUPnPItem
{
public:
  CUPnPImageItem();
  CUPnPImageItem(const std::string& classType, const std::string& className = "");
  CUPnPImageItem(const CUPnPImageItem& imageItem);
  virtual ~CUPnPImageItem();

  // implementations of IDidlLiteElement
  virtual IDidlLiteElement* Create() const { return Clone(); }
  virtual IDidlLiteElement* Clone() const { return new CUPnPImageItem(*this); }

  // specializations of IFileItemElement
  virtual bool ToFileItem(CFileItem& item) const;
  virtual bool FromFileItem(const CFileItem& item);

  // TODO

protected:
  void initializeProperties();

private:
  std::string m_longDescription;
  std::string m_storageMedium;
  std::vector<CUPnPRating*> m_rating;
  std::string m_description;
  std::vector<std::string> m_publisher;
  std::string m_date;
  std::vector<std::string> m_rights;
};
