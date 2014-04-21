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

#include "SongImportHandler.h"
#include "FileItem.h"
#include "dialogs/GUIDialogExtendedProgressBar.h"
#include "media/import/IMediaImportTask.h"
#include "music/MusicDatabase.h"
#include "music/MusicThumbLoader.h"
#include "settings/AdvancedSettings.h"
#include "utils/StringUtils.h"

typedef std::set<CFileItemPtr> AlbumSet;
typedef std::map<std::string, AlbumSet> AlbumMap;

/*!
 * Tries to find the album from the given map to which the given song belongs
 */
static int FindAlbum(const AlbumMap &albumMap, CFileItemPtr songItem)
{
  if (songItem == NULL)
    return -1;

  // no comparison possible without a title
  if (songItem->GetMusicInfoTag()->GetAlbum().empty())
    return -1;

  // check if there is an album with a matching title
  AlbumMap::const_iterator albumIter = albumMap.find(songItem->GetMusicInfoTag()->GetAlbum());
  if (albumIter == albumMap.end() ||
      albumIter->second.size() <= 0)
    return -1;

  // if there is only one matching album, we can go with that one
  if (albumIter->second.size() == 1)
    return albumIter->second.begin()->get()->GetMusicInfoTag()->GetDatabaseId();

  // use the artist of the song and album to find the right album
  for (AlbumSet::const_iterator it = albumIter->second.begin(); it != albumIter->second.end(); ++it)
  {
    if (songItem->GetMusicInfoTag()->GetAlbumArtist() == (*it)->GetMusicInfoTag()->GetAlbumArtist()) // TODO: does this work???
      return (*it)->GetMusicInfoTag()->GetDatabaseId();
  }

  return -1;
}

std::set<MediaType> CSongImportHandler::GetDependencies() const
{
  std::set<MediaType> types;
  types.insert(MediaTypeArtist);
  types.insert(MediaTypeAlbum);
  return types;
}

std::vector<MediaType> CSongImportHandler::GetGroupedMediaTypes() const
{
  std::vector<MediaType> types;
  types.push_back(MediaTypeArtist);
  types.push_back(MediaTypeAlbum);
  types.push_back(MediaTypeSong);
  return types;
}

