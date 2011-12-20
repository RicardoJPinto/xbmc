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

// python.h should always be included first before any other includes
#include <Python.h>
#include <osdefs.h>

#include "PythonInterpreter.h"
#include "Util.h"
#include "addons/AddonManager.h"
#include "cores/DllLoader/DllLoaderContainer.h"
#include "filesystem/SpecialProtocol.h"
#include "filesystem/File.h"
#include "interfaces/python/xbmcmodule/pythreadstate.h"
#include "interfaces/python/xbmcmodule/pyutil.h"
#include "threads/SingleLock.h"
#include "utils/CharsetConverter.h"
#include "utils/log.h"
#include "utils/URIUtils.h"

#define PY_PATH_SEP DELIM

extern "C" {
  void InitXBMCTypes(void);
  void InitPluginTypes(void);
  void InitGUITypes(void);
  void InitAddonTypes(void);
  void InitHttpTypes(void);
}

using namespace std;

bool CPythonInterpreter::s_initialized = false;
int CPythonInterpreter::s_nextId = 0;
CCriticalSection CPythonInterpreter::s_critSection;
void* CPythonInterpreter::s_mainThreadState = NULL;
CPythonInterpreter::PythonExtensionLibraries CPythonInterpreter::s_extensions;

CPythonInterpreter::CPythonInterpreter(ADDON::AddonPtr addon /* = ADDON::AddonPtr() */)
  : m_state(ScriptStateUninitialized),
    m_script(NULL),
    m_argc(0),
    m_argv(NULL),
    m_addon(addon),
    m_stop(false),
    m_threadState(NULL),
    m_critSection(new CCriticalSection()),
    m_customLock(false)
{
  m_id = s_nextId;
  s_nextId++;
}

CPythonInterpreter::~CPythonInterpreter()
{
  Stop();
  delete[] m_script;
  if (m_argv)
  {
    for (int i = 0; i < m_argc; i++)
      delete[] m_argv[i];
    delete[] m_argv;
  }

  if (!m_customLock)
    delete m_critSection;
}

bool CPythonInterpreter::ExecuteFile(const std::string &path, std::vector<CStdString> argv /* = std::vector<CStdString>() */)
{
  if (m_state != ScriptStateUninitialized || m_threadState != NULL || path.empty())
    return false;

  if (!Initialize())
  {
    Finalize();
    return false;
  }

  if (!s_initialized)
    return false;

  m_type = ScriptTypeFile;
  setScript(path);
  setArgv(argv);
  return onExecute();
}

bool CPythonInterpreter::ExecuteScript(const std::string &script, std::vector<CStdString> argv /* = std::vector<CStdString>() */)
{
  if (m_state != ScriptStateUninitialized || m_threadState != NULL || script.empty())
    return false;

  if (!Initialize())
  {
    Finalize();
    return false;
  }

  if (!s_initialized)
    return false;

  m_type = ScriptTypeText;
  setScript(script);
  setArgv(argv);
  return onExecute();
}

bool CPythonInterpreter::Stop()
{
  CSingleLock lock(getLock());
  if (m_state != ScriptStateRunning)
    return false;

  m_stop = true;

  if (m_threadState)
  {
    PyEval_AcquireLock();
    PyThreadState* old = PyThreadState_Swap((PyThreadState *)m_threadState);

    PyObject *m;
    m = PyImport_AddModule((char *)"xbmc");
    if(!m || PyObject_SetAttrString(m, (char *)"abortRequested", PyBool_FromLong(1)))
      CLog::Log(LOGERROR, "PythonInterpreter: failed to set abortRequested");

    PyThreadState_Swap(old);
    PyEval_ReleaseLock();

    if(!m_stoppedEvent.WaitMSec(5000))//let the script 5 secs for shut stuff down
    {
      CLog::Log(LOGERROR, "PythonInterpreter: script didn't stop in proper time - lets kill it");
    }
    
    //everything which didn't exit by now gets killed
    PyEval_AcquireLock();
    old = PyThreadState_Swap((PyThreadState*)m_threadState);    
    for(PyThreadState* state = ((PyThreadState*)m_threadState)->interp->tstate_head; state; state = state->next)
    {
      Py_XDECREF(state->async_exc);
      state->async_exc = PyExc_SystemExit;
      Py_XINCREF(state->async_exc);
    }

    PyThreadState_Swap(old);
    PyEval_ReleaseLock();
  }

  return true;
}

