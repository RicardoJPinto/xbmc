/*
 *      Copyright (C) 2013 Team XBMC
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

#include "SeasonImportHandler.h"
#include "FileItem.h"
#include "media/import/IMediaImportTask.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "video/VideoDatabase.h"

typedef std::set<CFileItemPtr> TvShowsSet;
typedef std::map<std::string, TvShowsSet> TvShowsMap;

/*!
 * Checks whether two seasons are the same by comparing them by title and year
 */
static bool IsSameSeason(const CVideoInfoTag& left, const CVideoInfoTag& right)
{
  return left.m_strShowTitle == right.m_strShowTitle
      && left.m_iYear        == right.m_iYear
      && left.m_iSeason      == right.m_iSeason;
}

std::set<MediaType> CSeasonImportHandler::GetDependencies() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeTvShow);
  return types;
}

std::set<MediaType> CSeasonImportHandler::GetRequiredMediaTypes() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeEpisode);
  return types;
}

std::vector<MediaType> CSeasonImportHandler::GetGroupedMediaTypes() const
{
  std::vector<MediaType> types;
  types.push_back(MediaTypeTvShow);
  types.push_back(MediaTypeSeason);
  types.push_back(MediaTypeEpisode);
  return types;
}

std::string CSeasonImportHandler::GetItemLabel(const CFileItem* item) const
{
  if (item != NULL && item->HasVideoInfoTag() && !item->GetVideoInfoTag()->m_strShowTitle.empty())
  {
    return StringUtils::Format(g_localizeStrings.Get(37037).c_str(),
      item->GetVideoInfoTag()->m_strShowTitle.c_str(),
      item->GetVideoInfoTag()->m_strTitle.c_str());
  }

  return CVideoImportHandler::GetItemLabel(item);
}

bool CSeasonImportHandler::StartSynchronisation(const CMediaImport &import)
{
  if (!CVideoImportHandler::StartSynchronisation(import))
    return false;

  // create a map of tvshows imported from the same source
  CFileItemList tvshows;
  if (!m_db.GetTvShowsByWhere("videodb://tvshows/titles/", GetFilter(import), tvshows))
    return false;

  m_tvshows.clear();

  TvShowsMap::iterator tvshowsIter;
  for (int tvshowsIndex = 0; tvshowsIndex < tvshows.Size(); tvshowsIndex++)
  {
    CFileItemPtr tvshow = tvshows.Get(tvshowsIndex);
    if (!tvshow->HasVideoInfoTag() || tvshow->GetVideoInfoTag()->m_strTitle.empty())
      continue;

    tvshowsIter = m_tvshows.find(tvshow->GetVideoInfoTag()->m_strTitle);
    if (tvshowsIter == m_tvshows.end())
    {
      TvShowsSet tvshowsSet; tvshowsSet.insert(tvshow);
      m_tvshows.insert(make_pair(tvshow->GetVideoInfoTag()->m_strTitle, tvshowsSet));
    }
    else
      tvshowsIter->second.insert(tvshow);
  }

  return true;
}

bool CSeasonImportHandler::AddImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == NULL)
    return false;

  PrepareItem(import, item);

  CVideoInfoTag *season = item->GetVideoInfoTag();

  // try to find an existing tvshow that the season belongs to
  season->m_iIdShow = FindTvShowId(item);

  // if the tvshow doesn't exist, create a very basic version of it with the info we got from the season
  if (season->m_iIdShow <= 0)
  {
    CVideoInfoTag tvshow;
    tvshow.m_basePath = season->m_basePath;
    tvshow.m_cast = season->m_cast;
    tvshow.m_country = season->m_country;
    tvshow.m_director = season->m_director;
    tvshow.m_genre = season->m_genre;
    tvshow.m_iYear = season->m_iYear;
    tvshow.m_parentPathID = season->m_parentPathID;
    tvshow.m_premiered = season->m_premiered;
    tvshow.m_strMPAARating = season->m_strMPAARating;
    tvshow.m_strPlot = season->m_strPlot;
    tvshow.m_strTitle = tvshow.m_strShowTitle = season->m_strShowTitle;
    tvshow.m_studio = season->m_studio;
    tvshow.m_type = MediaTypeTvShow;
    tvshow.m_writingCredits = season->m_writingCredits;

    // try to find a proper path by going up in the path hierarchy once
    tvshow.m_strPath = URIUtils::GetParentPath(season->GetPath());

    // add the basic tvshow to the database
    std::vector< std::pair<std::string, std::string> > tvshowPaths; tvshowPaths.push_back(std::make_pair(tvshow.m_strPath, tvshow.m_basePath));
    tvshow.m_iDbId = tvshow.m_iIdShow = m_db.SetDetailsForTvShow(tvshowPaths, tvshow, CGUIListItem::ArtMap(), std::map<int, std::map<std::string, std::string> >());

    // store the tvshow's database ID in the season
    season->m_iIdShow = tvshow.m_iDbId;

    // add the tvshow to the tvshow map
    CFileItemPtr tvshowItem(new CFileItem(tvshow));
    tvshowItem->SetSource(item->GetSource());
    tvshowItem->SetImportPath(item->GetImportPath());

    TvShowsMap::iterator tvshowsIter = m_tvshows.find(tvshow.m_strTitle);
    if (tvshowsIter == m_tvshows.end())
    {
      TvShowsSet tvshowsSet; tvshowsSet.insert(tvshowItem);
      m_tvshows.insert(make_pair(tvshow.m_strTitle, tvshowsSet));
    }
    else
      tvshowsIter->second.insert(tvshowItem);
  }

  season->m_iDbId = m_db.SetDetailsForSeason(*season, item->GetArt(), season->m_iIdShow);
  if (season->m_iDbId <= 0)
    return false;

  return SetImportForItem(item, import);
}

