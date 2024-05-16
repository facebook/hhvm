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

from thrift.py3.exceptions cimport BaseError, GeneratedError as Py3GeneratedError
from thrift.py3.types cimport Struct as Py3Struct
from thrift.python.exceptions cimport GeneratedError as PythonGeneratedError
from thrift.python.types cimport StructOrUnion as PythonStruct
import thrift.python.serializer as python_serializer
from folly.iobuf cimport IOBuf
cimport folly.iobuf as _fbthrift_iobuf
from thrift.python.common import Protocol


def serialize(tstruct, protocol=Protocol.COMPACT):
    return b''.join(serialize_iobuf(tstruct, protocol))


def serialize_iobuf(tstruct, protocol=Protocol.COMPACT):
    if not isinstance(protocol, Protocol):
        raise TypeError(f"{protocol} must of type Protocol")
    if isinstance(tstruct, (PythonStruct, PythonGeneratedError)):
        return python_serializer.serialize_iobuf(tstruct, protocol)
    if isinstance(tstruct, Py3GeneratedError):
        return (<Py3GeneratedError>tstruct)._fbthrift_serialize(protocol)
    return (<Py3Struct?>tstruct)._fbthrift_serialize(protocol)


def deserialize_with_length(structKlass, buf not None, protocol=Protocol.COMPACT):
    if not isinstance(protocol, Protocol):
        raise TypeError(f"{protocol} must of type Protocol")
    if issubclass(structKlass, (PythonStruct, PythonGeneratedError)):
        return python_serializer.deserialize_with_length(structKlass, buf, protocol)
    if not issubclass(structKlass, (Py3Struct, Py3GeneratedError)):
        raise TypeError(f"{structKlass} Must be a py3 thrift struct or exception class")
    cdef IOBuf iobuf = buf if isinstance(buf, IOBuf) else IOBuf(buf)
    instance = structKlass.__new__(structKlass)
    try:
        if issubclass(structKlass, Py3Struct):
            length = (<Py3Struct>instance)._fbthrift_deserialize(iobuf._this, protocol)
        else:
            length = (<Py3GeneratedError>instance)._fbthrift_deserialize(iobuf._this, protocol)
        return instance, length
    except Exception as e:
        raise BaseError.__new__(BaseError, *e.args) from None


def deserialize(structKlass, buf not None, protocol=Protocol.COMPACT):
    return deserialize_with_length(structKlass, buf, protocol)[0]


def serialize_with_header(tstruct, protocol=Protocol.COMPACT, transform=Transform.NONE):
    return b''.join(serialize_with_header_iobuf(tstruct, protocol, transform))


def serialize_with_header_iobuf(tstruct, protocol=Protocol.COMPACT, Transform transform=Transform.NONE):
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
        raise BaseError.__new__(BaseError, *e.args) from None
    if cbuf == NULL:
        raise BufferError("Bad data used for deserialize")
    protoid = Protocol(header.getProtocolId())
    return deserialize(structKlass, _fbthrift_iobuf.from_unique_ptr(_fbthrift_iobuf.move(cbuf)), protocol=protoid)
