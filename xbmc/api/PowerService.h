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

#include "powermanagement/IPowerSyscall.h"

// For systems without PowerSyscalls we have a NullObject
class CNullPowerSyscall : public IPowerSyscall
{
public:
  virtual bool Powerdown()    { return false; }
  virtual bool Suspend()      { return false; }
  virtual bool Hibernate()    { return false; }
  virtual bool Reboot()       { return false; }

  virtual bool CanPowerdown() { return true; }
  virtual bool CanSuspend()   { return true; }
  virtual bool CanHibernate() { return true; }
  virtual bool CanReboot()    { return true; }

  virtual int  BatteryLevel() { return 0; }

  virtual bool PumpPowerEvents(IPowerEventsCallback *callback) { return false; }
};

class CPowerService;

/*!
 \ingroup services
 \brief Service callback of the power service

 Service callback of the power service providing
 events before and on powerdown, reboot, hibernate
 and suspend and on wake and low battery.
 */
class CPowerServiceCallback : public CServiceBaseCallback<CPowerService, CPowerServiceCallback>
{
public:
  CPowerServiceCallback();
  
  /*!
   \brief Event triggered before the system is
   powered down. Allows listeners to stop the process.
   \return Whether or not to power down the system
   */
  virtual bool OnBeforePowerdown() { return true; }
  /*!
   \brief Event triggered before the system is
   rebooted. Allows listeners to stop the process.
   \return Whether or not to reboot the system
   */
  virtual bool OnBeforeReboot() { return true; }
  /*!
   \brief Event triggered before the system is
   hibernated. Allows listeners to stop the process.
   \return Whether or not to hibernate the system
   */
  virtual bool OnBeforeHibernate() { return true; }
  /*!
   \brief Event triggered before the system is
   suspended. Allows listeners to stop the process.
   \return Whether or not to suspend the system
   */
  virtual bool OnBeforeSuspend() { return true; }
  
  /*!
   \brief Event triggered when the system is
   powering down.
   */
  virtual void OnPowerdown() { };
  /*!
   \brief Event triggered when the system is
   rebooting.
   */
  virtual void OnReboot() { };
  /*!
   \brief Event triggered when the system is
   going into hibernation.
   */
  virtual void OnHibernate() { };
  /*!
   \brief Event triggered when the system is
   being suspended.
   */
  virtual void OnSuspend() { };
  /*!
   \brief Event triggered when the system is
   waking from sleep (hibernation or suspension).
   */
  virtual void OnWake() { };
  /*!
   \brief Event triggered when the system is
   running low on battery.
   */
  virtual void OnLowBattery() { };
};

/*!
 \ingroup services
 \brief Service to control the power state of the system
 */
class CPowerService : public CServiceBase<CPowerService, CPowerServiceCallback>, public IPowerEventsCallback
{
private:
  CPowerService();
  ~CPowerService();
  static CPowerService *GetInstance();

public:
  /*!
   \brief Sets the default shutdown action
   */
  void SetDefaults();

  /*!
   \brief Powers the system down.
   \return Whether or not the system is powering down
   */
  bool Powerdown();
  /*!
   \brief Reboots the system.
   \return Whether or not the system is rebooting
   */
  bool Reboot();
  /*!
   \brief Puts the system in hibernation.
   \return Whether or not the system is being hibernated
   */
  bool Hibernate();
  /*!
   \brief Suspends the system.
   \return Whether or not the system is suspending
   */
  bool Suspend();

  /*!
   \brief Process system-specific power events
   (e.g. wakeup)
   */
  void ProcessEvents();

private:
  void Initialize();

  void OnSleep();
  void OnWake();
  void OnLowBattery();

  IPowerSyscall *m_power;
  
  static CPowerService *m_instance;

  friend CPowerService *CServiceProxy<CPowerService>::GetInstance();
};

template<>
inline
CPowerService *CServiceProxy<CPowerService>::GetInstance()
{
  return CPowerService::GetInstance();
}