bool CSongImportHandler::HandleImportedItems(CMusicDatabase &musicdb, const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task)
{
  bool checkCancelled = task != NULL;
  if (checkCancelled && task->ShouldCancel(0, items.Size()))
    return false;

  task->SetProgressTitle(StringUtils::Format(g_localizeStrings.Get(37026).c_str(), MediaTypes::GetPluralLocalization(MediaTypeSong).c_str(), import.GetSource().GetFriendlyName().c_str()));
  task->SetProgressText("");

  CFileItemList storedItems;
  musicdb.GetSongsByWhere("musicdb://songs/", GetFilter(import), storedItems);
  
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
    CFileItemPtr pItem = newItems.Get(oldItem->GetMusicInfoTag()->GetURL());

    task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37031).c_str(),
                                              oldItem->GetMusicInfoTag()->GetAlbum().c_str(),
                                              oldItem->GetMusicInfoTag()->GetTitle().c_str()));

    // delete items that are not in newItems
    if (pItem == NULL)
    {
      task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37032).c_str(),
                                                oldItem->GetMusicInfoTag()->GetAlbum().c_str(),
                                                oldItem->GetMusicInfoTag()->GetTitle().c_str()));
      // TODO: musicdb.DeleteSong(oldItem->GetMusicInfoTag()->GetDatabaseId());
    }
    // item is in both lists
    else
    {
      // get rid of items we already have from the new items list
      newItems.Remove(pItem.get());
      total--;

      // only process the item with the thumb loader if updates to artwork etc. are allowed
      if (importSettings.UpdateImportedMediaItems())
        thumbLoader.LoadItem(oldItem.get());

      // check if we need to update (db writing is expensive)
      // but only if synchronisation is enabled
      if (importSettings.UpdateImportedMediaItems() &&
          !Compare(oldItem.get(), pItem.get()))
      {
        task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37033).c_str(),
                                                  pItem->GetMusicInfoTag()->GetAlbum().c_str(),
                                                  pItem->GetMusicInfoTag()->GetTitle().c_str()));

        PrepareExistingItem(pItem.get(), oldItem.get());

        CSong song(*pItem);
        musicdb.UpdateSong(pItem->GetMusicInfoTag()->GetDatabaseId(), song);
        if (importSettings.UpdatePlaybackMetadataFromSource())
          SetDetailsForFile(pItem.get(), musicdb);
      }
    }

    task->SetProgress(progress++, total);
  }

  if (importSettings.UpdateImportedMediaItems())
    thumbLoader.OnLoaderFinish();

  if (newItems.Size() <= 0)
    return true;

  // create a map of artists and albums imported from the same source
  CFileItemList albums;
  musicdb.GetAlbumsByWhere("musicdb://albums/", CDatabase::Filter(), albums);

  AlbumMap albumsMap;
  AlbumMap::iterator albumIter;
  for (int albumIndex = 0; albumIndex < albums.Size(); albumIndex++)
  {
    CFileItemPtr album = albums[albumIndex];

    if (!album->HasMusicInfoTag() || album->GetMusicInfoTag()->GetTitle().empty())
      continue;

    albumIter = albumsMap.find(album->GetMusicInfoTag()->GetTitle());
    if (albumIter == albumsMap.end())
    {
      AlbumSet albumsSet; albumsSet.insert(album);
      albumsMap.insert(make_pair(album->GetMusicInfoTag()->GetTitle(), albumsSet));
    }
    else
      albumIter->second.insert(album);
  }

  // add any (remaining) new items
  for (int i = 0; i < newItems.Size(); i++)
  {
    if (checkCancelled && task->ShouldCancel(progress, items.Size()))
      return false;

    CFileItemPtr pItem = newItems[i];
    MUSIC_INFO::CMusicInfoTag *song = pItem->GetMusicInfoTag();
    PrepareItem(import, pItem.get(), musicdb);

    task->SetProgressText(StringUtils::Format(g_localizeStrings.Get(37034).c_str(),
                                              song->GetAlbum().c_str(),
                                              song->GetTitle().c_str()));
    
    int albumId = FindAlbum(albumsMap, pItem);
    // if the album doesn't exist, create a very basic version of it with the info we got from the song
    if (albumId <= 0)
    {
      CAlbum album;
      album.art = pItem->GetArt();
      album.artist = song->GetArtist();
      album.bCompilation = false;
      album.enabled = true;
      album.genre = song->GetGenre();
      album.iRating = song->GetRating();
      album.iYear = song->GetYear();
      album.strAlbum = song->GetAlbum();
      album.strMusicBrainzAlbumID = song->GetMusicBrainzAlbumID();

      // add the basic album to the database
      if (!musicdb.AddAlbum(album))
        continue;

      albumId = album.idAlbum;

      // turn the album into a CFileItem
      CFileItemPtr albumItem(new CFileItem(album.strAlbum));
      albumItem->SetFromAlbum(album);
      // copy any artwork from the song
      albumItem->SetArt(pItem->GetArt());
      // set the source and import paths
      albumItem->SetSource(pItem->GetSource());
      albumItem->SetImportPath(pItem->GetImportPath());
      // set the import on the album
      SetImportForItem(albumItem.get(), import, musicdb);
      
      // add the album to the album map
      albumIter = albumsMap.find(song->GetAlbum());
      if (albumIter == albumsMap.end())
      {
        AlbumSet albumSet; albumSet.insert(albumItem);
        albumsMap.insert(make_pair(song->GetAlbum(), albumSet));
      }
      else
        albumIter->second.insert(albumItem);
    }

    musicdb.AddSong(albumId, song->GetTitle(), 
      song->GetMusicBrainzTrackID(),
      pItem->GetPath(), song->GetComment(),
      pItem->GetUserMusicThumb(true), StringUtils::Join(song->GetArtist(), g_advancedSettings.m_musicItemSeparator), // TODO: artist string
      song->GetGenre(), song->GetTrackNumber(),
      song->GetDuration(), song->GetYear(),
      song->GetPlayCount(), pItem->m_lStartOffset,
      pItem->m_lEndOffset, song->GetLastPlayed(),
      song->GetRating(), 0);
    SetDetailsForFile(pItem.get(), musicdb);
    SetImportForItem(pItem.get(), import, musicdb);

    task->SetProgress(progress++, total);
  }

  return true;
}
