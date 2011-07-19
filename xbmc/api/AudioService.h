#pragma once
/*
 *      Copyright (C) 2005-2011 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "ServiceBase.h"
#include "ServiceProxy.h"

class CAudioService;

/*!
 \ingroup services
 \brief Service callback of the audio service
 */
class CAudioServiceCallback : public CServiceBaseCallback<CAudioService, CAudioServiceCallback>
{
public:
  CAudioServiceCallback();
};

/*!
 \ingroup services
 \brief Service to control the audio output
 */
class CAudioService : public CServiceBase<CAudioService, CAudioServiceCallback>
{
private:
  CAudioService();
  static CAudioService *GetInstance();

public:
  /*!
   \brief Toggles the mute state.
   */
  void ToggleMute();
  /*!
   \brief Mutes the audio (if not already muted).
   */
  void Mute();
  /*!
   \brief Unmutes the audio (if not already unmuted).
   */
  void Unmute();
  
  /*!
   \brief Gets the current volume level as a percentage
   or dB based value.
   \param percentage Whether to return the volume level as a percentage
   or a dB based value
   \return Current volume level

   Returns the current volume level as a percentage or dB based value.
   The returned value takes both Dynamic Range Compression and the current
   mute state into consideration (i.e. if audio is currently muted, this
   method returns 0% independant of the actual volume level). For the pure
   volume level value (in dB) use GetProperty("Volume").
   */
  int GetVolume(bool percentage = true);
  /*!
   \brief Sets the current volume level to the given value
   \param iVolume Volume level to set
   \param isPercentage Whether the given volume level is percentage or
   dB based
   */
  void SetVolume(long iVolume, bool isPercentage = true);

private:
  int SetHardwareVolume(long hardwareVolume);
  
  static CAudioService *m_instance;

  friend CAudioService *CServiceProxy<CAudioService>::GetInstance();
};

template<>
inline
CAudioService *CServiceProxy<CAudioService>::GetInstance()
{
  return CAudioService::GetInstance();
}
