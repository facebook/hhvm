/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "bser.h"

static Py_ssize_t bserobj_tuple_length(PyObject* o) {
  bserObject* obj = (bserObject*)o;

  return PySequence_Length(obj->keys);
}

static PyObject* bserobj_tuple_item(PyObject* o, Py_ssize_t i) {
  bserObject* obj = (bserObject*)o;

  return PySequence_GetItem(obj->values, i);
}

// clang-format off
static PySequenceMethods bserobj_sq = {
  bserobj_tuple_length,      /* sq_length */
  0,                         /* sq_concat */
  0,                         /* sq_repeat */
  bserobj_tuple_item,        /* sq_item */
  0,                         /* sq_ass_item */
  0,                         /* sq_contains */
  0,                         /* sq_inplace_concat */
  0                          /* sq_inplace_repeat */
};
// clang-format on

static void bserobj_dealloc(PyObject* o) {
  bserObject* obj = (bserObject*)o;

  Py_CLEAR(obj->keys);
  Py_CLEAR(obj->values);
  PyObject_Del(o);
}

static PyObject* bserobj_getattrro(PyObject* o, PyObject* name) {
  bserObject* obj = (bserObject*)o;
  Py_ssize_t i, n;
  PyObject* name_bytes = NULL;
  PyObject* key_bytes = NULL;
  PyObject* ret = NULL;
  const char* namestr;
  const char* keystr;

  if (PyIndex_Check(name)) {
    i = PyNumber_AsSsize_t(name, PyExc_IndexError);
    if (i == -1 && PyErr_Occurred()) {
      goto bail;
    }
    ret = PySequence_GetItem(obj->values, i);
    goto bail;
  }

  // We can be passed in Unicode objects here -- we don't support anything other
  // than UTF-8 for keys.
  if (PyUnicode_Check(name)) {
    name_bytes = PyUnicode_AsUTF8String(name);
    if (name_bytes == NULL) {
      goto bail;
    }
    namestr = PyBytes_AsString(name_bytes);
  } else {
    namestr = PyBytes_AsString(name);
  }

  if (namestr == NULL) {
    goto bail;
  }
  // hack^Wfeature to allow mercurial to use "st_size" to reference "size"
  if (!strncmp(namestr, "st_", 3)) {
    namestr += 3;
  }

  n = PyTuple_GET_SIZE(obj->keys);
  for (i = 0; i < n; i++) {
    PyObject* key = PyTuple_GET_ITEM(obj->keys, i);

    if (PyUnicode_Check(key)) {
      key_bytes = PyUnicode_AsUTF8String(key);
      if (key_bytes == NULL) {
        goto bail;
      }
      keystr = PyBytes_AsString(key_bytes);
    } else {
      keystr = PyBytes_AsString(key);
    }

    if (keystr == NULL) {
      goto bail;
    }

    if (!strcmp(keystr, namestr)) {
      ret = PySequence_GetItem(obj->values, i);
      goto bail;
    }
    Py_XDECREF(key_bytes);
    key_bytes = NULL;
  }

  PyErr_Format(
      PyExc_AttributeError, "bserobject has no attribute '%.400s'", namestr);
bail:
  Py_XDECREF(name_bytes);
  Py_XDECREF(key_bytes);
  return ret;
}

// clang-format off
static PyMappingMethods bserobj_map = {
  bserobj_tuple_length,     /* mp_length */
  bserobj_getattrro,        /* mp_subscript */
  0                         /* mp_ass_subscript */
};

PyTypeObject bserObjectType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "bserobj_tuple",           /* tp_name */
  sizeof(bserObject),        /* tp_basicsize */
  0,                         /* tp_itemsize */
  bserobj_dealloc,           /* tp_dealloc */
  0,                         /* tp_print */
  0,                         /* tp_getattr */
  0,                         /* tp_setattr */
  0,                         /* tp_compare */
  0,                         /* tp_repr */
  0,                         /* tp_as_number */
  &bserobj_sq,               /* tp_as_sequence */
  &bserobj_map,              /* tp_as_mapping */
  0,                         /* tp_hash  */
  0,                         /* tp_call */
  0,                         /* tp_str */
  bserobj_getattrro,         /* tp_getattro */
  0,                         /* tp_setattro */
  0,                         /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT,        /* tp_flags */
  "bserobj tuple",           /* tp_doc */
  0,                         /* tp_traverse */
  0,                         /* tp_clear */
  0,                         /* tp_richcompare */
  0,                         /* tp_weaklistoffset */
  0,                         /* tp_iter */
  0,                         /* tp_iternext */
  0,                         /* tp_methods */
  0,                         /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  0,                         /* tp_init */
  0,                         /* tp_alloc */
  0,                         /* tp_new */
};
// clang-format on

