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

# pyre-unsafe


from __future__ import annotations

import enum
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
from thrift.python.any.serializer import (
    deserialize_list,
    deserialize_map,
    deserialize_primitive,
    deserialize_set,
    serialize_list,
    serialize_map,
    serialize_primitive,
    serialize_set,
)
from thrift.python.conformance.universal_name import (
    find_by_universal_hash,
    get_universal_hash,
    get_universal_hash_prefix,
    UniversalHashAlgorithm,
)
from thrift.python.exceptions import GeneratedError
from thrift.python.types import Enum, StructOrUnion, Union

# When storing an empty container, the elem type doesn't matter so we just use bool as placeholder
_ELEM_TYPE_FOR_EMPTY_CONTAINERS = Type(name=TypeName(boolType=Void.Unused))


from thrift.lib.python.any.typestub import (
    ClassWithUri,
    ObjWithUri,
    PrimitiveType,
    SerializableType,
    SerializableTypeOrContainers,
    StructOrUnionOrException,
    TKey,
    TSerializable,
    TValue,
)


class TypeUriOption(enum.Enum):
    URI = "uri"
    HASH_PREFIX = "typeHashPrefixSha2_256"


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
        raise ValueError(f"Thrift struct doesn't have URI. {obj}")
    return get_universal_hash_prefix(
        get_universal_hash(UniversalHashAlgorithm.Sha2_256, uri),
        16,
    )


def _infer_type_from_obj(
    obj: SerializableType,
    typeuri_option: TypeUriOption = TypeUriOption.HASH_PREFIX,
) -> Type:
    if isinstance(obj, (Enum, GeneratedError, StructOrUnion)):
        if typeuri_option == TypeUriOption.URI:
            type_uri = TypeUri(uri=obj.__get_thrift_uri__())
        elif typeuri_option == TypeUriOption.HASH_PREFIX:
            type_uri = TypeUri(typeHashPrefixSha2_256=_get_type_hash_prefix(obj))
        else:
            raise ValueError(f"Unsupported typeuri_option: {typeuri_option}")
        if isinstance(obj, Enum):
            return Type(name=TypeName(enumType=type_uri))
        if isinstance(obj, Union):
            return Type(name=TypeName(unionType=type_uri))
        if isinstance(obj, GeneratedError):
            return Type(name=TypeName(exceptionType=type_uri))
        return Type(name=TypeName(structType=type_uri))
    if isinstance(obj, bool):
        return Type(name=TypeName(boolType=Void.Unused))
    if isinstance(obj, int):
        return Type(name=TypeName(i64Type=Void.Unused))
    if isinstance(obj, float):
        return Type(name=TypeName(doubleType=Void.Unused))
    if isinstance(obj, str):
        return Type(name=TypeName(stringType=Void.Unused))
    if isinstance(obj, bytes):
        return Type(name=TypeName(binaryType=Void.Unused))
    if isinstance(obj, IOBuf):
        return Type(name=TypeName(binaryType=Void.Unused))
    raise ValueError(f"Can not infer thrift type from: {obj}")


