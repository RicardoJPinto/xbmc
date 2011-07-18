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

#include "PowerService.h"
#include "Application.h"
#include "settings/GUISettings.h"
#include "utils/log.h"

#ifdef __APPLE__
#include "powermanagement/osx/CocoaPowerSyscall.h"
#elif defined(_LINUX) && defined(HAS_DBUS)
#include "powermanagement/linux/ConsoleUPowerSyscall.h"
#include "powermanagement/linux/ConsoleDeviceKitPowerSyscall.h"
#ifdef HAS_HAL
#include "powermanagement/linux/HALPowerSyscall.h"
#endif
#elif defined(_WIN32)
#include "powermanagement/windows/Win32PowerSyscall.h"
extern HWND g_hWnd;
#endif

CPowerService *CPowerService::m_instance = NULL;

CPowerServiceCallback::CPowerServiceCallback() : CServiceBaseCallback<CPowerService, CPowerServiceCallback>()
{
}

CPowerService *CPowerService::GetInstance()
{
  if (m_instance == NULL)
    m_instance = new CPowerService();

  return m_instance;
}

CPowerService::CPowerService() : CServiceBase<CPowerService, CPowerServiceCallback>()
{
  m_power = NULL;
  #if defined(__APPLE__)
  m_power = new CCocoaPowerSyscall();
#elif defined(_LINUX) && defined(HAS_DBUS)
  if (CConsoleUPowerSyscall::HasDeviceConsoleKit())
    m_power = new CConsoleUPowerSyscall();
  else if (CConsoleDeviceKitPowerSyscall::HasDeviceConsoleKit())
    m_power = new CConsoleDeviceKitPowerSyscall();
#ifdef HAS_HAL
  else
    m_power = new CHALPowerSyscall();
#endif
#elif defined(_WIN32)
  m_power = new CWin32PowerSyscall();
#endif

  if (m_power == NULL)
    m_power = new CNullPowerSyscall();

  SetProperty("CanPowerdown", m_power->CanPowerdown());
  SetProperty("CanReboot", m_power->CanReboot());
  SetProperty("CanHibernate", m_power->CanHibernate());
  SetProperty("CanSuspend", m_power->CanSuspend());
  SetProperty("BatteryLevel", m_power->BatteryLevel());
}

CPowerService::~CPowerService()
{
  delete m_power;
}

void CPowerService::SetDefaults()
{
  int defaultShutdown = g_guiSettings.GetInt("powermanagement.shutdownstate");

  switch (defaultShutdown)
  {
    case POWERSTATE_QUIT:
    case POWERSTATE_MINIMIZE:
      // assume we can shutdown if --standalone is passed
      if (g_application.IsStandAlone())
        defaultShutdown = POWERSTATE_SHUTDOWN;
    break;
    case POWERSTATE_HIBERNATE:
      if (!m_power->CanHibernate())
      {
        if (m_power->CanSuspend())
          defaultShutdown = POWERSTATE_SUSPEND;
        else
          defaultShutdown = m_power->CanPowerdown() ? POWERSTATE_SHUTDOWN : POWERSTATE_QUIT;
      }
    break;
    case POWERSTATE_SUSPEND:
      if (!m_power->CanSuspend())
      {
        if (m_power->CanHibernate())
          defaultShutdown = POWERSTATE_HIBERNATE;
        else
          defaultShutdown = m_power->CanPowerdown() ? POWERSTATE_SHUTDOWN : POWERSTATE_QUIT;
      }
    break;
    case POWERSTATE_SHUTDOWN:
      if (!m_power->CanPowerdown())
      {
        if (m_power->CanSuspend())
          defaultShutdown = POWERSTATE_SUSPEND;
        else
          defaultShutdown = m_power->CanHibernate() ? POWERSTATE_HIBERNATE : POWERSTATE_QUIT;
      }
    break;
  }

  g_guiSettings.SetInt("powermanagement.shutdownstate", defaultShutdown);
}
  
bool CPowerService::Powerdown()
{
  if (!GetProperty("CanPowerdown").asBoolean())
    return false;

  CallbackVector::iterator itr;
  CallbackVector::iterator itrEnd = m_callbacks.end();
  for (itr = m_callbacks.begin(); itr != itrEnd; itr++)
  {
    if (!(*itr)->OnBeforePowerdown())
      return false;
  } 
  
  for (itr = m_callbacks.begin(); itr != itrEnd; itr++)
    (*itr)->OnPowerdown();
    
  return m_power->Powerdown();
}
  
bool CPowerService::Reboot()
{
  if (!GetProperty("CanReboot").asBoolean())
    return false;

  CallbackVector::iterator itr;
  CallbackVector::iterator itrEnd = m_callbacks.end();
  for (itr = m_callbacks.begin(); itr != itrEnd; itr++)
  {
    if (!(*itr)->OnBeforeReboot())
      return false;
  } 
  
  for (itr = m_callbacks.begin(); itr != itrEnd; itr++)
    (*itr)->OnReboot();
    
  return m_power->Reboot();
}
  
bool CPowerService::Hibernate()
{
  if (!GetProperty("CanHibernate").asBoolean())
    return false;

  CallbackVector::iterator itr;
  CallbackVector::iterator itrEnd = m_callbacks.end();
  for (itr = m_callbacks.begin(); itr != itrEnd; itr++)
  {
    if (!(*itr)->OnBeforeHibernate())
      return false;
  } 
  
  for (itr = m_callbacks.begin(); itr != itrEnd; itr++)
    (*itr)->OnHibernate();
    
  return m_power->Hibernate();
}
  
bool CPowerService::Suspend()
{
  if (!GetProperty("CanSuspend").asBoolean())
    return false;

  CallbackVector::iterator itr;
  CallbackVector::iterator itrEnd = m_callbacks.end();
  for (itr = m_callbacks.begin(); itr != itrEnd; itr++)
  {
    if (!(*itr)->OnBeforeSuspend())
      return false;
  } 
  
  for (itr = m_callbacks.begin(); itr != itrEnd; itr++)
    (*itr)->OnSuspend();
    
  return m_power->Suspend();
}

void CPowerService::ProcessEvents()
{
  SetProperty("BatteryLevel", m_power->BatteryLevel());
  m_power->PumpPowerEvents(this);
}

void CPowerService::OnSleep()
{
  CLog::Log(LOGNOTICE, "%s: Running sleep jobs", __FUNCTION__);
  VOID_SIGNAL OnSuspend();
}

void CPowerService::OnWake()
{
  CLog::Log(LOGNOTICE, "%s: Running resume jobs", __FUNCTION__);
  VOID_SIGNAL OnWake();
}

void CPowerService::OnLowBattery()
{
  CLog::Log(LOGNOTICE, "%s: Running low battery jobs", __FUNCTION__);
  VOID_SIGNAL OnLowBattery();
}
