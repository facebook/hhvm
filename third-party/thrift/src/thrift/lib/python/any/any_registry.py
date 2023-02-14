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


from __future__ import annotations

import types
import typing

from apache.thrift.type.any.thrift_types import Any
from apache.thrift.type.standard.thrift_types import StandardProtocol, TypeName, TypeUri
from apache.thrift.type.type.thrift_types import Protocol, Type
from thrift.python import serializer
from thrift.python.conformance.universal_name import (
    find_by_universal_hash,
    get_universal_hash,
    get_universal_hash_prefix,
    UniversalHashAlgorithm,
)
from thrift.python.types import StructOrUnion


def _standard_protocol_to_serializer_protocol(
    protocol: StandardProtocol,
) -> serializer.Protocol:
    if protocol is StandardProtocol.Binary:
        return serializer.Protocol.BINARY
    if protocol is StandardProtocol.Compact:
        return serializer.Protocol.COMPACT
    if protocol is StandardProtocol.SimpleJson:
        return serializer.Protocol.JSON
    raise NotImplementedError(f"Unsupported standard protocol: {protocol}")


class AnyRegistry:
    def __init__(self) -> None:
        self._uri_to_cls: typing.Dict[str, typing.Type[StructOrUnion]] = {}
        self._alg_to_hash_to_cls: typing.Dict[
            UniversalHashAlgorithm, typing.Dict[bytes, typing.Type[StructOrUnion]]
        ] = {alg: {} for alg in UniversalHashAlgorithm}

    def register_type(self, cls: typing.Type[StructOrUnion]) -> bool:
        uri = cls.__get_thrift_uri__()
        if (not uri) or (uri in self._uri_to_cls):
            return False
        self._uri_to_cls[uri] = cls
        for alg, hash_to_type in self._alg_to_hash_to_cls.items():
            hash = get_universal_hash(alg, uri)
            hash_to_type[hash] = cls
        return True

    def register_module(self, module: types.ModuleType) -> bool:
        any_change = False
        for cls in module._fbthrift_all_structs:
            if self.register_type(cls):
                any_change = True
        return any_change

    def store(
        self, obj: StructOrUnion, protocol: typing.Optional[Protocol] = None
    ) -> Any:
        if protocol is None:
            protocol = Protocol(standard=StandardProtocol.Compact)
        if protocol.type != Protocol.Type.standard:
            raise NotImplementedError(
                f"Unsupported non-standard protocol: {protocol.value}"
            )
        uri = obj.__get_thrift_uri__()
        if uri is None:
            raise ValueError("Thrift struct doesn't have URI")
        hash_prefix = get_universal_hash_prefix(
            get_universal_hash(UniversalHashAlgorithm.Sha2_256, uri),
            16,
        )
        return Any(
            type=Type(
                name=TypeName(structType=TypeUri(typeHashPrefixSha2_256=hash_prefix))
            ),
            protocol=protocol,
            data=serializer.serialize_iobuf(
                obj,
                protocol=_standard_protocol_to_serializer_protocol(protocol.standard),
            ),
        )

    def _type_uri_to_cls(self, uri: TypeUri) -> typing.Type[StructOrUnion]:
        if uri.type == TypeUri.Type.uri:
            return self._uri_to_cls[uri.uri]
        if uri.type == TypeUri.Type.typeHashPrefixSha2_256:
            return find_by_universal_hash(
                self._alg_to_hash_to_cls[UniversalHashAlgorithm.Sha2_256],
                uri.typeHashPrefixSha2_256,
            )
        raise ValueError("No type information found")

    def load(self, any_obj: Any) -> StructOrUnion:
        if any_obj.type.name.type != TypeName.Type.structType:
            raise NotImplementedError(f"Unsupported type: {any_obj.type.name.value}")
        if any_obj.protocol.type != Protocol.Type.standard:
            raise NotImplementedError(
                f"Unsupported non-standard protocol: {any_obj.protocol.value}"
            )
        return serializer.deserialize(
            self._type_uri_to_cls(any_obj.type.name.structType),
            any_obj.data,
            protocol=_standard_protocol_to_serializer_protocol(
                any_obj.protocol.standard
            ),
        )
