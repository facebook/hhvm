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
from apache.thrift.type.standard.thrift_types import (
    StandardProtocol,
    TypeName,
    TypeUri,
    Void,
)
from apache.thrift.type.type.thrift_types import Protocol, Type
from folly.iobuf import IOBuf
from thrift.python import serializer
from thrift.python.any.serializer import deserialize_primitive, serialize_primitive
from thrift.python.conformance.universal_name import (
    find_by_universal_hash,
    get_universal_hash,
    get_universal_hash_prefix,
    UniversalHashAlgorithm,
)
from thrift.python.exceptions import GeneratedError
from thrift.python.types import Enum, StructOrUnion, Union


if typing.TYPE_CHECKING:
    from thrift.python.any.serializer import (
        PrimitiveType,
        SerializableType,
        StructOrUnionOrException,
    )

    ObjWithUri = typing.Union[StructOrUnionOrException, Enum]
    ClassWithUri = typing.Type[ObjWithUri]


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


def _get_type_hash_prefix(obj: ObjWithUri) -> bytes:
    uri = obj.__get_thrift_uri__()
    if uri is None:
        raise ValueError("Thrift struct doesn't have URI")
    return get_universal_hash_prefix(
        get_universal_hash(UniversalHashAlgorithm.Sha2_256, uri),
        16,
    )


def _infer_type_name_from_value(value: PrimitiveType) -> TypeName:
    if isinstance(value, Enum):
        return TypeName(
            enumType=TypeUri(typeHashPrefixSha2_256=_get_type_hash_prefix(value))
        )
    if isinstance(value, bool):
        return TypeName(boolType=Void.Unused)
    if isinstance(value, int):
        return TypeName(i64Type=Void.Unused)
    if isinstance(value, float):
        return TypeName(doubleType=Void.Unused)
    if isinstance(value, str):
        return TypeName(stringType=Void.Unused)
    if isinstance(value, bytes):
        return TypeName(binaryType=Void.Unused)
    if isinstance(value, IOBuf):
        return TypeName(binaryType=Void.Unused)
    raise ValueError(f"Can not infer thrift type from: {value}")


