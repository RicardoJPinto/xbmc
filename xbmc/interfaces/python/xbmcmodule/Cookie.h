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

#define Cookie_Check(op) PyObject_TypeCheck(op, &Cookie_Type)
#define Cookie_CheckExact(op) ((op)->ob_type == &Cookie_Type)

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
  extern PyTypeObject Cookie_Type;

  typedef struct {
    PyObject_HEAD
    PyObject *name;
    PyObject *value;
    PyObject *attributes;
  } Cookie;

  void initCookie_Type();

  PyObject* parseCookies(std::string data);
  PyObject* Cookie_str(Cookie *cookie);
}

#ifdef __cplusplus
}
#endif
