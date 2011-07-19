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

#include "AudioService.h"
#include "Application.h"
#include "settings/Settings.h"
#include "guilib/GUIAudioManager.h"

CAudioService *CAudioService::m_instance = NULL;

CAudioServiceCallback::CAudioServiceCallback() : CServiceBaseCallback<CAudioService, CAudioServiceCallback>()
{
}

CAudioService *CAudioService::GetInstance()
{
  if (m_instance == NULL)
    m_instance = new CAudioService();

  return m_instance;
}

CAudioService::CAudioService() : CServiceBase<CAudioService, CAudioServiceCallback>()
{
  SetProperty("Volume", g_settings.m_nVolumeLevel);
  SetProperty("DynamicRangeCompressionLevel", g_settings.m_dynamicRangeCompressionLevel);
  SetProperty("Muted", g_settings.m_bMute);
}

void CAudioService::ToggleMute()
{
  if (GetProperty("Muted").asBoolean())
    Unmute();
  else
    Mute();
}

void CAudioService::Mute()
{
  if (GetProperty("Muted").asBoolean())
    return;

  SetProperty("Muted", true);
  g_settings.m_bMute = true;
}

void CAudioService::Unmute()
{
  if (!GetProperty("Muted").asBoolean())
    return;

  SetProperty("Muted", false);
  g_settings.m_bMute = false;
}

int CAudioService::GetVolume(bool percentage /* = true */)
{
  if (GetProperty("Muted").asBoolean())
  {
    if (percentage)
      return 0;

    return VOLUME_MINIMUM;
  }

  // converts the hardware volume (in mB) to a percentage
  if (percentage)
    return int(((float)(g_settings.m_nVolumeLevel + g_settings.m_dynamicRangeCompressionLevel - VOLUME_MINIMUM)) / (VOLUME_MAXIMUM - VOLUME_MINIMUM)*100.0f + 0.5f);

  return g_settings.m_nVolumeLevel + g_settings.m_dynamicRangeCompressionLevel;
}

void CAudioService::SetVolume(long iVolume, bool isPercentage /* = true */)
{
  // convert the percentage to a mB (milliBell) value (*100 for dB)
  if (isPercentage)
    iVolume = (long)((float)iVolume * 0.01f * (VOLUME_MAXIMUM - VOLUME_MINIMUM) + VOLUME_MINIMUM);

  SetProperty("Volume", SetHardwareVolume(iVolume));
}

int CAudioService::SetHardwareVolume(long hardwareVolume)
{
  // TODO DRC
  if (hardwareVolume >= VOLUME_MAXIMUM) // + VOLUME_DRC_MAXIMUM
    hardwareVolume = VOLUME_MAXIMUM;// + VOLUME_DRC_MAXIMUM;
  if (hardwareVolume <= VOLUME_MINIMUM)
    hardwareVolume = VOLUME_MINIMUM;

  // update our settings
  if (hardwareVolume > VOLUME_MAXIMUM)
  {
    g_settings.m_dynamicRangeCompressionLevel = hardwareVolume - VOLUME_MAXIMUM;
    g_settings.m_nVolumeLevel = VOLUME_MAXIMUM;

    return VOLUME_MAXIMUM;
  }
  else
  {
    g_settings.m_dynamicRangeCompressionLevel = 0;
    g_settings.m_nVolumeLevel = hardwareVolume;

    return hardwareVolume;
  }
}
