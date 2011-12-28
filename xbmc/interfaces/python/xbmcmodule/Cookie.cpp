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

#include "Cookie.h"
#include "pyutil.h"
#include "utils/StringUtils.h"
#include "XBDateTime.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
  /* Cookie Functions */

  PyObject* Cookie_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
  {
    Cookie *cookieObj = (Cookie*)type->tp_alloc(type, 0);
    if (cookieObj == NULL)
      return PyErr_NoMemory();

    cookieObj->name = NULL;
    cookieObj->value = NULL;
    cookieObj->attributes = (PyObject *)PyDict_New();

    return (PyObject*)cookieObj;
  }

  void Cookie_dealloc(Cookie *self)
  {
    Py_XDECREF(self->name);
    Py_XDECREF(self->value);
    Py_XDECREF(self->attributes);
    self->ob_type->tp_free((PyObject*)self);
  }

  int Cookie_init(Cookie *self, PyObject *args, PyObject *kwds)
  {
    char *nameObj = NULL, *valueObj = NULL, *pathObj = NULL, *domainObj = NULL;
    char *commentObj = NULL, *expiresObj = NULL, *commentUrlObj = NULL;
    int versionObj = -1, maxAgeObj = -1, portObj = -1;
    char secureObj = -1, discardObj = -1, httponlyObj = -1;
    static char *kwlist[] = { (char *)"name", (char *)"value",
                              (char *)"Version", (char *)"Path", (char *)"Domain", (char *)"Secure",
                              (char *)"Comment", (char *)"Expires", (char *)"Max_Age",
                              // RFC 2965
                              (char *)"CommentURL", (char *)"Discard", (char *)"Port",
                              // Microsoft
                              (char *)"HttpOnly", (char *)NULL };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, (char*)"ss|issbssisbib", (char**)kwlist, &nameObj, &valueObj,
                                                                           &versionObj, &pathObj, &domainObj, &secureObj,
                                                                           &commentObj, &expiresObj, &maxAgeObj,
                                                                           &commentUrlObj, &discardObj, &portObj,
                                                                           &httponlyObj))
      return -1;

    if (!nameObj || !valueObj)
      return -1;

    self->name = PyString_FromString(nameObj);
    self->value = PyString_FromString(valueObj);

    if (versionObj != -1)
      PyDict_SetItemString(self->attributes, "Version", PyInt_FromLong((long)versionObj));
    if (pathObj)
      PyDict_SetItemString(self->attributes, "Path", PyString_FromString(pathObj));
    if (domainObj)
      PyDict_SetItemString(self->attributes, "Domain", PyString_FromString(domainObj));
    if (secureObj != -1)
      PyDict_SetItemString(self->attributes, "Secure", secureObj ? Py_True : Py_False);
    if (commentObj)
      PyDict_SetItemString(self->attributes, "Comment", PyString_FromString(commentObj));
    if (expiresObj)
    {
      char wday[4] = { 0 };
      char month[4] = { 0 };
      unsigned int day, year, hour, minute, second = 0;
      if (sscanf(expiresObj, "%3s, %2u-%3s-%4d %2u:%2u:%2u GMT", wday, &day, month, &year, &hour, &minute, &second) != 7)
      {
        PyErr_SetString(PyExc_ValueError, "Invalid \"Expires\" value (Wdy, DD-Mon-YYYY HH:MM:SS GMT)");
        Py_DECREF(self);
        return NULL;
      }
      PyDict_SetItemString(self->attributes, "Expires", PyString_FromString(expiresObj));
    }
    if (maxAgeObj != -1)
      PyDict_SetItemString(self->attributes, "Max-Age", PyInt_FromLong((long)maxAgeObj));
    if (commentUrlObj)
      PyDict_SetItemString(self->attributes, "CommentURL", PyString_FromString(commentUrlObj));
    if (discardObj != -1)
      PyDict_SetItemString(self->attributes, "Discard", discardObj ? Py_True : Py_False);
    if (portObj != -1)
      PyDict_SetItemString(self->attributes, "Port", PyInt_FromLong((long)portObj));
    if (httponlyObj != -1)
      PyDict_SetItemString(self->attributes, "HttpOnly", httponlyObj ? Py_True : Py_False);

    return 0;
  }

  PyObject* asString(PyObject *obj)
  {
    if (obj == NULL)
      return NULL;

    string str = PyString_AsString(PyObject_Repr(obj));
    if (str.at(0) == '\'')
      str.replace(str.begin(), str.begin() + 1, "\"");
    if (str.at(str.size() - 1) == '\'')
      str.replace(str.begin() + str.size() - 1, str.begin() + str.size(), "\"");

    return PyString_FromString(str.c_str());
  }

  int Cookie_print(Cookie *self, FILE *fp, int flags)
  {
    if (self == NULL)
      return -1;

    fprintf(fp, "<Cookie: ");
    fprintf(fp, "%s=%s", PyString_AsString(self->name), PyString_AsString(self->value));

    PyObject *keys = PyDict_Keys(self->attributes);
    for (Py_ssize_t index = 0; index < PyList_Size(keys); index++)
    {
      const char *key = PyString_AsString(PyList_GetItem(keys, index));
      if (key == NULL)
        continue;

      PyObject *valueObj = PyDict_GetItemString(self->attributes, key);
      if (valueObj == NULL)
        continue;

      if (strcmp(key, "Secure") == 0 || strcmp(key, "Discard") == 0 || strcmp(key, "HttpOnly") == 0)
      {
        if (!PyBool_Check(valueObj) || valueObj != Py_True)
          continue;

        fprintf(fp, "; %s", key);
      }
      else
        fprintf(fp, "; %s=%s", key, PyString_AsString(asString(valueObj)));
    }

    fprintf(fp, ">");

    return 0;
  }

  PyObject* Cookie_str(Cookie *self)
  {
    if (self == NULL)
    {
      Py_INCREF(Py_None);
      return Py_None;
    }

    PyObject *s;

    s = PyString_FromString("");

    PyString_Concat(&s, self->name);
    PyString_ConcatAndDel(&s, PyString_FromString("="));
    PyString_Concat(&s, self->value);

    PyObject *keys = PyDict_Keys(self->attributes);
    for (Py_ssize_t index = 0; index < PyList_Size(keys); index++)
    {
      const char *key = PyString_AsString(PyList_GetItem(keys, index));
      if (key == NULL)
        continue;

      PyObject *valueObj = PyDict_GetItemString(self->attributes, key);
      if (valueObj == NULL)
        continue;

      if (strcmp(key, "Secure") == 0 || strcmp(key, "Discard") == 0 || strcmp(key, "HttpOnly") == 0)
      {
        if (!PyBool_Check(valueObj) || valueObj != Py_True)
          continue;

        PyString_ConcatAndDel(&s, PyString_FromString("; "));
        PyString_ConcatAndDel(&s, PyString_FromString(key));
        continue;
      }

      PyString_ConcatAndDel(&s, PyString_FromString("; "));
      PyString_ConcatAndDel(&s, PyString_FromString(key));
      PyString_ConcatAndDel(&s, PyString_FromString("="));
      PyString_ConcatAndDel(&s, asString(valueObj));
    }

    return s;
  }

  PyObject* Cookie_repr(Cookie *self)
  {
    if (self == NULL)
    {
      Py_INCREF(Py_None);
      return Py_None;
    }

    PyObject *s = PyString_FromString("<Cookie: ");
    PyString_ConcatAndDel(&s, Cookie_str(self));
    PyString_ConcatAndDel(&s, PyString_FromString(">"));

    return s;
  }

  PyDoc_STRVAR(parse__doc__,
    "parse(cookiestring) -- Returns a dictionary of new cookie by parsing the cookiestring.\n"
    "\n"
    "cookiestring   : string - string from a HTTP header representing one or multiple cookies.\n");

  PyObject* Cookie_parse(Cookie *self, PyObject *args)
  {
    PyObject *dataObj = NULL;
    if (!PyArg_ParseTuple(args, (char*)"O", &dataObj))
      return NULL;

    string data;
    if (dataObj && !PyXBMCGetUnicodeString(data, dataObj, 1))
      return NULL;

    return parseCookies(data);
  }

  PyObject* Cookie_getMember(Cookie *self, void *name)
  {
    if (strcmp((char*)name, "Name") == 0)
    {
      Py_INCREF(self->name);
      return self->name;
    }
    else if (strcmp((char*)name, "Value") == 0)
    {
      Py_INCREF(self->value);
      return self->value;
    }
    else if (strcmp((char*)name, "Version") == 0 ||
             strcmp((char*)name, "Path") == 0 ||
             strcmp((char*)name, "Domain") == 0 ||
             strcmp((char*)name, "Secure") == 0 ||
             strcmp((char*)name, "Comment") == 0 ||
             strcmp((char*)name, "Expires") == 0 ||
             strcmp((char*)name, "Max-Age") == 0 ||
             strcmp((char*)name, "CommentURL") == 0 ||
             strcmp((char*)name, "Discard") == 0 ||
             strcmp((char*)name, "Port") == 0 ||
             strcmp((char*)name, "HttpOnly") == 0)
    {
      PyObject *obj = PyDict_GetItemString(self->attributes, (char*)name);
      if (obj)
      {
        Py_INCREF(obj);
        return obj;
      }
    }

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* parseCookies(string data)
  {
    PyObject *cookies = PyDict_New();
    if (data.empty())
      return cookies;

    Cookie *cookie = (Cookie *)Cookie_new(&Cookie_Type, NULL, NULL);
    if (!Cookie_Check(cookie))
      return cookies;

    CStdStringArray parts;
    StringUtils::SplitString(data, ";", parts);
    for (unsigned int index = 0; index < parts.size(); index++)
    {
      if (parts[index].empty())
        continue;

      CStdString key, value;
      size_t pos;
      if ((pos = parts[index].find('=')) != string::npos)
      {
        key = parts[index].substr(0, pos);
        value = parts[index].substr(pos + 1);
      }
      else
        key = parts[index];

      key.Trim();
      value.Trim();

      // Let's check if this is a new cookie
      if (cookie->name == NULL)
      {
        cookie->name = PyString_FromString(key.c_str());
        cookie->value = PyString_FromString(value.c_str());
      }
      // Let's check if we found an attribute
      else if (key.compare("Path") == 0 ||
               key.compare("Domain") == 0 ||
               key.compare("Comment") == 0 ||
               key.compare("Expires") == 0 ||
               key.compare("CommentURL") == 0)
      {
        value.Replace("\"", "");
        PyDict_SetItemString(cookie->attributes, key.c_str(), PyString_FromString(value.c_str()));
      }
      else if (key.compare("Secure") == 0 ||
               key.compare("Discard") == 0 ||
               key.compare("HttpOnly") == 0)
        PyDict_SetItemString(cookie->attributes, key.c_str(), Py_True);
      else if (key.compare("Version") == 0 ||
               key.compare("Max-Age") == 0 ||
               key.compare("Port") == 0)
        PyDict_SetItemString(cookie->attributes, key.c_str(), PyInt_FromString((char *)value.c_str(), NULL, 10));
      // We got a key that does not match any of the valid attribute names
      // => must be a new cookie
      else
      {
        PyDict_SetItem(cookies, cookie->name, (PyObject *)cookie);
        cookie = (Cookie *)Cookie_new(&Cookie_Type, NULL, NULL);
        if (!Cookie_Check(cookie))
          return cookies;
        cookie->name = PyString_FromString(key.c_str());
        cookie->value = PyString_FromString(value.c_str());
      }
    }

    if (cookie && cookie->name && cookie->value)
      PyDict_SetItem(cookies, cookie->name, (PyObject *)cookie);

    return cookies;
  }

  PyMethodDef Cookie_methods[] = {
    {(char*)"parse", (PyCFunction)Cookie_parse, METH_VARARGS | METH_CLASS, parse__doc__},
    {NULL, NULL, 0, NULL}
  };

  PyGetSetDef Cookie_getsets[] = {
    {(char*)"Name",       (getter)Cookie_getMember, NULL, (char*)"Name of the cookie", (char*)"Name"},
    {(char*)"Value",      (getter)Cookie_getMember, NULL, (char*)"Value of the cookie", (char*)"Value"},
    {(char*)"Version",    (getter)Cookie_getMember, NULL, (char*)"Version of the cookie", (char*)"Version"},
    {(char*)"Path",       (getter)Cookie_getMember, NULL, (char*)"Path of the website which set the cookie", (char*)"Path"},
    {(char*)"Domain",     (getter)Cookie_getMember, NULL, (char*)"Domain of the website which set the cookie", (char*)"Domain"},
    {(char*)"Secure",     (getter)Cookie_getMember, NULL, (char*)"Whether the cookie can only be used in secure connections", (char*)"Secure"},
    {(char*)"Comment",    (getter)Cookie_getMember, NULL, (char*)"Comment on the purpose of the cookie", (char*)"Comment"},
    {(char*)"Expires",    (getter)Cookie_getMember, NULL, (char*)"Date and time when the cookie expires (Wdy, DD-Mth-YYYY HH:MM:SS)", (char*)"Expires"},
    {(char*)"Max_Age",    (getter)Cookie_getMember, NULL, (char*)"Maximum age (in seconds) of the cookie", (char*)"Max-Age"},
    {(char*)"CommentURL", (getter)Cookie_getMember, NULL, (char*)"URL to a comment on the purpose of the cookie (RFC 2965)", (char*)"CommentURL"},
    {(char*)"Discard",    (getter)Cookie_getMember, NULL, (char*)"Whether to discard the cookie (RFC 2965)", (char*)"Discard"},
    {(char*)"Port",       (getter)Cookie_getMember, NULL, (char*)"Port on which the cookie should be used (RFC 2965)", (char*)"Port"},
    {(char*)"HttpOnly",   (getter)Cookie_getMember, NULL, (char*)"Whether the cookie should only be used over HTTP (Microsoft only)", (char*)"HttpOnly"},
    {NULL}
  };

  PyDoc_STRVAR(cookie__doc__,
    "Cookie class.\n"
    "\n"
    "Represents a HTTP cookie.");

// Restore code and data sections to normal.

  PyTypeObject Cookie_Type;

  void initCookie_Type()
  {
    PyXBMCInitializeTypeObject(&Cookie_Type);

    Cookie_Type.tp_name = (char*)"xbmchttp.Cookie";
    Cookie_Type.tp_basicsize = sizeof(Cookie);
    Cookie_Type.tp_dealloc = (destructor)Cookie_dealloc;
    Cookie_Type.tp_print = (printfunc)Cookie_print;
    Cookie_Type.tp_str = (reprfunc)Cookie_str;
    Cookie_Type.tp_repr = (reprfunc)Cookie_repr;
    Cookie_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    Cookie_Type.tp_doc = cookie__doc__;
    Cookie_Type.tp_methods = Cookie_methods;
    Cookie_Type.tp_getset = Cookie_getsets;
    Cookie_Type.tp_init = (initproc)Cookie_init;
    Cookie_Type.tp_new = Cookie_new;
    Cookie_Type.tp_base = 0;
  }
}

#ifdef __cplusplus
}
#endif