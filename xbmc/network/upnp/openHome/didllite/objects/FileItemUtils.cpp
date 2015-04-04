/*
 *      Copyright (C) 2015 Team XBMC
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

#include <algorithm>
#include <map>
#include <vector>

#include "FileItemUtils.h"
#include "FileItem.h"
#include "network/upnp/openHome/didllite/DidlLiteDocument.h"
#include "network/upnp/openHome/didllite/objects/IFileItemElement.h"
#include "network/upnp/openHome/didllite/objects/classmappers/UPnPClassMapping.h"

bool FileItemUtils::DocumentToFileItemList(const CDidlLiteDocument& document, CFileItemList& items, const CUPnPClassMapping* classMapping /* = nullptr */)
{
  items.ClearItems();

  std::map<std::string, size_t> classes;
  std::vector<const IDidlLiteElement*> elements = document.GetElements();
  for (const auto& element : elements)
  {
    const IFileItemElement* fileItemElement = dynamic_cast<const IFileItemElement*>(element);
    if (fileItemElement == nullptr)
      continue;

    std::string mediaType;
    if (classMapping != nullptr)
      mediaType = classMapping->GetMediaType(fileItemElement);

    CFileItemPtr fileItem(new CFileItem());
    if (!fileItemElement->ToFileItem(*fileItem)) // TODO: pass on the media type
      return false;

    items.Add(fileItem);
    classes[fileItemElement->GetType()] += 1;
  }

  // determine the class with the most items
  const auto& maximumClass = std::max_element(classes.begin(), classes.end(), classes.value_comp());
  if (maximumClass != classes.end())
    items.SetContent(maximumClass->first);

  return true;
}