int bunser_int(const char** ptr, const char* end, int64_t* val) {
  int needed;
  const char* buf = *ptr;
  int8_t i8;
  int16_t i16;
  int32_t i32;
  int64_t i64;

  if (buf >= end) {
    PyErr_SetString(PyExc_ValueError, "input buffer to small for int encoding");
    return 0;
  }

  switch (buf[0]) {
    case BSER_INT8:
      needed = 2;
      break;
    case BSER_INT16:
      needed = 3;
      break;
    case BSER_INT32:
      needed = 5;
      break;
    case BSER_INT64:
      needed = 9;
      break;
    default:
      PyErr_Format(
          PyExc_ValueError, "invalid bser int encoding 0x%02x", buf[0]);
      return 0;
  }
  if (end - buf < needed) {
    PyErr_SetString(PyExc_ValueError, "input buffer to small for int encoding");
    return 0;
  }
  *ptr = buf + needed;
  switch (buf[0]) {
    case BSER_INT8:
      memcpy(&i8, buf + 1, sizeof(i8));
      *val = i8;
      return 1;
    case BSER_INT16:
      memcpy(&i16, buf + 1, sizeof(i16));
      *val = i16;
      return 1;
    case BSER_INT32:
      memcpy(&i32, buf + 1, sizeof(i32));
      *val = i32;
      return 1;
    case BSER_INT64:
      memcpy(&i64, buf + 1, sizeof(i64));
      *val = i64;
      return 1;
    default:
      return 0;
  }
}

static int bunser_bytestring(
    const char** ptr,
    const char* end,
    const char** start,
    int64_t* len) {
  const char* buf = *ptr;

  // skip string marker
  buf++;
  if (!bunser_int(&buf, end, len)) {
    return 0;
  }

  if (buf + *len > end) {
    PyErr_Format(PyExc_ValueError, "invalid string length in bser data");
    return 0;
  }

  *ptr = buf + *len;
  *start = buf;
  return 1;
}

static PyObject*
bunser_array(const char** ptr, const char* end, const unser_ctx_t* ctx) {
  const char* buf = *ptr;
  int64_t nitems, i;
  int mutable = ctx->is_mutable;
  PyObject* res;

  assert(buf < end);

  // skip array header
  buf++;
  if (!bunser_int(&buf, end, &nitems)) {
    return 0;
  }
  *ptr = buf;

  if (nitems > UINT32_MAX) {
    PyErr_Format(PyExc_ValueError, "too many items for python array");
    return NULL;
  }

  if (nitems > end - buf) {
    // BSER guarantees each value will consume at least one byte of the input.
    PyErr_Format(PyExc_ValueError, "document too short for array's size");
    return NULL;
  }

  if (mutable) {
    res = PyList_New((Py_ssize_t)nitems);
  } else {
    res = PyTuple_New((Py_ssize_t)nitems);
  }

  if (!res) {
    return NULL;
  }

  for (i = 0; i < nitems; i++) {
    PyObject* ele = bser_loads_recursive(ptr, end, ctx);

    if (!ele) {
      Py_DECREF(res);
      return NULL;
    }

    if (mutable) {
      PyList_SET_ITEM(res, i, ele);
    } else {
      PyTuple_SET_ITEM(res, i, ele);
    }
    // DECREF(ele) not required as SET_ITEM steals the ref
  }

  return res;
}