bool CPythonInterpreter::Initialize()
{
  CLog::Log(LOGINFO, "PythonInterpreter: initializing python engine");
  CSingleLock lock(s_critSection);
  
  if (!s_initialized)
  {
      // first we check if all necessary files are installed
#ifndef TARGET_LINUX
      if(!XFILE::CFile::Exists("special://xbmc/system/python/DLLs/_socket.pyd") ||
         !XFILE::CFile::Exists("special://xbmc/system/python/DLLs/_ssl.pyd") ||
         !XFILE::CFile::Exists("special://xbmc/system/python/DLLs/bz2.pyd") ||
         !XFILE::CFile::Exists("special://xbmc/system/python/DLLs/pyexpat.pyd") ||
         !XFILE::CFile::Exists("special://xbmc/system/python/DLLs/select.pyd") ||
         !XFILE::CFile::Exists("special://xbmc/system/python/DLLs/unicodedata.pyd"))
      {
        CLog::Log(LOGERROR, "PythonInterpreter: Missing files, unable to execute script");
        return false;
      }
#endif

      // Info about interesting python envvars available
      // at http://docs.python.org/using/cmdline.html#environment-variables
#ifndef TARGET_WINDOWS
      /* PYTHONOPTIMIZE is set off intentionally when using external Python.
         Reason for this is because we cannot be sure what version of Python
         was used to compile the various Python object files (i.e. .pyo,
         .pyc, etc.). */
        // check if we are running as real xbmc.app or just binary
      if (!CUtil::GetFrameworksPath(true).IsEmpty())
      {
        // using external python, it's build looking for xxx/lib/python2.6
        // so point it to frameworks which is where python2.6 is located
        setenv("PYTHONHOME", CSpecialProtocol::TranslatePath("special://frameworks").c_str(), 1);
        setenv("PYTHONPATH", CSpecialProtocol::TranslatePath("special://frameworks").c_str(), 1);
        CLog::Log(LOGDEBUG, "PythonInterpreter: PYTHONHOME -> %s", CSpecialProtocol::TranslatePath("special://frameworks").c_str());
        CLog::Log(LOGDEBUG, "PythonInterpreter: PYTHONPATH -> %s", CSpecialProtocol::TranslatePath("special://frameworks").c_str());
      }
      setenv("PYTHONCASEOK", "1", 1); //This line should really be removed
#else
      // because the third party build of python is compiled with vs2008 we need
      // a hack to set the PYTHONPATH
      // buf is corrupted after putenv and might need a strdup but it seems to
      // work this way
      CStdString buf;
      buf = "PYTHONPATH=" + CSpecialProtocol::TranslatePath("special://xbmc/system/python/DLLs") + ";" + CSpecialProtocol::TranslatePath("special://xbmc/system/python/Lib");
      pgwin32_putenv(buf.c_str());
      buf = "PYTHONOPTIMIZE=1";
      pgwin32_putenv(buf.c_str());
      buf = "PYTHONHOME=" + CSpecialProtocol::TranslatePath("special://xbmc/system/python");
      pgwin32_putenv(buf.c_str());
      buf = "OS=win32";
      pgwin32_putenv(buf.c_str());
#endif

      if (PyEval_ThreadsInitialized())
        PyEval_AcquireLock();
      else
        PyEval_InitThreads();

      Py_Initialize();
      PyEval_ReleaseLock();

      // If this is not the first time we initialize Python, the interpreter
      // lock already exists and we need to lock it as PyEval_InitThreads
      // would not do that in that case.
      PyEval_AcquireLock();
      char* python_argv[1] = { (char*)"" } ;
      PySys_SetArgv(1, python_argv);

      InitXBMCTypes();
      InitGUITypes();
      InitPluginTypes();
      InitAddonTypes();

      if (!(s_mainThreadState = PyThreadState_Get()))
        CLog::Log(LOGERROR, "PythonInterpreter: threadstate is NULL");
      PyEval_ReleaseLock();

      s_initialized = true;
  }

  return s_initialized;
}

