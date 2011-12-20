#pragma once

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

#include "XBPyThread.h"
#include "cores/IPlayer.h"
#include "threads/CriticalSection.h"
#include "addons/IAddon.h"

#include <vector>

typedef struct {
  int id;
  bool bDone;
  std::string strFile;
  XBPyThread *pyThread;
}PyElem;

typedef std::vector<PyElem> PyList;
typedef std::vector<PVOID> PlayerCallbackList;

class XBPython : public IPlayerCallback
{
public:
  XBPython();
  virtual ~XBPython();
  virtual void OnPlayBackEnded();
  virtual void OnPlayBackStarted();
  virtual void OnPlayBackPaused();
  virtual void OnPlayBackResumed();
  virtual void OnPlayBackStopped();
  virtual void OnQueueNextItem() {};
  void RegisterPythonPlayerCallBack(IPlayerCallback* pCallback);
  void UnregisterPythonPlayerCallBack(IPlayerCallback* pCallback);
  void FreeResources();
  void Process();

  void PulseGlobalEvent();
  void WaitForEvent(CEvent& hEvent, unsigned int timeout);

  int ScriptsSize();
  int GetPythonScriptId(int scriptPosition);
  int evalFile(const CStdString &src, ADDON::AddonPtr addon);
  int evalFile(const CStdString &src, const std::vector<CStdString> &argv, ADDON::AddonPtr addon);
  int evalString(const CStdString &src, const std::vector<CStdString> &argv);

  bool isRunning(int scriptId);
  bool isStopping(int scriptId);
  void setDone(int id);
  
  /*! \brief Stop a script if it's running
   \param path path to the script
   \return true if the script was running and is now stopped, false otherwise
   */
  bool StopScript(const CStdString &path);

  // inject xbmc stuff into the interpreter.
  // should be called for every new interpreter
  void InitializeInterpreter(ADDON::AddonPtr addon);

  // remove modules and references when interpreter done
  void DeInitializeInterpreter();

  //only should be called from thread which is running the script
  void  stopScript(int scriptId);

  // returns NULL if script doesn't exist or if script doesn't have a filename
  const char* getFileName(int scriptId);

  // returns -1 if no scripts exist with specified filename
  int getScriptId(const CStdString &strFile);

  static void InitializeModules();
  static void DeinitializeModules();

  bool m_bLogin;
  CCriticalSection    m_critSection;

private:
  ThreadIdentifier  m_ThreadId;

  //Vector with list of threads used for running scripts
  PyList              m_vecPyList;
  PlayerCallbackList  m_vecPlayerCallbackList;
  LibraryLoader*      m_pDll;

  // any global events that scripts should be using
  CEvent m_globalEvent;
};

extern XBPython g_pythonParser;
