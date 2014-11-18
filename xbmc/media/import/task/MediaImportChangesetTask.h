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

#include "FileItem.h"
#include "media/MediaType.h"
#include "media/import/IMediaImportTask.h"
#include "media/import/MediaImportChangesetTypes.h"

class CGUIDialogProgressBarHandle;
class IMediaImportHandler;

class CMediaImportChangesetTask : public IMediaImportTask
{
public:
  CMediaImportChangesetTask(const CMediaImport &import, IMediaImportHandler *importHandler, const CFileItemList &localItems, const ChangesetItems &retrievedItems);
  virtual ~CMediaImportChangesetTask();

  const ChangesetItems& GetChangeset() const { return m_retrievedItems; }

  // implementation of IMediaImportTask
  virtual bool DoWork();
  virtual const char *GetType() const { return "MediaImportChangesetTask"; }

protected:
  IMediaImportHandler *m_importHandler;
  CFileItemList m_localItems;
  ChangesetItems m_retrievedItems;
};
