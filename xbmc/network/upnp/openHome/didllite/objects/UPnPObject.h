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

#include "network/upnp/openHome/didllite/objects/IFileItemElement.h"
#include "network/upnp/openHome/didllite/objects/UPnPClass.h"
#include "network/upnp/openHome/didllite/objects/UPnPResource.h"

class CUPnPObject : public IFileItemElement
{
public:
  virtual ~CUPnPObject();

  // implementations of IFileItemElement
  virtual std::string GetType() const { return m_class.GetType(); }
  virtual bool ToFileItem(CFileItem& item) const;
  virtual bool FromFileItem(const CFileItem& item);

  const std::string& GetId() const { return m_id; }
  const std::string& GetParentId() const { return m_parentID; }
  bool IsRestricted() const { return m_restricted; }
  const CUPnPClass& GetClass() const { return m_class; }
  const std::string& GetTitle() const { return m_title; }
  const std::string& GetCreator() const { return m_creator; }
  const std::string& GetWriteStatus() const { return m_writeStatus; }
  std::vector<const CUPnPResource*> GetResources() const;

  CUPnPClass& GetClass() { return m_class; }

protected:
  CUPnPObject(const std::string& elementName, const std::string& classType, const std::string& className = "");
  CUPnPObject(const CUPnPObject& obj);

  void initializeProperties();

private:
  std::string m_id;
  std::string m_parentID;
  bool m_restricted;
  CUPnPClass m_class;
  std::string m_title;
  std::string m_creator;
  std::string m_writeStatus;
  std::vector<CUPnPResource*> m_res;
};
