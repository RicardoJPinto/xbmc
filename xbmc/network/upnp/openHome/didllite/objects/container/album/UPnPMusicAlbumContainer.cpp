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

#include "UPnPMusicAlbumContainer.h"
#include "FileItem.h"
#include "music/tags/MusicInfoTag.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"

CUPnPMusicAlbumContainer::CUPnPMusicAlbumContainer()
  : CUPnPMusicAlbumContainer("object.container.album.musicAlbum")
{ }

CUPnPMusicAlbumContainer::CUPnPMusicAlbumContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPAlbumContainer(classType, className)
{
  initializeProperties();
}

CUPnPMusicAlbumContainer::CUPnPMusicAlbumContainer(const CUPnPMusicAlbumContainer& musicAlbumContainer)
  : CUPnPAlbumContainer(musicAlbumContainer),
    m_producer(musicAlbumContainer.m_producer),
    m_albumArtURI(musicAlbumContainer.m_albumArtURI),
    m_toc(musicAlbumContainer.m_toc)
{
  copyElementProperty(m_artist, musicAlbumContainer.m_artist);
  copyElementProperty(m_genre, musicAlbumContainer.m_genre);

  initializeProperties();
  copyPropertyValidity(&musicAlbumContainer);
}

CUPnPMusicAlbumContainer::~CUPnPMusicAlbumContainer()
{
  for (const auto& artist : m_artist)
    delete artist;

  for (const auto& genre : m_genre)
    delete genre;
}

bool CUPnPMusicAlbumContainer::ToFileItem(CFileItem& item) const
{
  if (!CUPnPAlbumContainer::ToFileItem(item))
    return false;

  MUSIC_INFO::CMusicInfoTag& musicInfo = *item.GetMusicInfoTag();
  // TODO: differentiate between album artist and normal artist
  for (const auto& artist : m_artist)
    musicInfo.AppendArtist(artist->GetPerson());
  for (const auto& genre : m_genre)
    musicInfo.AppendGenre(genre->GetGenre());
  if (!m_albumArtURI.empty())
    item.SetArt("thumb", m_albumArtURI.at(0));

  return true;
}

bool CUPnPMusicAlbumContainer::FromFileItem(const CFileItem& item)
{
  if (!CUPnPAlbumContainer::FromFileItem(item))
    return false;

  // nothing to do if the item doesn't have video info
  if (!item.HasMusicInfoTag())
    return true;

  const MUSIC_INFO::CMusicInfoTag& musicInfo = *item.GetMusicInfoTag();
  // TODO

  return true;
}

void CUPnPMusicAlbumContainer::initializeProperties()
{
  // define all attributes
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "artist", &m_artist).SetOptional().SupportMultipleValues().SetGenerator(new CUPnPArtist());
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "genre", &m_genre).SetOptional().SupportMultipleValues().SetGenerator(new CUPnPGenre());
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "producer", &m_producer).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "albumArtURI", &m_albumArtURI).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "toc", &m_toc).SetOptional();
  // TODO
}