class AnyRegistry:
    def __init__(self) -> None:
        self._uri_to_cls: typing.Dict[str, ClassWithUri] = {}
        self._alg_to_hash_to_cls: typing.Dict[
            UniversalHashAlgorithm, typing.Dict[bytes, ClassWithUri]
        ] = {alg: {} for alg in UniversalHashAlgorithm}

    def register_type(self, cls: ClassWithUri) -> bool:
        if cls in (bool, int, float, str, bytes, IOBuf):
            return False
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
        self,
        obj: SerializableTypeOrContainers,
        protocol: typing.Optional[Protocol] = None,
        # If HASH_PREFIX, ThriftStruct's type uri in Any will be typeHashPrefixSha2_256
        # (https://fburl.com/code/ftvf5eb4); If URI, ThriftStruct's type uri in
        # Any will be human readable uri (https://fburl.com/code/s7nvqelm)
        typeuri_option: TypeUriOption = TypeUriOption.HASH_PREFIX,
    ) -> Any:
        if protocol is None:
            protocol = Protocol(standard=StandardProtocol.Compact)
        if protocol.type != Protocol.Type.standard:
            raise NotImplementedError(
                f"Unsupported non-standard protocol: {protocol.value}"
            )
        if isinstance(obj, (GeneratedError, StructOrUnion)):
            return self._store_struct(
                obj, protocol=protocol, typeuri_option=typeuri_option
            )
        if isinstance(obj, (bool, int, float, str, bytes, IOBuf, Enum)):
            return self._store_primitive(obj, protocol=protocol)
        if isinstance(obj, typing.Sequence):
            return self._store_list(obj, protocol=protocol)
        if isinstance(obj, typing.AbstractSet):
            return self._store_set(obj, protocol=protocol)
        if isinstance(obj, typing.Mapping):
            return self._store_map(obj, protocol=protocol)
        raise ValueError(f"Unsupported type: {type(obj)}")

    def _store_struct(
        self,
        obj: StructOrUnionOrException,
        protocol: Protocol,
        typeuri_option: TypeUriOption,
    ) -> Any:
        return Any(
            type=_infer_type_from_obj(obj, typeuri_option=typeuri_option),
            protocol=protocol,
            data=serializer.serialize_iobuf(
                obj,
                protocol=_standard_protocol_to_serializer_protocol(protocol.standard),
            ),
        )

    def _store_primitive(self, obj: PrimitiveType, protocol: Protocol) -> Any:
        thrift_type = _infer_type_from_obj(obj)
        return Any(
            type=thrift_type,
            protocol=protocol,
            data=serialize_primitive(
                obj,
                protocol=_standard_protocol_to_serializer_protocol(protocol.standard),
                thrift_type=thrift_type,
            ),
        )

    def _store_list(
        self, obj: typing.Sequence[TSerializable], protocol: Protocol
    ) -> Any:
        if obj:
            elem_type = _infer_type_from_obj(obj[0])
        else:
            elem_type = _ELEM_TYPE_FOR_EMPTY_CONTAINERS
        return Any(
            type=Type(
                name=TypeName(listType=Void.Unused),
                params=[elem_type],
            ),
            protocol=protocol,
            data=serialize_list(
                obj,
                protocol=_standard_protocol_to_serializer_protocol(protocol.standard),
            ),
        )

    def _store_set(
        self, obj: typing.AbstractSet[TSerializable], protocol: Protocol
    ) -> Any:
        if obj:
            elem_type = _infer_type_from_obj(next(iter(obj)))
        else:
            elem_type = _ELEM_TYPE_FOR_EMPTY_CONTAINERS
        return Any(
            type=Type(
                name=TypeName(setType=Void.Unused),
                params=[elem_type],
            ),
            protocol=protocol,
            data=serialize_set(
                obj,
                protocol=_standard_protocol_to_serializer_protocol(protocol.standard),
            ),
        )

    def _store_map(self, obj: typing.Mapping[TKey, TValue], protocol: Protocol) -> Any:
        if obj:
            key, value = next(iter(obj.items()))
            key_type = _infer_type_from_obj(key)
            value_type = _infer_type_from_obj(value)
        else:
            key_type = value_type = _ELEM_TYPE_FOR_EMPTY_CONTAINERS
        return Any(
            type=Type(
                name=TypeName(mapType=Void.Unused),
                params=[key_type, value_type],
            ),
            protocol=protocol,
            data=serialize_map(
                obj,
                protocol=_standard_protocol_to_serializer_protocol(protocol.standard),
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

    def load(self, any_obj: Any) -> SerializableTypeOrContainers:
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
        if any_obj.type.name.type is TypeName.Type.listType:
            return self._load_list(any_obj)
        if any_obj.type.name.type is TypeName.Type.setType:
            return self._load_set(any_obj)
        if any_obj.type.name.type is TypeName.Type.mapType:
            return self._load_map(any_obj)
        raise NotImplementedError(f"Unsupported type: {any_obj.type}")

    def _type_name_to_serializable_type(
        self, type_name: TypeName
    ) -> typing.Type[SerializableType]:
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
        if type_name.type in (
            TypeName.Type.enumType,
            TypeName.Type.structType,
            TypeName.Type.unionType,
            TypeName.Type.exceptionType,
        ):
            # pyre-fixme[6]: For 1st argument expected `TypeUri` but got
            #  `Union[None, TypeUri, Void]`.
            cls = self._type_uri_to_cls(type_name.value)
            if type_name.type is TypeName.Type.enumType and (not issubclass(cls, Enum)):
                raise ValueError(f"{type_name.enumType} is not an enum")
            if type_name.type in (
                TypeName.Type.structType,
                TypeName.Type.unionType,
            ) and (not issubclass(cls, StructOrUnion)):
                raise ValueError(f"{type_name.enumType} is not a struct/union")
            if type_name.type is TypeName.Type.exceptionType and (
                not issubclass(cls, GeneratedError)
            ):
                raise ValueError(f"{type_name.enumType} is not an exception")
            return cls
        raise ValueError(f"Unsupported type: {type_name}")

    def _load_struct(self, any_obj: Any) -> StructOrUnionOrException:
        return serializer.deserialize(
            # pyre-fixme[6]: For 1st argument expected `Type[Variable[sT (bound to
            #  Union[GeneratedError, StructOrUnion])]]` but got `Type[Union[Enum,
            #  GeneratedError, IOBuf, StructOrUnion, bool, bytes, float, int, str]]`.
            self._type_name_to_serializable_type(any_obj.type.name),
            any_obj.data,
            protocol=_standard_protocol_to_serializer_protocol(
                any_obj.protocol.standard
            ),
        )

    def _load_primitive(self, any_obj: Any) -> PrimitiveType:
        return deserialize_primitive(
            # pyre-fixme[6]: The deserializer expects primitive, but this to_serializable_type function can give any valid type
            self._type_name_to_serializable_type(any_obj.type.name),
            any_obj.data,
            protocol=_standard_protocol_to_serializer_protocol(
                any_obj.protocol.standard
            ),
        )

    def _load_list(self, any_obj: Any) -> typing.Sequence[SerializableType]:
        elem_cls = self._type_name_to_serializable_type(any_obj.type.params[0].name)
        return deserialize_list(
            elem_cls,
            any_obj.data,
            protocol=_standard_protocol_to_serializer_protocol(
                any_obj.protocol.standard
            ),
        )

    def _load_set(self, any_obj: Any) -> typing.AbstractSet[SerializableType]:
        elem_cls = self._type_name_to_serializable_type(any_obj.type.params[0].name)
        return deserialize_set(
            elem_cls,
            any_obj.data,
            protocol=_standard_protocol_to_serializer_protocol(
                any_obj.protocol.standard
            ),
        )

    def _load_map(
        self, any_obj: Any
    ) -> typing.Mapping[SerializableType, SerializableType]:
        key_cls = self._type_name_to_serializable_type(any_obj.type.params[0].name)
        value_cls = self._type_name_to_serializable_type(any_obj.type.params[1].name)
        return deserialize_map(
            key_cls,
            value_cls,
            any_obj.data,
            protocol=_standard_protocol_to_serializer_protocol(
                any_obj.protocol.standard
            ),
        )
