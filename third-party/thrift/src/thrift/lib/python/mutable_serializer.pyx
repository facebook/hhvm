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
from thrift.python.exceptions cimport Error
from thrift.python.mutable_exceptions cimport MutableGeneratedError
from thrift.python.mutable_types cimport MutableStruct, MutableStructOrUnion
from thrift.python.protocol import Protocol


def serialize_iobuf(strct, cProtocol protocol=cProtocol.COMPACT):
    if isinstance(strct, MutableStructOrUnion):
        return (<MutableStructOrUnion>strct)._fbthrift_serialize(protocol)
    if isinstance(strct, MutableGeneratedError):
        return (<MutableGeneratedError>strct)._fbthrift_serialize(protocol)

    raise TypeError("thrift-python serialization only supports thrift-python types")

def serialize(struct, cProtocol protocol=cProtocol.COMPACT):
    return b''.join(serialize_iobuf(struct, protocol))

def deserialize_with_length(klass, buf, cProtocol protocol=cProtocol.COMPACT):
    if not issubclass(klass, (MutableStructOrUnion, MutableGeneratedError)):
        raise TypeError("thrift-python deserialization only supports thrift-python types")
    if not isinstance(buf, (IOBuf, bytes, bytearray, memoryview)):
        raise TypeError("buf must be IOBuf, bytes, bytearray, or memoryview")
    cdef IOBuf iobuf = buf if isinstance(buf, IOBuf) else IOBuf(buf)
    inst = klass.__new__(klass)
    cdef uint32_t length
    try:
        if issubclass(klass, MutableStruct):
            length = (<MutableStruct>inst)._fbthrift_deserialize(iobuf, protocol)
        else:
            length = (<MutableGeneratedError>inst)._fbthrift_deserialize(iobuf, protocol)
    except Exception as e:
        raise Error.__new__(Error, *e.args) from None
    return inst, length

def deserialize(klass, buf, cProtocol protocol=cProtocol.COMPACT):
    return deserialize_with_length(klass, buf, protocol)[0]
