/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "GUIViewStateMusic.h"
#include "PlayListPlayer.h"
#include "video/VideoDatabase.h"
#include "settings/AdvancedSettings.h"
#include "settings/MediaSourceSettings.h"
#include "settings/Settings.h"
#include "FileItem.h"
#include "guilib/WindowIDs.h"
#include "Util.h"
#include "guilib/LocalizeStrings.h"
#include "utils/log.h"
#include "view/ViewStateSettings.h"

#include "filesystem/Directory.h"
#include "filesystem/MusicDatabaseDirectory.h"
#include "filesystem/VideoDatabaseDirectory.h"

using namespace XFILE;
using namespace MUSICDATABASEDIRECTORY;

int CGUIViewStateWindowMusic::GetPlaylist()
{
  //return PLAYLIST_MUSIC_TEMP;
  return PLAYLIST_MUSIC;
}

bool CGUIViewStateWindowMusic::AutoPlayNextItem()
{
  return CSettings::Get().GetBool("musicplayer.autoplaynextitem") &&
         !CSettings::Get().GetBool("musicplayer.queuebydefault");
}

CStdString CGUIViewStateWindowMusic::GetLockType()
{
  return "music";
}

CStdString CGUIViewStateWindowMusic::GetExtensions()
{
  return g_advancedSettings.m_musicExtensions;
}

VECSOURCES& CGUIViewStateWindowMusic::GetSources()
{
  AddAddonsSource("audio", g_localizeStrings.Get(1038), "DefaultAddonMusic.png");
  return CGUIViewState::GetSources();
}

CGUIViewStateMusicSearch::CGUIViewStateMusicSearch(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CStdString strTrackLeft=CSettings::Get().GetString("musicfiles.librarytrackformat");
  if (strTrackLeft.IsEmpty())
    strTrackLeft = CSettings::Get().GetString("musicfiles.trackformat");
  CStdString strTrackRight=CSettings::Get().GetString("musicfiles.librarytrackformatright");
  if (strTrackRight.IsEmpty())
    strTrackRight = CSettings::Get().GetString("musicfiles.trackformatright");

  CStdString strAlbumLeft = g_advancedSettings.m_strMusicLibraryAlbumFormat;
  if (strAlbumLeft.IsEmpty())
    strAlbumLeft = "%B"; // album
  CStdString strAlbumRight = g_advancedSettings.m_strMusicLibraryAlbumFormatRight;
  if (strAlbumRight.IsEmpty())
    strAlbumRight = "%A"; // artist

  SortAttribute sortAttribute = SortAttributeNone;
  if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
    sortAttribute = SortAttributeIgnoreArticle;

  AddSortMethod(SortByTitle, sortAttribute, 556, LABEL_MASKS("%T - %A", "%D", "%L", "%A"));  // Title, Artist, Duration| empty, empty
  SetSortMethod(SortByTitle, sortAttribute);

  const CViewState *viewState = CViewStateSettings::Get().Get("musicnavsongs");
  SetViewAsControl(viewState->m_viewMode);
  SetSortOrder(viewState->m_sortDescription.sortOrder);

  LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicSearch::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::Get().Get("musicnavsongs"));
}


