/*
 *      Copyright (C) 2014 Team XBMC
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

#include <algorithm>

#include "ohUPnPContentDirectoryControlPoint.h"
#include "FileItem.h"
#include "media/MediaType.h"
#include "network/upnp/openHome/didllite/DidlLiteDocument.h"
#include "network/upnp/openHome/didllite/IDidlLiteElement.h"
#include "network/upnp/openHome/didllite/objects/FileItemUtils.h"
#include "network/upnp/openHome/didllite/objects/IFileItemElement.h"
#include "network/upnp/openHome/didllite/objects/container/UPnPContainer.h"
#include "network/upnp/openHome/didllite/objects/container/UPnPPlaylistContainer.h"
#include "network/upnp/openHome/didllite/objects/container/album/UPnPAlbumContainer.h"
#include "network/upnp/openHome/didllite/objects/container/album/UPnPMusicAlbumContainer.h"
#include "network/upnp/openHome/didllite/objects/container/album/UPnPPhotoAlbumContainer.h"
#include "network/upnp/openHome/didllite/objects/container/genre/UPnPGenreContainer.h"
#include "network/upnp/openHome/didllite/objects/container/genre/UPnPMovieGenreContainer.h"
#include "network/upnp/openHome/didllite/objects/container/genre/UPnPMusicGenreContainer.h"
#include "network/upnp/openHome/didllite/objects/container/person/UPnPMusicArtistPersonContainer.h"
#include "network/upnp/openHome/didllite/objects/container/person/UPnPPersonContainer.h"
#include "network/upnp/openHome/didllite/objects/container/storage/UPnPStorageFolderContainer.h"
#include "network/upnp/openHome/didllite/objects/container/storage/UPnPStorageSystemContainer.h"
#include "network/upnp/openHome/didllite/objects/container/storage/UPnPStorageVolumeContainer.h"
#include "network/upnp/openHome/didllite/objects/item/UPnPItem.h"
#include "network/upnp/openHome/didllite/objects/item/image/UPnPImageItem.h"
#include "network/upnp/openHome/didllite/objects/item/image/UPnPPhotoImageItem.h"
#include "network/upnp/openHome/didllite/objects/item/video/UPnPMovieVideoItem.h"
#include "network/upnp/openHome/didllite/objects/item/video/UPnPVideoItem.h"
#include "network/upnp/openHome/didllite/objects/classmappers/SimpleUPnPClassMapper.h"
#include "utils/Variant.h"

using namespace OpenHome;
using namespace OpenHome::Net;

COhUPnPContentDirectoryControlPoint::COhUPnPContentDirectoryControlPoint()
  : IOhUPnPControlPoint<CpProxyUpnpOrgContentDirectory1Cpp>(UPNP_DOMAIN_NAME, UPNP_SERVICE_TYPE_CONTENTDIRECTORY, 1)
{
  // setup DIDL-Lite element factory
  m_elementFactory.RegisterElement(new CUPnPContainer());
  m_elementFactory.RegisterElement(new CUPnPAlbumContainer());
  m_elementFactory.RegisterElement(new CUPnPMusicAlbumContainer());
  m_elementFactory.RegisterElement(new CUPnPPhotoAlbumContainer());
  m_elementFactory.RegisterElement(new CUPnPPersonContainer());
  m_elementFactory.RegisterElement(new CUPnPMusicArtistPersonContainer());
  m_elementFactory.RegisterElement(new CUPnPGenreContainer());
  m_elementFactory.RegisterElement(new CUPnPMovieGenreContainer());
  m_elementFactory.RegisterElement(new CUPnPMusicGenreContainer());
  m_elementFactory.RegisterElement(new CUPnPPlaylistContainer());
  m_elementFactory.RegisterElement(new CUPnPStorageFolderContainer());
  m_elementFactory.RegisterElement(new CUPnPStorageSystemContainer());
  m_elementFactory.RegisterElement(new CUPnPStorageVolumeContainer());
  m_elementFactory.RegisterElement(new CUPnPItem());
  m_elementFactory.RegisterElement(new CUPnPImageItem());
  m_elementFactory.RegisterElement(new CUPnPPhotoImageItem());
  m_elementFactory.RegisterElement(new CUPnPVideoItem());
  m_elementFactory.RegisterElement(new CUPnPMovieVideoItem());

  // setup upnp:class mapping
  m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper("object.item.videoItem", MediaTypeMovie));
  m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper("object.item.videoitem.movie", MediaTypeMovie)); 
  m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper("object.item.videoItem.videoBroadcast", MediaTypeEpisode));
  m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper("object.item.videoItem.musicVideoClip", MediaTypeMusicVideo));
  m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper("object.item.audioItem", MediaTypeSong));
  m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper("object.item.audioItem.musicTrack", MediaTypeSong));
  m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper("object.item.imageItem", "photos"));
  m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper("object.item.imageItem.photo", "photos"));
  m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper("object.container.album", MediaTypeAlbum));
  m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper("object.container.album.videoAlbum", MediaTypeTvShow));
  m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper("object.container.album.musicAlbum", MediaTypeAlbum));
  m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper("object.container.album.photoAlbum", "photos"));
  m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper("object.container.person", MediaTypeArtist)); // TODO
  m_classMapping.RegisterMapper(new CSimpleUPnPClassMapper("object.container.person.musicArtist", MediaTypeArtist));
}

COhUPnPContentDirectoryControlPoint::~COhUPnPContentDirectoryControlPoint()
{ }

bool COhUPnPContentDirectoryControlPoint::BrowseSync(const std::string& uuid, const std::string& objectId, CFileItemList& items,
                                                     uint32_t start /* = 0 */, uint32_t count /* = 0 */,
                                                     const std::string& filter /* = "" */, const std::string& sorting /* = "" */) const
{
  std::string result;
  uint32_t resultCount, resultTotal;
  uint32_t total = count;
  uint32_t currentStart = start;
  do {
    if (!browseSync(uuid, objectId, false, result, resultCount, resultTotal, currentStart, count, filter, sorting))
      return false;

    if (count <= 0 && resultTotal > 0)
      count = resultTotal;
    currentStart += resultCount;

    CFileItemList tmpItems;
    if (!resultToFileItemList(result, resultCount, resultTotal, tmpItems))
      return false;

    items.Append(tmpItems);
    items.SetContent(tmpItems.GetContent());
  } while (static_cast<uint32_t>(items.Size()) < count);

  items.SetProperty("total", count);
  return true;
}