bool CSeasonImportHandler::UpdateImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == NULL || !item->HasVideoInfoTag() || item->GetVideoInfoTag()->m_iDbId <= 0)
    return false;

  return m_db.SetDetailsForSeason(*(item->GetVideoInfoTag()), item->GetArt(), item->GetVideoInfoTag()->m_iIdShow, item->GetVideoInfoTag()->m_iDbId) > 0;
}

bool CSeasonImportHandler::RemoveImportedItem(const CMediaImport &import, const CFileItem* item)
{
  if (item == NULL || !item->HasVideoInfoTag())
    return false;

  /* TODO: what if there are local episodes for the same tvshow?
  m_db.DeleteSeason(item->GetVideoInfoTag()->m_iDbId);
  */
  return true;
}

bool CSeasonImportHandler::GetLocalItems(CVideoDatabase &videodb, const CMediaImport &import, CFileItemList& items)
{
  return videodb.GetSeasonsByWhere("videodb://tvshows/titles/", GetFilter(import), items, true);
}

CFileItemPtr CSeasonImportHandler::FindMatchingLocalItem(const CFileItem* item, CFileItemList& localItems)
{
  for (int i = 0; i < localItems.Size(); ++i)
  {
    CFileItemPtr localItem = localItems.Get(i);
    if (IsSameSeason(*item->GetVideoInfoTag(), *localItem->GetVideoInfoTag()))
      return localItem;
  }

  return CFileItemPtr();
}

MediaImportChangesetType CSeasonImportHandler::DetermineChangeset(const CMediaImport &import, CFileItem* item, CFileItemPtr localItem, CFileItemList& localItems, bool updatePlaybackMetadata)
{
  return CVideoImportHandler::DetermineChangeset(import, item, localItem, localItems, false);
}

int CSeasonImportHandler::FindTvShowId(const CFileItem* seasonItem)
{
  if (seasonItem == NULL || !seasonItem->HasVideoInfoTag())
    return -1;

  // no comparison possible without a title
  if (seasonItem->GetVideoInfoTag()->m_strShowTitle.empty())
    return -1;

  // check if there is a tvshow with a matching title
  TvShowsMap::const_iterator tvshowsIter = m_tvshows.find(seasonItem->GetVideoInfoTag()->m_strShowTitle);
  if (tvshowsIter == m_tvshows.end() ||
    tvshowsIter->second.size() <= 0)
    return -1;

  // if there is only one matching tvshow, we can go with that one
  if (tvshowsIter->second.size() == 1)
    return tvshowsIter->second.begin()->get()->GetVideoInfoTag()->m_iDbId;

  // use the path of the episode and tvshow to find the right tvshow
  for (TvShowsSet::const_iterator it = tvshowsIter->second.begin(); it != tvshowsIter->second.end(); ++it)
  {
    if (URIUtils::IsInPath(seasonItem->GetVideoInfoTag()->GetPath(), (*it)->GetVideoInfoTag()->GetPath()))
      return (*it)->GetVideoInfoTag()->m_iDbId;
  }

  return -1;
}
