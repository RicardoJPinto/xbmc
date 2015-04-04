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

#include "network/upnp/openHome/didllite/objects/UPnPObject.h"

class CUPnPItem : public CUPnPObject
{
public:
  CUPnPItem();
  CUPnPItem(const std::string& classType, const std::string& className = "");
  CUPnPItem(const CUPnPItem& item);
  virtual ~CUPnPItem();

  // implementations of IDidlLiteElement
  virtual IDidlLiteElement* Create() const { return Clone(); }
  virtual IDidlLiteElement* Clone() const { return new CUPnPItem(*this); }

  // specialization of IFileItemElement
  virtual bool ToFileItem(CFileItem& item) const;

  // TODO

protected:
  void initializeProperties();

private:
  unsigned int m_objectUpdateID;
  std::string m_refID;
  std::vector<std::string> m_bookmarkIDs;
};
