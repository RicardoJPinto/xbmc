/*
 *      Copyright (C) 2005-2008 Team XBMC
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

#if defined(HAVE_CONFIG_H) && !defined(TARGET_WINDOWS)
  #include "config.h"
#endif

// python.h should always be included first before any other includes
#include <Python.h>

#include "system.h"
#include "XBPython.h"
#include "Util.h"
#include "addons/Addon.h"
#include "filesystem/File.h"
#include "filesystem/SpecialProtocol.h"
#include "settings/Settings.h"
#include "threads/SingleLock.h"
#include "utils/log.h"

extern "C" HMODULE __stdcall dllLoadLibraryA(LPCSTR file);
extern "C" BOOL __stdcall dllFreeLibrary(HINSTANCE hLibModule);

extern "C" {
  void InitXBMCModule(void);
  void DeinitXBMCModule(void);
  void InitPluginModule(void);
  void DeinitPluginModule(void);
  void InitGUIModule(void);
  void DeinitGUIModule(void);
  void InitAddonModule(void);
  void DeinitAddonModule(void);
  void InitVFSModule(void);
  void DeinitVFSModule(void);
}

XBPython::XBPython()
{
  m_bLogin            = false;
  m_ThreadId          = CThread::GetCurrentThreadId();
  m_vecPlayerCallbackList.clear();
}

XBPython::~XBPython()
{ }

// message all registered callbacks that xbmc stopped playing
void XBPython::OnPlayBackEnded()
{
  CSingleLock lock(m_critSection);
  if (CPythonInterpreter::IsInitialized())
  {
    PlayerCallbackList::iterator it = m_vecPlayerCallbackList.begin();
    while (it != m_vecPlayerCallbackList.end())
    {
      ((IPlayerCallback*)(*it))->OnPlayBackEnded();
      it++;
    }
  }
}

// message all registered callbacks that we started playing
void XBPython::OnPlayBackStarted()
{
  CSingleLock lock(m_critSection);
  if (CPythonInterpreter::IsInitialized())
  {
    PlayerCallbackList::iterator it = m_vecPlayerCallbackList.begin();
    while (it != m_vecPlayerCallbackList.end())
    {
      ((IPlayerCallback*)(*it))->OnPlayBackStarted();
      it++;
    }
  }
}

// message all registered callbacks that we paused playing
void XBPython::OnPlayBackPaused()
{
  CSingleLock lock(m_critSection);
  if (CPythonInterpreter::IsInitialized())
  {
    PlayerCallbackList::iterator it = m_vecPlayerCallbackList.begin();
    while (it != m_vecPlayerCallbackList.end())
    {
      ((IPlayerCallback*)(*it))->OnPlayBackPaused();
      it++;
    }
  }
}

// message all registered callbacks that we resumed playing
void XBPython::OnPlayBackResumed()
{
  CSingleLock lock(m_critSection);
  if (CPythonInterpreter::IsInitialized())
  {
    PlayerCallbackList::iterator it = m_vecPlayerCallbackList.begin();
    while (it != m_vecPlayerCallbackList.end())
    {
      ((IPlayerCallback*)(*it))->OnPlayBackResumed();
      it++;
    }
  }
}

// message all registered callbacks that user stopped playing
void XBPython::OnPlayBackStopped()
{
  CSingleLock lock(m_critSection);
  if (CPythonInterpreter::IsInitialized())
  {
    PlayerCallbackList::iterator it = m_vecPlayerCallbackList.begin();
    while (it != m_vecPlayerCallbackList.end())
    {
      ((IPlayerCallback*)(*it))->OnPlayBackStopped();
      it++;
    }
  }
}

void XBPython::RegisterPythonPlayerCallBack(IPlayerCallback* pCallback)
{
  CSingleLock lock(m_critSection);
  m_vecPlayerCallbackList.push_back(pCallback);
}

void XBPython::UnregisterPythonPlayerCallBack(IPlayerCallback* pCallback)
{
  CSingleLock lock(m_critSection);
  PlayerCallbackList::iterator it = m_vecPlayerCallbackList.begin();
  while (it != m_vecPlayerCallbackList.end())
  {
    if (*it == pCallback)
      it = m_vecPlayerCallbackList.erase(it);
    else
      it++;
  }
}

#define RUNSCRIPT_PRAMBLE \
        "" \
        "import xbmc\n" \
        "class xbmcout:\n" \
        "\tdef __init__(self, loglevel=xbmc.LOGNOTICE):\n" \
        "\t\tself.ll=loglevel\n" \
        "\tdef write(self, data):\n" \
        "\t\txbmc.log(data,self.ll)\n" \
        "\tdef close(self):\n" \
        "\t\txbmc.log('.')\n" \
        "\tdef flush(self):\n" \
        "\t\txbmc.log('.')\n" \
        "import sys\n" \
        "sys.stdout = xbmcout()\n" \
        "sys.stderr = xbmcout(xbmc.LOGERROR)\n"

#define RUNSCRIPT_OVERRIDE_HACK \
        "" \
        "import os\n" \
        "def getcwd_xbmc():\n" \
        "  import __main__\n" \
        "  import warnings\n" \
        "  if hasattr(__main__, \"__file__\"):\n" \
        "    warnings.warn(\"os.getcwd() currently lies to you so please use addon.getAddonInfo('path') to find the script's root directory and DO NOT make relative path accesses based on the results of 'os.getcwd.' \", DeprecationWarning, stacklevel=2)\n" \
        "    return os.path.dirname(__main__.__file__)\n" \
        "  else:\n" \
        "    return os.getcwd_original()\n" \
        "" \
        "def chdir_xbmc(dir):\n" \
        "  raise RuntimeError(\"os.chdir not supported in xbmc\")\n" \
        "" \
        "os_getcwd_original = os.getcwd\n" \
        "os.getcwd          = getcwd_xbmc\n" \
        "os.chdir_orignal   = os.chdir\n" \
        "os.chdir           = chdir_xbmc\n" \
        ""
 
#define RUNSCRIPT_POSTSCRIPT \
        "print '-->Python Interpreter Initialized<--'\n" \
        ""

#define RUNSCRIPT_BWCOMPATIBLE \
  RUNSCRIPT_PRAMBLE RUNSCRIPT_OVERRIDE_HACK RUNSCRIPT_POSTSCRIPT

#define RUNSCRIPT_COMPLIANT \
  RUNSCRIPT_PRAMBLE RUNSCRIPT_POSTSCRIPT

void XBPython::InitializeInterpreter(ADDON::AddonPtr addon)
{
  InitializeModules();

  CStdString addonVer = ADDON::GetXbmcApiVersionDependency(addon);
  bool bwcompatMode = (addon.get() == NULL || (ADDON::AddonVersion(addonVer) <= ADDON::AddonVersion("1.0")));
  const char* runscript = bwcompatMode ? RUNSCRIPT_BWCOMPATIBLE : RUNSCRIPT_COMPLIANT;

  // redirecting default output to debug console
  if (PyRun_SimpleString(runscript) == -1)
    CLog::Log(LOGFATAL, "Python Initialize Error");
}

void XBPython::DeInitializeInterpreter()
{
  DeinitializeModules();
}

void XBPython::InitializeModules()
{
  InitXBMCModule(); // init xbmc modules
  InitPluginModule(); // init xbmcplugin modules
  InitGUIModule(); // init xbmcgui modules
  InitAddonModule(); // init xbmcaddon modules
  InitVFSModule(); // init xbmcvfs modules
}

void XBPython::DeinitializeModules()
{
  DeinitXBMCModule();
  DeinitPluginModule();
  DeinitGUIModule();
  DeinitAddonModule();
  DeinitVFSModule();
}

void XBPython::FreeResources()
{
  CSingleLock lock(m_critSection);
  if (CPythonInterpreter::IsInitialized())
  {
    // cleanup threads that are still running
    PyList::iterator it = m_vecPyList.begin();
    while (it != m_vecPyList.end())
    {
      lock.Leave(); //unlock here because the python thread might lock when it exits
      delete it->pyThread;
      lock.Enter();
      it = m_vecPyList.erase(it);
    }
  }
}

void XBPython::Process()
{
  if (m_bLogin)
  {
    m_bLogin = false;

    // autoexec.py - profile
    CStdString strAutoExecPy = CSpecialProtocol::TranslatePath("special://profile/autoexec.py");

    if ( XFILE::CFile::Exists(strAutoExecPy) )
      evalFile(strAutoExecPy,ADDON::AddonPtr());
    else
      CLog::Log(LOGDEBUG, "%s - no profile autoexec.py (%s) found, skipping", __FUNCTION__, strAutoExecPy.c_str());
  }

  CSingleLock lock(m_critSection);

  if (CPythonInterpreter::IsInitialized())
  {
    PyList::iterator it = m_vecPyList.begin();
    while (it != m_vecPyList.end())
    {
      //delete scripts which are done
      if (it->bDone)
      {
        delete it->pyThread;
        it = m_vecPyList.erase(it);
      }
      else ++it;
    }
  }
}

bool XBPython::StopScript(const CStdString &path)
{
  int id = getScriptId(path);
  if (id != -1)
  {
    /* if we are here we already know that this script is running.
     * But we will check it again to be sure :)
     */
    if (isRunning(id))
    {
      stopScript(id);
      return true;
    }
  }
  return false;
}