void CPythonInterpreter::Finalize()
{
  if (s_initialized)
  {
    CLog::Log(LOGINFO, "PythonInterpreter: unloading python shared library because no scripts are running anymore");

    PyEval_AcquireLock();
    PyThreadState_Swap((PyThreadState*)s_mainThreadState);

    Py_Finalize();
    PyEval_ReleaseLock();

#if !(defined(TARGET_DARWIN) || defined(TARGET_WINDOWS))
    unloadExtensionLibraries();
#endif

    // first free all dlls loaded by python, after that python24.dll (this is done by UnloadPythonDlls
#if !(defined(TARGET_DARWIN) || defined(TARGET_WINDOWS))
    DllLoaderContainer::UnloadPythonDlls();
#endif
#if defined(TARGET_LINUX) && !defined(TARGET_DARWIN)
    // we can't release it on windows, as this is done in UnloadPythonDlls() for win32 (see above).
    // The implementation for linux needs looking at - UnloadPythonDlls() currently only searches for "python24.dll"
    // The implementation for osx can never unload the python dylib.
    DllLoaderContainer::ReleaseModule(m_pDll);
#endif

    s_mainThreadState = NULL;
    s_initialized = false;
  }
}

void CPythonInterpreter::RegisterExtensionLibrary(LibraryLoader *pLib)
{
  if (pLib == NULL)
    return;

  CSingleLock lock(s_critSection);

  CLog::Log(LOGDEBUG, "PythonInterpreter: adding extension library %s (0x%p)", pLib->GetName(), (void*)pLib);
  s_extensions.push_back(pLib);
}

void CPythonInterpreter::UnregisterExtensionLibrary(LibraryLoader *pLib)
{
  if (pLib == NULL)
    return;

  CSingleLock lock(s_critSection);
  CLog::Log(LOGDEBUG,"PythonInterpreter: removing extension library %s (0x%p)", pLib->GetName(), (void *)pLib);
  for (PythonExtensionLibraries::iterator iter = s_extensions.begin(); iter != s_extensions.end(); iter++)
  {
    if (*iter == pLib)
    {
      s_extensions.erase(iter);
      break;
    }
  }
}

void CPythonInterpreter::setLock(CCriticalSection *critSection)
{
  if (critSection == NULL)
    return;

  if (!m_customLock)
    delete m_critSection;
  m_critSection = critSection;
  m_customLock = true;
}

void CPythonInterpreter::addPath(const std::string path)
{
  if (path.empty())
    return;

  if (!m_pythonPath.empty())
    m_pythonPath += PY_PATH_SEP;

#ifdef TARGET_WINDOWS
  CStdString tmp(path);
  g_charsetConverter.utf8ToSystem(tmp);
  m_pythonPath += tmp;
#else
  m_pythonPath += path;
#endif
}

void CPythonInterpreter::setScript(const std::string &script)
{
#ifdef TARGET_WINDOWS
  CStdString strScript = script;
  g_charsetConverter.utf8ToSystem(strScript);
  m_script = new char[strScript.size() + 1];
  strcpy(m_script, strScript.c_str());
#else
  m_script  = new char[script.size() + 1];
  strcpy(m_script, script.c_str());
#endif
}

void CPythonInterpreter::setArgv(std::vector<CStdString> argv)
{
  m_argc = argv.size();
  m_argv = new char*[m_argc];
  for(int i = 0; i < m_argc; i++)
  {
    m_argv[i] = new char[argv[i].size() + 1];
    strcpy(m_argv[i], argv[i].c_str());
  }
}

