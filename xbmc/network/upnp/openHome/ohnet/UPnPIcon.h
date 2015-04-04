#pragma once
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

#include <stdint.h>
#include <string>

class CUPnPIcon
{
public:
  CUPnPIcon(const std::string &path, const std::string &mimetype,
            uint32_t width, uint32_t height, uint32_t depth)
    : m_path(path),
      m_mimetype(mimetype),
      m_width(width),
      m_height(height),
      m_depth(depth)
  { }

  bool operator<(const CUPnPIcon &other) const
  {
    if (m_width < other.m_width)
      return true;
    if (m_height < other.m_height)
      return true;
    if (m_depth < other.m_depth)
      return true;

    if (m_mimetype != "image/png" && other.m_mimetype == "image/png")
      return true;

    return false;
  }

  const std::string& GetPath() const { return m_path; }
  const std::string& GetMimeType() const { return m_mimetype; }
  uint32_t GetWidth() const { return m_width; }
  uint32_t GetHeight() const { return m_height; }
  uint32_t GetDepth() const { return m_depth; }

private:
  std::string m_path;
  std::string m_mimetype;
  uint32_t m_width;
  uint32_t m_height;
  uint32_t m_depth;
};
