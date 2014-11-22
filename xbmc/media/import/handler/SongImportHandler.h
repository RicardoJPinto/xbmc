#pragma once
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

#include "media/import/handler/MusicImportHandler.h"

class CSongImportHandler : public CMusicImportHandler
{
public:
  CSongImportHandler() { }
  virtual ~CSongImportHandler() { }

  virtual IMediaImportHandler* Create() const { return new CSongImportHandler(); }

  virtual MediaType GetMediaType() const { return MediaTypeSong; }
  virtual std::set<MediaType> GetDependencies() const;
  virtual std::vector<MediaType> GetGroupedMediaTypes() const;

  virtual std::string GetItemLabel(const CFileItem* item) const;

  virtual bool StartSynchronisation(const CMediaImport &import);

  virtual bool AddImportedItem(const CMediaImport &import, CFileItem* item);
  virtual bool UpdateImportedItem(const CMediaImport &import, CFileItem* item);
  virtual bool RemoveImportedItem(const CMediaImport &import, const CFileItem* item);

protected:
  virtual bool GetLocalItems(CMusicDatabase &musicdb, const CMediaImport &import, CFileItemList& items);

  virtual CFileItemPtr FindMatchingLocalItem(const CFileItem* item, CFileItemList& localItems);

private:
  int FindAlbumId(const CFileItem* songItem);

  typedef std::set<CFileItemPtr> AlbumSet;
  typedef std::map<std::string, AlbumSet> AlbumMap;

  AlbumMap m_albums;
};