bool CPythonInterpreter::run()
{
  CLog::Log(LOGDEBUG, "PythonInterpreter: initializing");

  int pyInputFile = Py_file_input;

  // get the global lock
  PyEval_AcquireLock();
  PyThreadState* state = Py_NewInterpreter();
  if (!state)
  {
    PyEval_ReleaseLock();
    CLog::Log(LOGERROR, "PythonInterpreter: failed to get a new sub-interpreter");
    return false;
  }
  // swap in my thread state
  PyThreadState_Swap(state);

  onInitialization();
  m_state = ScriptStateInitialized;

  CStdString scriptDir;
  if (m_type == ScriptTypeFile)
  {
    // get path from script file name and add python path's
    // this is used for python so it will search modules from script path first
    URIUtils::GetDirectory(CSpecialProtocol::TranslatePath(m_script), scriptDir);
    URIUtils::RemoveSlashAtEnd(scriptDir);
  }

  addPath(scriptDir);

  // add on any addon modules the user has installed
  ADDON::VECADDONS addons;
  ADDON::CAddonMgr::Get().GetAddons(ADDON::ADDON_SCRIPT_MODULE, addons);
  for (unsigned int i = 0; i < addons.size(); ++i)
    addPath(CSpecialProtocol::TranslatePath(addons[i]->LibPath()));

  // we want to use sys.path so it includes site-packages
  // if this fails, default to using Py_GetPath
  PyObject *sysMod(PyImport_ImportModule((char*)"sys")); // must call Py_DECREF when finished
  PyObject *sysModDict(PyModule_GetDict(sysMod)); // borrowed ref, no need to delete
  PyObject *pathObj(PyDict_GetItemString(sysModDict, "path")); // borrowed ref, no need to delete

  if (pathObj && PyList_Check(pathObj))
  {
    for (int i = 0; i < PyList_Size(pathObj); i++)
    {
      PyObject *e = PyList_GetItem(pathObj, i); // borrowed ref, no need to delete
      if (e && PyString_Check(e))
        addPath(PyString_AsString(e)); // returns internal data, don't delete or modify
    }
  }
  else
    addPath(Py_GetPath());

  Py_DECREF(sysMod); // release ref to sysMod

  // set current directory and python's path.
  if (m_argv != NULL)
    PySys_SetArgv(m_argc, m_argv);

  CLog::Log(LOGDEBUG, "PythonInterpreter: setting the Python path to %s", m_pythonPath.c_str());
  PySys_SetPath((char *)m_pythonPath.c_str());

  CLog::Log(LOGDEBUG, "PythonInterpreter: entering script directory \"%s\"", scriptDir.c_str());
  PyObject* module = PyImport_AddModule((char*)"__main__");
  PyObject* moduleDict = PyModule_GetDict(module);

  // when we are done initing we store thread state so we can be aborted
  PyThreadState_Swap(NULL);
  PyEval_ReleaseLock();

  // we need to check if we was asked to abort before we had inited
  bool stopping = false;
  { CSingleLock lock(getLock());
    m_threadState = state;
    stopping = m_stop;
  }

  PyEval_AcquireLock();
  PyThreadState_Swap(state);

  if (!stopping)
  {
    if (m_type == ScriptTypeFile)
    {
      // run script from file
      // We need to have python open the file because on Windows the DLL that python
      // is linked against may not be the DLL that xbmc is linked against so
      // passing a FILE* to python from an fopen has the potential to crash.
      PyObject* file = PyFile_FromString((char *)CSpecialProtocol::TranslatePath(m_script).c_str(), (char *)"r");
      FILE *fp = PyFile_AsFile(file);

      if (fp)
      {
        PyObject *f = PyString_FromString(CSpecialProtocol::TranslatePath(m_script).c_str());
        PyDict_SetItemString(moduleDict, "__file__", f);
        onPythonModuleInitialization(moduleDict);

        Py_DECREF(f);
        m_state = ScriptStateRunning;
        PyRun_FileExFlags(fp, CSpecialProtocol::TranslatePath(m_script).c_str(), pyInputFile, moduleDict, moduleDict, 1, NULL);
      }
      else
      {
        m_state = ScriptStateFailed;
        CLog::Log(LOGERROR, "PythonInterpreter: script \"%s\" not found", m_script);
      }
    }
    //run script
    else
    {
      m_state = ScriptStateRunning;
      PyRun_String(m_script, pyInputFile, moduleDict, moduleDict);
    }
  }

  if (!PyErr_Occurred())
  {
    CLog::Log(LOGINFO, "PythonInterpreter: script successfully run");
    m_state = ScriptStateDone;
    onSuccess();
  }
  else
  {
    m_state = ScriptStateFailed;
    if (PyErr_ExceptionMatches(PyExc_SystemExit))
    {
      CLog::Log(LOGINFO, "PythonInterpreter: script aborted");
      onAbort();
    }
    else
    {
      PyObject* exc_type;
      PyObject* exc_value;
      PyObject* exc_traceback;
      PyObject* pystring;
      pystring = NULL;

      PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);
      if (exc_type == 0 && exc_value == 0 && exc_traceback == 0)
        CLog::Log(LOGINFO, "PythonInterpreter: no Python exception occured");
      else
      {
        if (exc_type != NULL && (pystring = PyObject_Str(exc_type)) != NULL && PyString_Check(pystring))
        {
          PyObject *tracebackModule;

          CLog::Log(LOGINFO, "-->Python script returned the following error<--");
          CLog::Log(LOGERROR, "PythonInterpreter: Error Type: %s", PyString_AsString(PyObject_Str(exc_type)));
          if (PyObject_Str(exc_value))
            CLog::Log(LOGERROR, "PythonInterpreter: Error Contents: %s", PyString_AsString(PyObject_Str(exc_value)));

          tracebackModule = PyImport_ImportModule((char *)"traceback");
          if (tracebackModule != NULL)
          {
            PyObject *tbList, *emptyString, *strRetval;

            tbList = PyObject_CallMethod(tracebackModule, (char *)"format_exception", (char *)"OOO", exc_type, exc_value == NULL ? Py_None : exc_value, exc_traceback == NULL ? Py_None : exc_traceback);
            emptyString = PyString_FromString("");
            strRetval = PyObject_CallMethod(emptyString, (char *)"join", (char *)"O", tbList);

            CLog::Log(LOGERROR, "%s", PyString_AsString(strRetval));

            onError(PyString_AsString(PyObject_Str(exc_type)), PyString_AsString(PyObject_Str(exc_value)), PyString_AsString(strRetval));

            Py_DECREF(tbList);
            Py_DECREF(emptyString);
            Py_DECREF(strRetval);
            Py_DECREF(tracebackModule);
          }
          CLog::Log(LOGINFO, "-->End of Python script error report<--");
        }
        else
        {
          pystring = NULL;
          CLog::Log(LOGINFO, "<unknown exception type>");
        }
      }

      Py_XDECREF(exc_type);
      Py_XDECREF(exc_value); // caller owns all 3
      Py_XDECREF(exc_traceback); // already NULL'd out
      Py_XDECREF(pystring);
    }
  }

  PyObject *m = PyImport_AddModule((char *)"xbmc");
  if(!m || PyObject_SetAttrString(m, (char *)"abortRequested", PyBool_FromLong(1)))
    CLog::Log(LOGERROR, "PythonInterpreter: failed to set abortRequested");

  // make sure all sub threads have finished
  for(PyThreadState *s = state->interp->tstate_head, *old = NULL; s; )
  {
    if (s == state)
    {
      s = s->next;
      continue;
    }
    if (old != s)
    {
      CLog::Log(LOGINFO, "PythonInterpreter: waiting on thread %"PRIu64, (uint64_t)s->thread_id);
      old = s;
    }

    CPyThreadState pyState;
    Sleep(100);
    pyState.Restore();

    s = state->interp->tstate_head;
  }

  // pending calls must be cleared out
  PyXBMC_ClearPendingCalls(state);

  PyThreadState_Swap(NULL);
  PyEval_ReleaseLock();

  //set stopped event - this allows ::stop to run and kill remaining threads
  //this event has to be fired without holding m_pExecuter->m_critSection
  //before
  //Also the GIL (PyEval_AcquireLock) must not be held
  //if not obeyed there is still no deadlock because ::stop waits with timeout (smart one!)
  m_stoppedEvent.Set();

  { CSingleLock lock(getLock());
    m_threadState = NULL;
  }

  PyEval_AcquireLock();
  PyThreadState_Swap(state);

  onDeinitialization();

  Py_EndInterpreter(state);
  PyThreadState_Swap(NULL);

  PyEval_ReleaseLock();

  return true;
}

