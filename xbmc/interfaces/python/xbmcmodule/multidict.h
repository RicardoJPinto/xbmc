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

#include <string>
#include <map>

#define multidict_Check(op) PyObject_TypeCheck(op, &multidict_Type)
#define multidict_CheckExact(op) ((op)->ob_type == &multidict_Type)
#define multidictiter_Check(op) PyObject_TypeCheck(op, &multidictiter_Type)
#define multidictiter_CheckExact(op) ((op)->ob_type == &multidictiter_Type)

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
  extern PyTypeObject multidict_Type;
  extern PyTypeObject multidictiter_Type;

  typedef struct {
    PyObject_HEAD
    std::multimap<std::string, std::string> *map;
    bool free;
  } multidict;

  typedef PyObject* (*multidict_selectfunc)(std::multimap<std::string, std::string>::iterator &);

  typedef struct {
    PyObject_HEAD
    multidict *multidict;
    std::multimap<std::string, std::string>::iterator iter;
    int size;
    multidict_selectfunc select;
  } multidictiter;

  void initmultidict_Type();
  void initmultidictiter_Type();

  PyObject* multidict_FromMultiMap(std::multimap<std::string, std::string> *multiMap);
}

#ifdef __cplusplus
}
#endif
