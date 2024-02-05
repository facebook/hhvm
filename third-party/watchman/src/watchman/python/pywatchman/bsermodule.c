/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#define PY_SSIZE_T_CLEAN
#include <Python.h> // @manual=fbsource//third-party/python:python
#include <bytesobject.h> // @manual=fbsource//third-party/python:python
#ifdef _MSC_VER
#define inline __inline
#if _MSC_VER >= 1800
#include <stdint.h>
#else
// The compiler associated with Python 2.7 on Windows doesn't ship
// with stdint.h, so define the small subset that we use here.
typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#define UINT32_MAX 4294967295U
#endif
#endif

#include "bser.h"

// clang-format off
/* Return the smallest size int that can store the value */
#define INT_SIZE(x) (((x) == ((int8_t)x))  ? 1 :    \
                     ((x) == ((int16_t)x)) ? 2 :    \
                     ((x) == ((int32_t)x)) ? 4 : 8)

// clang-format on

static const char bser_true = BSER_TRUE;
static const char bser_false = BSER_FALSE;
static const char bser_null = BSER_NULL;
static const char bser_bytestring_hdr = BSER_BYTESTRING;
static const char bser_array_hdr = BSER_ARRAY;
static const char bser_object_hdr = BSER_OBJECT;

static inline uint32_t next_power_2(uint32_t n) {
  n |= (n >> 16);
  n |= (n >> 8);
  n |= (n >> 4);
  n |= (n >> 2);
  n |= (n >> 1);
  return n + 1;
}

// A buffer we use for building up the serialized result
struct bser_buffer {
  char* buf;
  uint32_t wpos;
  uint32_t allocd;
  uint32_t bser_version;
  uint32_t capabilities;
};
typedef struct bser_buffer bser_t;

static int bser_append(bser_t* bser, const char* data, uint32_t len) {
  if (bser->wpos >= UINT32_MAX - len) {
    // 4 GiB overflow
    errno = ENOMEM;
    return 0;
  }

  uint32_t newlen = next_power_2(bser->wpos + len);
  if (newlen == 0) {
    // We wrapped around - can't store 4G in allocd, so give up. This limits
    // total output to 2 GiB.
    errno = ENOMEM;
    return 0;
  }

  if (newlen > bser->allocd) {
    char* nbuf = realloc(bser->buf, newlen);
    if (!nbuf) {
      return 0;
    }

    bser->buf = nbuf;
    bser->allocd = newlen;
  }

  memcpy(bser->buf + bser->wpos, data, len);
  bser->wpos += len;
  return 1;
}

static int bser_init(bser_t* bser, uint32_t version, uint32_t capabilities) {
  bser->allocd = 8192;
  bser->wpos = 0;
  bser->buf = malloc(bser->allocd);
  bser->bser_version = version;
  bser->capabilities = capabilities;
  if (!bser->buf) {
    return 0;
  }

// Leave room for the serialization header, which includes
// our overall length.  To make things simpler, we'll use an
// int32 for the header
#define EMPTY_HEADER "\x00\x01\x05\x00\x00\x00\x00"

// Version 2 also carries an integer indicating the capabilities. The
// capabilities integer comes before the PDU size.
#define EMPTY_HEADER_V2 "\x00\x02\x00\x00\x00\x00\x05\x00\x00\x00\x00"
  if (version == 2) {
    bser_append(bser, EMPTY_HEADER_V2, sizeof(EMPTY_HEADER_V2) - 1);
  } else {
    bser_append(bser, EMPTY_HEADER, sizeof(EMPTY_HEADER) - 1);
  }

  return 1;
}

static void bser_dtor(bser_t* bser) {
  free(bser->buf);
  bser->buf = NULL;
}

static int bser_long(bser_t* bser, int64_t val) {
  int8_t i8;
  int16_t i16;
  int32_t i32;
  int64_t i64;
  char sz;
  int size = INT_SIZE(val);
  char* iptr;

  switch (size) {
    case 1:
      sz = BSER_INT8;
      i8 = (int8_t)val;
      iptr = (char*)&i8;
      break;
    case 2:
      sz = BSER_INT16;
      i16 = (int16_t)val;
      iptr = (char*)&i16;
      break;
    case 4:
      sz = BSER_INT32;
      i32 = (int32_t)val;
      iptr = (char*)&i32;
      break;
    case 8:
      sz = BSER_INT64;
      i64 = (int64_t)val;
      iptr = (char*)&i64;
      break;
    default:
      PyErr_SetString(PyExc_RuntimeError, "Cannot represent this long value!?");
      return 0;
  }

  if (!bser_append(bser, &sz, sizeof(sz))) {
    return 0;
  }

  return bser_append(bser, iptr, size);
}

