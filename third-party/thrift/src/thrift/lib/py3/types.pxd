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
from folly.range cimport Range as cRange
from libc.stdint cimport uint32_t, uint16_t
from libcpp.string cimport string
from libcpp.memory cimport shared_ptr, unique_ptr
from libcpp.optional cimport optional
from libcpp.vector cimport vector
from libcpp.pair cimport pair

from thrift.py3.exceptions cimport GeneratedError, SetMetaClass
from thrift.py3.std_libcpp cimport string_view, sv_to_str
from thrift.python.common cimport cThriftMetadata
from thrift.python.protocol cimport Protocol

# make_unique was changed in cython to have except+ which breaks thrift-py3
cdef extern from "<memory>" namespace "std" nogil:
    unique_ptr[T] make_unique[T](...)
    shared_ptr[T] make_shared[T](...)
    shared_ptr[const T] make_const_shared "std::make_shared"[T](...)

cdef extern from "<memory>" namespace "std" nogil:
    cdef shared_ptr[const T] const_pointer_cast "std::const_pointer_cast"[T](shared_ptr[T])

cdef extern from "thrift/lib/py3/types.h" namespace "::thrift::py3" nogil:
    shared_ptr[T] constant_shared_ptr[T](T)
    shared_ptr[T] reference_shared_ptr[T](const T& ref, ...)
    void assign_unique_ptr[T](unique_ptr[T]& x, unique_ptr[T] y)
    void assign_shared_ptr[T](shared_ptr[T]& x, shared_ptr[T] y)
    void assign_shared_const_ptr[T](shared_ptr[const T]& x, shared_ptr[const T] y)
    const T& default_inst[T]()
    bint richcmp[T](const shared_ptr[T]& a, const shared_ptr[T]& b, int op)
    void reset_field[T](T& obj, uint16_t index) except +
    string_view get_field_name_by_index[T](size_t idx) except +
    object init_unicode_from_cpp(...)
    T* get_union_field_value[T](...) except +
    const T& deref_const[T](...)
    void mixin_deprecation_log_error(const char* struct_name, const char* field_name)

    cdef cppclass set_iter[T]:
        set_iter()
        set_iter(const T& cpp_obj)
        void genNextItem(...)

    cdef cppclass map_iter[T]:
        map_iter()
        map_iter(const T& cpp_obj)
        void genNextKeyVal(...)


ctypedef PyObject* PyObjectPtr
ctypedef optional[int] cOptionalInt

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
    cdef list _py_obj
    cdef object _child_cls
    cdef int _normalize_index(self, int index) except *

cdef class Set(Container):
    cdef frozenset _py_obj
    cdef object _child_cls
    cdef _fbthrift_py_richcmp(self, other, int op)

cdef class Map(Container):
    cdef dict _py_obj
    cdef object _child_cls

cdef class StructFieldsSetter:
    cdef void set_field(StructFieldsSetter self, const char* name, object value) except *


cdef translate_cpp_enum_to_python(object EnumClass, int value)

cpdef _ensure_py3_or_raise(object thrift_value, str field_name, object py3_type)
cpdef _ensure_py3_container_or_raise(object thrift_value, object py3_container_type)
cpdef _from_python_or_raise(object thrift_value, str field_name, object py3_type)


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
        T value_or(T)
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
    cdef object py3_to_python[T, NamespaceTag](const shared_ptr[T]& cppThrift)
    cdef shared_ptr[T] python_to_py3[T, NamespaceTag](object obj) except *
