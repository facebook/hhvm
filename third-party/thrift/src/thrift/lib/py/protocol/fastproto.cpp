/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define __STDC_LIMIT_MACROS
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdint.h>
#include <type_traits>

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/ProtocolReaderWithRefill.h>

#include <folly/Format.h>
#include <folly/Range.h>
#include <folly/ScopeGuard.h>

using apache::thrift::BinaryProtocolReaderWithRefill;
using apache::thrift::BinaryProtocolWriter;
using apache::thrift::CompactProtocolReaderWithRefill;
using apache::thrift::CompactProtocolWriter;
using apache::thrift::protocol::TType;

#if PY_MAJOR_VERSION >= 3
#define FROM_LONG PyLong_FromLong
#define AS_LONG PyLong_AsLong
#else
#define FROM_LONG PyInt_FromLong
#define AS_LONG PyInt_AsLong
#include <cStringIO.h> // @manual
#endif

#if PY_MAJOR_VERSION >= 3
static PyTypeObject* BytesIOType;
#endif
static PyObject* ExceptionsModule;

// Stolen from cStringIO.c and also works for Python 3

#if PY_MAJOR_VERSION >= 3 && PY_MINOR_VERSION >= 5
/* The structure changes in https://hg.python.org/cpython/rev/2e29d54843a4.
 * The first affected Python version is 3.5.0a1. */
typedef struct {
  PyObject_HEAD PyObject* buf;
  Py_ssize_t pos, string_size;
} IOobject;
#define IOBUF(O) PyBytes_AS_STRING((O)->buf)
#else
typedef struct {
  PyObject_HEAD char* buf;
  Py_ssize_t pos, string_size;
} IOobject;
#define IOBUF(O) ((O)->buf)
#endif

#define IOOOBJECT(O) ((IOobject*)(O))

// Stolen from fastbinary.c
#define INTERN_STRING(value) _intern_##value

static PyObject* INTERN_STRING(cstringio_buf);
static PyObject* INTERN_STRING(cstringio_refill);
static PyObject* INTERN_STRING(to_thrift);
static PyObject* INTERN_STRING(from_thrift);

typedef struct {
  PyObject* stringiobuf;
  PyObject* refill_callable;
} DecodeBuffer;

static void free_decodebuf(DecodeBuffer* d) {
  Py_XDECREF(d->stringiobuf);
  Py_XDECREF(d->refill_callable);
}

