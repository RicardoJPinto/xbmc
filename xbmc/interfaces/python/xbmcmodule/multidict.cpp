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

#include "multidict.h"
#include "pyutil.h"

using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

namespace PYXBMC
{
  /* multidict Functions */

  PyObject* getFromKey(multidict *dict, const char *strKey)
  {
    if (dict == NULL || strKey == NULL)
      return NULL;

    PyObject *list = PyList_New((Py_ssize_t)0);
    if (!list)
      return NULL;

    unsigned int pos = 0;
    pair<multimap<string, string>::iterator, multimap<string, string>::iterator> range = dict->map->equal_range(strKey);
    for (multimap<string, string>::iterator it = range.first; it != range.second; it++)
    {
      PyList_Insert(list, (Py_ssize_t)pos, PyString_FromString(it->second.c_str()));
      pos++;
    }

    return list;
  }

  PyObject *multidict_select_key(std::multimap<std::string, std::string>::iterator &entry)
  {
    return PyString_FromString(entry->first.c_str());
  }

  PyObject *multidict_select_value(std::multimap<std::string, std::string>::iterator &entry)
  {
    return PyString_FromString(entry->second.c_str());
  }

  PyObject *multidict_select_item(std::multimap<std::string, std::string>::iterator &entry)
  {
    return Py_BuildValue("(s,s)", entry->first.c_str(), entry->second.c_str());
  }

  int multidict_length(multidict *self)
  {
    return self->map->size();
  }

  PyObject* multidict_subscript(multidict *self, register PyObject *key)
  {
    if (key && !PyString_CheckExact(key))
    {
      PyErr_SetString(PyExc_TypeError, "multidict keys must be strings");
      return NULL;
    }

    PyObject *list = getFromKey(self, PyString_AsString(key));

    /* if no match */
    if (PyList_Size(list) == (Py_ssize_t)0)
    {
      Py_DECREF(list);
      PyErr_SetObject(PyExc_KeyError, key);
      return NULL;
    }

    /* if we got one match */
    if (PyList_Size(list) == (Py_ssize_t)1)
    {
      PyObject *v = PyList_GetItem(list, (Py_ssize_t)0);
      Py_INCREF(v);
      Py_DECREF(list);
      return v;
    }

    return list;
  }

  int multidict_ass_subscript(multidict *self, PyObject *key, PyObject *val)
  {
    char *strKey;

    if (key && !PyString_CheckExact(key))
    {
        PyErr_SetString(PyExc_TypeError, "multidict keys must be strings");
        return -1;
    }

    strKey = PyString_AsString(key);
    multimap<string, string>::iterator it = self->map->find(strKey);

    if (val == NULL)
    {
      if (it != self->map->end())
        self->map->erase(it);
    }
    else
    {
      if (val && !PyString_CheckExact(val))
      {
          PyErr_SetString(PyExc_TypeError, "multidict values must be strings");
          return -1;
      }
      it->second = PyString_AsString(val);
    }

    return 0;
  }

