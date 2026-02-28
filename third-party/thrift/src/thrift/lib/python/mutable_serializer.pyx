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
from thrift.python.mutable_types cimport MutableStructOrUnion
from thrift.python.protocol import Protocol


def serialize_iobuf(thrift_object, cProtocol protocol=cProtocol.COMPACT):
    """
    Serializes the given mutable Thrift object into an `IOBuf`, according to the given
    serialization `protocol`.

    Args:
        thrift_object (MutableStructOrUnion | MutableGeneratedError)

        protocol (cProtocol): Enum value of the serialization protocol to use.

    Raises: TypeError if the given `thrift_object` is not a valid thrift-python object.

    Returns (IOBuf) Serialized `thrift_object`, using the specified `protocol`.
    """
    if isinstance(thrift_object, MutableStructOrUnion):
        return (<MutableStructOrUnion>thrift_object)._fbthrift_serialize(protocol)
    if isinstance(thrift_object, MutableGeneratedError):
        return (<MutableGeneratedError>thrift_object)._fbthrift_serialize(protocol)

    raise TypeError("thrift-python mutable serialization only supports mutable thrift-python types")


def serialize(thrift_object, cProtocol protocol=cProtocol.COMPACT):
    """
    Returns the serialized form of `thrift_object` using the given Thrift
    (de)serialization `protocol`.

    Like `serialize_iobuf()`, but returns `bytes` instead of `IOBuf`.
    """
    return b''.join(serialize_iobuf(thrift_object, protocol))


def deserialize_with_length(thrift_class, buf, cProtocol protocol=cProtocol.COMPACT):
    """
    Deserializes the contents of `buf` as an instance of `thrift_class`, using the given
    Thrift (de)serialization `protocol`.

    Args:
        thrift_class (class):
            Type of the Thrift struct, union or exception to deserialize.

        buf (IOBuf | bytes | bytearray | memoryview):
            Source containing the serialized representation of the Thrift object to
            deserialize (using the given `protocol`).

        protocol (cProtocol): Enum value of the (de)serialization protocol to use.

    Raises:
      * `TypeError` if any input is of invalid type.
      * (Thrift) `Error` if any other exception is raised during deserialization.


    Returns: tuple(object, length), where:
        object: instance of `thrift_class`, deserialized from the given buffer.

        length: number of bytes read during deserialization.
    """
    if not issubclass(thrift_class, (MutableStructOrUnion, MutableGeneratedError)):
        raise TypeError(
            "thrift-python deserialization only supports thrift-python types"
        )

    if not isinstance(buf, (IOBuf, bytes, bytearray, memoryview)):
        raise TypeError("buf must be IOBuf, bytes, bytearray, or memoryview")

    cdef IOBuf iobuf = buf if isinstance(buf, IOBuf) else IOBuf(buf)
    inst = thrift_class.__new__(thrift_class)
    cdef uint32_t length
    try:
        if issubclass(thrift_class, MutableStructOrUnion):
            length = (<MutableStructOrUnion>inst)._fbthrift_deserialize(iobuf, protocol)
        else:
            length = (
                <MutableGeneratedError>inst
            )._fbthrift_deserialize(iobuf, protocol)
    except Exception as e:
        raise Error.__new__(Error, *e.args) from None
    return inst, length


def deserialize(thrift_class, buf, cProtocol protocol=cProtocol.COMPACT):
    """
    Deserializes the contents of `buf` as an instance of `thrift_class`, using the given
    Thrift (de)serialization `protocol`.

    Like `deserialize_with_length()`, but only returns the deserialized object.
    """
    return deserialize_with_length(thrift_class, buf, protocol)[0]
