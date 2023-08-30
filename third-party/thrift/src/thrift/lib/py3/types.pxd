# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# distutils: language=c++
from cpython.bytes cimport PyBytes_AsStringAndSize
from cpython.object cimport PyObject, PyTypeObject
from folly.iobuf cimport cIOBuf, IOBuf
from folly.range cimport StringPiece as cStringPiece, Range as cRange
from libc.stdint cimport uint32_t, uint16_t
from libcpp.string cimport string
from libcpp.memory cimport shared_ptr, unique_ptr
from libcpp.optional cimport optional
from libcpp.vector cimport vector
from libcpp.pair cimport pair

from thrift.py3.std_libcpp cimport string_view, sv_to_str
from thrift.py3.common cimport Protocol, cThriftMetadata

cdef extern from *:
    """
    // Py_SET_TYPE is new in Python 3.9 and this is a suggested replacement for
    // older versions.
    #if defined(Py_SET_TYPE)
    #define APACHE_THRIFT_DETAIL_Py_SET_TYPE(obj, type) \
        Py_SET_TYPE(obj, type)
    #else
    #define APACHE_THRIFT_DETAIL_Py_SET_TYPE(obj, type) \
        ((Py_TYPE(obj) = (type)), (void)0)
    #endif
    static CYTHON_INLINE void SetMetaClass(PyTypeObject* t, PyTypeObject* m)
    {
        APACHE_THRIFT_DETAIL_Py_SET_TYPE(t, m);
        PyType_Modified(t);
    }
    """
    void SetMetaClass(PyTypeObject* t, PyTypeObject* m)

cdef extern from "<memory>" namespace "std" nogil:
    cdef shared_ptr[const T] const_pointer_cast "std::const_pointer_cast"[T](shared_ptr[T])

cdef extern from "thrift/lib/py3/types.h" namespace "::thrift::py3" nogil:
    cdef enum cSetOp "::thrift::py3::SetOp":
        AND
        OR
        SUB
        XOR
        REVSUB
    shared_ptr[T] constant_shared_ptr[T](T)
    shared_ptr[T] reference_shared_ptr[T](const T& ref, ...)
    void assign_unique_ptr[T](unique_ptr[T]& x, unique_ptr[T] y)
    void assign_shared_ptr[T](shared_ptr[T]& x, shared_ptr[T] y)
    void assign_shared_const_ptr[T](shared_ptr[const T]& x, shared_ptr[const T] y)
    const T& default_inst[T]()
    bint richcmp[T](const shared_ptr[T]& a, const shared_ptr[T]& b, int op)
    bint setcmp[T](const shared_ptr[T]& a, const shared_ptr[T]& b, int op)
    shared_ptr[T] set_op[T](const shared_ptr[T]& a, const shared_ptr[T]& b, cSetOp op)
    optional[size_t] list_index[T](const shared_ptr[T]& list, int start, int stop, ...)
    shared_ptr[T] list_slice[T](const shared_ptr[T]& cpp_obj, int start, int stop, int step)
    void list_getitem[T](const shared_ptr[T]& cpp_obj, int index, ...)
    size_t list_count[T](const shared_ptr[T]& list, ...)
    bint map_contains[T](const shared_ptr[T]& cpp_obj, ...)
    void map_getitem[T](const shared_ptr[T]& cpp_obj, ...)
    void reset_field[T](T& obj, uint16_t index) except +
    string_view get_field_name_by_index[T](size_t idx) except +
    T* get_union_field_value[T](...) except +

    cdef cppclass set_iter[T]:
        set_iter()
        set_iter(const shared_ptr[T]& cpp_obj)
        void genNext(const shared_ptr[T]& cpp_obj, ...)
    cdef cppclass map_iter[T]:
        map_iter()
        map_iter(const shared_ptr[T]& cpp_obj)
        void genNextKey(const shared_ptr[T]& cpp_obj, ...)
        void genNextValue(const shared_ptr[T]& cpp_obj, ...)
        void genNextItem(const shared_ptr[T]& cpp_obj, ...)


ctypedef PyObject* PyObjectPtr
ctypedef optional[int] cOptionalInt

cdef extern from "thrift/lib/py3/enums.h" namespace "::thrift::py3" nogil:
    cdef cppclass cEnumData "::thrift::py3::EnumData":
        pair[PyObjectPtr, cOptionalInt] tryGetByName(string_view name) except +
        pair[PyObjectPtr, string_view] tryGetByValue(int value) except +
        PyObject* tryAddToCache(int value, PyObject* obj) except +
        size_t size()
        string_view getPyName(string_view name)
        cRange[const cStringPiece*] getNames()
    cdef cppclass cEnumFlagsData "::thrift::py3::EnumFlagsData"(cEnumData):
        PyObject* tryAddToFlagValuesCache(int value, PyObject* obj) except +
        string getNameForDerivedValue(int value) except +
        int getInvertValue(int value) except +
        int convertNegativeValue(int value) except +
    cEnumData* createEnumData[T]() except +
    cEnumFlagsData* createEnumFlagsData[T]() except +
    cEnumData* createEnumDataForUnionType[T]() except +


