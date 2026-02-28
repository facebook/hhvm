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

# distutils: language = c++

from libcpp.string cimport string
from cpython.ref cimport PyObject
from cpython.object cimport PyObject, PyTypeObject
from folly cimport cFollyExceptionWrapper
from folly.iobuf cimport IOBuf, cIOBuf
from libc.stdint cimport uint32_t
from libcpp.memory cimport shared_ptr
from thrift.python.common cimport RpcOptions, cThriftMetadata
from thrift.py3.std_libcpp cimport string_view, sv_to_str
from thrift.python.exceptions cimport Error as BaseError
from thrift.python.protocol cimport Protocol


cdef extern from "Python.h":
    ctypedef extern class builtins.Exception[object PyBaseExceptionObject]:
        pass


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
        Py_INCREF((PyObject*)m);
        APACHE_THRIFT_DETAIL_Py_SET_TYPE(t, m);
        PyType_Modified(t);
    }
    """
    void SetMetaClass(PyTypeObject* t, PyTypeObject* m)


cdef extern from "thrift/lib/py3/exceptions.h" namespace "::thrift::py3::exception":
    cdef shared_ptr[T] try_make_shared_exception[T](
        const cFollyExceptionWrapper& excepton)
    cdef const T& unwrap_exception[T](const cFollyExceptionWrapper& excepton)


cdef class GeneratedError(BaseError):
    cdef object __weakref__
    cdef object _fbthrift_hash
    cdef IOBuf _fbthrift_serialize(self, Protocol proto)
    cdef uint32_t _fbthrift_deserialize(self, const cIOBuf* buf, Protocol proto) except? 0
    cdef object _fbthrift_isset(self)
    cdef object _fbthrift_cmp_sametype(self, other, int op)
    cdef void _fbthrift_set_field(self, str name, object value) except *
