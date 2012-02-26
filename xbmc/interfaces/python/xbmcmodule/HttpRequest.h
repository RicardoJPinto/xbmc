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

#include <Python.h>
#include "network/httprequesthandler/python/HTTPPythonInterpreter.h"

#define HttpRequest_Check(op) PyObject_TypeCheck(op, &HttpRequest_Type)
#define HttpRequest_CheckExact(op) ((op)->ob_type == &HttpRequest_Type)

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
  extern PyTypeObject HttpRequest_Type;

  typedef struct {
    PyObject_HEAD
    HTTPPythonRequest *request;
    PyObject *dictHeadersIn;
    PyObject *dictHeadersOut;
    PyObject *dictHeadersOutError;
    PyObject *dictGet;
    PyObject *dictPost;
  } HttpRequest;

  void initHttpRequest_Type();

  PyObject* HttpRequest_fromHTTPPythonRequest(HTTPPythonRequest *httpPythonRequest);
}

#ifdef __cplusplus
}
#endif