static int bser_bytestring(bser_t* bser, PyObject* sval) {
  char* buf = NULL;
  Py_ssize_t len;
  int res;
  PyObject* utf = NULL;

  if (PyUnicode_Check(sval)) {
    utf = PyUnicode_AsEncodedString(sval, "utf-8", "ignore");
    sval = utf;
  }

  res = PyBytes_AsStringAndSize(sval, &buf, &len);
  if (res == -1) {
    res = 0;
    goto out;
  }

  if (!bser_append(bser, &bser_bytestring_hdr, sizeof(bser_bytestring_hdr))) {
    res = 0;
    goto out;
  }

  if (!bser_long(bser, len)) {
    res = 0;
    goto out;
  }

  if (len > UINT32_MAX) {
    PyErr_Format(PyExc_ValueError, "string too big");
    res = 0;
    goto out;
  }

  res = bser_append(bser, buf, (uint32_t)len);

out:
  if (utf) {
    Py_DECREF(utf);
  }

  return res;
}

static int bser_recursive(bser_t* bser, PyObject* val) {
  if (PyBool_Check(val)) {
    if (val == Py_True) {
      return bser_append(bser, &bser_true, sizeof(bser_true));
    }
    return bser_append(bser, &bser_false, sizeof(bser_false));
  }

  if (val == Py_None) {
    return bser_append(bser, &bser_null, sizeof(bser_null));
  }

// Python 3 has one integer type.
#if PY_MAJOR_VERSION < 3
  if (PyInt_Check(val)) {
    return bser_long(bser, PyInt_AS_LONG(val));
  }
#endif // PY_MAJOR_VERSION < 3

  if (PyLong_Check(val)) {
    return bser_long(bser, PyLong_AsLongLong(val));
  }

  if (PyBytes_Check(val) || PyUnicode_Check(val)) {
    return bser_bytestring(bser, val);
  }

  if (PyFloat_Check(val)) {
    double dval = PyFloat_AS_DOUBLE(val);
    char sz = BSER_REAL;

    if (!bser_append(bser, &sz, sizeof(sz))) {
      return 0;
    }

    return bser_append(bser, (char*)&dval, sizeof(dval));
  }

  if (PyList_Check(val)) {
    Py_ssize_t i, len = PyList_GET_SIZE(val);

    if (!bser_append(bser, &bser_array_hdr, sizeof(bser_array_hdr))) {
      return 0;
    }

    if (!bser_long(bser, len)) {
      return 0;
    }

    for (i = 0; i < len; i++) {
      PyObject* ele = PyList_GET_ITEM(val, i);

      if (!bser_recursive(bser, ele)) {
        return 0;
      }
    }

    return 1;
  }

  if (PyTuple_Check(val)) {
    Py_ssize_t i, len = PyTuple_GET_SIZE(val);

    if (!bser_append(bser, &bser_array_hdr, sizeof(bser_array_hdr))) {
      return 0;
    }

    if (!bser_long(bser, len)) {
      return 0;
    }

    for (i = 0; i < len; i++) {
      PyObject* ele = PyTuple_GET_ITEM(val, i);

      if (!bser_recursive(bser, ele)) {
        return 0;
      }
    }

    return 1;
  }

  if (PyMapping_Check(val)) {
    Py_ssize_t len = PyMapping_Length(val);
    Py_ssize_t pos = 0;
    PyObject *key, *ele;

    if (!bser_append(bser, &bser_object_hdr, sizeof(bser_object_hdr))) {
      return 0;
    }

    if (!bser_long(bser, len)) {
      return 0;
    }

    while (PyDict_Next(val, &pos, &key, &ele)) {
      if (!bser_bytestring(bser, key)) {
        return 0;
      }
      if (!bser_recursive(bser, ele)) {
        return 0;
      }
    }

    return 1;
  }

  PyErr_SetString(PyExc_ValueError, "Unsupported value type");
  return 0;
}

