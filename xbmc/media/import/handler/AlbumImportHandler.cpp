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

#include "AlbumImportHandler.h"
#include "FileItem.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "media/import/IMediaImportTask.h"
#include "music/MusicDatabase.h"
#include "music/MusicThumbLoader.h"
#include "utils/StringUtils.h"

/*!
 * Checks whether two albums are the same by comparing them by title and album artist
 */
static bool IsSameAlbum(MUSIC_INFO::CMusicInfoTag& left, MUSIC_INFO::CMusicInfoTag& right)
{
  return left.GetAlbum()      == right.GetAlbum()
    && left.GetAlbumArtist()  == right.GetAlbumArtist();
}

std::set<MediaType> CAlbumImportHandler::GetDependencies() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeArtist);
  return types;
}

std::set<MediaType> CAlbumImportHandler::GetRequiredMediaTypes() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeSong);
  return types;
}

std::vector<MediaType> CAlbumImportHandler::GetGroupedMediaTypes() const
{
  std::vector<MediaType> types;
  types.push_back(MediaTypeArtist);
  types.push_back(MediaTypeAlbum);
  types.push_back(MediaTypeSong);
  return types;
}

bool CAlbumImportHandler::HandleImportedItems(CMusicDatabase &musicdb, const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task)
{
  bool checkCancelled = task != NULL;
  if (checkCancelled && task->ShouldCancel(0, items.Size()))
    return false;

  task->SetProgressTitle(StringUtils::Format(g_localizeStrings.Get(37026).c_str(), MediaTypes::GetPluralLocalization(MediaTypeAlbum).c_str(), import.GetSource().GetFriendlyName().c_str()));
  task->SetProgressText("");
  
  CFileItemList storedItems;
  musicdb.GetAlbumsByWhere("musicdb://albums/", GetFilter(import), storedItems);
  
  int total = storedItems.Size() + items.Size();
  if (checkCancelled && task->ShouldCancel(0, total))
    return false;

  const CMediaImportSettings &importSettings = import.GetSettings();
  CMusicThumbLoader thumbLoader;
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
      task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37031).c_str(),
                                                newItem->GetMusicInfoTag()->GetAlbumArtist().at(0).c_str(), // TODO
                                                newItem->GetMusicInfoTag()->GetAlbum().c_str()));

      if (IsSameAlbum(*oldItem->GetMusicInfoTag(), *newItem->GetMusicInfoTag()))
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
            !Compare(oldItem.get(), newItem.get()))
        {
          task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37033).c_str(),
                                                    newItem->GetMusicInfoTag()->GetAlbumArtist().at(0).c_str(), // TODO
                                                    newItem->GetMusicInfoTag()->GetAlbum().c_str()));

          PrepareExistingItem(newItem.get(), oldItem.get());

          CAlbum album(*newItem);
          musicdb.UpdateAlbum(album);
        }

        break;
      }
    }

    /* TODO: delete items that are not in newItems
     * what if the same album exists with some local items
     * or items from a different source???
    if (!found)
    {
      task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37032).c_str(),
                                                oldItem->GetMusicInfoTag()->GetAlbumArtist().at(0).c_str(), // TODO
                                                oldItem->GetMusicInfoTag()->GetAlbum().c_str()));
      // TODO: musicdb.DeleteAlbum(oldItem->GetMusicInfoTag()->GetDatabaseId());
    }
    */

    task->SetProgress(progress++, total);
  }

  if (importSettings.UpdateImportedMediaItems())
    thumbLoader.OnLoaderFinish();

  if (newItems.Size() <= 0)
    return true;

  // add any (remaining) new items
  for (int i = 0; i < newItems.Size(); i++)
  {
    if (checkCancelled && task->ShouldCancel(progress, items.Size()))
      return false;

    CFileItemPtr pItem = newItems[i];
    PrepareItem(import, pItem.get(), musicdb);

    task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37034).c_str(),
                                              pItem->GetMusicInfoTag()->GetAlbumArtist().at(0).c_str(), // TODO
                                              pItem->GetMusicInfoTag()->GetAlbum().c_str()));

    CAlbum album(*pItem);
    if (musicdb.AddAlbum(album))
    {
      pItem->SetFromAlbum(album);
      SetImportForItem(pItem.get(), import, musicdb);
    }

    task->SetProgress(progress++, total);
  }

  return true;
}