CGUIViewStateMusicDatabase::CGUIViewStateMusicDatabase(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CMusicDatabaseDirectory dir;
  NODE_TYPE NodeType=dir.GetDirectoryChildType(items.GetPath());

  CStdString strTrackLeft=CSettings::Get().GetString("musicfiles.librarytrackformat");
  if (strTrackLeft.IsEmpty())
    strTrackLeft = CSettings::Get().GetString("musicfiles.trackformat");
  CStdString strTrackRight=CSettings::Get().GetString("musicfiles.librarytrackformatright");
  if (strTrackRight.IsEmpty())
    strTrackRight = CSettings::Get().GetString("musicfiles.trackformatright");

  CStdString strAlbumLeft = g_advancedSettings.m_strMusicLibraryAlbumFormat;
  if (strAlbumLeft.IsEmpty())
    strAlbumLeft = "%B"; // album
  CStdString strAlbumRight = g_advancedSettings.m_strMusicLibraryAlbumFormatRight;
  if (strAlbumRight.IsEmpty())
    strAlbumRight = "%A"; // artist

  CLog::Log(LOGDEBUG,"Album format left  = [%s]", strAlbumLeft.c_str());
  CLog::Log(LOGDEBUG,"Album format right = [%s]", strAlbumRight.c_str());

  SortDescription sorting;
  if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
    sorting.sortAttributes = SortAttributeIgnoreArticle;
  LABEL_MASKS labelMask;
  int view = DEFAULT_VIEW_LIST;

  switch (NodeType)
  {
  case NODE_TYPE_OVERVIEW:
    AddSortMethod(SortByNone, 551, LABEL_MASKS("%F", "", "%L", ""));  // Filename, empty | Foldername, empty
    break;
  case NODE_TYPE_TOP100:
    AddSortMethod(SortByNone, 551, LABEL_MASKS("%F", "", "%L", ""));  // Filename, empty | Foldername, empty
    break;
  case NODE_TYPE_GENRE:
    {
      AddSortMethod(SortByGenre, 515, LABEL_MASKS("%F", "", "%G", ""));  // Filename, empty | Genre, empty
      sorting.sortBy = SortByGenre;
      sorting.sortOrder = SortOrderAscending;
    }
    break;
  case NODE_TYPE_YEAR:
    {
      AddSortMethod(SortByLabel, 551, LABEL_MASKS("%F", "", "%Y", ""));  // Filename, empty | Year, empty
      sorting.sortBy = SortByLabel;
      sorting.sortOrder = SortOrderAscending;
    }
    break;
  case NODE_TYPE_ARTIST:
    {
      sorting.sortBy = SortByArtist;
      labelMask = LABEL_MASKS("%F", "", "%A", "");

      AddSortMethod(sorting, 557, labelMask);  // Filename, empty | Artist, empty

      const CViewState *viewState = CViewStateSettings::Get().Get("musicnavartists");
      sorting.sortOrder = viewState->m_sortDescription.sortOrder;
      view = viewState->m_viewMode;
    }
    break;
  case NODE_TYPE_ALBUM_COMPILATIONS:
  case NODE_TYPE_ALBUM:
  case NODE_TYPE_YEAR_ALBUM:
    {
      labelMask = LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight);  // Filename, empty | Userdefined, Userdefined

      // album
      AddSortMethod(SortByAlbum, sorting.sortAttributes, 558, labelMask);
      // artist
      AddSortMethod(SortByArtist, sorting.sortAttributes, 557, labelMask);
      // year
      AddSortMethod(SortByYear, 562, labelMask);

      const CViewState *viewState = CViewStateSettings::Get().Get("musicnavalbums");
      sorting = viewState->m_sortDescription;
      view = viewState->m_viewMode;
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_ADDED:
    {
      AddSortMethod(SortByNone, 552, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      view = CViewStateSettings::Get().Get("musicnavalbums")->m_viewMode;
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_ADDED_SONGS:
    {
      AddSortMethod(SortByNone, 552, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined | empty, empty
      view = CViewStateSettings::Get().Get("musicnavsongs")->m_viewMode;
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_PLAYED:
    {
      AddSortMethod(SortByNone, 551, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      view = CViewStateSettings::Get().Get("musicnavalbums")->m_viewMode;
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_PLAYED_SONGS:
    {
      AddSortMethod(SortByNone, 551, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined | empty, empty
      view = CViewStateSettings::Get().Get("musicnavalbums")->m_viewMode;
    }
    break;
  case NODE_TYPE_ALBUM_TOP100:
    AddSortMethod(SortByNone, 551, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
    break;
  case NODE_TYPE_SINGLES:
    {
      AddSortMethod(SortByArtist, sorting.sortAttributes, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Title, Duration| empty, empty
      AddSortMethod(SortByTitle, sorting.sortAttributes, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
      AddSortMethod(SortByLabel, sorting.sortAttributes, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
      AddSortMethod(SortByTime, 180, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
      AddSortMethod(SortByRating, 563, LABEL_MASKS("%T - %A", "%R"));  // Title - Artist, Rating

      const CViewState *viewState = CViewStateSettings::Get().Get("musicnavsongs");
      sorting = viewState->m_sortDescription;
      view = viewState->m_viewMode;
    }
    break;
  case NODE_TYPE_ALBUM_COMPILATIONS_SONGS:
  case NODE_TYPE_ALBUM_TOP100_SONGS:
  case NODE_TYPE_YEAR_SONG:
  case NODE_TYPE_SONG:
    {
      AddSortMethod(SortByTrackNumber, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
      AddSortMethod(SortByTitle, sorting.sortAttributes, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
      AddSortMethod(SortByAlbum, sorting.sortAttributes, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Title, Artist, Duration| empty, empty
      AddSortMethod(SortByArtist, sorting.sortAttributes, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Title, Duration| empty, empty
      AddSortMethod(SortByLabel, sorting.sortAttributes, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
      AddSortMethod(SortByTime, 180, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
      AddSortMethod(SortByRating, 563, LABEL_MASKS("%T - %A", "%R"));  // Title - Artist, Rating
      AddSortMethod(SortByYear, 562, LABEL_MASKS("%T - %A", "%Y")); // Title, Artist, Year
      AddSortMethod(SortByPlaycount, 567, LABEL_MASKS("%T - %A", "%V"));  // Titel - Artist, PlayCount

      const CViewState *viewState = CViewStateSettings::Get().Get("musicnavsongs");
      // the "All Albums" entries always default to SortByAlbum as this is most logical - user can always
      // change it and the change will be saved for this particular path
      sorting.sortBy = viewState->m_sortDescription.sortBy;
      if (dir.IsAllItem(items.GetPath()))
        sorting.sortBy = SortByAlbum;
      sorting.sortOrder = viewState->m_sortDescription.sortOrder;
      labelMask = LABEL_MASKS(strTrackLeft, strTrackRight);
      view = viewState->m_viewMode;
    }
    break;
  case NODE_TYPE_SONG_TOP100:
    {
      AddSortMethod(SortByNone, 576, LABEL_MASKS("%T - %A", "%V"));
      sorting.sortBy = SortByPlaycount;
      view = CViewStateSettings::Get().Get("musicnavsongs")->m_viewMode;
    }
    break;
  default:
    break;
  }

  AddAndSetSortMethod(items, labelMask, sorting);
  SetViewAsControl(view);

  LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicDatabase::SaveViewState()
{
  CMusicDatabaseDirectory dir;
  NODE_TYPE NodeType=dir.GetDirectoryChildType(m_items.GetPath());

  switch (NodeType)
  {
    case NODE_TYPE_ARTIST:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::Get().Get("musicnavartists"));
      break;
    case NODE_TYPE_ALBUM_COMPILATIONS:
    case NODE_TYPE_ALBUM:
    case NODE_TYPE_YEAR_ALBUM:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::Get().Get("musicnavalbums"));
      break;
    case NODE_TYPE_ALBUM_RECENTLY_ADDED:
    case NODE_TYPE_ALBUM_TOP100:
    case NODE_TYPE_ALBUM_RECENTLY_PLAYED:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
      break;
    case NODE_TYPE_SINGLES:
    case NODE_TYPE_ALBUM_COMPILATIONS_SONGS:
    case NODE_TYPE_SONG:
    case NODE_TYPE_YEAR_SONG:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::Get().Get("musicnavsongs"));
      break;
    case NODE_TYPE_ALBUM_RECENTLY_PLAYED_SONGS:
    case NODE_TYPE_ALBUM_RECENTLY_ADDED_SONGS:
    case NODE_TYPE_SONG_TOP100:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
      break;
    default:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
      break;
  }
}

CGUIViewStateMusicSmartPlaylist::CGUIViewStateMusicSmartPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  SortDescription sorting;
  if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
    sorting.sortAttributes = SortAttributeIgnoreArticle;
  LABEL_MASKS labelMask;
  int view = DEFAULT_VIEW_LIST;

  if (items.GetContent() == "songs" || items.GetContent() == "mixed") 
  {
    CStdString strTrackLeft=CSettings::Get().GetString("musicfiles.trackformat");
    CStdString strTrackRight=CSettings::Get().GetString("musicfiles.trackformatright");

    AddSortMethod(SortByTrackNumber, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
    AddSortMethod(SortByTitle, sorting.sortAttributes, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
    AddSortMethod(SortByAlbum, sorting.sortAttributes, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Titel, Artist, Duration| empty, empty
    AddSortMethod(SortByArtist, sorting.sortAttributes, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Titel, Duration| empty, empty
    AddSortMethod(SortByLabel, sorting.sortAttributes, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
    AddSortMethod(SortByTime, 180, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
    AddSortMethod(SortByRating, 563, LABEL_MASKS("%T - %A", "%R"));  // Titel, Artist, Rating| empty, empty
    
    labelMask = LABEL_MASKS(strTrackLeft, strTrackRight);
    view = CViewStateSettings::Get().Get("musicnavsongs")->m_viewMode;
  } 
  else if (items.GetContent() == "albums") 
  {
    CStdString strAlbumLeft = g_advancedSettings.m_strMusicLibraryAlbumFormat;
    if (strAlbumLeft.IsEmpty())
      strAlbumLeft = "%B"; // album
    CStdString strAlbumRight = g_advancedSettings.m_strMusicLibraryAlbumFormatRight;
    if (strAlbumRight.IsEmpty())
      strAlbumRight = "%A"; // artist

    // album
    AddSortMethod(SortByAlbum, sorting.sortAttributes, 558, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
    // artist
    AddSortMethod(SortByArtist, sorting.sortAttributes, 557, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
    // year
    AddSortMethod(SortByYear, 562, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));

    labelMask = LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight);
    view = CViewStateSettings::Get().Get("musicnavalbums")->m_viewMode;
  }
  else if (items.GetContent() == "artists")
  {
    sorting.sortBy = SortByArtist;
    labelMask = LABEL_MASKS("%F", "", "%A", "");
    view = CViewStateSettings::Get().Get("musicnavartists")->m_viewMode;

    AddSortMethod(sorting.sortBy, sorting.sortAttributes, 557, labelMask);  // Filename, empty | Artist, empty
  }
  else
    CLog::Log(LOGERROR,"Music Smart Playlist must be one of songs, mixed, albums or artists");

  AddAndSetSortMethod(items, labelMask, sorting);
  SetViewAsControl(view);
  
  LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicSmartPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::Get().Get("musicnavsongs"));
}


CGUIViewStateMusicPlaylist::CGUIViewStateMusicPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  SortAttribute sortAttribute = SortAttributeNone;
  if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
    sortAttribute = SortAttributeIgnoreArticle;
  
  CStdString strTrackLeft=CSettings::Get().GetString("musicfiles.trackformat");
  CStdString strTrackRight=CSettings::Get().GetString("musicfiles.trackformatright");

  AddSortMethod(SortByPlaylistOrder, 559, LABEL_MASKS(strTrackLeft, strTrackRight));
  AddSortMethod(SortByTrackNumber, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
  AddSortMethod(SortByTitle, sortAttribute, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
  AddSortMethod(SortByAlbum, sortAttribute, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Titel, Artist, Duration| empty, empty
  AddSortMethod(SortByArtist, sortAttribute, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Titel, Duration| empty, empty
  AddSortMethod(SortByLabel, sortAttribute, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
  AddSortMethod(SortByTime, 180, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
  AddSortMethod(SortByRating, 563, LABEL_MASKS("%T - %A", "%R"));  // Titel, Artist, Rating| empty, empty

  SetSortMethod(SortByPlaylistOrder);
  const CViewState *viewState = CViewStateSettings::Get().Get("musicfiles");
  SetViewAsControl(viewState->m_viewMode);
  SetSortOrder(viewState->m_sortDescription.sortOrder);

  LoadViewState(items.GetPath(), WINDOW_MUSIC_FILES);
}

void CGUIViewStateMusicPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_FILES);
}

CGUIViewStateWindowMusicNav::CGUIViewStateWindowMusicNav(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  SortDescription sorting;
  if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
    sorting.sortAttributes = SortAttributeIgnoreArticle;

  if (items.IsVirtualDirectoryRoot())
    AddSortMethod(SortByNone, 551, LABEL_MASKS("%F", "%I", "%L", ""));  // Filename, Size | Foldername, empty
  else
  {
    if (items.IsVideoDb() && items.Size() > (CSettings::Get().GetBool("filelists.showparentdiritems")?1:0))
    {
      XFILE::VIDEODATABASEDIRECTORY::CQueryParams params;
      XFILE::CVideoDatabaseDirectory::GetQueryParams(items[CSettings::Get().GetBool("filelists.showparentdiritems")?1:0]->GetPath(),params);
      if (params.GetMVideoId() != -1)
      {
        AddSortMethod(SortByLabel, sorting.sortAttributes, 551, LABEL_MASKS("%T", "%Y"));  // Filename, Duration | Foldername, empty
        AddSortMethod(SortByYear, 562, LABEL_MASKS("%T", "%Y"));
        AddSortMethod(SortByArtist, sorting.sortAttributes, 557, LABEL_MASKS("%A - %T", "%Y"));
        AddSortMethod(SortByAlbum, sorting.sortAttributes, 558, LABEL_MASKS("%B - %T", "%Y"));
        
        CStdString strTrackLeft=CSettings::Get().GetString("musicfiles.trackformat");
        CStdString strTrackRight=CSettings::Get().GetString("musicfiles.trackformatright");
        AddSortMethod(SortByTrackNumber, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
      }
      else
      {
        AddSortMethod(SortByLabel, 551, LABEL_MASKS("%F", "%D", "%L", ""));  // Filename, Duration | Foldername, empty
        sorting.sortBy = SortByLabel;
      }
    }
    else
    {
      AddSortMethod(SortByLabel, 551, LABEL_MASKS("%F", "%D", "%L", ""));  // Filename, Duration | Foldername, empty
      sorting.sortBy = SortByLabel;
    }
    sorting.sortOrder = SortOrderAscending;
  }

  AddAndSetSortMethod(items, LABEL_MASKS(), sorting);
  SetViewAsControl(DEFAULT_VIEW_LIST);

  LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateWindowMusicNav::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateWindowMusicNav::AddOnlineShares()
{
  if (!g_advancedSettings.m_bVirtualShares)
    return;

  VECSOURCES *musicSources = CMediaSourceSettings::Get().GetSources("music");

  for (int i = 0; i < (int)musicSources->size(); ++i)
  {
    CMediaSource share = musicSources->at(i);
  }
}

VECSOURCES& CGUIViewStateWindowMusicNav::GetSources()
{
  //  Setup shares we want to have
  m_sources.clear();
  //  Musicdb shares
  CFileItemList items;
  CDirectory::GetDirectory("musicdb://", items, "");
  for (int i=0; i<items.Size(); ++i)
  {
    CFileItemPtr item=items[i];
    CMediaSource share;
    share.strName=item->GetLabel();
    share.strPath = item->GetPath();
    share.m_strThumbnailImage = item->GetIconImage();
    share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
    m_sources.push_back(share);
  }

  //  Playlists share
  CMediaSource share;
  share.strName=g_localizeStrings.Get(136); // Playlists
  share.strPath = "special://musicplaylists/";
  share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultMusicPlaylists.png");
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
  m_sources.push_back(share);

  AddOnlineShares();

  // Search share
  share.strName=g_localizeStrings.Get(137); // Search
  share.strPath = "musicsearch://";
  share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultMusicSearch.png");
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
  m_sources.push_back(share);

  // music video share
  CVideoDatabase database;
  database.Open();
  if (database.HasContent(VIDEODB_CONTENT_MUSICVIDEOS))
  {
    share.strName = g_localizeStrings.Get(20389);
    share.strPath = "videodb://musicvideos/";
    share.m_strThumbnailImage = CUtil::GetDefaultFolderThumb("DefaultMusicVideos.png");
    share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
    m_sources.push_back(share);
  }

  return CGUIViewStateWindowMusic::GetSources();
}


CGUIViewStateWindowMusicSongs::CGUIViewStateWindowMusicSongs(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  SortDescription sorting;
  if (CSettings::Get().GetBool("filelists.ignorethewhensorting"))
    sorting.sortAttributes = SortAttributeIgnoreArticle;
  LABEL_MASKS labelMask;
  int view = VIEW_TYPE_NONE;

  if (items.IsVirtualDirectoryRoot())
  {
    AddSortMethod(SortByLabel, 551, LABEL_MASKS()); // Preformated
    AddSortMethod(SortByDriveType, 564, LABEL_MASKS()); // Preformated

    sorting.sortBy = SortByLabel;
    sorting.sortOrder = SortOrderAscending;
    view = DEFAULT_VIEW_LIST;
  }
  else if (items.GetPath() == "special://musicplaylists/")
  { // playlists list sorts by label only, ignoring folders
    AddSortMethod(SortByLabel, SortAttributeIgnoreFolders, 551, LABEL_MASKS("%F", "%D", "%L", ""));  // Filename, Duration | Foldername, empty
    sorting.sortBy = SortByLabel;
    sorting.sortAttributes = SortAttributeIgnoreFolders;
  }
  else
  {
    CStdString strTrackLeft=CSettings::Get().GetString("musicfiles.trackformat");
    CStdString strTrackRight=CSettings::Get().GetString("musicfiles.trackformatright");

    AddSortMethod(SortByLabel, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""),  // Userdefined, Userdefined | FolderName, empty
      CSettings::Get().GetBool("filelists.ignorethewhensorting") ? SortAttributeIgnoreArticle : SortAttributeNone);
    AddSortMethod(SortBySize, 553, LABEL_MASKS(strTrackLeft, "%I", "%L", "%I"));  // Userdefined, Size | FolderName, Size
    AddSortMethod(SortByBitrate, 623, LABEL_MASKS(strTrackLeft, "%X", "%L", "%X"));  // Userdefined, Bitrate | FolderName, Bitrate  
    AddSortMethod(SortByDate, 552, LABEL_MASKS(strTrackLeft, "%J", "%L", "%J"));  // Userdefined, Date | FolderName, Date
    AddSortMethod(SortByFile, 561, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
    AddSortMethod(SortByListeners, 20455,LABEL_MASKS(strTrackLeft, "%W", "%L", "%W"));
    
    const CViewState *viewState = CViewStateSettings::Get().Get("musicfiles");
    sorting.sortBy = viewState->m_sortDescription.sortBy;
    sorting.sortOrder = viewState->m_sortDescription.sortOrder;
    view = viewState->m_viewMode;
  }

  AddAndSetSortMethod(items, LABEL_MASKS(), sorting);
  if (view != VIEW_TYPE_NONE)
    SetViewAsControl(view);

  LoadViewState(items.GetPath(), WINDOW_MUSIC_FILES);
}

void CGUIViewStateWindowMusicSongs::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_FILES, CViewStateSettings::Get().Get("musicfiles"));
}

VECSOURCES& CGUIViewStateWindowMusicSongs::GetSources()
{
  VECSOURCES *musicSources = CMediaSourceSettings::Get().GetSources("music");
  AddOrReplace(*musicSources, CGUIViewStateWindowMusic::GetSources());
  return *musicSources;
}


CGUIViewStateWindowMusicPlaylist::CGUIViewStateWindowMusicPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CStdString strTrackLeft=CSettings::Get().GetString("musicfiles.nowplayingtrackformat");
  if (strTrackLeft.IsEmpty())
    strTrackLeft = CSettings::Get().GetString("musicfiles.trackformat");
  CStdString strTrackRight=CSettings::Get().GetString("musicfiles.nowplayingtrackformatright");
  if (strTrackRight.IsEmpty())
    strTrackRight = CSettings::Get().GetString("musicfiles.trackformatright");

  AddSortMethod(SortByNone, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
  SetSortMethod(SortByNone);

  SetViewAsControl(DEFAULT_VIEW_LIST);

  SetSortOrder(SortOrderNone);

  LoadViewState(items.GetPath(), WINDOW_MUSIC_PLAYLIST);
}

void CGUIViewStateWindowMusicPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_PLAYLIST);
}

int CGUIViewStateWindowMusicPlaylist::GetPlaylist()
{
  return PLAYLIST_MUSIC;
}

bool CGUIViewStateWindowMusicPlaylist::AutoPlayNextItem()
{
  return false;
}

bool CGUIViewStateWindowMusicPlaylist::HideParentDirItems()
{
  return true;
}

VECSOURCES& CGUIViewStateWindowMusicPlaylist::GetSources()
{
  m_sources.clear();
  //  Playlist share
  CMediaSource share;
  share.strPath = "playlistmusic://";
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
  m_sources.push_back(share);

  // CGUIViewState::GetSources would add music plugins
  return m_sources;
}