static PyObject* bser_dumps(PyObject* self, PyObject* args, PyObject* kw) {
  PyObject *val = NULL, *res;
  bser_t bser;
  uint32_t len, bser_version = 1, bser_capabilities = 0;

  (void)self;

  static char* kw_list[] = {"val", "version", "capabilities", NULL};

  if (!PyArg_ParseTupleAndKeywords(
          args,
          kw,
          "O|ii:dumps",
          kw_list,
          &val,
          &bser_version,
          &bser_capabilities)) {
    return NULL;
  }

  if (!bser_init(&bser, bser_version, bser_capabilities)) {
    return PyErr_NoMemory();
  }

  if (!bser_recursive(&bser, val)) {
    bser_dtor(&bser);
    if (errno == ENOMEM) {
      return PyErr_NoMemory();
    }
    // otherwise, we've already set the error to something reasonable
    return NULL;
  }

  // Now fill in the overall length
  if (bser_version == 1) {
    len = bser.wpos - (sizeof(EMPTY_HEADER) - 1);
    memcpy(bser.buf + 3, &len, sizeof(len));
  } else {
    len = bser.wpos - (sizeof(EMPTY_HEADER_V2) - 1);
    // The BSER capabilities block comes before the PDU length
    memcpy(bser.buf + 2, &bser_capabilities, sizeof(bser_capabilities));
    memcpy(bser.buf + 7, &len, sizeof(len));
  }

  res = PyBytes_FromStringAndSize(bser.buf, bser.wpos);
  bser_dtor(&bser);

  return res;
}

static int _pdu_info_helper(
    const char* data,
    const char* end,
    uint32_t* bser_version_out,
    uint32_t* bser_capabilities_out,
    int64_t* expected_len_out,
    off_t* position_out) {
  uint32_t bser_version;
  uint32_t bser_capabilities = 0;
  int64_t expected_len;

  const char* start;
  start = data;
  // Validate the header and length
  if (memcmp(data, EMPTY_HEADER, 2) == 0) {
    bser_version = 1;
  } else if (memcmp(data, EMPTY_HEADER_V2, 2) == 0) {
    bser_version = 2;
  } else {
    PyErr_SetString(PyExc_ValueError, "invalid bser header");
    return 0;
  }

  data += 2;

  if (bser_version == 2) {
    // Expect an integer telling us what capabilities are supported by the
    // remote server (currently unused).
    if (!memcpy(&bser_capabilities, &data, sizeof(bser_capabilities))) {
      return 0;
    }
    data += sizeof(bser_capabilities);
  }

  // Expect an integer telling us how big the rest of the data
  // should be
  if (!bunser_int(&data, end, &expected_len)) {
    return 0;
  }

  *bser_version_out = bser_version;
  *bser_capabilities_out = (uint32_t)bser_capabilities;
  *expected_len_out = expected_len;
  *position_out = (off_t)(data - start);
  return 1;
}

// This function parses the PDU header and provides info about the packet
// Returns false if unsuccessful
static int pdu_info_helper(
    PyObject* self,
    PyObject* args,
    uint32_t* bser_version_out,
    uint32_t* bser_capabilities_out,
    int64_t* total_len_out) {
  const char* start = NULL;
  const char* data = NULL;
  Py_ssize_t datalen = 0;
  const char* end;
  int64_t expected_len;
  off_t position;

  (void)self;

  if (!PyArg_ParseTuple(args, "s#", &start, &datalen)) {
    return 0;
  }
  data = start;
  end = data + datalen;

  if (!_pdu_info_helper(
          data,
          end,
          bser_version_out,
          bser_capabilities_out,
          &expected_len,
          &position)) {
    return 0;
  }
  *total_len_out = (int64_t)(expected_len + position);
  return 1;
}

// Expected use case is to read a packet from the socket and then call
// bser.pdu_info on the packet.  It returns the BSER version, BSER capabilities,
// and the total length of the entire response that the peer is sending,
// including the bytes already received. This allows the client  to compute the
// data size it needs to read before it can decode the data.
static PyObject* bser_pdu_info(PyObject* self, PyObject* args) {
  uint32_t version, capabilities;
  int64_t total_len;
  if (!pdu_info_helper(self, args, &version, &capabilities, &total_len)) {
    return NULL;
  }
  return Py_BuildValue("kkL", version, capabilities, total_len);
}

static PyObject* bser_pdu_len(PyObject* self, PyObject* args) {
  uint32_t version, capabilities;
  int64_t total_len;
  if (!pdu_info_helper(self, args, &version, &capabilities, &total_len)) {
    return NULL;
  }
  return Py_BuildValue("L", total_len);
}

