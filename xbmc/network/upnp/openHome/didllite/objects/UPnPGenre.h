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

class CUPnPGenre : public IDidlLiteElement
{
public:
  CUPnPGenre();
  explicit CUPnPGenre(const std::string& name);
  CUPnPGenre(const std::string& name, const std::string& id, const std::string& extended);
  CUPnPGenre(const CUPnPGenre& upnpGenre);
  virtual ~CUPnPGenre();

  // implementations of IDidlLiteElement
  virtual IDidlLiteElement* Create() const { return new CUPnPGenre(); }
  virtual IDidlLiteElement* Clone() const { return new CUPnPGenre(*this); }

  const std::string& GetGenre() const { return m_genreName; }
  const std::string& GetGenreId() const { return m_genreId; }
  const std::string& GetGenreExtended() const { return m_genreExtended; }

protected:
  // implementations of IDidlLiteElement
  virtual bool serialize(TiXmlElement* element) const;
  virtual bool deserialize(const TiXmlElement* element);

  void initializeProperties();

private:
  std::string m_genreName;
  std::string m_genreId;
  std::string m_genreExtended;
};
