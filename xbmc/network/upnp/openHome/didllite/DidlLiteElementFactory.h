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
#include <memory>
#include <string>

class IDidlLiteElement;
class TiXmlNode;

class CDidlLiteElementFactory
{
public:
  CDidlLiteElementFactory();
  ~CDidlLiteElementFactory();

  static CDidlLiteElementFactory& Get();

  IDidlLiteElement* GetElement(const TiXmlNode* xmlNode) const;

  bool RegisterElement(const IDidlLiteElement* element);

private:
  std::map<std::string, std::shared_ptr<const IDidlLiteElement>> m_elements;
};