  /*int multidict_merge(multidict *a, PyObject *b, int override)
  {
    // Do it the generic, slower way
    PyObject *keys = PyMapping_Keys(b);
    PyObject *iter;
    PyObject *key, *value, *skey, *svalue;
    int status;

    if (keys == NULL)
      return -1;

    iter = PyObject_GetIter(keys);
    Py_DECREF(keys);
    if (iter == NULL)
      return -1;

    for (key = PyIter_Next(iter); key; key = PyIter_Next(iter))
    {
      skey = PyObject_Str(key);
      if (skey == NULL) 
      {
        Py_DECREF(iter);
        Py_DECREF(key);
        return -1;
      }
      if (!override && a->map->find(PyString_AsString(skey)) != a->map->end()) 
      {
        Py_DECREF(key);
        Py_DECREF(skey);
        continue;
      }

      value = PyObject_GetItem(b, key);
      if (value == NULL)
      {
        Py_DECREF(iter);
        Py_DECREF(key);
        Py_DECREF(skey);
        return -1;
      }
      svalue = PyObject_Str(value);
      if (svalue == NULL)
      {
        Py_DECREF(iter);
        Py_DECREF(key);
        Py_DECREF(skey);
        Py_DECREF(value);
        return -1;
      }
      status = multidict_ass_subscript(a, skey, svalue);
      Py_DECREF(key);
      Py_DECREF(value);
      Py_DECREF(skey);
      Py_DECREF(svalue);
      if (status < 0)
      {
        Py_DECREF(iter);
        return -1;
      }
    }

    Py_DECREF(iter);
    // Iterator completed, via error
    if (PyErr_Occurred())
      return -1;

    return 0;
  }

  int multidict_mergefromseq2(multidict *a, PyObject *seq2, int override)
  {
    PyObject *it;       // iter(seq2)
    int i;              // index into seq2 of current element
    PyObject *item;     // seq2[i]
    PyObject *fast;     // item as a 2-tuple or 2-list

    it = PyObject_GetIter(seq2);
    if (it == NULL)
      return -1;

    for (i = 0; ; ++i)
    {
      PyObject *key, *value, *skey, *svalue;
      int n;

      fast = NULL;
      item = PyIter_Next(it);
      if (item == NULL)
      {
        if (PyErr_Occurred())
          goto Fail;
        break;
      }

      // Convert item to sequence, and verify length 2.
      fast = PySequence_Fast(item, "");
      if (fast == NULL)
      {
        if (PyErr_ExceptionMatches(PyExc_TypeError))
          PyErr_Format(PyExc_TypeError, "cannot convert multidict update sequence element #%d to a sequence", i);
        goto Fail;
      }
      n = PySequence_Fast_GET_SIZE(fast);
      if (n != 2)
      {
        PyErr_Format(PyExc_ValueError, "multidict update sequence element #%d has length %d; 2 is required", i, n);
        goto Fail;
      }

      // Update/merge with this (key, value) pair.
      key = PySequence_Fast_GET_ITEM(fast, 0);
      value = PySequence_Fast_GET_ITEM(fast, 1);
      skey = PyObject_Str(key);
      if (skey == NULL)
        goto Fail;
      svalue = PyObject_Str(value);
      if (svalue == NULL)
      {
        Py_DECREF(svalue);
        goto Fail;
      }

      if (override || apr_table_get(self->table, PyString_AsString(skey)) == NULL) 
      {
        int status = multidict_ass_subscript(self, skey, svalue);
        if (status < 0)
        {
          Py_DECREF(skey);
          Py_DECREF(svalue);
          goto Fail;
        }
      }

      Py_DECREF(skey);
      Py_DECREF(svalue);
      Py_DECREF(fast);
      Py_DECREF(item);
    }

    i = 0;
    goto Return;
  Fail:
    Py_XDECREF(item);
    Py_XDECREF(fast);
    i = -1;
  Return:
    Py_DECREF(it);
    return i;
  }*/

  PyObject *multidict_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
  {
    multidict *table = (multidict*)multidict_FromMultiMap(new std::multimap<std::string, std::string>());
    table->free = true;

    return (PyObject*)table;
  }

  PyObject* multidict_FromMultiMap(std::multimap<std::string, std::string> *multiMap)
  {
    multidict *tableObj = (multidict*)multidict_Type.tp_alloc(&multidict_Type, 0);
    if (tableObj == NULL)
      return PyErr_NoMemory();

    tableObj->map = multiMap;
    tableObj->free = false;

    return (PyObject*)tableObj;
  }

  void multidict_dealloc(multidict *self)
  {
    if (self->free && self->map)
        delete self->map;
    self->ob_type->tp_free((PyObject*)self);
  }

  int multidict_init(multidict *self, PyObject *args, PyObject *kwds)
  {
    PyObject *arg = NULL;
    static char *kwlist[] = { (char *)"items", (char *)0 };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O:multidict", kwlist, &arg))
      return -1;

    /*if (arg != NULL) {
      if (PyObject_HasAttrString(arg, "keys"))
        return multidict_merge(self, arg, 1);
      else
        return multidict_mergefromseq2(self, arg, 1);
    }*/

    return 0;
  }

  long multidict_nohash(PyObject *self)
  {
    PyErr_SetString(PyExc_TypeError, "multidict objects are unhashable");
    return -1;
  }

  // forward declaration
  PyObject* multidictiter_new(multidict *, multidict_selectfunc);

  PyObject *multidict_iter(multidict *self)
  {
    return multidictiter_new(self, multidict_select_key);
  }

