/*
 *      Copyright (C) 2005-2009 Team XBMC
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
#include <osdefs.h>

#include "system.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "utils/URIUtils.h"

#include "XBPyThread.h"
#include "XBPython.h"

#include "xbmcmodule/pyutil.h"

XBPyThread::XBPyThread(XBPython *pExecuter, ADDON::AddonPtr addon /* = ADDON::AddonPtr() */)
  : CThread("XBPyThread"),
    CPythonInterpreter(addon)
{
  CLog::Log(LOGDEBUG,"new python thread created. id=%d", m_id);
  m_pExecuter = pExecuter;

  setLock(&m_pExecuter->m_critSection);
}

XBPyThread::~XBPyThread()
{
  Stop();
  g_pythonParser.PulseGlobalEvent();
  CLog::Log(LOGDEBUG,"waiting for python thread %d to stop", m_id);
  StopThread();
  CLog::Log(LOGDEBUG,"python thread %d destructed", m_id);
}

void XBPyThread::Process()
{
  run();
}

void XBPyThread::OnExit()
{
  m_pExecuter->setDone(m_id);
}

void XBPyThread::OnException()
{
  PyThreadState_Swap(NULL);
  PyEval_ReleaseLock();

  CSingleLock lock(getLock());
  m_threadState = NULL;
  CLog::Log(LOGERROR,"%s, abnormally terminating python thread", __FUNCTION__);
  m_pExecuter->setDone(m_id);
}

bool XBPyThread::onExecute()
{
  Create();
  return true;
}

void XBPyThread::onInitialization()
{
  m_pExecuter->InitializeInterpreter(m_addon);
}

void XBPyThread::onError(const std::string &type, const std::string &value, const std::string &traceback)
{
  PYXBMC::PyXBMCGUILock();
  CGUIDialogKaiToast *pDlgToast = (CGUIDialogKaiToast*)g_windowManager.GetWindow(WINDOW_DIALOG_KAI_TOAST);
  if (pDlgToast)
  {
    CStdString desc;
    CStdString path;
    CStdString script;
    URIUtils::Split(m_script, path, script);
    if (script.Equals("default.py"))
    {
      CStdString path2;
      URIUtils::RemoveSlashAtEnd(path);
      URIUtils::Split(path, path2, script);
    }

    desc.Format(g_localizeStrings.Get(2100), script);
    pDlgToast->QueueNotification(CGUIDialogKaiToast::Error, g_localizeStrings.Get(257), desc);
  }
  PYXBMC::PyXBMCGUIUnlock();
}

void XBPyThread::onDeinitialization()
{
  m_pExecuter->DeInitializeInterpreter();
}
