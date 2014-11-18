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

#include "TvShowImportHandler.h"
#include "FileItem.h"
#include "video/VideoDatabase.h"

/*!
 * Checks whether two tvshows are the same by comparing them by title and year
 */
static bool IsSameTVShow(const CVideoInfoTag& left, const CVideoInfoTag& right)
{
  return left.m_strShowTitle == right.m_strShowTitle
      && left.m_iYear        == right.m_iYear;
}

std::set<MediaType> CTvShowImportHandler::GetRequiredMediaTypes() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeEpisode);
  return types;
}

std::vector<MediaType> CTvShowImportHandler::GetGroupedMediaTypes() const
{
  std::vector<MediaType> types;
  types.push_back(MediaTypeTvShow);
  types.push_back(MediaTypeSeason);
  types.push_back(MediaTypeEpisode);
  return types;
}

bool CTvShowImportHandler::AddImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == NULL)
    return false;

  PrepareItem(import, item);

  std::vector< std::pair<std::string, std::string> > tvshowPaths;
  tvshowPaths.push_back(std::make_pair(item->GetPath(),item->GetVideoInfoTag()->m_basePath));
  std::map<int, std::map<std::string, std::string> > seasonArt;
  item->GetVideoInfoTag()->m_iDbId = m_db.SetDetailsForTvShow(tvshowPaths, *(item->GetVideoInfoTag()), item->GetArt(), seasonArt);
  if (item->GetVideoInfoTag()->m_iDbId <= 0)
    return false;

  return SetImportForItem(item, import);
}

bool CTvShowImportHandler::UpdateImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == NULL || !item->HasVideoInfoTag() || item->GetVideoInfoTag()->m_iDbId <= 0)
    return false;

  std::vector< std::pair<std::string, std::string> > tvshowPaths;
  tvshowPaths.push_back(std::make_pair(item->GetPath(), item->GetVideoInfoTag()->m_basePath));
  std::map<int, std::map<std::string, std::string> > seasonArt;
  return m_db.SetDetailsForTvShow(tvshowPaths, *(item->GetVideoInfoTag()), item->GetArt(), seasonArt, item->GetVideoInfoTag()->m_iDbId) > 0;
}

bool CTvShowImportHandler::RemoveImportedItem(const CMediaImport &import, const CFileItem* item)
{
  if (item == NULL || !item->HasVideoInfoTag())
    return false;

  /* TODO: what if there are local episodes for the same tvshow?
  m_db.DeleteTvShow(item->GetVideoInfoTag()->m_iDbId);
  */
  return true;
}

bool CTvShowImportHandler::GetLocalItems(CVideoDatabase &videodb, const CMediaImport &import, CFileItemList& items)
{
  return videodb.GetTvShowsByWhere("videodb://tvshows/titles/", GetFilter(import), items, SortDescription(), import.GetSettings().UpdateImportedMediaItems());
}

CFileItemPtr CTvShowImportHandler::FindMatchingLocalItem(const CFileItem* item, CFileItemList& localItems)
{
  for (int i = 0; i < localItems.Size(); ++i)
  {
    CFileItemPtr localItem = localItems.Get(i);
    if (IsSameTVShow(*item->GetVideoInfoTag(), *localItem->GetVideoInfoTag()))
      return localItem;
  }

  return CFileItemPtr();
}

MediaImportChangesetType CTvShowImportHandler::DetermineChangeset(const CMediaImport &import, CFileItem* item, CFileItemPtr localItem, CFileItemList& localItems, bool updatePlaybackMetadata)
{
  return CVideoImportHandler::DetermineChangeset(import, item, localItem, localItems, false);
}