  int multidict_print(multidict *self, FILE *fp, int flags)
  {
    fprintf(fp, "{");
    if (self->map->empty())
    {
      fprintf(fp, "}");
      return 0;
    }

    unsigned int index = 0;
    for (multimap<string, string>::const_iterator it = self->map->begin(); it != self->map->end(); it++)
    {
      fprintf(fp, "'%s': '%s'", it->first.c_str(), it->second.c_str());

      if (index < self->map->size() - 1)
        fprintf(fp, ", ");

      index++;
    }
    fprintf(fp, "}");

    return 0;
  }

  PyObject* multidict_repr(multidict *self)
  {
    PyObject *s;
    PyObject *t = NULL;

    s = PyString_FromString("{");

    if (self->map->empty())
    {
      PyString_ConcatAndDel(&s, PyString_FromString("}"));
      return s;
    }

    unsigned int index = 0;
    for (multimap<string, string>::const_iterator it = self->map->begin(); it != self->map->end(); it++)
    {
      t = PyString_FromString(it->first.c_str());
      PyString_ConcatAndDel(&s, PyObject_Repr(t));
      Py_XDECREF(t);

      PyString_ConcatAndDel(&s, PyString_FromString(": "));

      t = PyString_FromString(it->second.c_str());
      PyString_ConcatAndDel(&s, PyObject_Repr(t));
      Py_XDECREF(t);

      if (index < self->map->size() - 1)
        PyString_ConcatAndDel(&s, PyString_FromString(", "));
      index++;
    }
    PyString_ConcatAndDel(&s, PyString_FromString("}"));

    return s;
  }

  PyDoc_STRVAR(keys__doc__,
    "T.keys() -> list of T's keys");
  PyObject* multidict_keys(multidict *self)
  {
    PyObject *v = PyList_New((Py_ssize_t)self->map->size());

    Py_ssize_t index = 0;
    for (multimap<string, string>::const_iterator it = self->map->begin(); it != self->map->end(); it++)
    {
      PyList_SetItem(v, index, PyString_FromString(it->first.c_str()));
      index++;
    }

    return v;
  }

  PyDoc_STRVAR(values__doc__,
    "T.values() -> list of T's values");
  PyObject* multidict_values(multidict *self)
  {
    PyObject *v = PyList_New((Py_ssize_t)self->map->size());

    Py_ssize_t index = 0;
    for (multimap<string, string>::const_iterator it = self->map->begin(); it != self->map->end(); it++)
    {
      PyList_SetItem(v, index, PyString_FromString(it->second.c_str()));
      index++;
    }

    return v;
  }

  PyDoc_STRVAR(items__doc__,
    "T.items() -> list of T's (key, value) pairs, as 2-tuples");
  PyObject* multidict_items(multidict *self)
  {
    PyObject *v = PyList_New((Py_ssize_t)self->map->size());

    Py_ssize_t index = 0;
    for (multimap<string, string>::const_iterator it = self->map->begin(); it != self->map->end(); it++)
    {
      PyList_SetItem(v, index, Py_BuildValue("(s,s)", it->first.c_str(), it->second.c_str()));
      index++;
    }

    return v;
  }

  /*PyDoc_STRVAR(update__doc__,
    "T.update(E) -> None.  Update T from E: for k in E.keys(): T[k] = E[k]");
  PyObject* multidict_update(multidict *self, PyObject *other)
  {
    if (multidict_merge(self, other, 1) < 0)
      return NULL

    Py_INCREF(Py_None);
    return Py_None;
  }*/

  PyDoc_STRVAR(copy__doc__,
    "T.copy() -> a shallow copy of T");
  PyObject* multidict_copy(multidict *self)
  {
    multidict *copy = (multidict*)multidict_new(&multidict_Type, NULL, NULL);
    if (copy != NULL)
    {
      copy->free = true;

      for (multimap<string, string>::const_iterator it = self->map->begin(); it != self->map->end(); it++)
        copy->map->insert(pair<string, string>(it->first, it->second));
    }

    return (PyObject*)copy;
  }

  int multidict_compare(multidict *a, multidict *b)
  {
    PyObject *ad, *bd;
    int result;

    ad = PyDict_New();
    bd = PyDict_New();

    PyDict_Merge(ad, (PyObject*)a, 0);
    PyDict_Merge(bd, (PyObject*)b, 0);

    result = PyObject_Compare(ad, bd);

    Py_DECREF(ad);
    Py_DECREF(bd);

    return result;
  }

