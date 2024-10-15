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

from cython.view cimport memoryview
from folly.iobuf cimport IOBuf
from thrift.python.exceptions cimport Error, GeneratedError
from thrift.python.types cimport Struct, StructOrUnion, Union
from thrift.python.protocol import Protocol

cdef extern from *:
    """
    #undef _serialize
    """


def serialize_iobuf(strct, cProtocol protocol=cProtocol.COMPACT):
    if not isinstance(strct, (StructOrUnion, GeneratedError)):
        raise TypeError("thrift-python serialization only supports thrift-python types")
    if isinstance(strct, StructOrUnion):
        return (<StructOrUnion>strct)._serialize(protocol)
    return (<GeneratedError>strct)._serialize(protocol)

def serialize(struct, cProtocol protocol=cProtocol.COMPACT):
    return b''.join(serialize_iobuf(struct, protocol))

def deserialize_with_length(klass, buf, cProtocol protocol=cProtocol.COMPACT, *, fully_populate_cache=False):
    if not issubclass(klass, (StructOrUnion, GeneratedError)):
        raise TypeError("thrift-python deserialization only supports thrift-python types")
    if not isinstance(buf, (IOBuf, bytes, bytearray, memoryview)):
        raise TypeError("buf must be IOBuf, bytes, bytearray, or memoryview")
    cdef IOBuf iobuf = buf if isinstance(buf, IOBuf) else IOBuf(buf)
    inst = klass.__new__(klass)
    cdef uint32_t length
    try:
        if issubclass(klass, Struct):
            length = (<Struct>inst)._deserialize(iobuf, protocol)
            if fully_populate_cache:
                (<Struct>inst)._fbthrift_fully_populate_cache()
        elif issubclass(klass, Union):
            length = (<Union>inst)._deserialize(iobuf, protocol)
        else:
            length = (<GeneratedError>inst)._deserialize(iobuf, protocol)
    except Exception as e:
        raise Error.__new__(Error, *e.args) from None
    return inst, length

def deserialize(klass, buf, cProtocol protocol=cProtocol.COMPACT, *, fully_populate_cache=False):
    return deserialize_with_length(klass, buf, protocol, fully_populate_cache=fully_populate_cache)[0]
