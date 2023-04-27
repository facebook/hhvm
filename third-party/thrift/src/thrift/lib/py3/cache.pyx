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

from functools import wraps
from types import MappingProxyType
from weakref import WeakKeyDictionary
import threading

from thrift.py3.types cimport List,  Set, Map


cdef threadsafe_object_key_cached(fn):
    __cache = WeakKeyDictionary()
    lock = threading.Lock()
    @wraps(fn)
    def wrapper(key):
        # lock free read when present in cache
        if key in __cache:
            return __cache[key]
        with lock:
            # check again if in cache
            if key in __cache:
                return __cache[key]
            # update cache
            result = __cache[key] = fn(key)
            return result
    return wrapper

@threadsafe_object_key_cached
def to_tuple(List py3_list):
    return tuple(py3_list)


@threadsafe_object_key_cached
def to_frozenset(Set py3_set):
    return frozenset(py3_set)


@threadsafe_object_key_cached
def to_mappingproxy(Map py3_map):
    return MappingProxyType(dict(py3_map))