int XBPython::evalFile(const CStdString &src, ADDON::AddonPtr addon)
{
  std::vector<CStdString> argv;
  return evalFile(src, argv, addon);
}

// execute script, returns -1 if script doesn't exist
int XBPython::evalFile(const CStdString &src, const std::vector<CStdString> &argv, ADDON::AddonPtr addon)
{
  CSingleExit ex(g_graphicsContext);
  // return if file doesn't exist
  if (!XFILE::CFile::Exists(src))
  {
    CLog::Log(LOGERROR, "Python script \"%s\" does not exist", CSpecialProtocol::TranslatePath(src).c_str());
    return -1;
  }

  // check if locked
  if (g_settings.GetCurrentProfile().programsLocked() && !g_passwordManager.IsMasterLockUnlocked(true))
    return -1;

  CSingleLock lock(m_critSection);
  XBPyThread *pyThread = new XBPyThread(this, addon);
  pyThread->ExecuteFile(src, argv);
  PyElem inf;
  inf.id        = pyThread->GetId();
  inf.bDone     = false;
  inf.strFile   = src;
  inf.pyThread  = pyThread;

  m_vecPyList.push_back(inf);

  return pyThread->GetId();
}

void XBPython::setDone(int id)
{
  CSingleLock lock(m_critSection);
  PyList::iterator it = m_vecPyList.begin();
  while (it != m_vecPyList.end())
  {
    if (it->id == id)
    {
      if (it->pyThread->IsStopping())
        CLog::Log(LOGINFO, "Python script interrupted by user");
      else
        CLog::Log(LOGINFO, "Python script stopped");
      it->bDone = true;
    }
    ++it;
  }
}