  PyObject* multidict_richcompare(PyObject *v, PyObject *w, int op)
  {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }

  PyDoc_STRVAR(has_key__doc__,
    "T.has_key(k) -> 1 if T has a key k, else 0");
  PyObject* multidict_has_key(multidict *self, PyObject *key)
  {
    if (!PyString_CheckExact(key))
    {
      PyErr_SetString(PyExc_TypeError, "multidict keys must be strings");
      return NULL;
    }

    if (self->map->find(PyString_AsString(key)) != self->map->end())
      return PyInt_FromLong(1);

    return PyInt_FromLong(0);
  }

  PyDoc_STRVAR(get__doc__,
    "T.get(k[,d]) -> T[k] if T.has_key(k), else d.  d defaults to None.");
  PyObject* multidict_get(multidict *self, PyObject *args)
  {
    PyObject *key;
    PyObject *failobj = Py_None;

    if (!PyArg_ParseTuple(args, "S|O:get", &key, &failobj))
        return NULL;

    PyObject *list = getFromKey(self, PyString_AsString(key));

    /* if no match */
    if (PyList_Size(list) == (Py_ssize_t)0)
    {
      Py_DECREF(list);
      Py_INCREF(failobj);
      return failobj;
    }

    /* if we got one match */
    if (PyList_Size(list) == (Py_ssize_t)1)
    {
      PyObject *v = PyList_GetItem(list, (Py_ssize_t)0);
      Py_INCREF(v);
      Py_DECREF(list);
      return v;
    }

    return list;
  }

  PyDoc_STRVAR(setdefault__doc__,
    "T.setdefault(k[,d]) -> T.get(k,d), also set T[k]=d if not T.has_key(k)");
  PyObject* multidict_setdefault(multidict *self, PyObject *args)
  {
    PyObject *key;
    PyObject *defaultVal = NULL;
    PyObject *val = NULL;

    if (!PyArg_ParseTuple(args, "O|O:setdefault", &key, &defaultVal))
      return NULL;

    if (!PyString_CheckExact(key))
    {
      PyErr_SetString(PyExc_TypeError, "multidict keys must be strings");
      return NULL;
    }

    if (defaultVal && !PyString_CheckExact(defaultVal))
    {
      PyErr_SetString(PyExc_TypeError, "multidict values must be strings");
      return NULL;
    }

    char *strKey = PyString_AsString(key);
    PyObject *list = getFromKey(self, strKey);

    /* if no match */
    if (PyList_Size(list) == (Py_ssize_t)0)
    {
      Py_DECREF(list);
      if (defaultVal == NULL)
      {
        self->map->insert(pair<string, string>(strKey, ""));
        val = PyString_FromString("");
      }
      else
      {
        self->map->insert(pair<string, string>(strKey, PyString_AsString(defaultVal)));
        val = defaultVal;
        Py_XINCREF(val);
      }

      return val;
    }

    /* if we got one match */
    if (PyList_Size(list) == (Py_ssize_t)1)
    {
      PyObject *v = PyList_GetItem(list, (Py_ssize_t)0);
      Py_INCREF(v);
      Py_DECREF(list);
      return v;
    }

    return list;
  }

  PyDoc_STRVAR(clear__doc__,
    "T.clear() -> None.  Remove all items from T.");
  PyObject* multidict_clear(multidict *self)
  {
      self->map->clear();

      Py_INCREF(Py_None);
      return Py_None;
  }

  PyDoc_STRVAR(popitem__doc__,
    "T.popitem() -> (k, v), remove and return some (key, value) pair as a 2-tuple; but raise KeyError if T is empty");
  PyObject* multidict_popitem(multidict *self)
  {
    PyObject *res;

    if (self->map->empty()) {
      PyErr_SetString(PyExc_KeyError, "popitem(): multidict is empty");
      return NULL;
    }

    multimap<string, string>::iterator it = self->map->begin();
    res = Py_BuildValue("(s,s)", it->first.c_str(), it->second.c_str());
    self->map->erase(it);

    return res;
  }

  int multidict_traverse(multidict *self, visitproc visit, void *arg)
  {
    for (multimap<string, string>::const_iterator it = self->map->begin(); it != self->map->end(); it++)
    {
      PyObject *v = PyString_FromString(it->second.c_str());

      int err = visit(v, arg);
      Py_XDECREF(v);
      if (err)
        return err;
    }

    return 0;
  }

