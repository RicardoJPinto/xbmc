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

#include "Session.h"
#include "pyutil.h"
#include "network/httprequesthandler/python/HTTPSessionManager.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
  /* Session Functions */

  Session* Session_New(const char* sessionId, bool isNew)
  {
    Session *sessionObj = (Session*)Session_Type.tp_new(&Session_Type, NULL, NULL);
    if (sessionObj == NULL)
      return NULL;

    sessionObj->sessionId = sessionId;
    sessionObj->isNew = isNew;

    return sessionObj;
  }

  void Session_dealloc(Session *self)
  {
    self->dict.ob_type->tp_free((PyObject*)self);
  }

  int Session_print(Session *self, FILE *fp, int flags)
  {
    if (self == NULL || self->sessionId.empty())
      return -1;

    fprintf(fp, "<Session: %s>", self->sessionId);
    return 0;
  }

  PyObject* Session_id(Session *self, PyObject *args);

  PyObject* Session_str(Session *self)
  {
    return Session_id(self, NULL);
  }

  PyObject* Session_repr(Session *self)
  {
    if (self == NULL || self->sessionId.empty())
      return NULL;

    PyObject *s = PyString_FromString("<Session: ");
    PyString_ConcatAndDel(&s, Session_str(self));
    PyString_ConcatAndDel(&s, PyString_FromString(">"));

    return s;
  }
  
  PyDoc_STRVAR(is_new__doc__,
    "is_new() -- Returns True if the session is new or after an attempt to load an expired session.\n");

  PyObject* Session_is_new(Session *self, PyObject *args)
  {
    if (self == NULL)
      return NULL;

    return Py_BuildValue("b", self->isNew);
  }
  
  PyDoc_STRVAR(id__doc__,
    "id() -- Returns the session's id.\n");

  PyObject* Session_id(Session *self, PyObject *args)
  {
    if (self == NULL || self->sessionId.empty())
      return NULL;

    return PyString_FromString(self->sessionId.c_str());
  }
  
  PyDoc_STRVAR(invalidate__doc__,
    "invalidate() -- Invalidates the session by removing it from the persistent storage and by setting a header to invalidate the sessionid cookie.\n");

  PyObject* Session_invalidate(Session *self, PyObject *args)
  {
    if (self == NULL || self->sessionId.empty())
      return NULL;

    CHTTPSessionManager::Get().RemoveSession(self->sessionId, true);
    PyDict_Clear((PyObject*)self);

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyDoc_STRVAR(delete__doc__,
    "delete() -- Removes the session from the persistant storage.\n");

  PyObject* Session_delete(Session *self, PyObject *args)
  {
    if (self == NULL || self->sessionId.empty())
      return NULL;

    CHTTPSessionManager::Get().RemoveSession(self->sessionId);
    PyDict_Clear((PyObject*)self);

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyMethodDef Session_methods[] = {
    {(char*)"is_new",       (PyCFunction)Session_is_new, METH_NOARGS, is_new__doc__},
    {(char*)"id",           (PyCFunction)Session_id, METH_NOARGS, id__doc__},
    {(char*)"invalidate",   (PyCFunction)Session_invalidate, METH_NOARGS, invalidate__doc__},
    {(char*)"delete",       (PyCFunction)Session_delete, METH_NOARGS, delete__doc__},
    {NULL, NULL, 0, NULL}
  };

  PyDoc_STRVAR(session__doc__,
    "Session class.\n"
    "\n"
    "Represents a HTTP session.");

// Restore code and data sections to normal.

  PyTypeObject Session_Type;

  void initSession_Type()
  {
    PyXBMCInitializeTypeObject(&Session_Type);

    Session_Type.tp_name = (char*)"xbmchttp.Session";
    Session_Type.tp_basicsize = sizeof(Session);
    Session_Type.tp_dealloc = (destructor)Session_dealloc;
    Session_Type.tp_print = (printfunc)Session_print;
    Session_Type.tp_str = (reprfunc)Session_str;
    Session_Type.tp_repr = (reprfunc)Session_repr;
    Session_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    Session_Type.tp_doc = session__doc__;
    Session_Type.tp_methods = Session_methods;
    Session_Type.tp_base = &PyDict_Type;
  }
}

#ifdef __cplusplus
}
#endif