static PyObject*
bunser_object(const char** ptr, const char* end, const unser_ctx_t* ctx) {
  const char* buf = *ptr;
  int64_t nitems, i;
  int mutable = ctx->is_mutable;
  PyObject* res;
  bserObject* obj;

  // skip array header
  buf++;
  if (!bunser_int(&buf, end, &nitems)) {
    return 0;
  }
  *ptr = buf;

  if (nitems > UINT32_MAX) {
    PyErr_Format(PyExc_ValueError, "object too big");
    return NULL;
  }

  if (2 * nitems > end - buf) {
    // Each key-value pair in the input will be at least two bytes long. This
    // check ensures we only pre-allocate an amount of memory for the key and
    // value tuples proportional to the length of the input.
    PyErr_Format(PyExc_ValueError, "document too short for object's size");
    return NULL;
  }

  if (mutable) {
    res = PyDict_New();
  } else {
    obj = PyObject_New(bserObject, &bserObjectType);
    obj->keys = PyTuple_New((Py_ssize_t)nitems);
    obj->values = PyTuple_New((Py_ssize_t)nitems);
    res = (PyObject*)obj;
  }

  for (i = 0; i < nitems; i++) {
    const char* keystr;
    int64_t keylen;
    PyObject* key;
    PyObject* ele;

    if (!bunser_bytestring(ptr, end, &keystr, &keylen)) {
      Py_DECREF(res);
      return NULL;
    }

    if (keylen > LONG_MAX) {
      PyErr_Format(PyExc_ValueError, "string too big for python");
      Py_DECREF(res);
      return NULL;
    }

    if (mutable) {
      // This will interpret the key as UTF-8.
      key = PyUnicode_FromStringAndSize(keystr, (Py_ssize_t)keylen);
    } else {
      // For immutable objects we'll manage key lookups, so we can avoid going
      // through the Unicode APIs. This avoids a potentially expensive and
      // definitely unnecessary conversion to UTF-16 and back for Python 2.
      // TODO: On Python 3 the Unicode APIs are smarter: we might be able to use
      // Unicode keys there without an appreciable performance loss.
      key = PyBytes_FromStringAndSize(keystr, (Py_ssize_t)keylen);
    }

    if (!key) {
      Py_DECREF(res);
      return NULL;
    }

    ele = bser_loads_recursive(ptr, end, ctx);

    if (!ele) {
      Py_DECREF(key);
      Py_DECREF(res);
      return NULL;
    }

    if (mutable) {
      PyDict_SetItem(res, key, ele);
      Py_DECREF(key);
      Py_DECREF(ele);
    } else {
      /* PyTuple_SET_ITEM steals ele, key */
      PyTuple_SET_ITEM(obj->values, i, ele);
      PyTuple_SET_ITEM(obj->keys, i, key);
    }
  }

  return res;
}

static PyObject*
bunser_template(const char** ptr, const char* end, const unser_ctx_t* ctx) {
  const char* buf = *ptr;
  int64_t nitems, i;
  int mutable = ctx->is_mutable;
  PyObject* arrval;
  PyObject* keys;
  Py_ssize_t numkeys, keyidx;
  unser_ctx_t keys_ctx = {0};
  if (mutable) {
    keys_ctx.is_mutable = 1;
    // Decode keys as UTF-8 in this case.
    keys_ctx.value_encoding = "utf-8";
    keys_ctx.value_errors = "strict";
  } else {
    // Treat keys as bytestrings in this case -- we'll do Unicode conversions at
    // lookup time.
  }

  if (buf + 1 >= end) {
    PyErr_SetString(
        PyExc_ValueError, "input buffer to small for template encoding");
    return 0;
  }

  if (buf[1] != BSER_ARRAY) {
    PyErr_Format(PyExc_ValueError, "Expect ARRAY to follow TEMPLATE");
    return NULL;
  }

  // skip header
  buf++;
  *ptr = buf;

  // Load template keys.
  // For keys we don't want to do any decoding right now.
  keys = bunser_array(ptr, end, &keys_ctx);
  if (!keys) {
    return NULL;
  }

  numkeys = PySequence_Length(keys);
  if (numkeys == 0) {
    PyErr_Format(PyExc_ValueError, "Expected non-empty ARRAY in TEMPLATE");
    return NULL;
  }

  // Load number of array elements
  if (!bunser_int(ptr, end, &nitems)) {
    Py_DECREF(keys);
    return 0;
  }

  if (nitems > UINT32_MAX) {
    PyErr_Format(PyExc_ValueError, "Too many items for python");
    Py_DECREF(keys);
    return NULL;
  }

  arrval = PyList_New(0);
  if (!arrval) {
    Py_DECREF(keys);
    return NULL;
  }

  for (i = 0; i < nitems; i++) {
    PyObject* dict = NULL;
    bserObject* obj = NULL;

    if (mutable) {
      dict = PyDict_New();
    } else {
      obj = PyObject_New(bserObject, &bserObjectType);
      if (obj) {
        obj->keys = keys;
        Py_INCREF(obj->keys);
        obj->values = PyTuple_New(numkeys);
      }
      dict = (PyObject*)obj;
    }
    if (!dict) {
    fail:
      Py_DECREF(keys);
      Py_DECREF(arrval);
      return NULL;
    }

    for (keyidx = 0; keyidx < numkeys; keyidx++) {
      PyObject* key;
      PyObject* ele;

      if (*ptr >= end) {
        PyErr_SetString(PyExc_ValueError, "input buffer too small");
        return 0;
      }

      if (**ptr == BSER_SKIP) {
        *ptr = *ptr + 1;
        ele = Py_None;
        Py_INCREF(ele);
      } else {
        ele = bser_loads_recursive(ptr, end, ctx);
      }

      if (!ele) {
        goto fail;
      }

      if (mutable) {
        key = PyList_GET_ITEM(keys, keyidx);
        PyDict_SetItem(dict, key, ele);
        Py_DECREF(ele);
      } else {
        PyTuple_SET_ITEM(obj->values, keyidx, ele);
        // DECREF(ele) not required as SET_ITEM steals the ref
      }
    }

    int error = PyList_Append(arrval, dict);
    Py_DECREF(dict);
    if (error != 0) {
      goto fail;
    }
  }

  Py_DECREF(keys);

  return arrval;
}