class AnyRegistry:
    def __init__(self) -> None:
        self._uri_to_cls: typing.Dict[str, ClassWithUri] = {}
        self._alg_to_hash_to_cls: typing.Dict[
            UniversalHashAlgorithm, typing.Dict[bytes, ClassWithUri]
        ] = {alg: {} for alg in UniversalHashAlgorithm}

    def register_type(self, cls: ClassWithUri) -> bool:
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
        for cls in module._fbthrift_all_enums:
            if self.register_type(cls):
                any_change = True
        return any_change

    def store(
        self, obj: SerializableType, protocol: typing.Optional[Protocol] = None
    ) -> Any:
        if protocol is None:
            protocol = Protocol(standard=StandardProtocol.Compact)
        if protocol.type != Protocol.Type.standard:
            raise NotImplementedError(
                f"Unsupported non-standard protocol: {protocol.value}"
            )
        if isinstance(obj, (GeneratedError, StructOrUnion)):
            return self._store_struct(obj, protocol=protocol)
        if isinstance(obj, (bool, int, float, str, bytes, IOBuf, Enum)):
            return self._store_primitive(obj, protocol=protocol)
        raise ValueError(f"Unsupported type: f{type(obj)}")

    def _store_struct(self, obj: StructOrUnionOrException, protocol: Protocol) -> Any:
        type_uri = TypeUri(typeHashPrefixSha2_256=_get_type_hash_prefix(obj))
        if isinstance(obj, Union):
            type_name = TypeName(unionType=type_uri)
        elif isinstance(obj, GeneratedError):
            type_name = TypeName(exceptionType=type_uri)
        else:
            type_name = TypeName(structType=type_uri)
        return Any(
            type=Type(name=type_name),
            protocol=protocol,
            data=serializer.serialize_iobuf(
                obj,
                protocol=_standard_protocol_to_serializer_protocol(protocol.standard),
            ),
        )

    def _store_primitive(self, obj: PrimitiveType, protocol: Protocol) -> Any:
        thrift_type = Type(name=_infer_type_name_from_value(obj))
        return Any(
            type=thrift_type,
            protocol=protocol,
            data=serialize_primitive(
                obj,
                protocol=_standard_protocol_to_serializer_protocol(protocol.standard),
                thrift_type=thrift_type,
            ),
        )

    def _type_uri_to_cls(self, uri: TypeUri) -> ClassWithUri:
        if uri.type == TypeUri.Type.uri:
            return self._uri_to_cls[uri.uri]
        if uri.type == TypeUri.Type.typeHashPrefixSha2_256:
            return find_by_universal_hash(
                self._alg_to_hash_to_cls[UniversalHashAlgorithm.Sha2_256],
                uri.typeHashPrefixSha2_256,
            )
        raise ValueError("No type information found")

    def load(self, any_obj: Any) -> SerializableType:
        if any_obj.protocol.type != Protocol.Type.standard:
            raise NotImplementedError(
                f"Unsupported non-standard protocol: {any_obj.protocol.value}"
            )
        if any_obj.type.name.type in (
            TypeName.Type.structType,
            TypeName.Type.unionType,
            TypeName.Type.exceptionType,
        ):
            return self._load_struct(any_obj)
        if any_obj.type.name.type in (
            TypeName.Type.boolType,
            TypeName.Type.i16Type,
            TypeName.Type.i32Type,
            TypeName.Type.i64Type,
            TypeName.Type.floatType,
            TypeName.Type.doubleType,
            TypeName.Type.stringType,
            TypeName.Type.binaryType,
            TypeName.Type.enumType,
        ):
            return self._load_primitive(any_obj)
        raise NotImplementedError(f"Unsupported type: {any_obj.type.name}")

    def _load_struct(self, any_obj: Any) -> StructOrUnionOrException:
        type_uri = any_obj.type.name.value
        if not isinstance(type_uri, TypeUri):
            raise ValueError("Any object does not contain a struct or union")
        cls = self._type_uri_to_cls(type_uri)
        if not issubclass(cls, (GeneratedError, StructOrUnion)):
            raise ValueError(f"{type_uri} is not a struct")
        return serializer.deserialize(
            # pyre-fixme[6]: For 1st argument expected `Type[Variable[sT (bound to
            #  Union[GeneratedError, StructOrUnion])]]` but got
            #  `Union[Type[Union[GeneratedError, StructOrUnion]], Type[Enum]]`.
            cls,
            any_obj.data,
            protocol=_standard_protocol_to_serializer_protocol(
                any_obj.protocol.standard
            ),
        )

    def _type_name_to_primitive_type(
        self, type_name: TypeName
    ) -> typing.Type[PrimitiveType]:
        if type_name.type is TypeName.Type.boolType:
            return bool
        if type_name.type in (
            TypeName.Type.i16Type,
            TypeName.Type.i32Type,
            TypeName.Type.i64Type,
        ):
            return int
        if type_name.type in (
            TypeName.Type.floatType,
            TypeName.Type.doubleType,
        ):
            return float
        if type_name.type is TypeName.Type.stringType:
            return str
        if type_name.type is TypeName.Type.binaryType:
            return bytes
        if type_name.type is TypeName.Type.enumType:
            cls = self._type_uri_to_cls(type_name.enumType)
            if not issubclass(cls, Enum):
                raise ValueError(f"{type_name.enumType} is not an enum")
            # pyre-fixme[7]: Expected `Type[Union[Enum, IOBuf, bool, bytes, float,
            #  int, str]]` but got `Union[Type[Union[GeneratedError, StructOrUnion]],
            #  Type[Enum]]`.
            return cls
        raise ValueError(f"Unsupported primitive type: {type_name}")

    def _load_primitive(self, any_obj: Any) -> PrimitiveType:
        return deserialize_primitive(
            self._type_name_to_primitive_type(any_obj.type.name),
            any_obj.data,
            protocol=_standard_protocol_to_serializer_protocol(
                any_obj.protocol.standard
            ),
        )
