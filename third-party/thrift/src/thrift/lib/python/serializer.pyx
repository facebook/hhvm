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

from cython.operator cimport dereference as deref
from cython.view cimport memoryview
from folly.iobuf cimport IOBuf
from libcpp.utility cimport move as cmove
from thrift.python.exceptions cimport Error, GeneratedError
from thrift.python.types cimport Struct, StructOrUnion, StructInfo, Union
from thrift.python.protocol import Protocol
from thrift.python.serializer cimport cJsonWriterOptions, cJson5ProtocolWriterOptions

cdef extern from *:
    """
    #undef _serialize
    """

cdef extern from "<Python.h>":
    cdef int PyObject_CheckBuffer(object)


cdef class JsonWriterOptions:
    """Options for the JSON writer that are passed to C++."""
    cdef cJsonWriterOptions _c_writer

    def __init__(
        self,
        *,
        list_trailing_comma=False,
        object_trailing_comma=False,
        unquote_object_name=False,
        allow_nan_inf=False,
        indent_width=2,
    ):
        self._c_writer.listTrailingComma = list_trailing_comma
        self._c_writer.objectTrailingComma = object_trailing_comma
        self._c_writer.unquoteObjectName = unquote_object_name
        self._c_writer.allowNanInf = allow_nan_inf
        self._c_writer.indentWidth = indent_width

JSON5_MODE = Json5ProtocolWriterOptions(
    writer=JsonWriterOptions(
        list_trailing_comma=True,
        object_trailing_comma=True,
        unquote_object_name=True,
        allow_nan_inf=True,
        indent_width=2,
    )
)


cdef class Json5ProtocolWriterOptions:
    """Options for the JSON5 protocol writer that are passed to C++."""
    cdef public JsonWriterOptions writer

    def __init__(self, *, writer=None):
        self.writer = writer if writer is not None else JsonWriterOptions()


cdef cJson5ProtocolWriterOptions _to_c_options(Json5ProtocolWriterOptions options):
    cdef cJson5ProtocolWriterOptions c_options
    c_options.writer = options.writer._c_writer
    return c_options


cdef _serialize_json5_iobuf(strct, Json5ProtocolWriterOptions options):
    cdef cJson5ProtocolWriterOptions c_options = _to_c_options(options)
    cdef StructInfo info
    if isinstance(strct, Struct):
        info = (<Struct>strct)._fbthrift_struct_info
        data = (<Struct>strct)._fbthrift_data
    elif isinstance(strct, Union):
        info = (<Union>strct)._fbthrift_struct_info
        data = (<Union>strct)._fbthrift_data
    else:
        info = (<GeneratedError>strct)._fbthrift_struct_info
        data = (<GeneratedError>strct)._fbthrift_data
    return folly.iobuf.from_unique_ptr(
        cmove(cserializeJson5(deref(info.cpp_obj), data, c_options))
    )


def serialize_iobuf(strct, cProtocol protocol=cProtocol.COMPACT, options=None):
    if options is not None and protocol != cProtocol.JSON5:
        raise ValueError("options only valid with Protocol.JSON5")
    if not isinstance(strct, (StructOrUnion, GeneratedError)):
        raise TypeError("thrift-python serialization only supports thrift-python types")
    if options is not None:
        return _serialize_json5_iobuf(strct, options)
    if isinstance(strct, StructOrUnion):
        return (<StructOrUnion>strct)._serialize(protocol)
    return (<GeneratedError>strct)._serialize(protocol)

def serialize(struct, cProtocol protocol=cProtocol.COMPACT, options=None):
    return b''.join(serialize_iobuf(struct, protocol, options))

# some users define custom cpp extensions that implement buffer protocol
cdef inline _is_buffer(object obj):
    return PyObject_CheckBuffer(obj) == 1

def deserialize_with_length(klass, buf, cProtocol protocol=cProtocol.COMPACT, *, fully_populate_cache=False):
    if not issubclass(klass, (StructOrUnion, GeneratedError)):
        raise TypeError("thrift-python deserialization only supports thrift-python types")
    if not isinstance(buf, (IOBuf, bytes, bytearray, memoryview)) and not _is_buffer(buf):
        raise TypeError("buf must be IOBuf, bytes, bytearray, or memoryview")
    cdef IOBuf iobuf = buf if isinstance(buf, IOBuf) else IOBuf(buf)
    cdef uint32_t length
    try:
        if issubclass(klass, Struct):
            inst = klass._fbthrift_new()
            length = (<Struct>inst)._deserialize(iobuf, protocol)
            if fully_populate_cache:
                (<Struct>inst)._fbthrift_fully_populate_cache()
        elif issubclass(klass, Union):
            inst = klass.__new__(klass)
            length = (<Union>inst)._deserialize(iobuf, protocol)
        else:
            inst = klass.__new__(klass)
            length = (<GeneratedError>inst)._deserialize(iobuf, protocol)
    except Exception as e:
        raise Error.__new__(Error, *e.args) from None
    return inst, length

def deserialize(klass, buf, cProtocol protocol=cProtocol.COMPACT, *, fully_populate_cache=False):
    return deserialize_with_length(klass, buf, protocol, fully_populate_cache=fully_populate_cache)[0]
