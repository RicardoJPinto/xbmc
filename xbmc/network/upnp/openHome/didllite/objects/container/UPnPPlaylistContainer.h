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

#include "network/upnp/openHome/didllite/objects/UPnPGenre.h"
#include "network/upnp/openHome/didllite/objects/UPnPPerson.h"
#include "network/upnp/openHome/didllite/objects/container/UPnPContainer.h"

class CUPnPPlaylistContainer : public CUPnPContainer
{
public:
  CUPnPPlaylistContainer();
  CUPnPPlaylistContainer(const std::string& classType, const std::string& className = "");
  CUPnPPlaylistContainer(const CUPnPPlaylistContainer& playlistContainer);
  virtual ~CUPnPPlaylistContainer();

  // implementations of IDidlLiteElement
  virtual IDidlLiteElement* Create() const { return Clone(); }
  virtual IDidlLiteElement* Clone() const { return new CUPnPPlaylistContainer(*this); }

  // specializations of IFileItemElement
  virtual bool ToFileItem(CFileItem& item) const;
  virtual bool FromFileItem(const CFileItem& item);

protected:
  void initializeProperties();

private:
  std::vector<CUPnPArtist*> m_artist;
  std::vector<CUPnPGenre*> m_genre;
  std::string m_longDescription;
  std::vector<std::string> m_producer;
  std::string m_storageMedium;
  std::string m_description;
  std::vector<std::string> m_contributor;
  std::string m_date;
  std::vector<std::string> m_language;
  std::vector<std::string> m_rights;
};
