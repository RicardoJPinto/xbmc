#pragma once
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

#include <map>

#include "media/MediaType.h"
#include "media/import/IMediaImportTask.h"
#include "media/import/MediaImportChangesetTypes.h"

class IMediaImporter;
class IMediaImportHandler;

class CMediaImportRetrievalTask : public IMediaImportTask
{
public:
  CMediaImportRetrievalTask(const CMediaImport &import, IMediaImportHandler *importHandler);
  virtual ~CMediaImportRetrievalTask();

  /*!
   * \brief Get the IMediaImporter instance used by the import job
   */
  IMediaImporter* GetImporter() const { return m_importer; }

  /*!
   * \brief Get the media type of the media import
   */
  const MediaType& GetMediaType() const { return GetImport().GetMediaType(); }

  /*!
   * \brief Get a list of imported items
   *
   * \param mediaType media type of the imported items
   * \return list of imported items
   */
  const ChangesetItems& GetRetrievedItems() const { return m_retrievedItems; }

  /*!
  * \brief Get a list of previously imported items
  *
  * \param mediaType media type of the previously imported items
  * \return list of previously imported items
  */
  const CFileItemList& GetLocalItems() const { return m_localItems; }

  /*!
   * \brief Add an imported item of a specific changeset type
   *
   * \param item imported item
   * \param changesetType changeset type of the imported item
   */
  void AddItem(const CFileItemPtr& item, MediaImportChangesetType changesetType = MediaImportChangesetTypeNone);

  /*!
  * \brief Add a list of imported items of a specific changeset type
  *
  * \param items imported items
  * \param changesetType changeset type of the imported items
  */
  void AddItems(const CFileItemList& items, MediaImportChangesetType changesetType = MediaImportChangesetTypeNone);

  /*!
   * \brief Add a list of imported items of a specific media type
   *
   * \param items imported items
   * \param changesetType changeset type of the imported items
   */
  void SetItems(const ChangesetItems& items);

  // implementation of IMediaImportTask
  virtual bool DoWork();
  virtual const char *GetType() const { return "MediaImportRetrievalTask"; }

protected:
  IMediaImporter* m_importer;
  IMediaImportHandler *m_importHandler;
  ChangesetItems m_retrievedItems;
  CFileItemList m_localItems;
};
