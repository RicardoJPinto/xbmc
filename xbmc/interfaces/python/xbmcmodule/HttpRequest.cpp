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

#include "HttpRequest.h"
#include "multidict.h"
#include "Cookie.h"
#include "pyutil.h"
#include "network/WebServer.h"
#include "utils/log.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
  PyObject* HttpRequest_fromHTTPPythonRequest(HTTPPythonRequest *httpPythonRequest)
  {
    if (httpPythonRequest == NULL)
      return NULL;

    HttpRequest* request = (HttpRequest*)HttpRequest_Type.tp_alloc(&HttpRequest_Type, 0);
    if (!request)
      return NULL;

    request->request = httpPythonRequest;
    request->dictHeadersIn = NULL;
    request->dictGet = NULL;
    request->dictPost = NULL;
    request->dictHeadersOut = multidict_FromMultiMap(&request->request->responseHeaders);
    request->dictHeadersOutError = multidict_FromMultiMap(&request->request->responseHeadersError);

    if (request->dictHeadersOut && request->dictHeadersOutError)
      return (PyObject*)request;

    Py_DECREF(request);

    return NULL;
  }

/* HttpRequest Functions */

  void HttpRequest_dealloc(HttpRequest* self)
  {
    Py_XDECREF(self->dictHeadersIn);
    Py_XDECREF(self->dictHeadersOut);
    Py_XDECREF(self->dictHeadersOutError);
    Py_XDECREF(self->dictGet);
    Py_XDECREF(self->dictPost);
    self->ob_type->tp_free((PyObject*)self);
  }

  PyDoc_STRVAR(logerror__doc__,
    "log_error(message[, level=LOGERROR]) -- Logs the given message with the given level.\n"
    "\n"
    "message        : string - text to output.\n"
    "level          : [opt] integer - log level to ouput at. (default=LOGERROR)\n");

  PyObject* HttpRequest_logError(HttpRequest *self, PyObject *args)
  {
    if (!self->request)
      return NULL;

    char *message = NULL;
    int level = LOGERROR;
    if (!PyArg_ParseTuple(args, (char*)"s|i", &message, &level))
      return NULL;

    // check for a valid loglevel
    if (level < LOGDEBUG || level > LOGNONE)
      level = LOGNOTICE;
    CLog::Log(level, "%s", message);

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyDoc_STRVAR(write__doc__,
    "write(text) -- Writes the given text to the HTTP response."
    "\n"
    "text          : string or unicode - text to append to the output.\n"
    "\n"
    "example:\n"
    "  - response.write('<b>Hello World!</b>')\n");

  PyObject* HttpRequest_write(HttpRequest *self, PyObject *args)
  {
    if (!self->request)
      return NULL;

    PyObject* textObj = NULL;
    if (!PyArg_ParseTuple(args, (char*)"O", &textObj))
      return NULL;

    string text;
    if (textObj && !PyXBMCGetUnicodeString(text, textObj, 1))
      return NULL;

    self->request->responseData.append(text);

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyDoc_STRVAR(addcookie__doc__,
    "add_cookie(cookie) -- Adds the cookie to the HTTP response."
    "\n"
    "cookie          : Cookie - cookie to add to the HTTP response.\n");

  PyObject* HttpRequest_addCookie(HttpRequest *self, PyObject *args)
  {
    if (!self->request)
    {
      Py_INCREF(Py_False);
      return Py_False;
    }

    Cookie *cookie = NULL;
    if (!PyArg_ParseTuple(args, (char*)"O:Cookie", &cookie))
    {
      Py_INCREF(Py_False);
      return Py_False;
    }

    if (!cookie || !Cookie_Check(cookie))
    {
      Py_INCREF(Py_False);
      return Py_False;
    }

    self->request->responseHeaders.insert(std::pair<string, string>("Set-Cookie", PyString_AsString(Cookie_str(cookie))));

    Py_INCREF(Py_True);
    return Py_True;
  }

  PyDoc_STRVAR(getcookies__doc__,
    "get_cookies() -- Gets a dictionary of all cookies from the HTTP request.\n");

  PyObject* HttpRequest_getCookies(HttpRequest *self, PyObject *args)
  {
    if (!self->request)
      return NULL;

    string cookieData;
    pair<multimap<string, string>::const_iterator, multimap<string, string>::const_iterator> range = self->request->headerValues.equal_range("Cookie");
    for (multimap<string, string>::const_iterator it = range.first; it != range.second; it++)
    {
      cookieData.append(it->second);
      cookieData.push_back(';'); // make sure two different cookies are seperated
    }

    if (cookieData.empty())
      return PyDict_New();

    return parseCookies(cookieData);
  }

  PyDoc_STRVAR(getcookie__doc__,
    "get_cookie(name) -- Gets a specific cookie from the HTTP request."
    "\n"
    "name          : string or unicode - name of the cookie to retrieve.\n");

  PyObject* HttpRequest_getCookie(HttpRequest *self, PyObject *args)
  {
    if (!self->request)
      return NULL;

    PyObject* nameObj = NULL;
    if (!PyArg_ParseTuple(args, (char*)"O", &nameObj))
      return NULL;

    string name;
    if (nameObj && !PyXBMCGetUnicodeString(name, nameObj, 1))
      return NULL;

    PyObject *cookies = HttpRequest_getCookies(self, NULL);
    if (cookies && PyDict_Size(cookies) > 0)
    {
      PyObject *cookie = PyDict_GetItemString(cookies, name.c_str());
      if (cookie != NULL)
        return cookie;
    }

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyObject* HttpRequest_getMember(HttpRequest *self, void *name)
  {
    if (!self->request)
      return NULL;

    if (strcmp((char*)name, "header_only") == 0)
      return Py_BuildValue((char*)"b", self->request->method == HEAD);
    else if (strcmp((char*)name, "protocol") == 0)
    {
      string protocol = "HTTP/0.9";
      if (!self->request->version.empty())
        protocol = self->request->version;

      return PyString_FromString(protocol.c_str());
    }
    else if (strcmp((char*)name, "proto_num") == 0)
    {
      int major = 0;
      int minor = 0;

      size_t posSlash = self->request->version.find("/");
      size_t posDot = self->request->version.find(".");
      if (posSlash != string::npos && posDot != string::npos)
      {
        major = atoi(self->request->version.substr(posSlash + 1, posDot - posSlash - 1).c_str());
        minor = atoi(self->request->version.substr(posDot + 1, self->request->version.size() - posDot - 1).c_str());
      }

      return PyInt_FromLong(major * 1000 + minor);
    }
    else if (strcmp((char*)name, "status") == 0)
      return PyInt_FromLong(self->request->responseStatus);
    else if (strcmp((char*)name, "method") == 0)
    {
      switch (self->request->method)
      {
      case POST:
        return PyString_FromString("POST");

      case HEAD:
        return PyString_FromString("HEAD");

      case GET:
      default:
        return PyString_FromString("GET");
      }
    }
    else if (strcmp((char*)name, "headers_in") == 0)
    {
      if (!self->dictHeadersIn)
      {
        self->dictHeadersIn = PyDict_New();
        if (!self->dictHeadersIn)
          return NULL;

        for (multimap<string, string>::const_iterator it = self->request->headerValues.begin(); it != self->request->headerValues.end(); it++)
          PyDict_SetItemString(self->dictHeadersIn, it->first.c_str(), PyString_FromString(it->second.c_str()));
      }

      Py_INCREF(self->dictHeadersIn);
      return self->dictHeadersIn;
    }
    else if (strcmp((char*)name, "headers_out") == 0)
    {
      Py_INCREF(self->dictHeadersOut);
      return (PyObject*)self->dictHeadersOut;
    }
    else if (strcmp((char*)name, "err_headers_out") == 0)
    {
      Py_INCREF(self->dictHeadersOutError);
      return (PyObject*)self->dictHeadersOutError;
    }
    else if (strcmp((char*)name, "content_type") == 0)
      return PyString_FromString(self->request->responseContentType.c_str());
    else if (strcmp((char*)name, "uri") == 0)
      return PyString_FromString(self->request->url.c_str());
    else if (strcmp((char*)name, "filename") == 0)
      return PyString_FromString(self->request->file.c_str());
    else if (strcmp((char*)name, "get") == 0)
    {
      if (!self->dictGet)
      {
        self->dictGet = PyDict_New();
        if (!self->dictGet)
          return NULL;

        for (map<string, string>::const_iterator it = self->request->getValues.begin(); it != self->request->getValues.end(); it++)
          PyDict_SetItemString(self->dictGet, it->first.c_str(), PyString_FromString(it->second.c_str()));
      }

      Py_INCREF(self->dictGet);
      return self->dictGet;
    }
    else if (strcmp((char*)name, "post") == 0)
    {
      if (!self->dictPost)
      {
        self->dictPost = PyDict_New();
        if (!self->dictPost)
          return NULL;

        for (map<string, string>::const_iterator it = self->request->postValues.begin(); it != self->request->postValues.end(); it++)
          PyDict_SetItemString(self->dictPost, it->first.c_str(), PyString_FromString(it->second.c_str()));
      }

      Py_INCREF(self->dictPost);
      return self->dictPost;
    }

    Py_INCREF(Py_None);
    return Py_None;
  }

  int HttpRequest_setMember(HttpRequest *self, PyObject *value, void *name)
  {
    if (!self->request)
      return -1;

    if (strcmp((char*)name, "status") == 0)
    {
      if (!PyInt_Check(value))
      {
        PyErr_SetString(PyExc_TypeError, "status must be an integer");
        return -1;
      }

      self->request->responseStatus = (int)PyInt_AsLong(value);
    }
    else if (strcmp((char*)name, "content_type") == 0)
    {
      if (!PyString_Check(value)) {
        PyErr_SetString(PyExc_TypeError, "content_type must be a string");
        return -1;
      }

      self->request->responseContentType = PyString_AsString(value);
    }

    return 0;
  }

  PyMethodDef HttpRequest_methods[] = {
    {(char*)"log_error",    (PyCFunction)HttpRequest_logError, METH_VARARGS, logerror__doc__},
    {(char*)"write",        (PyCFunction)HttpRequest_write, METH_VARARGS, write__doc__},
    {(char*)"add_cookie",   (PyCFunction)HttpRequest_addCookie, METH_VARARGS, addcookie__doc__},
    {(char*)"get_cookie",   (PyCFunction)HttpRequest_getCookie, METH_VARARGS, getcookie__doc__},
    {(char*)"get_cookies",  (PyCFunction)HttpRequest_getCookies, METH_VARARGS, getcookies__doc__},
    {NULL, NULL, 0, NULL}
  };

  PyGetSetDef HttpRequest_getsets[] = {
    {(char*)"header_only",       (getter)HttpRequest_getMember, NULL, (char*)"HEAD request, as oppsed to GET", (char*)"header_only"},
    {(char*)"protocol",          (getter)HttpRequest_getMember, NULL, (char*)"Protocol as given to us, or HTTP/0.9", (char*)"protocol"},
    {(char*)"proto_num",         (getter)HttpRequest_getMember, NULL, (char*)"Protocol version. 1.1 = 1001", (char*)"proto_num"},
    {(char*)"status",            (getter)HttpRequest_getMember, (setter)HttpRequest_setMember, (char*)"Status", (char*)"status"},
    {(char*)"method",            (getter)HttpRequest_getMember, NULL, (char*)"Request method", (char*)"method"},
    {(char*)"headers_in",        (getter)HttpRequest_getMember, NULL, (char*)"Incoming headers", (char*)"headers_in"},
    {(char*)"headers_out",       (getter)HttpRequest_getMember, NULL, (char*)"Outgoing headers", (char*)"headers_out"},
    {(char*)"err_headers_out",   (getter)HttpRequest_getMember, NULL, (char*)"Outgoing headersfor errors", (char*)"err_headers_out"},
    {(char*)"content_type",      (getter)HttpRequest_getMember, (setter)HttpRequest_setMember, (char*)"Content type", (char*)"content_type"},
    {(char*)"uri",               (getter)HttpRequest_getMember, NULL, (char*)"The path portion of URI", (char*)"uri"},
    {(char*)"filename",          (getter)HttpRequest_getMember, NULL, (char*)"The file name on disk that this request corresponds to", (char*)"filename"},
    {(char*)"get",               (getter)HttpRequest_getMember, NULL, (char*)"Dictionary of all get fields/values", (char*)"get"},
    {(char*)"post",              (getter)HttpRequest_getMember, NULL, (char*)"Dictionary of all post fields/values", (char*)"post"},
    {NULL}
  };

  PyDoc_STRVAR(httpRequest__doc__,
    "HttpRequest class.\n"
    "\n"
    "Provides access to details of a specific HTTP request and allows to write to the HTTP response.");

// Restore code and data sections to normal.

  PyTypeObject HttpRequest_Type;
  void initHttpRequest_Type()
  {
    PyXBMCInitializeTypeObject(&HttpRequest_Type);

    HttpRequest_Type.tp_name = (char*)"xbmchttp.HttpRequest";
    HttpRequest_Type.tp_basicsize = sizeof(HttpRequest);
    HttpRequest_Type.tp_dealloc = (destructor)HttpRequest_dealloc;
    HttpRequest_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    HttpRequest_Type.tp_doc = httpRequest__doc__;
    HttpRequest_Type.tp_methods = HttpRequest_methods;
    HttpRequest_Type.tp_getset = HttpRequest_getsets;
    HttpRequest_Type.tp_base = 0;
  }
}

#ifdef __cplusplus
}
#endif
