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

"""
Intermediate solution to be able to have a generic serializer from
"""


from enum import Enum
from functools import singledispatch

import thrift.py3.types
import thrift.python.types


class Protocol(Enum):
    COMPACT = 0
    BINARY = 1
    JSON = 2


GENERIC_THRIFT_STRUCT = (thrift.py3.types.Struct, thrift.python.types.StructOrUnion)


@singledispatch
def serialize(obj, protocol=Protocol.COMPACT) -> bytes:
    from thrift.py3 import Protocol as py3_Protocol, serialize as py3_serialize

    return py3_serialize(obj, protocol=py3_Protocol[protocol.name])


@serialize.register(thrift.python.types.StructOrUnion)
def _(
    obj: thrift.python.types.StructOrUnion, protocol: Protocol = Protocol.COMPACT
) -> bytes:
    from thrift.python.serializer import (
        Protocol as python_Protocol,
        serialize as python_serialize,
    )

    return python_serialize(obj, protocol=python_Protocol[protocol.name])


@singledispatch
def deserialize(cls, data: bytes, protocol: Protocol = Protocol.COMPACT):
    from thrift.py3 import deserialize as py3_deserialize, Protocol as py3_Protocol

    return py3_deserialize(cls, data, py3_Protocol[protocol.name])


@deserialize.register(thrift.python.types.StructOrUnion)
def _(cls, data: bytes, protocol: Protocol = Protocol.COMPACT):
    from thrift.python.serializer import (
        deserialize as python_deserialize,
        Protocol as python_Protocol,
    )

    return python_deserialize(cls, data, python_Protocol[protocol.name])
