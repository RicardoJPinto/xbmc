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

#include "UPnPVideoItem.h"
#include "FileItem.h"
#include "network/upnp/openHome/didllite/DidlLiteUtils.h"
#include "network/upnp/openHome/ohNet/ohUPnPDefinitions.h"
#include "settings/AdvancedSettings.h"
#include "utils/StringUtils.h"
#include "video/VideoInfoTag.h"

CUPnPVideoItem::CUPnPVideoItem()
  : CUPnPVideoItem("object.item.videoItem")
{ }

CUPnPVideoItem::CUPnPVideoItem(const std::string& classType, const std::string& className /* = "" */)
  : CUPnPItem(classType, className),
    m_playbackCount(0)
{
  initializeProperties();
}

CUPnPVideoItem::CUPnPVideoItem(const CUPnPVideoItem& videoItem)
  : CUPnPItem(videoItem),
    m_longDescription(videoItem.m_longDescription),
    m_producer(videoItem.m_producer),
    m_director(videoItem.m_director),
    m_description(videoItem.m_description),
    m_publisher(videoItem.m_publisher),
    m_language(videoItem.m_language),
    m_relation(videoItem.m_relation),
    m_playbackCount(videoItem.m_playbackCount),
    m_lastPlaybackTime(videoItem.m_lastPlaybackTime),
    m_lastPlaybackPosition(videoItem.m_lastPlaybackPosition),
    m_recordedDayOfWeek(videoItem.m_recordedDayOfWeek),
    m_srsRecordScheduleID(videoItem.m_srsRecordScheduleID)
{
  copyElementProperty(m_genre, videoItem.m_genre);
  copyElementProperty(m_actor, videoItem.m_actor);
  copyElementProperty(m_rating, videoItem.m_rating);

  initializeProperties();
  copyPropertyValidity(&videoItem);
}

CUPnPVideoItem::~CUPnPVideoItem()
{
  for (const auto& genre : m_genre)
    delete genre;

  for (const auto& rating : m_rating)
    delete rating;

  for (const auto& actor : m_actor)
    delete actor;
}

bool CUPnPVideoItem::ToFileItem(CFileItem& item) const
{
  if (!CUPnPItem::ToFileItem(item))
    return false;

  CVideoInfoTag& videoInfo = *item.GetVideoInfoTag();
  for (const auto& genre : m_genre)
    videoInfo.m_genre.push_back(genre->GetGenre());
  videoInfo.m_strPlot = m_longDescription;
  // TODO: m_producer
  if (!m_rating.empty())
  {
    std::vector<std::string> ratings;
    for (const auto& rating : m_rating)
      ratings.push_back(rating->GetRating());

    videoInfo.m_strMPAARating = StringUtils::Join(ratings, g_advancedSettings.m_videoItemSeparator);
  }
  for (const auto& actor : m_actor)
  {
    SActorInfo actorInfo;
    actorInfo.strName = actor->GetPerson();
    actorInfo.strRole = actor->GetPersonRole();
    videoInfo.m_cast.push_back(actorInfo);
  }
  videoInfo.m_director = m_director;
  videoInfo.m_strTagLine = m_description;
  videoInfo.m_studio = m_publisher;
  // TODO: m_language
  // TODO: m_relation
  if (m_playbackCount >= 0)
    videoInfo.m_playCount = m_playbackCount;
  videoInfo.m_lastPlayed = DidlLiteUtils::GetDateTime(m_lastPlaybackTime);
  videoInfo.m_resumePoint.timeInSeconds = static_cast<double>(DidlLiteUtils::GetDurationInSeconds(m_lastPlaybackPosition));
  // TODO: m_recordedDayOfWeek
  // TODO: m_srsRecordScheduleID

  // copy some properties from CFileItem
  if (!item.m_strTitle.empty())
    videoInfo.m_strTitle = item.m_strTitle;
  else if (!item.GetLabel().empty())
    videoInfo.m_strTitle = item.GetLabel();

  return true;
}

bool CUPnPVideoItem::FromFileItem(const CFileItem& item)
{
  if (!CUPnPItem::FromFileItem(item))
    return false;

  // nothing to do if the item doesn't have video info
  if (!item.HasVideoInfoTag())
    return true;

  const CVideoInfoTag& videoInfo = *item.GetVideoInfoTag();
  for (const auto& genre : videoInfo.m_genre)
    m_genre.push_back(new CUPnPGenre(genre));
  if (!m_genre.empty())
    setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "genre");
  m_longDescription = videoInfo.m_strPlot;
  if (!m_longDescription.empty())
    setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "longDescription");
  // TODO: m_producer
  if (!videoInfo.m_strMPAARating.empty())
  {
    m_rating.push_back(new CUPnPRating(videoInfo.m_strMPAARating));
    setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "rating");
  }
  for (const auto& actor : videoInfo.m_cast)
    m_actor.push_back(new CUPnPActor(actor.strName, actor.strRole));
  m_director = videoInfo.m_director;
  if (!m_director.empty())
    setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "director");
  m_description = videoInfo.m_strTagLine;
  if (!m_description.empty())
    setPropertyValid(UPNP_DIDL_DC_NAMESPACE, "description");
  m_publisher = videoInfo.m_studio;
  if (!m_publisher.empty())
    setPropertyValid(UPNP_DIDL_DC_NAMESPACE, "publisher");
  // TODO: m_language
  // TODO: m_relation
  m_playbackCount = videoInfo.m_playCount;
  if (m_playbackCount >= 0)
    setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "playbackCount");
  m_lastPlaybackTime = DidlLiteUtils::GetDateTime(videoInfo.m_lastPlayed);
  if (!m_lastPlaybackTime.empty())
    setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "lastPlaybackTime");
  m_lastPlaybackPosition = DidlLiteUtils::GetDurationFromSeconds(static_cast<int64_t>(videoInfo.m_resumePoint.timeInSeconds));
  if (videoInfo.m_resumePoint.timeInSeconds > 0.0)
    setPropertyValid(UPNP_DIDL_UPNP_NAMESPACE, "lastPlaybackPosition");
  // TODO: m_recordedDayOfWeek
  // TODO: m_srsRecordScheduleID

  return true;
}

void CUPnPVideoItem::initializeProperties()
{
  // define all attributes
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "genre", &m_genre).SetOptional().SupportMultipleValues().SetGenerator(new CUPnPGenre());
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "longDescription", &m_longDescription).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "producer", &m_producer).SetOptional().SupportMultipleValues();
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "rating", &m_rating).SetOptional().SupportMultipleValues().SetGenerator(new CUPnPRating());
  addElementProperty(UPNP_DIDL_UPNP_NAMESPACE, "actor", &m_actor).SetOptional().SupportMultipleValues().SetGenerator(new CUPnPActor());
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "director", &m_director).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "description", &m_description).SetOptional();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "publisher", &m_publisher).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "language", &m_language).SetOptional().SupportMultipleValues();
  addStringProperty(UPNP_DIDL_DC_NAMESPACE, "relation", &m_relation).SetOptional().SupportMultipleValues();
  addIntegerProperty(UPNP_DIDL_UPNP_NAMESPACE, "playbackCount", &m_playbackCount).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "lastPlaybackTime", &m_lastPlaybackTime).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "lastPlaybackPosition", &m_lastPlaybackPosition).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "recordedDayOfWeek", &m_recordedDayOfWeek).SetOptional();
  addStringProperty(UPNP_DIDL_UPNP_NAMESPACE, "srsRecordScheduleID", &m_srsRecordScheduleID).SetOptional();
}
