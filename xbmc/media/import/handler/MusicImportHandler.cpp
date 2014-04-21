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

#include "MusicImportHandler.h"
#include "FileItem.h"
#include "media/import/IMediaImportTask.h"
#include "music/MusicDatabase.h"
#include "utils/StringUtils.h"

void CMusicImportHandler::SetImportedItemsEnabled(const CMediaImport &import, bool enable)
{
  CMusicDatabase musicdb;
  if (!musicdb.Open())
    return;

  musicdb.SetImportItemsEnabled(enable, import);
}

void CMusicImportHandler::HandleImportedItems(const CMediaImport &import, const CFileItemList &items, IMediaImportTask *task)
{
  if (task != NULL && task->ShouldCancel(0, items.Size()))
    return;

  CMusicDatabase musicdb;
  if (!musicdb.Open())
    return;

  musicdb.BeginTransaction();

  if (!import.GetPath().empty())
    HandleImportedItems(musicdb, import, items, task);

  musicdb.CommitTransaction();
  musicdb.Close();
}

void CMusicImportHandler::PrepareItem(const CMediaImport &import, CFileItem* pItem, CMusicDatabase &musicdb)
{
  if (pItem == NULL || !pItem->HasMusicInfoTag() ||
      import.GetPath().empty() || import.GetSource().GetIdentifier().empty())
    return;

  const std::string &sourceID = import.GetSource().GetIdentifier();
  musicdb.AddPath(sourceID);
  int idPath = musicdb.AddPath(import.GetPath());

  // set the proper source
  pItem->SetSource(sourceID);
  pItem->SetImportPath(import.GetPath());
}

void CMusicImportHandler::PrepareExistingItem(CFileItem *updatedItem, const CFileItem *originalItem)
{
  if (updatedItem == NULL || originalItem == NULL ||
      !updatedItem->HasMusicInfoTag() || !originalItem->HasMusicInfoTag())
    return;

  updatedItem->GetMusicInfoTag()->SetDatabaseId(originalItem->GetMusicInfoTag()->GetDatabaseId(), originalItem->GetMusicInfoTag()->GetType());
  updatedItem->GetMusicInfoTag()->SetAlbumId(originalItem->GetMusicInfoTag()->GetAlbumId());

  updatedItem->SetSource(originalItem->GetSource());
  updatedItem->SetImportPath(originalItem->GetImportPath());
}

void CMusicImportHandler::SetDetailsForFile(const CFileItem *pItem, CMusicDatabase &musicdb)
{
  musicdb.SetPlayCount(*pItem, pItem->GetMusicInfoTag()->GetPlayCount(), pItem->GetMusicInfoTag()->GetLastPlayed());
}

bool CMusicImportHandler::SetImportForItem(const CFileItem *pItem, const CMediaImport &import, CMusicDatabase &musicdb)
{
  return musicdb.SetImportForItem(pItem->GetMusicInfoTag()->GetDatabaseId(), import);
}

CDatabase::Filter CMusicImportHandler::GetFilter(const CMediaImport &import, bool enabledItems /* = false */)
{
  std::string strWhere;
  if (!import.GetPath().empty())
    strWhere += StringUtils::Format("importPath = '%s'", import.GetPath().c_str());

  return CDatabase::Filter(strWhere);
}

bool CMusicImportHandler::Compare(const CFileItem *originalItem, const CFileItem *newItem)
{
  if (originalItem == NULL || !originalItem->HasMusicInfoTag() ||
      newItem == NULL || !newItem->HasMusicInfoTag())
    return false;

  /* TODO
  if (originalItem->GetArt() != newItem->GetArt())
    return false;
  */

  /* TODO
  if (originalItem->GetVideoInfoTag()->Equals(*newItem->GetVideoInfoTag(), true))
    return true;

  std::set<Field> differences;
  if (!originalItem->GetVideoInfoTag()->GetDifferences(*newItem->GetVideoInfoTag(), differences, true))
    return true;

  return differences.empty();
  */

  return true;
}