void XBPython::stopScript(int id)
{
  CSingleExit ex(g_graphicsContext);
  CSingleLock lock(m_critSection);
  PyList::iterator it = m_vecPyList.begin();
  while (it != m_vecPyList.end())
  {
    if (it->id == id) {
      CLog::Log(LOGINFO, "Stopping script with id: %i", id);
      it->pyThread->Stop();
      return;
    }
    ++it;
  }
}

int XBPython::ScriptsSize()
{
  CSingleLock lock(m_critSection);
  return m_vecPyList.size();
}

const char* XBPython::getFileName(int scriptId)
{
  const char* cFileName = NULL;

  CSingleLock lock(m_critSection);
  PyList::iterator it = m_vecPyList.begin();
  while (it != m_vecPyList.end())
  {
    if (it->id == scriptId)
      cFileName = it->strFile.c_str();
    ++it;
  }

  return cFileName;
}

int XBPython::getScriptId(const CStdString &strFile)
{
  int iId = -1;

  CSingleLock lock(m_critSection);

  PyList::iterator it = m_vecPyList.begin();
  while (it != m_vecPyList.end())
  {
    if (it->strFile == strFile)
      iId = it->id;
    ++it;
  }

  return iId;
}

bool XBPython::isRunning(int scriptId)
{
  CSingleLock lock(m_critSection);

  for(PyList::iterator it = m_vecPyList.begin(); it != m_vecPyList.end(); it++)
  {
    if (it->id == scriptId)
    {
      if(it->bDone)
        return false;
      else
        return true;
    }
  }
  return false;
}

bool XBPython::isStopping(int scriptId)
{
  bool bStopping = false;

  CSingleLock lock(m_critSection);
  PyList::iterator it = m_vecPyList.begin();
  while (it != m_vecPyList.end())
  {
    if (it->id == scriptId)
      bStopping = it->pyThread->IsStopping();
    ++it;
  }

  return bStopping;
}

int XBPython::GetPythonScriptId(int scriptPosition)
{
  CSingleLock lock(m_critSection);
  return (int)m_vecPyList[scriptPosition].id;
}

void XBPython::PulseGlobalEvent()
{
  m_globalEvent.Set();
}

void XBPython::WaitForEvent(CEvent& hEvent, unsigned int timeout)
{
  // wait for either this event our our global event
  XbmcThreads::CEventGroup eventGroup(&hEvent, &m_globalEvent, NULL);
  eventGroup.wait(timeout);
  m_globalEvent.Reset();
}

// execute script, returns -1 if script doesn't exist
int XBPython::evalString(const CStdString &src, const std::vector<CStdString> &argv)
{
  CLog::Log(LOGDEBUG, "XBPython::evalString (python)");
  CSingleLock lock(m_critSection);

  // Previous implementation would create a new thread for every script
  XBPyThread *pyThread = new XBPyThread(this);
  pyThread->ExecuteScript(src, argv);

  PyElem inf;
  inf.id        = pyThread->GetId();
  inf.bDone     = false;
  inf.strFile   = "<string>";
  inf.pyThread  = pyThread;

  m_vecPyList.push_back(inf);

  return pyThread->GetId();
}