void CPythonInterpreter::onPythonModuleInitialization(void* moduleDict)
{
  if (m_addon.get() == NULL)
    return;

  PyObject *moduleDictionary = (PyObject *)moduleDict;

  PyObject *pyaddonid = PyString_FromString(m_addon->ID().c_str());
  PyDict_SetItemString(moduleDictionary, "__xbmcaddonid__", pyaddonid);

  CStdString version = ADDON::GetXbmcApiVersionDependency(m_addon);
  PyObject *pyxbmcapiversion = PyString_FromString(version.c_str());
  PyDict_SetItemString(moduleDictionary, "__xbmcapiversion__", pyxbmcapiversion);

  CLog::Log(LOGDEBUG, "PythonInterpreter: instantiating addon using automatically obtained id of \"%s\" dependent on version %s of the xbmc.python api", m_addon->ID().c_str(), version.c_str());
}

void CPythonInterpreter::unloadExtensionLibraries()
{
  CLog::Log(LOGDEBUG, "PythonInterpreter: clearing python extension libraries");
  CSingleLock lock(s_critSection);

  for (PythonExtensionLibraries::iterator iter = s_extensions.begin(); iter != s_extensions.end(); iter++)
    DllLoaderContainer::ReleaseModule(*iter);

  s_extensions.clear();
}