static PyObject* bser_loads(PyObject* self, PyObject* args, PyObject* kw) {
  const char* data = NULL;
  Py_ssize_t datalen = 0;
  const char* start;
  const char* end;
  int64_t expected_len;
  off_t position;
  PyObject* mutable_obj = NULL;
  const char* value_encoding = NULL;
  const char* value_errors = NULL;
  unser_ctx_t ctx = {1, 0};

  static char* kw_list[] = {
      "buf", "mutable", "value_encoding", "value_errors", NULL};

  (void)self;

  if (!PyArg_ParseTupleAndKeywords(
          args,
          kw,
          "s#|Ozz:loads",
          kw_list,
          &start,
          &datalen,
          &mutable_obj,
          &value_encoding,
          &value_errors)) {
    return NULL;
  }

  if (mutable_obj) {
    ctx.is_mutable = PyObject_IsTrue(mutable_obj) > 0 ? 1 : 0;
  }
  ctx.value_encoding = value_encoding;
  if (value_encoding == NULL) {
    ctx.value_errors = NULL;
  } else if (value_errors == NULL) {
    ctx.value_errors = "strict";
  } else {
    ctx.value_errors = value_errors;
  }
  data = start;
  end = data + datalen;

  if (!_pdu_info_helper(
          data,
          end,
          &ctx.bser_version,
          &ctx.bser_capabilities,
          &expected_len,
          &position)) {
    return NULL;
  }

  data = start + position;
  // Verify
  if (expected_len + data != end) {
    PyErr_SetString(PyExc_ValueError, "bser data len != header len");
    return NULL;
  }

  return bser_loads_recursive(&data, end, &ctx);
}

static PyObject* bser_load(PyObject* self, PyObject* args, PyObject* kw) {
  PyObject* load;
  PyObject* load_method;
  PyObject* string;
  PyObject* load_method_args;
  PyObject* load_method_kwargs;
  PyObject* fp = NULL;
  PyObject* mutable_obj = NULL;
  PyObject* value_encoding = NULL;
  PyObject* value_errors = NULL;

  static char* kw_list[] = {
      "fp", "mutable", "value_encoding", "value_errors", NULL};

  (void)self;

  if (!PyArg_ParseTupleAndKeywords(
          args,
          kw,
          "O|OOO:load",
          kw_list,
          &fp,
          &mutable_obj,
          &value_encoding,
          &value_errors)) {
    return NULL;
  }

  load = PyImport_ImportModule("pywatchman.load");
  if (load == NULL) {
    return NULL;
  }
  load_method = PyObject_GetAttrString(load, "load");
  if (load_method == NULL) {
    return NULL;
  }
  // Mandatory method arguments
  load_method_args = Py_BuildValue("(O)", fp);
  if (load_method_args == NULL) {
    return NULL;
  }
  // Optional method arguments
  load_method_kwargs = PyDict_New();
  if (load_method_kwargs == NULL) {
    return NULL;
  }
  if (mutable_obj) {
    PyDict_SetItemString(load_method_kwargs, "mutable", mutable_obj);
  }
  if (value_encoding) {
    PyDict_SetItemString(load_method_kwargs, "value_encoding", value_encoding);
  }
  if (value_errors) {
    PyDict_SetItemString(load_method_kwargs, "value_errors", value_errors);
  }
  string = PyObject_Call(load_method, load_method_args, load_method_kwargs);
  Py_DECREF(load_method_kwargs);
  Py_DECREF(load_method_args);
  Py_DECREF(load_method);
  Py_DECREF(load);
  return string;
}

// clang-format off
static PyMethodDef bser_methods[] = {
  {"loads", (PyCFunction)bser_loads, METH_VARARGS | METH_KEYWORDS,
   "Deserialize string."},
  {"load", (PyCFunction)bser_load, METH_VARARGS | METH_KEYWORDS,
   "Deserialize a file object"},
  {"pdu_info", (PyCFunction)bser_pdu_info, METH_VARARGS,
   "Extract PDU information."},
  {"pdu_len", (PyCFunction)bser_pdu_len, METH_VARARGS,
   "Extract total PDU length."},
  {"dumps",  (PyCFunction)bser_dumps, METH_VARARGS | METH_KEYWORDS,
   "Serialize string."},
  {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef bser_module = {
  PyModuleDef_HEAD_INIT,
  "bser",
  "Efficient encoding and decoding of BSER.",
  -1,
  bser_methods
};
// clang-format on

PyMODINIT_FUNC PyInit_bser(void) {
  PyObject* mod;

  mod = PyModule_Create(&bser_module);
  PyType_Ready(&bserObjectType);

  return mod;
}
#else

PyMODINIT_FUNC initbser(void) {
  (void)Py_InitModule("bser", bser_methods);
  PyType_Ready(&bserObjectType);
}
#endif // PY_MAJOR_VERSION >= 3

/* vim:ts=2:sw=2:et:
 */