static bool decode_buffer_from_obj(DecodeBuffer* dest, PyObject* obj) {
  dest->stringiobuf = PyObject_GetAttr(obj, INTERN_STRING(cstringio_buf));
  if (!dest->stringiobuf) {
    return false;
  }

#if PY_MAJOR_VERSION >= 3
  if (!PyObject_TypeCheck(dest->stringiobuf, BytesIOType)) {
#else
  if (!PycStringIO_InputCheck(dest->stringiobuf)) {
#endif
    free_decodebuf(dest);
    PyErr_SetString(PyExc_TypeError, "expecting stringio input");
    return false;
  }

  dest->refill_callable =
      PyObject_GetAttr(obj, INTERN_STRING(cstringio_refill));
  if (!dest->refill_callable) {
    free_decodebuf(dest);
    return false;
  }

  if (!PyCallable_Check(dest->refill_callable)) {
    free_decodebuf(dest);
    PyErr_SetString(PyExc_TypeError, "expecting callable");
    return false;
  }

  return true;
}

#define INT_CONV_ERROR_OCCURRED(v) (((v) == -1) && PyErr_Occurred())
#define CHECK_RANGE(v, min, max) (((v) <= (max)) && ((v) >= (min)))

typedef struct {
  TType element_type;
  PyObject* typeargs;
} SetListTypeArgs;

typedef struct {
  TType ktype;
  TType vtype;
  PyObject* ktypeargs;
  PyObject* vtypeargs;
} MapTypeArgs;

typedef struct {
  PyObject* klass;
  PyObject* spec;
  bool isunion;
  PyObject* adapter;
} StructTypeArgs;

typedef struct {
  int tag;
  TType type;
  PyObject* attrname;
  PyObject* typeargs;
  PyObject* defval;
} StructItemSpec;

static bool parse_set_list_args(SetListTypeArgs* dest, PyObject* typeargs) {
  long element_type;

  if (PyTuple_Size(typeargs) != 2) {
    PyErr_SetString(
        PyExc_TypeError, "expecting tuple of size 2 for list/set type args");
    return false;
  }

  element_type = AS_LONG(PyTuple_GET_ITEM(typeargs, 0));
  if (INT_CONV_ERROR_OCCURRED(element_type)) {
    return false;
  }

  dest->element_type = static_cast<TType>(element_type);
  dest->typeargs = PyTuple_GET_ITEM(typeargs, 1);

  return true;
}

static bool parse_map_args(MapTypeArgs* dest, PyObject* typeargs) {
  long ktype, vtype;

  if (PyTuple_Size(typeargs) != 4) {
    PyErr_SetString(
        PyExc_TypeError, "expecting tuple of size 4 for map type args");
    return false;
  }

  ktype = AS_LONG(PyTuple_GET_ITEM(typeargs, 0));
  if (INT_CONV_ERROR_OCCURRED(ktype)) {
    return false;
  }

  vtype = AS_LONG(PyTuple_GET_ITEM(typeargs, 2));
  if (INT_CONV_ERROR_OCCURRED(vtype)) {
    return false;
  }

  dest->ktype = static_cast<TType>(ktype);
  dest->vtype = static_cast<TType>(vtype);
  dest->ktypeargs = PyTuple_GET_ITEM(typeargs, 1);
  dest->vtypeargs = PyTuple_GET_ITEM(typeargs, 3);

  return true;
}

static bool parse_struct_args(StructTypeArgs* dest, PyObject* typeargs) {
  const auto size = PyList_Size(typeargs);
  if (size != 3 && size != 4) {
    PyErr_SetString(
        PyExc_TypeError, "expecting list of size 3 or 4 for struct args");
    return false;
  }

  dest->klass = PyList_GET_ITEM(typeargs, 0);
  dest->spec = PyList_GET_ITEM(typeargs, 1);
  dest->isunion = (PyObject_IsTrue(PyList_GET_ITEM(typeargs, 2)) == 1);
  dest->adapter = (size >= 4) ? PyList_GET_ITEM(typeargs, 3) : Py_None;

  return true;
}

static int parse_struct_item_spec(StructItemSpec* dest, PyObject* spec_tuple) {
  long tag, type;

  // i'd like to use ParseArgs here, but it seems to be a bottleneck.
  if (PyTuple_Size(spec_tuple) != 6) {
    PyErr_SetString(PyExc_TypeError, "expecting 6 arguments for spec tuple");
    return false;
  }

  tag = AS_LONG(PyTuple_GET_ITEM(spec_tuple, 0));
  if (INT_CONV_ERROR_OCCURRED(tag)) {
    return false;
  }

  type = AS_LONG(PyTuple_GET_ITEM(spec_tuple, 1));
  if (INT_CONV_ERROR_OCCURRED(type)) {
    return false;
  }

  dest->tag = tag;
  dest->type = static_cast<TType>(type);
  dest->attrname = PyTuple_GET_ITEM(spec_tuple, 2);
  dest->typeargs = PyTuple_GET_ITEM(spec_tuple, 3);
  dest->defval = PyTuple_GET_ITEM(spec_tuple, 4);
  // Arg #5 is the 'required' field, which is ignored when serializing.

  return true;
}

static inline bool check_ssize_t_32(Py_ssize_t len) {
  // error from getting the int
  if (INT_CONV_ERROR_OCCURRED(len)) {
    return false;
  }
  if (!CHECK_RANGE(len, 0, INT32_MAX)) {
    PyErr_SetString(PyExc_OverflowError, "string size out of range");
    return false;
  }
  return true;
}

static inline bool parse_pyint(
    PyObject* o, int32_t* ret, int32_t min, int32_t max) {
  long val;
  if (PyLong_Check(o)) {
    val = AS_LONG(o);
  } else {
    PyObject* tmp = PyNumber_Long(o);
    if (tmp == nullptr) {
      PyErr_Format(PyExc_TypeError, "Unable to convert %R to int()", o);
      return false;
    } else {
      val = AS_LONG(tmp);
      Py_DECREF(tmp);
    }
  }
  if (INT_CONV_ERROR_OCCURRED(val)) {
    return false;
  }
  if (!CHECK_RANGE(val, min, max)) {
    PyErr_SetString(PyExc_OverflowError, "int out of range");
    return false;
  }

  *ret = (int32_t)val;
  return true;
}

static inline bool parse_pyfloat(PyObject* o, double* ret) {
  double val = PyFloat_AsDouble(o);
  if (val == -1.0 && PyErr_Occurred()) {
    return false;
  }
  *ret = val;
  return true;
}

/**
 * Main loop for reading a struct and serializing fields.
 */
template <typename Writer>
static bool encode_impl(
    Writer* writer,
    PyObject* value,
    PyObject* typeargs,
    TType type,
    int utf8strings) {
  switch (type) {
    case TType::T_BOOL: {
      int v = PyObject_IsTrue(value);
      if (v == -1) {
        return false;
      }
      writer->writeBool(v == 1);
      break;
    }
    case TType::T_I08: {
      int32_t val;
      if (!parse_pyint(value, &val, INT8_MIN, INT8_MAX)) {
        return false;
      }
      writer->writeByte((int8_t)val);
      break;
    }
    case TType::T_I16: {
      int32_t val;
      if (!parse_pyint(value, &val, INT16_MIN, INT16_MAX)) {
        return false;
      }
      writer->writeI16((int16_t)val);
      break;
    }
    case TType::T_I32: {
      int32_t val;
      if (!parse_pyint(value, &val, INT32_MIN, INT32_MAX)) {
        return false;
      }
      writer->writeI32(val);
      break;
    }
    case TType::T_I64: {
      int64_t val;
      if (PyLong_Check(value)) {
        val = PyLong_AsLongLong(value);
      } else {
        PyObject* tmp = PyNumber_Long(value);
        if (tmp == nullptr) {
          PyErr_Format(PyExc_TypeError, "Unable to convert %R to int()", value);
          return false;
        } else {
          val = PyLong_AsLongLong(tmp);
          Py_DECREF(tmp);
        }
      }
      if (INT_CONV_ERROR_OCCURRED(val)) {
        return false;
      }
      if (!CHECK_RANGE(val, INT64_MIN, INT64_MAX)) {
        PyErr_SetString(PyExc_OverflowError, "int out of range");
        return false;
      }
      writer->writeI64(val);
      break;
    }
    case TType::T_DOUBLE: {
      double val;
      if (!parse_pyfloat(value, &val)) {
        return false;
      }
      writer->writeDouble(val);
      break;
    }
    case TType::T_FLOAT: {
      double val;
      if (!parse_pyfloat(value, &val)) {
        return false;
      }
      writer->writeFloat((float)val);
      break;
    }
    case TType::T_STRING: {
      bool encoded = false;
      Py_ssize_t len;

#if PY_MAJOR_VERSION >= 3
      if (!PyBytes_Check(value)) {
        value = PyUnicode_AsUTF8String(value);
        if (value == nullptr) {
          PyErr_SetString(PyExc_TypeError, "can not encode");
          return false;
        }
        encoded = true;
      }
      len = PyBytes_Size(value);
#else
      if (utf8strings && value->ob_type == &PyUnicode_Type) {
        value = PyUnicode_AsUTF8String(value);
        if (value == nullptr) {
          PyErr_SetString(PyExc_TypeError, "can not encode using utf8");
          return false;
        }
        encoded = true;
      }
      len = PyString_Size(value);
#endif

      if (!check_ssize_t_32(len)) {
        return false;
      }

#if PY_MAJOR_VERSION >= 3
      folly::StringPiece str(PyBytes_AsString(value), len);
#else
      folly::StringPiece str(PyString_AsString(value), len);
#endif
      writer->writeString(str);
      if (encoded) {
        Py_DECREF(value);
      }
      break;
    }
    case TType::T_LIST:
    case TType::T_SET: {
      Py_ssize_t len;
      SetListTypeArgs parsedargs;
      PyObject* item;
      PyObject* iterator;

      if (!parse_set_list_args(&parsedargs, typeargs)) {
        return false;
      }

      len = PyObject_Length(value);

      if (!check_ssize_t_32(len)) {
        return false;
      }

      writer->writeListBegin(parsedargs.element_type, (uint32_t)len);
      iterator = PyObject_GetIter(value);
      if (iterator == nullptr) {
        return false;
      }

      while ((item = PyIter_Next(iterator))) {
        if (!encode_impl(
                writer,
                item,
                parsedargs.typeargs,
                parsedargs.element_type,
                utf8strings)) {
          Py_DECREF(item);
          Py_DECREF(iterator);
          return false;
        }
        Py_DECREF(item);
      }

      Py_DECREF(iterator);
      writer->writeListEnd();

      if (PyErr_Occurred()) {
        return false;
      }
      break;
    }
    case TType::T_MAP: {
      PyObject *k, *v;
      Py_ssize_t pos = 0, len;

      MapTypeArgs parsedargs;

      len = PyDict_Size(value);
      if (!check_ssize_t_32(len)) {
        return false;
      }

      if (!parse_map_args(&parsedargs, typeargs)) {
        return false;
      }

      writer->writeMapBegin(parsedargs.ktype, parsedargs.vtype, (uint32_t)len);
      while (PyDict_Next(value, &pos, &k, &v)) {
        Py_INCREF(k);
        Py_INCREF(v);

        if (!encode_impl(
                writer,
                k,
                parsedargs.ktypeargs,
                parsedargs.ktype,
                utf8strings) ||
            !encode_impl(
                writer,
                v,
                parsedargs.vtypeargs,
                parsedargs.vtype,
                utf8strings)) {
          Py_DECREF(k);
          Py_DECREF(v);
          return false;
        }
        Py_DECREF(k);
        Py_DECREF(v);
      }
      writer->writeMapEnd();
      break;
    }
    case TType::T_STRUCT: {
      StructTypeArgs parsedargs;

      if (!parse_struct_args(&parsedargs, typeargs)) {
        return false;
      }

      Py_ssize_t nspec = PyTuple_Size(parsedargs.spec);
      if (nspec == -1) {
        return false;
      }

      auto guard = folly::makeDismissedGuard([&] { Py_DECREF(value); });
      if (parsedargs.adapter != Py_None && value != Py_None) {
        value = PyObject_CallMethodObjArgs(
            parsedargs.adapter, INTERN_STRING(to_thrift), value, nullptr);
        if (!value) {
          return false;
        }
        guard.rehire();
      }

      if (parsedargs.isunion) {
        // Union only has a field and a value.
        writer->writeStructBegin("");
        PyObject* field = PyObject_GetAttrString(value, "field");
        if (!field) {
          return false;
        }

        int fid = static_cast<int>(AS_LONG(field));
        PyObject* spec_tuple = PyTuple_GET_ITEM(parsedargs.spec, fid);
        if (spec_tuple != Py_None) {
          StructItemSpec parsedspec;
          PyObject* instval = nullptr;

          if (!parse_struct_item_spec(&parsedspec, spec_tuple)) {
            return false;
          }

          instval = PyObject_GetAttrString(value, "value");
          if (!instval) {
            return false;
          }

          if (instval == Py_None) {
            Py_DECREF(instval);
            return false;
          }

#if PY_MAJOR_VERSION >= 3
          const char* fieldname = PyUnicode_AsUTF8(parsedspec.attrname);
#else
          const char* fieldname = PyString_AsString(parsedspec.attrname);
#endif
          writer->writeFieldBegin(fieldname, parsedspec.type, parsedspec.tag);
          if (!encode_impl(
                  writer,
                  instval,
                  parsedspec.typeargs,
                  parsedspec.type,
                  utf8strings)) {
            Py_DECREF(instval);
            return false;
          }

          Py_DECREF(instval);
          writer->writeFieldEnd();
        }

        writer->writeFieldStop();
        writer->writeStructEnd();
        break;
      }

      // Both binary and compact ignore the struct name.
      writer->writeStructBegin("");
      for (Py_ssize_t i = 0; i < nspec; i++) {
        StructItemSpec parsedspec;
        PyObject* spec_tuple;
        PyObject* instval = nullptr;

        spec_tuple = PyTuple_GET_ITEM(parsedargs.spec, i);
        if (spec_tuple == Py_None) {
          continue;
        }

        if (!parse_struct_item_spec(&parsedspec, spec_tuple)) {
          return false;
        }

        instval = PyObject_GetAttr(value, parsedspec.attrname);
        if (!instval) {
          return false;
        }

        if (instval == Py_None) {
          Py_DECREF(instval);
          continue;
        }

#if PY_MAJOR_VERSION >= 3
        const char* fieldname = PyUnicode_AsUTF8(parsedspec.attrname);
#else
        const char* fieldname = PyString_AsString(parsedspec.attrname);
#endif
        writer->writeFieldBegin(fieldname, parsedspec.type, parsedspec.tag);
        if (!encode_impl(
                writer,
                instval,
                parsedspec.typeargs,
                parsedspec.type,
                utf8strings)) {
          Py_DECREF(instval);
          return false;
        }

        Py_DECREF(instval);
        writer->writeFieldEnd();
      }

      writer->writeFieldStop();
      writer->writeStructEnd();
      break;
    }
    case TType::T_STOP:
    case TType::T_VOID:
    case TType::T_UTF8:
    case TType::T_UTF16:
    case TType::T_U64:
    case TType::T_STREAM:
    default:
      PyErr_SetString(PyExc_TypeError, "Unexpected TType");
      return false;
  }

  return true;
}

template <typename Reader>
static PyObject* decode_val(
    Reader* reader,
    TType type,
    PyObject* typeargs,
    int utf8strings,
    StructTypeArgs* args);

template <typename Reader>
static bool decode_struct(
    Reader* reader, PyObject* value, StructTypeArgs* args, int utf8strings) {
  int speclen = PyTuple_Size(args->spec);
  if (speclen == -1) {
    return false;
  }

  std::string sname;
  reader->readStructBegin(sname);

  std::string fname;
  TType ftype;
  int16_t fid;
  PyObject* itemspec;
  StructItemSpec parsedspec;

  int first_tag = 0;
  bool first_tag_read = false;
  PyObject* first_item_spec;
  StructItemSpec first_parsed_spec;

  while (true) {
    PyObject* fieldval = nullptr;
    reader->readFieldBegin(fname, ftype, fid);
    if (ftype == TType::T_STOP) {
      break;
    }

    if (!first_tag_read) {
      first_tag_read = true;
      if (speclen == 0) { // Empty struct and all fields will be skipped
        first_item_spec = Py_None;
      } else {
        first_item_spec = PyTuple_GET_ITEM(args->spec, 0);
        if (first_item_spec != Py_None) {
          if (!parse_struct_item_spec(&first_parsed_spec, first_item_spec)) {
            return false;
          }
          first_tag = first_parsed_spec.tag;
        }
      }
    }

    fid -= first_tag;
    if (fid > 0 && fid < speclen) {
      itemspec = PyTuple_GET_ITEM(args->spec, fid);
    } else if (fid == 0) {
      itemspec = first_item_spec;
    } else {
      itemspec = Py_None;
    }

    if (itemspec == Py_None) {
      reader->skip(ftype);
      continue;
    }

    if (fid == 0) {
      parsedspec = first_parsed_spec;
    } else if (!parse_struct_item_spec(&parsedspec, itemspec)) {
      return false;
    }

    if (parsedspec.type != ftype) {
      reader->skip(ftype);
      continue;
    }

    fieldval = decode_val(
        reader, parsedspec.type, parsedspec.typeargs, utf8strings, args);

    if (!fieldval) {
      PyObject *pType, *pValue, *pTraceback;
      PyErr_Fetch(&pType, &pValue, &pTraceback);
      if (!PyErr_GivenExceptionMatches(pType, PyExc_UnicodeDecodeError)) {
        PyErr_Restore(pType, pValue, pTraceback);
        return false;
      }
#if PY_MAJOR_VERSION >= 3
      const char* fieldName = PyUnicode_AsUTF8(parsedspec.attrname);
#else
      const char* fieldName = PyString_AsString(parsedspec.attrname);
#endif
      PyObject* create_func_method_name = Py_BuildValue(
          "s", "create_ThriftUnicodeDecodeError_from_UnicodeDecodeError");
      PyObject* pFieldName = Py_BuildValue("s", fieldName);

      PyObject* decode_error = PyObject_CallMethodObjArgs(
          ExceptionsModule, create_func_method_name, pValue, pFieldName, NULL);
      PyObject* thrift_decode_error_type = PyObject_Type(decode_error);
      PyErr_SetObject(thrift_decode_error_type, decode_error);
      Py_DECREF(pType);
      Py_DECREF(pValue);
      if (pTraceback != nullptr) {
        Py_DECREF(pTraceback);
      }
      return false;
    }

    if (args->isunion) {
      PyObject* fieldobj = PyObject_GetAttrString(value, "field");
      if (!fieldobj) {
        Py_DECREF(fieldval);
        return false;
      }
      int field = (int)AS_LONG(fieldobj);
      if (field == -1 && PyErr_Occurred()) {
        Py_DECREF(fieldval);
        Py_DECREF(fieldobj);
        return false;
      }
      if (field != 0) {
        PyErr_SetString(PyExc_AssertionError, "field already set in union");
        Py_DECREF(fieldval);
        Py_DECREF(fieldobj);
        return false;
      }
      Py_DECREF(fieldobj);

      PyObject* valueobj = PyObject_GetAttrString(value, "value");
      if (!valueobj) {
        Py_DECREF(fieldobj);
        return false;
      }
      if (valueobj != Py_None) {
        PyErr_SetString(PyExc_AssertionError, "value already set in union");
        Py_DECREF(fieldval);
        Py_DECREF(valueobj);
        return false;
      }
      Py_DECREF(valueobj);

      PyObject* tagobj = FROM_LONG(parsedspec.tag);
      if (!tagobj) {
        return false;
      }
      if (PyObject_SetAttrString(value, "value", fieldval) == -1 ||
          PyObject_SetAttrString(value, "field", tagobj) == -1) {
        Py_DECREF(fieldval);
        Py_DECREF(tagobj);
        return false;
      }
      Py_DECREF(tagobj);
    } else {
      if (PyObject_SetAttr(value, parsedspec.attrname, fieldval) == -1) {
        Py_DECREF(fieldval);
        return false;
      }
    }
    Py_DECREF(fieldval);
    reader->readFieldEnd();
  }

  reader->readStructEnd();
  return true;
}

template <typename Reader>
static PyObject* decode_val(
    Reader* reader,
    TType type,
    PyObject* typeargs,
    int utf8strings,
    StructTypeArgs* args) {
  switch (type) {
    case TType::T_BOOL: {
      bool v;
      reader->readBool(v);
      if (v) {
        Py_RETURN_TRUE;
      } else {
        Py_RETURN_FALSE;
      }
    }
    case TType::T_I08: {
      int8_t v;
      reader->readByte(v);
      return FROM_LONG(v);
    }
    case TType::T_I16: {
      int16_t v;
      reader->readI16(v);
      return FROM_LONG(v);
    }
    case TType::T_I32: {
      int32_t v;
      reader->readI32(v);
      return FROM_LONG(v);
    }
    case TType::T_I64: {
      int64_t v;
      reader->readI64(v);
      if (CHECK_RANGE(v, LONG_MIN, LONG_MAX)) {
        return FROM_LONG((long)v);
      }
      return PyLong_FromLongLong(v);
    }
    case TType::T_DOUBLE: {
      double v;
      reader->readDouble(v);
      return PyFloat_FromDouble(v);
    }
    case TType::T_FLOAT: {
      float v;
      reader->readFloat(v);
      return PyFloat_FromDouble((double)v);
    }
    case TType::T_STRING: {
      std::string s;
      reader->readString(s);
      if (utf8strings && PyObject_IsTrue(typeargs)) {
        return PyUnicode_FromStringAndSize(s.data(), s.length());
      } else {
#if PY_MAJOR_VERSION >= 3
        return PyBytes_FromStringAndSize(s.data(), s.length());
#else
        return PyString_FromStringAndSize(s.data(), s.length());
#endif
      }
    }
    case TType::T_LIST:
    case TType::T_SET: {
      SetListTypeArgs parsedargs;
      TType ttype;
      uint32_t len;

      if (!parse_set_list_args(&parsedargs, typeargs)) {
        return nullptr;
      }

      reader->readListBegin(ttype, len);
      if (ttype != parsedargs.element_type) {
        PyErr_SetString(
            PyExc_TypeError, "got wrong ttype while reading list/set field");
        return nullptr;
      }

      PyObject* ret = PyList_New(len);
      if (!ret) {
        return nullptr;
      }

      for (auto i = 0u; i < len; i++) {
        PyObject* item = decode_val(
            reader,
            parsedargs.element_type,
            parsedargs.typeargs,
            utf8strings,
            args);
        if (!item) {
          Py_DECREF(ret);
          return nullptr;
        }
        PyList_SET_ITEM(ret, i, item);
      }

      reader->readListEnd();
      if (type == TType::T_SET) {
        PyObject* setret = PySet_New(ret);
        Py_DECREF(ret);
        return setret;
      }
      return ret;
    }
    case TType::T_MAP: {
      MapTypeArgs parsedargs;
      TType ktype = apache::thrift::protocol::T_STOP;
      TType vtype = apache::thrift::protocol::T_STOP;
      uint32_t len;

      if (!parse_map_args(&parsedargs, typeargs)) {
        return nullptr;
      }

      reader->readMapBegin(ktype, vtype, len);
      if (ktype != parsedargs.ktype || vtype != parsedargs.vtype) {
        if (len == 0 &&
            std::is_same<Reader, CompactProtocolReaderWithRefill>::value) {
          reader->readMapEnd();
          return PyDict_New();
        }
        PyErr_SetString(
            PyExc_TypeError, "got wrong ttype while reading map field");
        return nullptr;
      }

      PyObject* ret = PyDict_New();
      if (!ret) {
        return nullptr;
      }

      for (auto i = 0u; i < len; i++) {
        PyObject* k = nullptr;
        PyObject* v = nullptr;
        k = decode_val(reader, ktype, parsedargs.ktypeargs, utf8strings, args);
        if (!k) {
          Py_DECREF(ret);
          return nullptr;
        }
        v = decode_val(reader, vtype, parsedargs.vtypeargs, utf8strings, args);
        if (!v) {
          Py_DECREF(ret);
          Py_DECREF(k);
          return nullptr;
        }
        if (PyDict_SetItem(ret, k, v) == -1) {
          Py_DECREF(ret);
          Py_DECREF(k);
          Py_DECREF(v);
          return nullptr;
        }

        Py_DECREF(k);
        Py_DECREF(v);
      }

      reader->readMapEnd();
      return ret;
    }
    case TType::T_STRUCT: {
      StructTypeArgs parsedargs;
      PyObject* ret;

      if (!parse_struct_args(&parsedargs, typeargs)) {
        return nullptr;
      }

      ret = PyObject_CallObject(parsedargs.klass, nullptr);
      if (!ret) {
        return nullptr;
      }

      if (!decode_struct(reader, ret, &parsedargs, utf8strings)) {
        Py_DECREF(ret);
        return nullptr;
      }

      if (parsedargs.adapter != Py_None && ret != Py_None) {
        PyObject* adapted = PyObject_CallMethodObjArgs(
            parsedargs.adapter, INTERN_STRING(from_thrift), ret, nullptr);
        Py_DECREF(ret);
        return adapted;
      }

      return ret;
    }
    case TType::T_STOP:
    case TType::T_VOID:
    case TType::T_UTF16:
    case TType::T_UTF8:
    case TType::T_U64:
    default:
      PyErr_SetString(PyExc_TypeError, "Unexpected TType");
      return nullptr;
  }
}

/* --- TOP-LEVEL WRAPPER AND TEMPLATES --- */

template <typename Writer>
static PyObject* encodeT(PyObject* enc_obj, PyObject* spec, int utf8strings) {
  folly::IOBufQueue encoded(folly::IOBufQueue::cacheChainLength());
  Writer writer;
  writer.setOutput(&encoded);

  encode_impl(&writer, enc_obj, spec, TType::T_STRUCT, utf8strings);

  if (PyErr_Occurred()) {
    return nullptr;
  }

  auto buf = encoded.split(encoded.chainLength());
  buf->coalesce();
#if PY_MAJOR_VERSION >= 3
  return PyBytes_FromStringAndSize((const char*)buf->data(), buf->length());
#else
  return PyString_FromStringAndSize((const char*)buf->data(), buf->length());
#endif
}

static std::unique_ptr<folly::IOBuf> refill(
    DecodeBuffer* input,
    const uint8_t* remaining,
    Py_ssize_t rlen,
    int read,
    int len) {
  PyObject* newiobuf;

  IOobject* ioobj = IOOOBJECT(input->stringiobuf);
  ioobj->pos += read;

#if PY_MAJOR_VERSION >= 3
  newiobuf = PyObject_CallFunction(
      input->refill_callable, (char*)"y#i", remaining, rlen, len, nullptr);
#else
  newiobuf = PyObject_CallFunction(
      input->refill_callable, (char*)"s#i", remaining, rlen, len, nullptr);
#endif

  if (!newiobuf) {
    return nullptr;
  }
  Py_CLEAR(input->stringiobuf);
  input->stringiobuf = newiobuf;

  ioobj = IOOOBJECT(newiobuf);
  // This IOBuf is returned to the protocol reader and it doesn't own
  // the underlying buffer. The protocol reader should only call refill
  // when it no longer needs its current IOBuf because the underlying
  // buffer is freed here.
  return folly::IOBuf::wrapBuffer(
      IOBUF(ioobj) + ioobj->pos, ioobj->string_size - ioobj->pos);
}

template <typename Reader>
static bool decodeT(
    DecodeBuffer* input,
    PyObject* dec_obj,
    StructTypeArgs* args,
    int utf8strings) {
  auto refiller = [input](
                      const uint8_t* remaining, int rlen, int read, int len) {
    return refill(input, remaining, rlen, read, len);
  };
  Reader reader(refiller);
  IOobject* ioobj = IOOOBJECT(input->stringiobuf);
  auto buf = folly::IOBuf::wrapBuffer(
      static_cast<char*>(IOBUF(ioobj)) + ioobj->pos, // IOBUF(ioobj) is char[1]
      ioobj->string_size - ioobj->pos);
  reader.setInput(buf.get());

  try {
    bool ret = decode_struct(&reader, dec_obj, args, utf8strings);
    ioobj = IOOOBJECT(input->stringiobuf);
    ioobj->pos += reader.totalBytesRead();
    return ret;
  } catch (const std::exception& e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return false;
  }
}

static PyObject* encode(PyObject* /*self*/, PyObject* args, PyObject* kws) {
  PyObject* enc_obj;
  PyObject* spec;
  int utf8strings = 0;
  int protoid = 0;

  static char* kwlist[] = {
      (char*)"enc",
      (char*)"spec",
      (char*)"utf8strings",
      (char*)"protoid",
      nullptr};

  if (!PyArg_ParseTupleAndKeywords(
          args,
          kws,
          "OO|ii",
          kwlist,
          &enc_obj,
          &spec,
          &utf8strings,
          &protoid)) {
    return nullptr;
  }

  if (protoid == 0) {
    return encodeT<BinaryProtocolWriter>(enc_obj, spec, utf8strings);
  } else if (protoid == 2) {
    return encodeT<CompactProtocolWriter>(enc_obj, spec, utf8strings);
  } else {
    PyErr_SetString(PyExc_TypeError, "Unexpected proto id");
    return nullptr;
  }
}

static PyObject* decode(PyObject* /*self*/, PyObject* args, PyObject* kws) {
  PyObject* dec_obj;
  PyObject* transport;
  PyObject* spec;
  int utf8strings = 0;
  int protoid = 0;
  StructTypeArgs parsedargs;
  DecodeBuffer input = {};

  static char* kwlist[] = {
      (char*)"dec",
      (char*)"transport",
      (char*)"spec",
      (char*)"utf8strings",
      (char*)"protoid",
      nullptr};

  if (!PyArg_ParseTupleAndKeywords(
          args,
          kws,
          "OOO|ii",
          kwlist,
          &dec_obj,
          &transport,
          &spec,
          &utf8strings,
          &protoid)) {
    return nullptr;
  }

  if (!parse_struct_args(&parsedargs, spec)) {
    return nullptr;
  }

  if (!decode_buffer_from_obj(&input, transport)) {
    return nullptr;
  }

  SCOPE_EXIT { free_decodebuf(&input); };

  if (protoid == 0) {
    if (!decodeT<BinaryProtocolReaderWithRefill>(
            &input, dec_obj, &parsedargs, utf8strings)) {
      return nullptr;
    }
  } else if (protoid == 2) {
    if (!decodeT<CompactProtocolReaderWithRefill>(
            &input, dec_obj, &parsedargs, utf8strings)) {
      return nullptr;
    }
  } else {
    PyErr_SetString(PyExc_TypeError, "Unexpected proto id");
    return nullptr;
  }

  Py_RETURN_NONE;
}

/* -- PYTHON MODULE SETUP STUFF --- */

static PyMethodDef ThriftFastProtoMethods[] = {

    {"encode", (PyCFunction)encode, METH_VARARGS | METH_KEYWORDS, ""},
    {"decode", (PyCFunction)decode, METH_VARARGS | METH_KEYWORDS, ""},

    {nullptr, nullptr, 0, nullptr} /* Sentinel */
};

extern "C" {
struct module_state {
  PyObject* error;
};

#if PY_MAJOR_VERSION >= 3
#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))

static int fastproto_traverse(PyObject* m, visitproc visit, void* arg) {
  Py_VISIT(GETSTATE(m)->error);
  return 0;
}

static int fastproto_clear(PyObject* m) {
  Py_CLEAR(GETSTATE(m)->error);
  return 0;
}

static struct PyModuleDef ThriftFastProtoModuleDef = {
    PyModuleDef_HEAD_INIT,
    "thrift.protocol.fastproto",
    nullptr,
    sizeof(struct module_state),
    ThriftFastProtoMethods,
    nullptr,
    fastproto_traverse,
    fastproto_clear,
    nullptr};

#define INITERROR return nullptr

PyObject* PyInit_fastproto(void);

PyObject* PyInit_fastproto(void)
#else
#define INITERROR return
#define GETSTATE(m) (&_state)
static struct module_state _state;

PyMODINIT_FUNC initfastproto(void);

PyMODINIT_FUNC initfastproto(void)
#endif
{
  PyObject* module;
  struct module_state* st;
#if PY_MAJOR_VERSION >= 3
  PyObject* iomodule = PyImport_ImportModule("io");
  if (iomodule == nullptr) {
    return nullptr;
  }
  PyObject* bio = PyObject_CallMethod(iomodule, (char*)"BytesIO", (char*)"()");
  if (bio == nullptr) {
    Py_DECREF(iomodule);
    return nullptr;
  }
  BytesIOType = (PyTypeObject*)PyObject_Type(bio);
  ExceptionsModule = PyImport_ImportModule("thrift.protocol.exceptions");
  if (ExceptionsModule == nullptr) {
    Py_DECREF(iomodule);
    Py_DECREF(bio);
    return nullptr;
  }
  module = PyModule_Create(&ThriftFastProtoModuleDef);
#else
  PycString_IMPORT;
  if (PycStringIO == nullptr) {
    return;
  }
  module = Py_InitModule("thrift.protocol.fastproto", ThriftFastProtoMethods);
#endif

  if (module == nullptr) {
    INITERROR;
  }
  st = GETSTATE(module);
  st->error = PyErr_NewException((char*)"fastproto.Error", nullptr, nullptr);
  if (st->error == nullptr) {
    Py_DECREF(module);
    INITERROR;
  }

#if PY_MAJOR_VERSION >= 3
#define INIT_INTERN_STRING(value)                              \
  do {                                                         \
    INTERN_STRING(value) = PyUnicode_InternFromString(#value); \
    if (!INTERN_STRING(value)) {                               \
      return nullptr;                                          \
    }                                                          \
  } while (0)
#else
#define INIT_INTERN_STRING(value)                             \
  do {                                                        \
    INTERN_STRING(value) = PyString_InternFromString(#value); \
    if (!INTERN_STRING(value)) {                              \
      return;                                                 \
    }                                                         \
  } while (0)
#endif

  INIT_INTERN_STRING(cstringio_buf);
  INIT_INTERN_STRING(cstringio_refill);
  INIT_INTERN_STRING(to_thrift);
  INIT_INTERN_STRING(from_thrift);
#undef INIT_INTERN_STRING

#if PY_MAJOR_VERSION >= 3
  return module;
#endif
}
} // end extern "C"
