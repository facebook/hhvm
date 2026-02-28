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

from cpython.object cimport PyTypeObject
from thrift.python.types cimport TypeInfoBase

cdef extern from "<thrift/lib/python/types.h>" namespace "::apache::thrift::python":
    cdef void tag_object_as_sequence(PyTypeObject*)
    cdef void tag_object_as_mapping(PyTypeObject*)

cdef class MutableList:
    cdef TypeInfoBase _val_typeinfo
    cdef list _list_data
    cdef bint _value_type_is_container
    cdef _value_to_internal_data(self, object)

cdef class MutableSet:
    cdef TypeInfoBase _val_typeinfo
    cdef set _set_data

cdef class ValueIterator:
    cdef TypeInfoBase _val_typeinfo
    cdef object _iter

cdef class MutableMap:
    cdef TypeInfoBase _key_typeinfo
    cdef TypeInfoBase _val_typeinfo
    cdef dict _map_data
    cdef bint _value_type_is_container
    cdef bint _use_internal_data_to_compare
    cdef _value_to_internal_data(self, object)

cdef class MapKeysView:
    cdef TypeInfoBase _key_typeinfo
    cdef object _dict_keys

cdef class MapItemsView:
    cdef TypeInfoBase _key_typeinfo
    cdef TypeInfoBase _val_typeinfo
    cdef object _dict_items

cdef class MapItemIterator:
    cdef TypeInfoBase _key_typeinfo
    cdef TypeInfoBase _val_typeinfo
    cdef object _iter

cdef class MapValuesView:
    cdef TypeInfoBase _val_typeinfo
    cdef object _dict_values
