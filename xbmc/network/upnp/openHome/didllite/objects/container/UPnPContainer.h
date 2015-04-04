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

#include "network/upnp/openHome/didllite/objects/UPnPClass.h"
#include "network/upnp/openHome/didllite/objects/UPnPObject.h"

class CUPnPContainer : public CUPnPObject
{
public:
  class CUPnPClassDefinition : public CUPnPClass
  {
  public:
    CUPnPClassDefinition(const std::string& classType = "", const std::string& className = "", bool includeDerived = false);
    CUPnPClassDefinition(const CUPnPClassDefinition& classDefinition);
    virtual ~CUPnPClassDefinition();

    // implementations of IDidlLiteElement
    virtual IDidlLiteElement* Create() const { return new CUPnPClassDefinition(); }
    virtual IDidlLiteElement* Clone() const { return new CUPnPClassDefinition(*this); }

    bool IncludeDerived() const { return m_includeDerived; }

  protected:
    // implementation of IDidlLiteElement
    void initializeProperties();

  private:
    bool m_includeDerived;
  };

public:
  CUPnPContainer();
  CUPnPContainer(const std::string& classType, const std::string& className = "");
  CUPnPContainer(const CUPnPContainer& container);
  virtual ~CUPnPContainer();

  // implementations of IDidlLiteElement
  virtual IDidlLiteElement* Create() const { return Clone(); }
  virtual IDidlLiteElement* Clone() const { return new CUPnPContainer(*this); }

  // specializations of IFileItemElement
  virtual bool ToFileItem(CFileItem& item) const;
  virtual bool FromFileItem(const CFileItem& item);

  unsigned int GetObjectUpdateID() const { return m_objectUpdateID; }
  unsigned int GetContainerUpdateID() const { return m_containerUpdateID; }
  unsigned int GetTotalDeletedChildCount() const { return m_totalDeletedChildCount; }
  unsigned int GetChildCount() const { return m_childCount; }
  unsigned int GetChildContainerCount() const { return m_childContainerCount; }
  const std::vector<CUPnPClassDefinition>& GetCreateClasses() const { return m_createClass; }
  const std::vector<CUPnPClassDefinition>& GetSearchClasses() const { return m_searchClass; }
  bool IsSearchable() const { return m_searchable; }
  bool IsNeverPlayable() const { return m_neverPlayable; }

protected:
  void initializeProperties();

private:
  unsigned int m_objectUpdateID;
  unsigned int m_containerUpdateID;
  unsigned int m_totalDeletedChildCount;
  unsigned int m_childCount;
  unsigned int m_childContainerCount;
  std::vector<CUPnPClassDefinition> m_createClass;
  std::vector<CUPnPClassDefinition> m_searchClass;
  bool m_searchable;
  bool m_neverPlayable;
};
