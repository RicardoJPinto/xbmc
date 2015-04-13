/*
 * UPnP Support for XBMC
 *      Copyright (c) 2006 c0diq (Sylvain Rebaud)
 *      Portions Copyright (c) by the authors of libPlatinum
 *      http://www.plutinosoft.com/blog/category/platinum/
 *
 *      Copyright (C) 2010-2013 Team XBMC
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
#include <Platinum/Source/Platinum/Platinum.h>
#include <Platinum/Source/Devices/MediaServer/PltSyncMediaBrowser.h>

#include "UPnPDirectory.h"
#include "URL.h"
#include "network/upnp/UPnP.h"
#include "network/upnp/UPnPInternal.h"
#include "network/upnp/openHome/ohUPnP.h"
#include "network/upnp/openHome/controlpoints/ohUPnPContentDirectoryControlPoint.h"
#include "video/VideoInfoTag.h"
#include "FileItem.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "utils/StringUtils.h"

using namespace MUSIC_INFO;
using namespace XFILE;
using namespace UPNP;

namespace XFILE
{

static std::string GetContentMapping(const std::string &objectClass)
{
  struct SClassMapping
  {
    const char* ObjectClass;
    const char* Content;
  };

  static const SClassMapping mapping[] = {
      { "object.item.videoItem.videoBroadcast", "episodes"     },
      { "object.item.videoItem.musicVideoClip", "musicvideos"  },
      { "object.item.videoItem"               , "movies"       },
      { "object.item.audioItem.musicTrack"    , "songs"        },
      { "object.item.audioItem"               , "songs"        },
      { "object.item.imageItem.photo"         , "photos"       },
      { "object.item.imageItem"               , "photos"       },
      { "object.container.album.videoAlbum"   , "tvshows"      },
      { "object.container.album.musicAlbum"   , "albums"       },
      { "object.container.album.photoAlbum"   , "photos"       },
      { "object.container.album"              , "albums"       },
      { "object.container.person"             , "artists"      },
      { NULL                                  , NULL           }
  };

  for (const SClassMapping* map = mapping; map->ObjectClass; map++)
  {
    if (StringUtils::StartsWithNoCase(objectClass, map->ObjectClass))
      return map->Content;
  }

  return "unknown";
}

static bool FindDeviceWait(const std::string &uuid, COhUPnPControlPointDevice& device, int timeoutSeconds = 5)
{
  if (timeoutSeconds <= 0)
    timeoutSeconds = 5;

  bool clientStarted = COhUPnP::Get().IsContentDirectoryClientRunning();
  if (!clientStarted && !COhUPnP::Get().StartContentDirectoryClient())
    return false;

  const COhUPnPContentDirectoryControlPoint& client = COhUPnP::Get().GetContentDirectoryClient();

  CDateTime endTime = CDateTime::GetCurrentDateTime() + CDateTimeSpan(0, 0, 0, timeoutSeconds);
  while (endTime > CDateTime::GetCurrentDateTime())
  {
    if (client.GetDevice(uuid, device))
      break;

    // fail right away if device not found and upnp client was already running
    if (clientStarted)
      return false;

    // sleep for 500 millisecond
    Sleep(500);
  }

  return device.IsValid();
}

/*----------------------------------------------------------------------
|   CUPnPDirectory::GetFriendlyName
+---------------------------------------------------------------------*/
const char*
CUPnPDirectory::GetFriendlyName(const CURL& url)
{
  std::string strUrl = url.Get();
  if (!URIUtils::IsUPnP(strUrl))
    return NULL;

  if (StringUtils::EqualsNoCase(strUrl, "upnp://"))
    return "UPnP Media Servers (Auto-Discover)";
    
  std::string path = strUrl;
  URIUtils::AddSlashAtEnd(path);
  CURL upnpUrl(path);
  if (upnpUrl.GetHostName().empty())
    return NULL;

  COhUPnPControlPointDevice device;
  if (!FindDeviceWait(upnpUrl.GetHostName(), device) ||
      !device.IsValid())
    return "";

  return device.GetFriendlyName().c_str();
}

/*----------------------------------------------------------------------
|   CUPnPDirectory::GetResource
+---------------------------------------------------------------------*/
bool CUPnPDirectory::GetResource(const CURL& path, CFileItem &item)
{
    if(!path.IsProtocol("upnp"))
      return false;

    std::string uuid = path.GetHostName();
    std::string objectId = path.GetFileName();
    URIUtils::RemoveSlashAtEnd(objectId);
    objectId = CURL::Decode(objectId);

    COhUPnPControlPointDevice device;
    if (!FindDeviceWait(uuid, device) ||
        !device.IsValid())
    {
      CLog::Log(LOGERROR, "CUPnPDirectory::GetResource - unable to find uuid %s", uuid.c_str());
      return false;
    }

    // TODO

    return false;

  /* TODO
    PLT_MediaObjectListReference list;
    if (NPT_FAILED(upnp->m_MediaBrowser->BrowseSync(device, object.c_str(), list, true))) {
        CLog::Log(LOGERROR, "CUPnPDirectory::GetResource - unable to find object %s", object.c_str());
        return false;
    }

    if (list.IsNull() || !list->GetItemCount()) {
      CLog::Log(LOGERROR, "CUPnPDirectory::GetResource - no items returned for object %s", object.c_str());
      return false;
    }

    PLT_MediaObjectList::Iterator entry = list->GetFirstItem();
    if (entry == 0)
        return false;

  return UPNP::GetResource(*entry, item);
  */
}


