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

# pyre-strict

from __future__ import annotations

import types
import typing

from thrift.conformance.any.thrift_types import Any
from thrift.conformance.protocol.thrift_types import StandardProtocol
from thrift.python.conformance.universal_name import (
    find_by_universal_hash,
    get_universal_hash,
    get_universal_hash_prefix,
    UniversalHashAlgorithm,
)
from thrift.python.serializer import deserialize, Protocol, serialize_iobuf
from thrift.python.types import StructOrUnion


def _to_serializer_protocol(protocol: typing.Optional[StandardProtocol]) -> Protocol:
    if protocol is None:
        return Protocol.COMPACT
    if protocol == StandardProtocol.Binary:
        return Protocol.BINARY
    if protocol == StandardProtocol.Compact:
        return Protocol.COMPACT
    if protocol == StandardProtocol.SimpleJson:
        return Protocol.JSON
    raise ValueError(f"Unsupported protocol: {protocol}")


class AnyRegistry:
    def __init__(self) -> None:
        self._uri_to_type: typing.Dict[str, typing.Type[StructOrUnion]] = {}
        self._type_to_uri: typing.Dict[typing.Type[StructOrUnion], str] = {}
        self._alg_to_hash_to_type: typing.Dict[
            UniversalHashAlgorithm, typing.Dict[bytes, typing.Type[StructOrUnion]]
        ] = {alg: {} for alg in UniversalHashAlgorithm}

    def register_type(self, cls: typing.Type[StructOrUnion]) -> bool:
        uri = cls.__get_thrift_uri__()
        if (not uri) or (uri in self._uri_to_type) or (cls in self._type_to_uri):
            return False
        self._uri_to_type[uri] = cls
        self._type_to_uri[cls] = uri
        for alg, hash_to_type in self._alg_to_hash_to_type.items():
            hash = get_universal_hash(alg, uri)
            hash_to_type[hash] = cls
        return True

    def register_module(self, module: types.ModuleType) -> None:
        for cls in module._fbthrift_all_structs:
            self.register_type(cls)

    def store(
        self, obj: StructOrUnion, protocol: typing.Optional[StandardProtocol] = None
    ) -> Any:
        uri = self._type_to_uri[type(obj)]
        hash = get_universal_hash(UniversalHashAlgorithm.Sha2_256, uri)
        hash_prefix = get_universal_hash_prefix(hash, 16)
        serializer_protocol = _to_serializer_protocol(protocol)
        return Any(
            type=uri,
            typeHashPrefixSha2_256=hash_prefix,
            protocol=protocol,
            data=serialize_iobuf(obj, protocol=serializer_protocol),
        )

    def load(self, obj: Any) -> StructOrUnion:
        if obj.type:
            cls = self._uri_to_type[obj.type]
        elif obj.typeHashPrefixSha2_256:
            cls = find_by_universal_hash(
                self._alg_to_hash_to_type[UniversalHashAlgorithm.Sha2_256],
                obj.typeHashPrefixSha2_256,
            )
        else:
            raise ValueError("No type information found")
        serializer_protocol = _to_serializer_protocol(obj.protocol)
        return deserialize(cls, obj.data, protocol=serializer_protocol)
