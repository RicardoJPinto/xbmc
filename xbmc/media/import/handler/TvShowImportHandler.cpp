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
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "media/import/IMediaImportTask.h"
#include "utils/StringUtils.h"
#include "video/VideoDatabase.h"
#include "video/VideoThumbLoader.h"

/*!
 * Checks whether two tvshows are the same by comparing them by title and year
 */
static bool IsSameTVShow(CVideoInfoTag& left, CVideoInfoTag& right)
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

bool CTvShowImportHandler::HandleImportedItems(CVideoDatabase &videodb, const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task)
{
  bool checkCancelled = task != NULL;
  if (checkCancelled && task->ShouldCancel(0, items.Size()))
    return false;
  
  task->SetProgressTitle(StringUtils::Format(g_localizeStrings.Get(37032).c_str(), MediaTypes::GetPluralLocalization(MediaTypeTvShow).c_str(), import.GetSource().GetFriendlyName().c_str()));
  task->SetProgressText("");

  const CMediaImportSettings &importSettings = import.GetSettings();
  CFileItemList storedItems;
  videodb.GetTvShowsByWhere("videodb://tvshows/titles/", GetFilter(import), storedItems, SortDescription(), importSettings.UpdateImportedMediaItems());
  
  int total = storedItems.Size() + items.Size();
  if (checkCancelled && task->ShouldCancel(0, total))
    return false;

  CVideoThumbLoader thumbLoader;
  if (importSettings.UpdateImportedMediaItems())
    thumbLoader.OnLoaderStart();
  
  int progress = 0;
  CFileItemList newItems; newItems.Copy(items);
  for (int i = 0; i < storedItems.Size(); i++)
  {
    if (checkCancelled && task->ShouldCancel(progress, items.Size()))
      return false;

    CFileItemPtr oldItem = storedItems[i];
    bool found = false;
    for (int j = 0; j < newItems.Size() ; j++)
    {
      if (checkCancelled && task->ShouldCancel(progress, items.Size()))
        return false;

      CFileItemPtr newItem = newItems[j];
      task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37033).c_str(), oldItem->GetVideoInfoTag()->m_strTitle.c_str()));

      if (IsSameTVShow(*oldItem->GetVideoInfoTag(), *newItem->GetVideoInfoTag()))
      {
        // get rid of items we already have from the new items list
        newItems.Remove(j);
        total--;
        found = true;

        if (importSettings.UpdateImportedMediaItems())
          thumbLoader.LoadItem(oldItem.get());
        
        // check if we need to update (db writing is expensive)
        // but only if synchronisation is enabled
        if (importSettings.UpdateImportedMediaItems() &&
            !Compare(oldItem.get(), newItem.get(), true, false))
        {
          task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37035).c_str(), newItem->GetVideoInfoTag()->m_strTitle.c_str()));

          PrepareExistingItem(newItem.get(), oldItem.get());

          std::vector< std::pair<std::string, std::string> > tvshowPaths;
          tvshowPaths.push_back(std::make_pair(newItem->GetPath(), newItem->GetVideoInfoTag()->m_basePath));
          std::map<int, std::map<std::string, std::string> > seasonArt;
          videodb.SetDetailsForTvShow(tvshowPaths, *(newItem->GetVideoInfoTag()), newItem->GetArt(), seasonArt, newItem->GetVideoInfoTag()->m_iDbId);
        }
        break;
      }
    }

    /* TODO: delete items that are not in newItems
     * what if the same tvshow exists with some local items
     * or items from a different source???
    if (!found)
    {
      task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37034).c_str(), oldItem->GetVideoInfoTag()->m_strTitle.c_str()));
      videodb.DeleteTvShow(oldItem->GetVideoInfoTag()->m_iDbId);
    }
    */

    task->SetProgress(progress++, total);
  }

  if (importSettings.UpdateImportedMediaItems())
    thumbLoader.OnLoaderFinish();
  
  // add any (remaining) new items
  for (int i = 0; i < newItems.Size(); i++)
  {
    if (checkCancelled && task->ShouldCancel(progress, items.Size()))
      return false;

    CFileItemPtr pItem = newItems[i];
    PrepareItem(import, pItem.get(), videodb);

    task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37036).c_str(), pItem->GetVideoInfoTag()->m_strTitle.c_str()));

    std::vector< std::pair<std::string, std::string> > tvshowPaths;
    tvshowPaths.push_back(std::make_pair(pItem->GetPath(), pItem->GetVideoInfoTag()->m_basePath));
    std::map<int, std::map<std::string, std::string> > seasonArt;
    pItem->GetVideoInfoTag()->m_iDbId = videodb.SetDetailsForTvShow(tvshowPaths, *(pItem->GetVideoInfoTag()), pItem->GetArt(), seasonArt);
    SetImportForItem(pItem.get(), import, videodb);

    task->SetProgress(progress++, total);
  }

  return true;
}