cdef class EnumData:
    cdef unique_ptr[cEnumData] _cpp_obj
    cdef type _py_type
    cdef get_by_name(self, str name)
    cdef get_by_value(self, int value)
    cdef PyObject* _add_to_cache(self, str name, int value) except *
    cdef int size(self)
    cdef void _value_error(self, int value) except *
    @staticmethod
    cdef EnumData _fbthrift_create(cEnumData* ptr, py_type)

cdef class EnumFlagsData(EnumData):
    cdef get_invert(self, uint32_t value)
    @staticmethod
    cdef EnumFlagsData _fbthrift_create(cEnumFlagsData* ptr, py_type)

cdef class UnionTypeEnumData(EnumData):
    cdef object __empty
    @staticmethod
    cdef UnionTypeEnumData _fbthrift_create(cEnumData* ptr, py_type)

cdef class EnumMeta(type):
    pass


cdef class __NotSet:
    pass

cdef __NotSet NOTSET


cdef class Struct:
    cdef object _fbthrift_hash
    cdef object __weakref__
    cdef IOBuf _fbthrift_serialize(self, Protocol proto)
    cdef uint32_t _fbthrift_deserialize(self, const cIOBuf* buf, Protocol proto) except? 0
    cdef object _fbthrift_isset(self)
    cdef bint _fbthrift_noncomparable_eq(self, other)
    cdef object _fbthrift_cmp_sametype(self, other, int op)
    cdef void _fbthrift_set_field(self, str name, object value) except *


cdef class Union(Struct):
    pass


cdef class Container:
    cdef object _fbthrift_hash
    cdef object __weakref__


cdef class List(Container):
    cdef int _normalize_index(self, int index) except *
    cdef _get_slice(self, slice index_obj)
    cdef _get_single_item(self, size_t index)
    cdef _check_item_type(self, item)

cdef class Set(Container):
    cdef _fbthrift_py_richcmp(self, other, int op)
    cdef _fbthrift_do_set_op(self, other, cSetOp op)


cdef class Map(Container):
    cdef _check_key_type(self, key)


cdef class CompiledEnum:
    cdef object __weakref__
    cdef readonly int value
    cdef readonly str name
    cdef object _fbthrift_hash
    cdef object __str
    cdef object __repr
    cdef get_by_name(self, str name)


cdef class Flag(CompiledEnum):
    pass

cdef class BadEnum:
    cdef object _enum
    cdef readonly int value
    cdef readonly str name


cdef class StructFieldsSetter:
    cdef void set_field(StructFieldsSetter self, const char* name, object value) except *


cdef translate_cpp_enum_to_python(object EnumClass, int value)


# For cpp.type'd binary values we need a "string" that cython doesn't think
# is a string (so it doesn't generate all the string stuff)
cdef extern from "<string>" namespace "std" nogil:
    cdef cppclass bstring "std::basic_string<char>":
        bstring(string&) except +
        const char* data()
        size_t size()
        size_t length()


cdef extern from "<utility>" namespace "std" nogil:
    cdef string move(string)


cdef inline string bytes_to_string(bytes b) except*:
    cdef Py_ssize_t length
    cdef char* data
    PyBytes_AsStringAndSize(b, &data, &length)
    return move(string(data, length))  # there is a temp because string can raise


cdef extern from "thrift/lib/cpp2/FieldRef.h" namespace "apache::thrift" nogil:
    cdef cppclass field_ref[T]:
        void assign "operator="(T)
        T value()
        # Cython doesn't handle references very well, so use a different name
        # for value in the contexts where references actually work.
        T& ref "value" ()
        # Cython doesn't handle references very well, so use a different name
        # for value_unchecked in the contexts where references actually work.
        T& ref_unchecked "value_unchecked" ()
        bint has_value()

    cdef cppclass optional_field_ref[T]:
        void assign "operator="(T)
        T value()
        T value_unchecked()
        # Cython doesn't handle references very well, so use a different name
        # for value in the contexts where references actually work.
        T& ref "value" ()
        # Cython doesn't handle references very well, so use a different name
        # for value_unchecked in the contexts where references actually work.
        T& ref_unchecked "value_unchecked" ()
        void reset()
        bint has_value()

    cdef cppclass required_field_ref[T]:
        void assign "operator="(T)
        T value()
        # Cython doesn't handle references very well, so use a different name
        # for value in the contexts where references actually work.
        T& ref "value" ()
        bint has_value()

    cdef cppclass optional_boxed_field_ref[T]:
        void assign "operator="(T)
        T value()
        # Cython doesn't handle references very well, so use a different name
        # for value in the contexts where references actually work.
        T& ref "value" ()
        bint has_value()

    cdef cppclass terse_field_ref[T]:
        void assign "operator="(T)
        T value()
        # Cython doesn't handle references very well, so use a different name
        # for value in the contexts where references actually work.
        T& ref "value" ()

    cdef cppclass union_field_ref[T]:
        void assign "operator="(T)
        void assign_ptr "operator="(...)
        T value()
        # Cython doesn't handle references very well, so use a different name
        # for value in the contexts where references actually work.
        T& ref "value" ()
        bint has_value()

# Use python-types-capi to serialize of marshal _cpp_obj to thrift-python
cdef extern from "thrift/lib/python/capi/py3_converter.h" namespace "apache::thrift::python::capi":
    cdef object py3_to_python[T](shared_ptr[T] cppThrift)
    cdef shared_ptr[T] python_to_py3[T](object obj) except *
