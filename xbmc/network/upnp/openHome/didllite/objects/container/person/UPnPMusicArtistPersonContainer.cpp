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

#include "UPnPMusicArtistPersonContainer.h"
#include "FileItem.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"

CUPnPMusicArtistPersonContainer::CUPnPMusicArtistPersonContainer()
  : CUPnPMusicArtistPersonContainer("object.container.person.musicArtist")
{ }

CUPnPMusicArtistPersonContainer::CUPnPMusicArtistPersonContainer(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPPersonContainer(classType, className)
{
  initializeProperties();
}

CUPnPMusicArtistPersonContainer::CUPnPMusicArtistPersonContainer(const CUPnPMusicArtistPersonContainer& musicArtistContainer)
  : CUPnPPersonContainer(musicArtistContainer),
    m_artistDiscographyURI(musicArtistContainer.m_artistDiscographyURI)
{
  copyElementProperty(m_genre, musicArtistContainer.m_genre);

  initializeProperties();
  copyPropertyValidity(&musicArtistContainer);
}

CUPnPMusicArtistPersonContainer::~CUPnPMusicArtistPersonContainer()
{
  for (const auto& genre : m_genre)
    delete genre;
}

bool CUPnPMusicArtistPersonContainer::ToFileItem(CFileItem& item) const
{
  if (!CUPnPPersonContainer::ToFileItem(item))
    return false;

  // TODO

  return true;
}

bool CUPnPMusicArtistPersonContainer::FromFileItem(const CFileItem& item)
{
  if (!CUPnPPersonContainer::FromFileItem(item))
    return false;

  // TODO

  return true;
}

void CUPnPMusicArtistPersonContainer::initializeProperties()
{
  // define all attributes
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "genre", &m_genre).SetOptional().SupportMultipleValues().SetGenerator(new CUPnPGenre());
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "artistDiscographyURI", &m_artistDiscographyURI).SetOptional();
}