/*----------------------------------------------------------------------
|   CUPnPDirectory::GetDirectory
+---------------------------------------------------------------------*/
bool
CUPnPDirectory::GetDirectory(const CURL& url, CFileItemList &items)
{
  if (!COhUPnP::Get().IsContentDirectoryClientRunning())
    return false;

  const COhUPnPContentDirectoryControlPoint &client = COhUPnP::Get().GetContentDirectoryClient();

  // list all available ContentDirectory services
  if (url.GetHostName().empty())
  {
    // get the list of devices
    std::vector<COhUPnPControlPointDevice> services = client.GetDevices();
    for (const auto& service : services)
    {
      CFileItemPtr item(new CFileItem("upnp://" + service.GetUuid() + "/", true));
      item->SetLabel(service.GetFriendlyName());
      item->SetArt("thumb", service.GetIconUrl());

      items.Add(item);
    }

    return true;
  }

  std::string uuid = url.GetHostName();
  std::string objectId = url.GetFileName();
  URIUtils::RemoveSlashAtEnd(objectId);

  if (!objectId.empty())
    objectId = CURL::Decode(objectId);

  COhUPnPControlPointDevice device;
  if (!FindDeviceWait(uuid, device) ||
      !device.IsValid())
    return false;

  // if the object identifier is empty, use "0" for the root listing
  if (objectId.empty())
    objectId = "0";

  if (!client.BrowseSync(uuid, objectId, items))
    return false;

  for (int i = 0; i < items.Size(); ++i)
  {
    CFileItemPtr item = items[i];

    // put together the path of the item
    std::string id = CURL::Encode(item->GetPath());
    URIUtils::AddSlashAtEnd(id);
    item->SetPath("upnp://" + uuid + "/" + id);
  }

  std::string content = items.GetContent();
  if (!content.empty())
  {
    content = GetContentMapping(content);
    if (content == "unknown")
    {
      content.clear();

      // TODO: what is this doing here?
      items.AddSortMethod(SortByNone, 571, LABEL_MASKS("%L", "%I", "%L", ""));
      items.AddSortMethod(SortByLabel, SortAttributeIgnoreFolders, 551, LABEL_MASKS("%L", "%I", "%L", ""));
      items.AddSortMethod(SortBySize, 553, LABEL_MASKS("%L", "%I", "%L", "%I"));
      items.AddSortMethod(SortByDate, 552, LABEL_MASKS("%L", "%J", "%L", "%J"));
    }

    items.SetContent(content);
  }

  /* TODO
      // just a guess as to what types of files we want
      bool video = true;
      bool audio = true;
      bool image = true;
      StringUtils::TrimLeft(m_strFileMask, "/");
      if (!m_strFileMask.empty()) {
          video = m_strFileMask.find(".wmv") != std::string::npos;
          audio = m_strFileMask.find(".wma") != std::string::npos;
          image = m_strFileMask.find(".jpg") != std::string::npos;
      }

      // special case for Windows Media Connect and WMP11 when looking for root
      // We can target which root subfolder we want based on directory mask
      if (object_id == "0" && ((device->GetFriendlyName().Find("Windows Media Connect", 0, true) >= 0) ||
                                (device->m_ModelName == "Windows Media Player Sharing"))) {

          // look for a specific type to differentiate which folder we want
          if (audio && !video && !image) {
              // music
              object_id = "1";
          } else if (!audio && video && !image) {
              // video
              object_id = "2";
          } else if (!audio && !video && image) {
              // pictures
              object_id = "3";
          }
      }

#ifdef DISABLE_SPECIALCASE
      // same thing but special case for XBMC
      if (object_id == "0" && ((device->m_ModelName.Find("XBMC", 0, true) >= 0) ||
                                (device->m_ModelName.Find("Xbox Media Center", 0, true) >= 0))) {
          // look for a specific type to differentiate which folder we want
          if (audio && !video && !image) {
              // music
              object_id = "virtualpath://upnpmusic";
          } else if (!audio && video && !image) {
              // video
              object_id = "virtualpath://upnpvideo";
          } else if (!audio && !video && image) {
              // pictures
              object_id = "virtualpath://upnppictures";
          }
      }
#endif

      // if error, return now, the device could have gone away
      // this will make us go back to the sources list
      PLT_MediaObjectListReference list;
      NPT_Result res = upnp->m_MediaBrowser->BrowseSync(device, object_id, list);
      if (NPT_FAILED(res)) goto failure;

      // empty list is ok
      if (list.IsNull()) goto cleanup;

      PLT_MediaObjectList::Iterator entry = list->GetFirstItem();
      while (entry) {
          // disregard items with wrong class/type
          if( (!video && (*entry)->m_ObjectClass.type.CompareN("object.item.videoitem", 21,true) == 0)
            || (!audio && (*entry)->m_ObjectClass.type.CompareN("object.item.audioitem", 21,true) == 0)
            || (!image && (*entry)->m_ObjectClass.type.CompareN("object.item.imageitem", 21,true) == 0) )
          {
              ++entry;
              continue;
          }

            // never show empty containers in media views
            if((*entry)->IsContainer()) {
                if( (audio || video || image)
                 && ((PLT_MediaContainer*)(*entry))->m_ChildrenCount == 0) {
                    ++entry;
                    continue;
                }
            }

            CFileItemPtr pItem = BuildObject(*entry);
            if(!pItem) {
                ++entry;
                continue;
            }

            ++entry;
        }
    }

cleanup:
  */
  return true;

  /* TODO
failure:
  */
  return false;
}
}
