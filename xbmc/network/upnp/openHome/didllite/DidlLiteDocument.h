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

#include "network/upnp/openHome/didllite/IDidlLiteElement.h"

class CDidlLiteElementFactory;

class CDidlLiteDocument : IDidlLiteElement
{
public:
  CDidlLiteDocument();
  CDidlLiteDocument(const CDidlLiteElementFactory& factory);
  virtual ~CDidlLiteDocument();

  // implementations of IDidlLiteElement
  virtual IDidlLiteElement* Create() const { return new CDidlLiteDocument(); }
  virtual IDidlLiteElement* Clone() const { return NULL; }

  void AddNamespace(const std::string& name, const std::string& uri);
  void AddElement(IDidlLiteElement* element);

  bool Serialize(std::string& result) const;
  bool Deserialize(const std::string& data);

  std::vector<const IDidlLiteElement*> GetElements() const;

protected:
  // implementations of IDidlLiteElement
  virtual bool serialize(TiXmlElement* element) const;
  virtual bool deserialize(const TiXmlElement* element);

  void clear();

private:
  const CDidlLiteElementFactory& m_factory;
  std::map<std::string, std::string> m_namespaces;
  std::vector<IDidlLiteElement*> m_elements;
};