PyObject* bser_loads_recursive(
    const char** ptr,
    const char* end,
    const unser_ctx_t* ctx) {
  const char* buf = *ptr;

  if (buf >= end) {
    PyErr_SetString(PyExc_ValueError, "input buffer too small");
    return 0;
  }

  switch (buf[0]) {
    case BSER_INT8:
    case BSER_INT16:
    case BSER_INT32:
    case BSER_INT64: {
      int64_t ival;
      if (!bunser_int(ptr, end, &ival)) {
        return NULL;
      }
// Python 3 has one integer type.
#if PY_MAJOR_VERSION >= 3
      return PyLong_FromLongLong(ival);
#else
      if (ival < LONG_MIN || ival > LONG_MAX) {
        return PyLong_FromLongLong(ival);
      }
      return PyInt_FromSsize_t(Py_SAFE_DOWNCAST(ival, int64_t, Py_ssize_t));
#endif // PY_MAJOR_VERSION >= 3
    }

    case BSER_REAL: {
      if (buf + 1 + sizeof(double) > end) {
        PyErr_SetString(
            PyExc_ValueError, "input buffer too small for real encoding");
        return 0;
      }
      double dval;
      memcpy(&dval, buf + 1, sizeof(dval));
      *ptr = buf + 1 + sizeof(double);
      return PyFloat_FromDouble(dval);
    }

    case BSER_TRUE:
      *ptr = buf + 1;
      Py_INCREF(Py_True);
      return Py_True;

    case BSER_FALSE:
      *ptr = buf + 1;
      Py_INCREF(Py_False);
      return Py_False;

    case BSER_NULL:
      *ptr = buf + 1;
      Py_INCREF(Py_None);
      return Py_None;

    case BSER_BYTESTRING: {
      const char* start;
      int64_t len;

      if (!bunser_bytestring(ptr, end, &start, &len)) {
        return NULL;
      }

      if (len > LONG_MAX) {
        PyErr_Format(PyExc_ValueError, "string too long for python");
        return NULL;
      }

      if (ctx->value_encoding != NULL) {
        return PyUnicode_Decode(
            start, (long)len, ctx->value_encoding, ctx->value_errors);
      } else {
        return PyBytes_FromStringAndSize(start, (long)len);
      }
    }

    case BSER_UTF8STRING: {
      const char* start;
      int64_t len;

      if (!bunser_bytestring(ptr, end, &start, &len)) {
        return NULL;
      }

      if (len > LONG_MAX) {
        PyErr_Format(PyExc_ValueError, "string too long for python");
        return NULL;
      }

      return PyUnicode_Decode(start, (long)len, "utf-8", "strict");
    }

    case BSER_ARRAY:
      return bunser_array(ptr, end, ctx);

    case BSER_OBJECT:
      return bunser_object(ptr, end, ctx);

    case BSER_TEMPLATE:
      return bunser_template(ptr, end, ctx);

    default:
      PyErr_Format(PyExc_ValueError, "unhandled bser opcode 0x%02x", buf[0]);
  }

  return NULL;
}
