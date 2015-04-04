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

#include "UPnPMovieVideoItem.h"
#include "FileItem.h"
#include "network/upnp/openHome/didllite/DidlLiteUtils.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"
#include "utils/StringUtils.h"
#include "video/VideoInfoTag.h"

CUPnPMovieVideoItem::CUPnPMovieVideoItem()
  : CUPnPMovieVideoItem("object.item.videoItem.movie")
{ }

CUPnPMovieVideoItem::CUPnPMovieVideoItem(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPVideoItem(classType, className),
    m_storageMedium("UNKNOWN"),
    m_DVDRegionCode(0),
    m_episodeCount(0),
    m_episodeNumber(-1),
    m_episodeSeason(-1)
{
  initializeProperties();
}

CUPnPMovieVideoItem::CUPnPMovieVideoItem(const CUPnPMovieVideoItem& movieVideoItem)
  : CUPnPVideoItem(movieVideoItem),
    m_storageMedium(movieVideoItem.m_storageMedium),
    m_DVDRegionCode(movieVideoItem.m_DVDRegionCode),
    m_channelName(movieVideoItem.m_channelName),
    // TODO: m_scheduledStartTime(movieVideoItem.m_scheduledStartTime),
    // TODO: m_scheduledEndTime(movieVideoItem.m_scheduledEndTime),
    m_scheduledDuration(movieVideoItem.m_scheduledDuration),
    m_programTitle(movieVideoItem.m_programTitle),
    m_seriesTitle(movieVideoItem.m_seriesTitle),
    m_episodeCount(movieVideoItem.m_episodeCount),
    m_episodeNumber(movieVideoItem.m_episodeNumber),
    m_episodeSeason(movieVideoItem.m_episodeSeason)
{
  initializeProperties();
  copyPropertyValidity(&movieVideoItem);
}

CUPnPMovieVideoItem::~CUPnPMovieVideoItem()
{ }

bool CUPnPMovieVideoItem::ToFileItem(CFileItem& item) const
{
  if (!CUPnPVideoItem::ToFileItem(item))
    return false;

  CVideoInfoTag& videoInfo = *item.GetVideoInfoTag();
  // TODO

  return true;
}

bool CUPnPMovieVideoItem::FromFileItem(const CFileItem& item)
{
  if (!CUPnPVideoItem::FromFileItem(item))
    return false;

  // nothing to do if the item doesn't have video info
  if (!item.HasVideoInfoTag())
    return true;

  const CVideoInfoTag& videoInfo = *item.GetVideoInfoTag();
  // TODO

  return true;
}

void CUPnPMovieVideoItem::initializeProperties()
{
  // define all attributes
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "storageMedium", &m_storageMedium).SetOptional();
  addIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "DVDRegionCode", &m_DVDRegionCode).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "channelName", &m_channelName).SetOptional();
  // TODO: upnp:scheduledStartTime;
  // TODO: upnp:scheduledEndTime;
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "scheduledDuration", &m_scheduledDuration).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "programTitle", &m_programTitle).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "seriesTitle", &m_seriesTitle).SetOptional();
  addUnsignedIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "episodeCount", &m_episodeCount).SetOptional();
  addUnsignedIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "episodeNumber", &m_episodeNumber).SetOptional();
  addUnsignedIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "episodeSeason", &m_episodeSeason).SetOptional();
}
