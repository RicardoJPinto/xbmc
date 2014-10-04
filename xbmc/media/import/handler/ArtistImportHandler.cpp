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

#include "ArtistImportHandler.h"
#include "FileItem.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "media/import/IMediaImportTask.h"
#include "music/MusicDatabase.h"
#include "music/MusicThumbLoader.h"
#include "settings/AdvancedSettings.h"
#include "utils/StringUtils.h"

std::set<MediaType> CArtistImportHandler::GetRequiredMediaTypes() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeSong);
  return types;
}

std::vector<MediaType> CArtistImportHandler::GetGroupedMediaTypes() const
{
  std::vector<MediaType> types;
  types.push_back(MediaTypeArtist);
  types.push_back(MediaTypeAlbum);
  types.push_back(MediaTypeSong);
  return types;
}

bool CArtistImportHandler::HandleImportedItems(CMusicDatabase &musicdb, const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task)
{
  bool checkCancelled = task != NULL;
  if (checkCancelled && task->ShouldCancel(0, items.Size()))
    return false;

  task->SetProgressTitle(StringUtils::Format(g_localizeStrings.Get(37032).c_str(), MediaTypes::GetPluralLocalization(MediaTypeArtist).c_str(), import.GetSource().GetFriendlyName().c_str()));
  task->SetProgressText("");
  
  CFileItemList storedItems;
  musicdb.GetArtistsByWhere("musicdb://artists/", GetFilter(import), storedItems);
  
  int total = storedItems.Size() + items.Size();
  if (checkCancelled && task->ShouldCancel(0, total))
    return false;

  CMusicThumbLoader thumbLoader;
  const CMediaImportSettings &importSettings = import.GetSettings();
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
      task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37033).c_str(), oldItem->GetLabel().c_str()));

      if (oldItem->GetLabel() == newItem->GetLabel())
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
          task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37035).c_str(), newItem->GetLabel().c_str()));

          PrepareExistingItem(newItem.get(), oldItem.get());
          UpdateArtist(*newItem, musicdb);
        }

        break;
      }
    }

    /* TODO: delete items that are not in newItems
     * what if the same artist exists with some local items
     * or items from a different source???
    if (!found)
    {
      task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37034).c_str(), oldItem->GetLabel().c_str()));
      // TODO: musicdb.DeleteArtist(oldItem->GetMusicInfoTag()->GetDatabaseId());
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
    PrepareItem(import, pItem.get(), musicdb);

    task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37036).c_str(), pItem->GetLabel().c_str()));

    std::string mbArtistId;
    if (!pItem->GetMusicInfoTag()->GetMusicBrainzArtistID().empty())
      mbArtistId = pItem->GetMusicInfoTag()->GetMusicBrainzArtistID().at(0);

    pItem->GetMusicInfoTag()->SetDatabaseId(musicdb.AddArtist(pItem->GetLabel(), mbArtistId), MediaTypeArtist);
    if (pItem->GetMusicInfoTag()->GetDatabaseId() > 0)
    {
      UpdateArtist(*pItem, musicdb);
      SetImportForItem(pItem.get(), import, musicdb);
    }

    task->SetProgress(progress++, total);
  }

  return true;
}

void CArtistImportHandler::UpdateArtist(const CFileItem &artistItem, CMusicDatabase &musicdb)
{
  if (!artistItem.HasMusicInfoTag() || artistItem.GetMusicInfoTag()->GetDatabaseId() <= 0)
    return;

  const MUSIC_INFO::CMusicInfoTag& artist = *artistItem.GetMusicInfoTag();

  std::string mbArtistId;
  if (!artist.GetMusicBrainzArtistID().empty())
    mbArtistId = artist.GetMusicBrainzArtistID().at(0);

  musicdb.UpdateArtist(artist.GetDatabaseId(), artistItem.GetLabel(), mbArtistId,
    "", "", StringUtils::Join(artist.GetGenre(), g_advancedSettings.m_musicItemSeparator) /* TODO */,
    "", "", "", "", "", "", "", artistItem.GetArt("thumb"), artistItem.GetArt("fanart"));
  /* TODO
  musicdb.UpdateArtist(artistId, pItem->GetLabel(), mbArtistId,
    "born", "formed", "genres", "moods",
    "styles", "instruments", "biography", "died",
    "disbanded", "yearsactive", "image", "fanart");
  */
  musicdb.SetArtForItem(artist.GetDatabaseId(), MediaTypeArtist, artistItem.GetArt());
}
