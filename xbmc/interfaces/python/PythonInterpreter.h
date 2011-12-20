#pragma once
/*
 *      Copyright (C) 2011 Team XBMC
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

#include <string>
#include <vector>

#include "addons/IAddon.h"
#include "threads/CriticalSection.h"
#include "threads/Event.h"
#include "utils/StdString.h"

class LibraryLoader;

class CPythonInterpreter
{
public:
  CPythonInterpreter(ADDON::AddonPtr addon = ADDON::AddonPtr());
  virtual ~CPythonInterpreter();

  int GetId() { return m_id; }

  bool ExecuteFile(const std::string &path, std::vector<CStdString> argv = std::vector<CStdString>());
  bool ExecuteScript(const std::string &script, std::vector<CStdString> argv = std::vector<CStdString>());

  bool IsRunning() const { return m_state == ScriptStateRunning; }
  bool IsStopping() const { return m_state == ScriptStateStopping; }
  bool Stop();

  static bool IsInitialized() { return s_initialized; }
  static bool Initialize();
  static void Finalize();

  static void RegisterExtensionLibrary(LibraryLoader *pLib);
  static void UnregisterExtensionLibrary(LibraryLoader *pLib);

protected:
  inline CCriticalSection& getLock() const { return *m_critSection; }
  void setLock(CCriticalSection *critSection);
  void addPath(const std::string path);
  bool run();

  virtual bool onExecute() { return run(); }
  virtual void onInitialization() { }
  // actually a PyObject* but don't wanna draw Python.h include into the header
  virtual void onPythonModuleInitialization(void* moduleDict);
  virtual void onSuccess() { }
  virtual void onAbort() { }
  virtual void onError(const std::string &type, const std::string &value, const std::string &traceback) { }
  virtual void onDeinitialization() { }

  typedef enum ScriptType
  {
    ScriptTypeFile,
    ScriptTypeText
  } ScriptType;

  typedef enum ScriptState
  {
    ScriptStateUninitialized,
    ScriptStateInitialized,
    ScriptStateRunning,
    ScriptStateStopping,
    ScriptStateDone,
    ScriptStateFailed
  } ScriptState;

  int m_id;
  ScriptState m_state;
  ScriptType m_type;

  std::string m_pythonPath;
  char *m_script;
  int m_argc;
  char **m_argv;
  ADDON::AddonPtr m_addon;

  bool m_stop;
  CEvent m_stoppedEvent;
  void *m_threadState;

  static bool s_initialized;

private:
  void setScript(const std::string &script);
  void setArgv(std::vector<CStdString> argv);

  static void unloadExtensionLibraries();

  CCriticalSection *m_critSection;
  bool m_customLock;

  static int s_nextId;
  static CCriticalSection s_critSection;
  static void* s_mainThreadState;

  // in order to finalize and unload the python library, need to save all the extension libraries that are
  // loaded by it and unload them first (not done by finalize)
  typedef std::vector<LibraryLoader*> PythonExtensionLibraries;
  static PythonExtensionLibraries s_extensions;
};
