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

#include "UPnPPlaylistContainer.h"
#include "FileItem.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"

CUPnPPlaylistContainer::CUPnPPlaylistContainer()
  : CUPnPPlaylistContainer("object.container.playlistContainer")
{ }

CUPnPPlaylistContainer::CUPnPPlaylistContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPContainer(classType, className)
{
  initializeProperties();
}

CUPnPPlaylistContainer::CUPnPPlaylistContainer(const CUPnPPlaylistContainer& playlistContainer)
  : CUPnPContainer(playlistContainer),
    m_longDescription(playlistContainer.m_longDescription),
    m_producer(playlistContainer.m_producer),
    m_storageMedium(playlistContainer.m_storageMedium),
    m_description(playlistContainer.m_description),
    m_contributor(playlistContainer.m_contributor),
    m_date(playlistContainer.m_date),
    m_language(playlistContainer.m_language),
    m_rights(playlistContainer.m_rights)
{
  copyElementProperty(m_artist, playlistContainer.m_artist);
  copyElementProperty(m_genre, playlistContainer.m_genre);

  initializeProperties();
  copyPropertyValidity(&playlistContainer);
}

CUPnPPlaylistContainer::~CUPnPPlaylistContainer()
{
  for (const auto& artist : m_artist)
    delete artist;

  for (const auto& genre : m_genre)
    delete genre;
}

bool CUPnPPlaylistContainer::ToFileItem(CFileItem& item) const
{
  if (!CUPnPContainer::ToFileItem(item))
    return false;

  // TODO

  return true;
}

bool CUPnPPlaylistContainer::FromFileItem(const CFileItem& item)
{
  if (!CUPnPContainer::FromFileItem(item))
    return false;

  // TODO

  return true;
}

void CUPnPPlaylistContainer::initializeProperties()
{
  // define all attributes
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "artist", &m_artist).SetOptional().SupportMultipleValues().SetGenerator(new CUPnPArtist());
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "genre", &m_genre).SetOptional().SupportMultipleValues().SetGenerator(new CUPnPGenre());
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "longDescription", &m_longDescription).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "producer", &m_producer).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageMedium", &m_storageMedium).SetOptional();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "description", &m_description).SetOptional();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "contributor", &m_contributor).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "date", &m_date).SetOptional();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "language", &m_language).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "rights", &m_rights).SetOptional().SupportMultipleValues();
}