  int multidict_tp_clear(multidict *self)
  {
    multidict_clear(self);
    return 0;
  }

  PyDoc_STRVAR(add__doc__,
    "T.add(k, v) -> add (as oppsed to replace) a key k and value v");
  PyObject* multidict_add(multidict *self, PyObject *args)
  {
    const char *val, *key;

    if (!PyArg_ParseTuple(args, "ss", &key, &val))
        return NULL;

    self->map->insert(pair<string, string>(key, val));

    Py_INCREF(Py_None);
    return Py_None;
  }

  PyDoc_STRVAR(iterkeys__doc__,
    "T.iterkeys() -> an iterator over the keys of T");
  PyObject* multidict_iterkeys(multidict *self)
  {
    return multidictiter_new(self, multidict_select_key);
  }

  PyDoc_STRVAR(itervalues__doc__,
    "T.itervalues() -> an iterator over the values of T");
  PyObject* multidict_itervalues(multidict *self)
  {
    return multidictiter_new(self, multidict_select_value);
  }

  PyDoc_STRVAR(iteritems__doc__,
    "T.iteritems() -> an iterator over the (key, value) items of T");
  PyObject* multidict_iteritems(multidict *self)
  {
    return multidictiter_new(self, multidict_select_item);
  }

  int multidict_contains(multidict *self, PyObject *key)
  {
    return (int)PyInt_AsLong(multidict_has_key(self, key));
  }

/* multidictiter Functions */

  PyObject* multidictiter_new(multidict *multidict, multidict_selectfunc select)
  {
    multidictiter *it = (multidictiter*)multidictiter_Type.tp_alloc(&multidictiter_Type, 0);
    if (it == NULL)
      return PyErr_NoMemory();

    Py_INCREF(multidict);
    it->multidict = multidict;
    it->size = multidict->map->size();
    it->iter = multidict->map->begin();
    it->select = select;
    return (PyObject *)it;
  }

  void multidictiter_dealloc(multidictiter *self)
  {
    Py_DECREF(self->multidict);
    self->ob_type->tp_free((PyObject*)self);
  }

  PyObject* multidictiter_iternext(multidictiter *ti)
  {
    // make sure the table hasn't changed while being iterated
    if (ti->size != (int)ti->multidict->map->size()) {
      PyErr_SetString(PyExc_RuntimeError, "multidict changed size during iteration");
      return NULL;
    }

    // return the next key/value pair
    if (ti->iter != ti->multidict->map->end())
    {
      ti->iter++;
      if (ti->iter != ti->multidict->map->end()) {
        return (*ti->select)(ti->iter);
      }
    }

    PyErr_SetObject(PyExc_StopIteration, Py_None);
    return NULL;
  }

  PyObject* multidictiter_next(multidictiter *ti, PyObject *args)
  {
    return multidictiter_iternext(ti);
  }

  PyObject* multidictiter_getiter(PyObject *it)
  {
    Py_INCREF(it);
    return it;
  }

  PyMethodDef multidict_methods[] = {
    {"has_key",     (PyCFunction)multidict_has_key,      METH_O,       has_key__doc__},
    {"get",         (PyCFunction)multidict_get,          METH_VARARGS, get__doc__},
    {"setdefault",  (PyCFunction)multidict_setdefault,   METH_VARARGS, setdefault__doc__},
    {"popitem",     (PyCFunction)multidict_popitem,      METH_NOARGS,  popitem__doc__},
    {"keys",        (PyCFunction)multidict_keys,         METH_NOARGS,  keys__doc__},
    {"items",       (PyCFunction)multidict_items,        METH_NOARGS,  items__doc__},
    {"values",      (PyCFunction)multidict_values,       METH_NOARGS,  values__doc__},
    //{"update",      (PyCFunction)multidict_update,       METH_O,       update__doc__},
    {"clear",       (PyCFunction)multidict_clear,        METH_NOARGS,  clear__doc__},
    {"copy",        (PyCFunction)multidict_copy,         METH_NOARGS,  copy__doc__},
    {"iterkeys",    (PyCFunction)multidict_iterkeys,     METH_NOARGS,  iterkeys__doc__},
    {"itervalues",  (PyCFunction)multidict_itervalues,   METH_NOARGS,  itervalues__doc__},
    {"iteritems",   (PyCFunction)multidict_iteritems,    METH_NOARGS,  iteritems__doc__},
    {"add",         (PyCFunction)multidict_add,          METH_VARARGS, add__doc__},
    {NULL,          NULL,                                       0,            NULL}       /* sentinel */
  };

