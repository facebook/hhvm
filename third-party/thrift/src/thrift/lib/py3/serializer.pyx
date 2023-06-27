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

from cython cimport fused_type

from thrift.py3.exceptions cimport GeneratedError, Error
from thrift.py3.types cimport Struct
from folly.iobuf cimport IOBuf
cimport folly.iobuf as _fbthrift_iobuf
from thrift.py3.common import Protocol

StructOrError = fused_type(Struct, GeneratedError)

def serialize(tstruct, protocol=Protocol.COMPACT):
    return b''.join(serialize_iobuf(tstruct, protocol))


def serialize_iobuf(StructOrError tstruct, protocol=Protocol.COMPACT):
    if not isinstance(protocol, Protocol):
        raise TypeError(f"{protocol} must of type Protocol")
    if isinstance(tstruct, Struct):
        return (<Struct>tstruct)._fbthrift_serialize(protocol)
    return (<GeneratedError>tstruct)._fbthrift_serialize(protocol)



def deserialize_with_length(structKlass, buf not None, protocol=Protocol.COMPACT):
    if not issubclass(structKlass, (Struct, GeneratedError)):
        raise TypeError(f"{structKlass} Must be a py3 thrift struct or exception class")
    if not isinstance(protocol, Protocol):
        raise TypeError(f"{protocol} must of type Protocol")
    cdef IOBuf iobuf = buf if isinstance(buf, IOBuf) else IOBuf(buf)
    instance = structKlass.__new__(structKlass)
    try:
        if issubclass(structKlass, Struct):
            length = (<Struct>instance)._fbthrift_deserialize(iobuf._this, protocol)
        else:
            length = (<GeneratedError>instance)._fbthrift_deserialize(iobuf._this, protocol)
        return instance, length
    except Exception as e:
        raise Error.__new__(Error, *e.args) from None

def deserialize(structKlass, buf not None, protocol=Protocol.COMPACT):
    return deserialize_with_length(structKlass, buf, protocol)[0]

def serialize_with_header(tstruct, protocol=Protocol.COMPACT, transform=Transform.NONE):
    return b''.join(serialize_with_header_iobuf(tstruct, protocol, transform))

def serialize_with_header_iobuf(StructOrError tstruct, protocol=Protocol.COMPACT, Transform transform=Transform.NONE):
    cdef cTHeader header
    cdef IOBuf buf = <IOBuf>serialize_iobuf(tstruct, protocol)
    header.setProtocolId(protocol)
    if transform is not Transform.NONE:
        header.setTransform(transform)
    return _fbthrift_iobuf.from_unique_ptr(header.addHeader(_fbthrift_iobuf.move(buf._ours)))


def deserialize_from_header(structKlass, buf not None):
    # Clone because we will take the guts.
    cdef IOBuf iobuf = buf.clone() if isinstance(buf, IOBuf) else IOBuf(buf)
    cdef _fbthrift_iobuf.cIOBufQueue queue = _fbthrift_iobuf.cIOBufQueue(_fbthrift_iobuf.cacheChainLength())
    queue.append(_fbthrift_iobuf.move(iobuf._ours))
    cdef cTHeader header
    cdef F14NodeMap[string, string] pheaders
    cdef size_t needed = 0
    cdef unique_ptr[cIOBuf] cbuf
    try:
        cbuf = _fbthrift_iobuf.move(header.removeHeader(&queue, needed, pheaders))
    except Exception as e:
        raise Error.__new__(Error, *e.args) from None
    if cbuf == NULL:
        raise BufferError("Bad data used for deserialize")
    protoid = Protocol(header.getProtocolId())
    return deserialize(structKlass, _fbthrift_iobuf.from_unique_ptr(_fbthrift_iobuf.move(cbuf)), protocol=protoid)
