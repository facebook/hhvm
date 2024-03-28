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

cimport cython
from libcpp.memory cimport make_shared
from libcpp.utility cimport move
from cython.operator cimport dereference as deref, preincrement as inc
from enum import Enum
from collections.abc import Mapping
from functools import total_ordering

from thrift.python.protocol import Protocol


class Priority(Enum):
    HIGH_IMPORTANCE = cHIGH_IMPORTANT
    HIGH = cHIGH
    IMPORTANT = cIMPORTANT
    NORMAL = cNORMAL
    BEST_EFFORT = cBEST_EFFORT
    N_PRIORITIES = cN_PRIORITIES


cdef class Headers:
    """ Generic Headers wrapper that could be subclassed with _getMap impl """
    def __init__(self):
        raise TypeError('This class is for wrapping maps originating in C++')

    cdef const F14NodeMap[string, string]* _getMap(self):
        """ This method should be overloaded """
        pass

    def __getitem__(self, str key):
        err = KeyError(key)
        if not self or key is None:
            raise err
        cdef string ckey = key.encode('utf-8')
        it = deref(self._getMap()).find(ckey)
        if it == deref(self._getMap()).end():
            raise err
        return (<bytes>deref(it).second).decode('utf-8')

    def __len__(self):
        return deref(self._getMap()).size()

    def __iter__(self):
        if not self:
            return

        cdef string ckey
        it = deref(self._getMap()).begin()
        while it != deref(self._getMap()).end():
            ckey = deref(it).first
            yield (<bytes>ckey).decode('utf-8')
            inc(it)

    def __eq__(self, other):
        if self is other:
            return True
        if not (isinstance(self, Mapping) and isinstance(other, Mapping)):
            return False
        if len(self) != len(other):
            return False
        for key in self:
            if key not in other:
                return False
            if other[key] != self[key]:
                return False
        return True

    def __ne__(self, other):
        return not self.__eq__(other)

    def __repr__(self):
        if not self:
            return 'i{}'

        return f'i{{{", ".join(map(lambda i: f"{repr(i[0])}: {repr(i[1])}", self.items()))}}}'

    def __contains__(self, str key):
        if not self or key is None:
            return False

        cdef string ckey = key.encode('utf-8')
        return deref(self._getMap()).count(ckey) > 0

    def get(self, str key, default=None):
        if not self or key is None:
            return default
        try:
            return self[key]
        except KeyError:
            return default

    def keys(self):
        return self.__iter__()

    def values(self):
        if not self:
            return

        cdef string cvalue
        it = deref(self._getMap()).begin()
        while it != deref(self._getMap()).end():
            cvalue = deref(it).second
            yield (<bytes>cvalue).decode('utf-8')
            inc(it)

    def items(self):
        if not self:
            return

        cdef string ckey
        cdef string cvalue
        item = deref(self._getMap()).begin()
        while item != deref(self._getMap()).end():
            ckey = deref(item).first
            cvalue = deref(item).second

            yield ((<bytes>ckey).decode('utf-8'), (<bytes>cvalue).decode('utf-8'))
            inc(item)


Mapping.register(Headers)


cdef class ReadHeaders(Headers):
    @staticmethod
    cdef _fbthrift_create(RpcOptions rpc_options):
        inst = <ReadHeaders>ReadHeaders.__new__(ReadHeaders)
        inst._parent = rpc_options
        return inst

    cdef const F14NodeMap[string, string]* _getMap(self):
        return &self._parent._cpp_obj.getReadHeaders()


cdef class WriteHeaders(Headers):
    @staticmethod
    cdef _fbthrift_create(RpcOptions rpc_options):
        inst = <WriteHeaders>WriteHeaders.__new__(WriteHeaders)
        inst._parent = rpc_options
        return inst

    cdef const F14NodeMap[string, string]* _getMap(self):
        return &self._parent._cpp_obj.getWriteHeaders()


cdef class RpcOptions:
    @property
    def timeout(self):
        """Get Timeout in seconds"""
        return self._cpp_obj.getTimeout().count() / 1000

    @timeout.setter
    def timeout(self, double seconds):
        """Set Timeout in seconds"""
        self._cpp_obj.setTimeout(milliseconds(<int64_t>(seconds * 1000)))

    @property
    def priority(self):
        return Priority(<int>self._cpp_obj.getPriority())

    @priority.setter
    def priority(self, value):
        if not isinstance(value, Priority):
            raise TypeError(f'priority can only be set to values of {Priority!r}')
        self._cpp_obj.setPriority(Priority_to_cpp(value))

    @property
    def chunk_timeout(self):
        """Get chunkTimeout in seconds"""
        return self._cpp_obj.getChunkTimeout().count() / 1000

    @chunk_timeout.setter
    def chunk_timeout(self, double seconds):
        """Set chunkTimeout in seconds"""
        self._cpp_obj.setChunkTimeout(milliseconds(<int64_t>(seconds * 1000)))

    @property
    def queue_timeout(self):
        """Get QueueTimeout in seconds"""
        return self._cpp_obj.getQueueTimeout().count() / 1000

    @queue_timeout.setter
    def queue_timeout(self, double seconds):
        """Set QueueTimeout in seconds"""
        self._cpp_obj.setQueueTimeout(milliseconds(<int64_t>(seconds * 1000)))

    def set_header(self, str key not None, str value not None):
        self._cpp_obj.setWriteHeader(key.encode('utf-8'), value.encode('utf-8'))

    @property
    def chunk_buffer_size(self):
        """Get chunkBufferSize"""
        return self._cpp_obj.getChunkBufferSize()

    @chunk_buffer_size.setter
    def chunk_buffer_size(self, int buffer_size):
        """Set chunkBufferSize"""
        self._cpp_obj.setChunkBufferSize(buffer_size)

    @property
    def read_headers(self):
        # So we don't create a cycle
        if not self._readheaders:
            self._readheaders = ReadHeaders._fbthrift_create(self)
        return self._readheaders

    @property
    def write_headers(self):
        # So we don't create a cycle
        if not self._writeheaders:
            self._writeheaders = WriteHeaders._fbthrift_create(self)
        return self._writeheaders


@cython.auto_pickle(False)
cdef class MetadataBox:
    def __init__(self):
        raise TypeError

    @staticmethod
    cdef box(cThriftMetadata&& meta):
        inst = <MetadataBox>MetadataBox.__new__(MetadataBox)
        inst._cpp_obj = make_shared[cThriftMetadata](move(meta))
        return inst