  PyMappingMethods multidict_as_mapping = {
    (lenfunc)       multidict_length,          /*mp_length*/
    (binaryfunc)    multidict_subscript,       /*mp_subscript*/
    (objobjargproc) multidict_ass_subscript,   /*mp_ass_subscript*/
  };

  /* Hack to implement "key in table" */
 PySequenceMethods multidict_as_sequence = {
    0,                                      /* sq_length */
    0,                                      /* sq_concat */
    0,                                      /* sq_repeat */
    0,                                      /* sq_item */
    0,                                      /* sq_slice */
    0,                                      /* sq_ass_item */
    0,                                      /* sq_ass_slice */
    (objobjproc)multidict_contains,         /* sq_contains */
    0,                                      /* sq_inplace_concat */
    0,                                      /* sq_inplace_repeat */
  };

  PyMethodDef multidictiter_methods[] = {
    {"next", (PyCFunction)multidictiter_next, METH_VARARGS, "it.next() -- get the next value, or raise StopIteration"},
    {NULL, NULL, 0, NULL}
  };

  PyDoc_STRVAR(multidict__doc__,
    "multidict() -> new empty table.\n"
    "multidict(mapping) -> new table initialized from a mapping object's\n"
    "    (key, value) pairs.\n"
    "multidict(seq) -> new table initialized as if via:\n"
    "    d = {}\n"
    "    for k, v in seq:\n"
    "        d[k] = v";);

// Restore code and data sections to normal.

  PyTypeObject multidict_Type;
  PyTypeObject multidictiter_Type;

  void initmultidict_Type()
  {
    PyXBMCInitializeTypeObject(&multidict_Type);

    multidict_Type.tp_name = (char*)"xbmchttp.multidict";
    multidict_Type.tp_basicsize = sizeof(multidict);
    multidict_Type.tp_dealloc = (destructor)multidict_dealloc;
    multidict_Type.tp_print = (printfunc)multidict_print;
    multidict_Type.tp_compare = (cmpfunc)multidict_compare;
    multidict_Type.tp_repr = (reprfunc)multidict_repr;
    multidict_Type.tp_as_sequence = &multidict_as_sequence;
    multidict_Type.tp_as_mapping = &multidict_as_mapping;
    multidict_Type.tp_hash = multidict_nohash;
    multidict_Type.tp_getattro = PyObject_GenericGetAttr;
    multidict_Type.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE;
    multidict_Type.tp_doc = multidict__doc__;
    multidict_Type.tp_traverse = (traverseproc)multidict_traverse;
    multidict_Type.tp_clear = (inquiry)multidict_tp_clear;
    multidict_Type.tp_richcompare = multidict_richcompare;
    multidict_Type.tp_iter = (getiterfunc)multidict_iter;
    multidict_Type.tp_methods = multidict_methods;
    multidict_Type.tp_init = (initproc)multidict_init;
    multidict_Type.tp_new = multidict_new;
    multidict_Type.tp_free = (freefunc)multidict_dealloc;
    multidict_Type.tp_base = &PyDict_Type;
  }

  void initmultidictiter_Type()
  {
    PyXBMCInitializeTypeObject(&multidictiter_Type);

    multidictiter_Type.tp_name = (char*)"xbmchttp.multidictiter";
    multidictiter_Type.tp_basicsize = sizeof(multidictiter);
    multidictiter_Type.tp_dealloc = (destructor)multidictiter_dealloc;
    multidictiter_Type.tp_getattro = PyObject_GenericGetAttr;
    multidictiter_Type.tp_flags = Py_TPFLAGS_DEFAULT;
    multidictiter_Type.tp_iter = (getiterfunc)multidictiter_getiter;
    multidictiter_Type.tp_iternext = (iternextfunc)multidictiter_iternext;
    multidictiter_Type.tp_methods = multidictiter_methods;
    multidictiter_Type.tp_base = &PyDict_Type;
  }
}

#ifdef __cplusplus
}
#endif