bool COhUPnPContentDirectoryControlPoint::BrowseMetadataSync(const std::string& uuid, const std::string& objectId, CFileItem& item, const std::string& filter /* = "" */) const
{
  std::string result;
  uint32_t resultCount, resultTotal;
  if (!browseSync(uuid, objectId, true, result, resultCount, resultTotal, 0, 1, filter, "") ||
      resultCount != 1 || resultTotal != resultCount)
    return false;

  CFileItemList items;
  if (!resultToFileItemList(result, resultCount, resultTotal, items))
    return false;

  item = *items.Get(0);
  return true;
}

void COhUPnPContentDirectoryControlPoint::onServiceAdded(const UPnPControlPointService& service)
{
  // TODO
}

void COhUPnPContentDirectoryControlPoint::onServiceRemoved(const UPnPControlPointService& service)
{
  // TODO
}

bool COhUPnPContentDirectoryControlPoint::browseSync(const std::string& uuid, const std::string& objectId, bool metadata,
                                               std::string& result, uint32_t& resultCount, uint32_t& resultTotal,
                                               uint32_t start, uint32_t count, const std::string& filter, const std::string& sorting) const
{
  if (uuid.empty())
    return false;

  const auto& service = m_services.find(uuid);
  if (service == m_services.end())
    return false;

  uint32_t updateId;
  service->second.service->SyncBrowse(objectId, metadata ? "BrowseMetadata" : "BrowseDirectChildren",
                                      filter, start, count, sorting, result, resultCount, resultTotal, updateId);
  return true;
}

bool COhUPnPContentDirectoryControlPoint::resultToFileItemList(const std::string& result, uint32_t resultCount, uint32_t resultTotal, CFileItemList& items) const
{
  if (result.empty())
    return false;

  items.ClearItems();
  if (resultCount == 0)
    return true;

  // parse and deserialize the result as a DIDL-Lite document
  CDidlLiteDocument doc(m_elementFactory);
  if (!doc.Deserialize(result))
    return false;

  // turn the deserialized DIDL-Lite document into a list of items
  if (!FileItemUtils::DocumentToFileItemList(doc, items, &m_classMapping) || static_cast<uint32_t>(items.Size()) != resultCount)
    return false;

  items.SetProperty("total", resultTotal);
  return true;
}
