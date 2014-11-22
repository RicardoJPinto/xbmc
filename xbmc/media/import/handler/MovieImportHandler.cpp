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

#include "MovieImportHandler.h"
#include "FileItem.h"
#include "video/VideoDatabase.h"

bool CMovieImportHandler::GetLocalItems(CVideoDatabase &videodb, const CMediaImport &import, CFileItemList& items)
{
  return videodb.GetMoviesByWhere("videodb://movies/titles/", GetFilter(import), items, SortDescription(), import.GetSettings().UpdateImportedMediaItems());
}

bool CMovieImportHandler::AddImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == NULL)
    return false;

  PrepareItem(import, item);

  item->GetVideoInfoTag()->m_iDbId = m_db.SetDetailsForMovie(item->GetPath(), *(item->GetVideoInfoTag()), item->GetArt());
  if (item->GetVideoInfoTag()->m_iDbId <= 0)
    return false;

  SetDetailsForFile(item, false);
  return SetImportForItem(item, import);
}

bool CMovieImportHandler::UpdateImportedItem(const CMediaImport &import, CFileItem* item)
{
  if (item == NULL || !item->HasVideoInfoTag() || item->GetVideoInfoTag()->m_iDbId <= 0)
    return false;

  if (m_db.SetDetailsForMovie(item->GetPath(), *(item->GetVideoInfoTag()), item->GetArt(), item->GetVideoInfoTag()->m_iDbId) <= 0)
    return false;

  if (import.GetSettings().UpdatePlaybackMetadataFromSource())
    SetDetailsForFile(item, true);

  return true;
}

bool CMovieImportHandler::RemoveImportedItem(const CMediaImport &import, const CFileItem* item)
{
  if (item == NULL || !item->HasVideoInfoTag())
    return false;

  m_db.DeleteMovie(item->GetVideoInfoTag()->m_iDbId);
  return true;
}
