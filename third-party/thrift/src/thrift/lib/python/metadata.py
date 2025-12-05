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
from types import MappingProxyType
from typing import cast, Iterator, Mapping, Optional, Sequence, Tuple, Type, Union

from apache.thrift.metadata.thrift_types import (
    ThriftBidiType,
    ThriftConstStruct,
    ThriftConstValue,
    ThriftEnum,
    ThriftEnumType,
    ThriftException,
    ThriftField,
    ThriftFunction,
    ThriftListType,
    ThriftMapType,
    ThriftMetadata,
    ThriftPrimitiveType,
    ThriftService,
    ThriftSetType,
    ThriftSinkType,
    ThriftStreamType,
    ThriftStruct,
    ThriftStructType,
    ThriftType,
    ThriftTypedefType,
    ThriftUnionType,
)
from thrift.python.client import Client
from thrift.python.exceptions import GeneratedError
from thrift.python.types import Enum, ServiceInterface, StructOrUnion


class ThriftKind(enum.Enum):
    PRIMITIVE = 0
    LIST = 1
    SET = 2
    MAP = 3
    ENUM = 4
    STRUCT = 5
    UNION = 6
    TYPEDEF = 7
    STREAM = 8
    SINK = 9
    BIDI = 10


class ThriftConstKind(enum.Enum):
    CV_BOOL = 0
    CV_INT = 1
    CV_FLOAT = 2
    CV_STRING = 3
    CV_MAP = 4
    CV_LIST = 5
    CV_STRUCT = 6


ValidThriftTypes = Union[
    ThriftStruct,
    ThriftEnum,
    ThriftPrimitiveType,
    ThriftListType,
    ThriftSetType,
    ThriftMapType,
    ThriftTypedefType,
    ThriftSinkType,
    ThriftBidiType,
    ThriftStreamType,
]


class ThriftTypeProxy:
    # A union of a bunch of thrift metadata types
    thriftType: ValidThriftTypes
    thriftMeta: ThriftMetadata
    kind: ThriftKind

    def __init__(
        self, thriftType: ValidThriftTypes, thriftMeta: ThriftMetadata
    ) -> None:
        if not isinstance(
            thriftType,
            (
                ThriftStruct,
                ThriftEnum,
                ThriftPrimitiveType,
                ThriftListType,
                ThriftSetType,
                ThriftMapType,
                ThriftTypedefType,
                ThriftBidiType,
                ThriftSinkType,
                ThriftStreamType,
            ),
        ):
            raise TypeError(f"{thriftType!r} is not a known thrift type.")
        self.thriftType = thriftType
        self.thriftMeta = thriftMeta
        self.kind = ThriftKind.PRIMITIVE

    @staticmethod
    def _fbthrift_create(
        thriftType: Optional[ThriftType], thriftMeta: ThriftMetadata
    ) -> "ThriftTypeProxy":
        # Not quite sure what situation this would be optional in
        assert thriftType is not None
        # Determine value and kind
        if thriftType.type is ThriftType.Type.t_list:
            val = thriftType.value
            assert isinstance(val, ThriftListType)
            return ThriftListProxy(val, thriftMeta)
        elif thriftType.type is ThriftType.Type.t_set:
            val = thriftType.value
            assert isinstance(val, ThriftSetType)
            return ThriftSetProxy(val, thriftMeta)
        elif thriftType.type is ThriftType.Type.t_map:
            val = thriftType.value
            assert isinstance(val, ThriftMapType)
            return ThriftMapProxy(val, thriftMeta)
        elif thriftType.type is ThriftType.Type.t_enum:
            val = thriftType.value
            assert isinstance(val, ThriftEnumType)
            specialType = ThriftTypeProxy(
                thriftMeta.enums[thriftType.value.name], thriftMeta
            )
            specialType.kind = ThriftKind.ENUM
            return specialType
        elif thriftType.type is ThriftType.Type.t_struct:
            val = thriftType.value
            assert isinstance(val, ThriftStructType)
            try:
                return ThriftStructProxy(val.name, thriftMeta)
            except KeyError as ex:  # val.name not in thriftMeta.structs
                try:
                    return ThriftExceptionProxy(val.name, thriftMeta)
                except Exception:
                    raise ex
        elif thriftType.type is ThriftType.Type.t_union:
            val = thriftType.value
            assert isinstance(val, ThriftUnionType)
            return ThriftStructProxy(val.name, thriftMeta)
        elif thriftType.type is ThriftType.Type.t_typedef:
            val = thriftType.value
            assert isinstance(val, ThriftTypedefType)
            return ThriftTypedefProxy(val, thriftMeta)
        elif thriftType.type is ThriftType.Type.t_stream:
            val = thriftType.value
            assert isinstance(val, ThriftStreamType)
            return ThriftStreamProxy(val, thriftMeta)
        elif thriftType.type is ThriftType.Type.t_sink:
            val = thriftType.value
            assert isinstance(val, ThriftSinkType)
            return ThriftSinkProxy(val, thriftMeta)
        elif thriftType.type is ThriftType.Type.t_bidi:
            val = thriftType.value
            assert isinstance(val, ThriftBidiType)
            return ThriftBidiProxy(val, thriftMeta)
        val = thriftType.value
        assert isinstance(val, ThriftPrimitiveType)
        specialType = ThriftTypeProxy(val, thriftMeta)
        specialType.kind = ThriftKind.PRIMITIVE
        return specialType

    def as_primitive(self) -> ThriftPrimitiveType:
        if self.kind == ThriftKind.PRIMITIVE:
            return cast(ThriftPrimitiveType, self.thriftType)
        raise TypeError("Type is not primitive")

    def as_struct(self) -> "ThriftStructProxy":
        if self.kind == ThriftKind.STRUCT or self.kind == ThriftKind.UNION:
            return cast(ThriftStructProxy, self)
        raise TypeError("Type is not a struct")

    def as_union(self) -> "ThriftStructProxy":
        if self.kind == ThriftKind.UNION:
            return cast(ThriftStructProxy, self)
        raise TypeError("Type is not a union")

    def as_enum(self) -> ThriftEnum:
        if self.kind == ThriftKind.ENUM:
            return cast(ThriftEnum, self.thriftType)
        raise TypeError("Type is not an enum")

    def as_list(self) -> "ThriftListProxy":
        if self.kind == ThriftKind.LIST:
            return cast(ThriftListProxy, self)
        raise TypeError("Type is not a list")

    def as_set(self) -> "ThriftSetProxy":
        if self.kind == ThriftKind.SET:
            return cast(ThriftSetProxy, self)
        raise TypeError("Type is not a set")

    def as_map(self) -> "ThriftMapProxy":
        if self.kind == ThriftKind.MAP:
            return cast(ThriftMapProxy, self)
        raise TypeError("Type is not a map")

    def as_typedef(self) -> "ThriftTypedefProxy":
        if self.kind == ThriftKind.TYPEDEF:
            return cast(ThriftTypedefProxy, self)
        raise TypeError("Type is not a typedef")

    def as_stream(self) -> "ThriftStreamProxy":
        if self.kind == ThriftKind.STREAM:
            return cast(ThriftStreamProxy, self)
        raise TypeError("Type is not a stream")

    def as_sink(self) -> "ThriftSinkProxy":
        if self.kind == ThriftKind.SINK:
            return cast(ThriftSinkProxy, self)
        raise TypeError("Type is not a sink")

    def as_bidi(self) -> "ThriftBidiProxy":
        if self.kind == ThriftKind.BIDI:
            return cast(ThriftBidiProxy, self)
        raise TypeError("Type is not a bidirectional stream")


class ThriftSetProxy(ThriftTypeProxy):
    valueType: ThriftTypeProxy

    def __init__(self, thriftType: ThriftSetType, thriftMeta: ThriftMetadata) -> None:
        super().__init__(thriftType, thriftMeta)
        self.kind: ThriftKind = ThriftKind.SET
        self.valueType = ThriftTypeProxy._fbthrift_create(
            thriftType.valueType, self.thriftMeta
        )


class ThriftListProxy(ThriftTypeProxy):
    valueType: ThriftTypeProxy

    def __init__(self, thriftType: ThriftListType, thriftMeta: ThriftMetadata) -> None:
        super().__init__(thriftType, thriftMeta)
        self.kind: ThriftKind = ThriftKind.LIST
        self.valueType = ThriftTypeProxy._fbthrift_create(
            thriftType.valueType, self.thriftMeta
        )


class ThriftMapProxy(ThriftTypeProxy):
    valueType: ThriftTypeProxy
    keyType: ThriftTypeProxy

    def __init__(self, thriftType: ThriftMapType, thriftMeta: ThriftMetadata) -> None:
        super().__init__(thriftType, thriftMeta)
        self.kind: ThriftKind = ThriftKind.MAP
        self.valueType = ThriftTypeProxy._fbthrift_create(
            thriftType.valueType, self.thriftMeta
        )
        self.keyType = ThriftTypeProxy._fbthrift_create(
            thriftType.keyType, self.thriftMeta
        )


class ThriftTypedefProxy(ThriftTypeProxy):
    underlyingType: ThriftTypeProxy
    name: str

    def __init__(
        self, thriftType: ThriftTypedefType, thriftMeta: ThriftMetadata
    ) -> None:
        super().__init__(thriftType, thriftMeta)
        self.kind: ThriftKind = ThriftKind.TYPEDEF
        self.name = thriftType.name
        self.underlyingType = ThriftTypeProxy._fbthrift_create(
            thriftType.underlyingType, self.thriftMeta
        )


class ThriftSinkProxy(ThriftTypeProxy):
    elemType: ThriftTypeProxy
    initialResponseType: ThriftTypeProxy
    finalResponseType: ThriftTypeProxy

    def __init__(self, thriftType: ThriftSinkType, thriftMeta: ThriftMetadata) -> None:
        super().__init__(thriftType, thriftMeta)
        self.kind: ThriftKind = ThriftKind.SINK
        self.elemType = ThriftTypeProxy._fbthrift_create(
            thriftType.elemType, self.thriftMeta
        )
        if thriftType.initialResponseType is not None:
            self.initialResponseType = ThriftTypeProxy._fbthrift_create(
                thriftType.initialResponseType, self.thriftMeta
            )
        if thriftType.finalResponseType is not None:
            self.finalResponseType = ThriftTypeProxy._fbthrift_create(
                thriftType.finalResponseType, self.thriftMeta
            )


class ThriftStreamProxy(ThriftTypeProxy):
    elemType: ThriftTypeProxy
    initialResponseType: ThriftTypeProxy

    def __init__(
        self, thriftType: ThriftStreamType, thriftMeta: ThriftMetadata
    ) -> None:
        super().__init__(thriftType, thriftMeta)
        self.kind: ThriftKind = ThriftKind.STREAM
        self.elemType = ThriftTypeProxy._fbthrift_create(
            thriftType.elemType, self.thriftMeta
        )
        if thriftType.initialResponseType is not None:
            self.initialResponseType = ThriftTypeProxy._fbthrift_create(
                thriftType.initialResponseType, self.thriftMeta
            )


class ThriftBidiProxy(ThriftTypeProxy):
    streamElemType: ThriftTypeProxy
    sinkElemType: ThriftTypeProxy
    initialResponseType: ThriftTypeProxy

    def __init__(self, thriftType: ThriftBidiType, thriftMeta: ThriftMetadata) -> None:
        super().__init__(thriftType, thriftMeta)
        self.kind: ThriftKind = ThriftKind.BIDI
        self.streamElemType = ThriftTypeProxy._fbthrift_create(
            thriftType.streamElemType, self.thriftMeta
        )
        self.sinkElemType = ThriftTypeProxy._fbthrift_create(
            thriftType.sinkElemType, self.thriftMeta
        )
        if thriftType.initialResponseType is not None:
            self.initialResponseType = ThriftTypeProxy._fbthrift_create(
                thriftType.initialResponseType, self.thriftMeta
            )


class ThriftFieldProxy:
    type: ThriftTypeProxy
    thriftType: ThriftField
    thriftMeta: ThriftMetadata
    id: int
    name: str
    is_optional: int

    def __init__(self, thriftType: ThriftField, thriftMeta: ThriftMetadata) -> None:
        self.type = ThriftTypeProxy._fbthrift_create(thriftType.type, thriftMeta)
        self.thriftType = thriftType
        self.thriftMeta = thriftMeta
        self.id = self.thriftType.id
        self.name = self.thriftType.name
        self.is_optional = self.thriftType.is_optional
        self.structuredAnnotations: Tuple[ThriftConstStructProxy] = tuple(
            ThriftConstStructProxy(annotation)
            for annotation in self.thriftType.structured_annotations
        )

    @property
    def pyname(self) -> str:
        if self.thriftType.structured_annotations is not None:
            for annotation in self.thriftType.structured_annotations:
                if annotation.type.name == "python.Name":
                    return annotation.fields["name"].cv_string

        return self.name


class ThriftStructProxy(ThriftTypeProxy):
    name: str
    is_union: int

    def __init__(self, name: str, thriftMeta: ThriftMetadata) -> None:
        super().__init__(thriftMeta.structs[name], thriftMeta)
        assert isinstance(self.thriftType, ThriftStruct)
        self.name = self.thriftType.name
        self.is_union = self.thriftType.is_union
        self.structuredAnnotations: Tuple[ThriftConstStructProxy] = tuple(
            ThriftConstStructProxy(annotation)
            for annotation in self.thriftType.structured_annotations
        )

        if self.is_union:
            self.kind: ThriftKind = ThriftKind.UNION
        else:
            self.kind: ThriftKind = ThriftKind.STRUCT

    @property
    def fields(self) -> Iterator[ThriftFieldProxy]:
        thriftType = self.thriftType
        assert isinstance(thriftType, ThriftStruct)
        for field in thriftType.fields:
            yield ThriftFieldProxy(field, self.thriftMeta)


ConstType = Union[  # type: ignore
    bool,
    int,
    float,
    str,
    Sequence["ThriftConstValueProxy"],
    Mapping["ConstType", "ThriftConstValueProxy"],
    "ThriftConstStructProxy",
]


class ThriftConstValueProxy:
    thriftType: ThriftConstValue
    kind: ThriftConstKind
    type: ConstType

    def __init__(self, value: ThriftConstValue) -> None:
        self.thriftType = value
        if self.thriftType.type in (
            ThriftConstValue.Type.cv_bool,
            ThriftConstValue.Type.cv_integer,
            ThriftConstValue.Type.cv_double,
            ThriftConstValue.Type.cv_string,
        ):
            self.type = self.thriftType.value
            if self.thriftType.type is ThriftConstValue.Type.cv_bool:
                self.kind = ThriftConstKind.CV_BOOL
            elif self.thriftType.type is ThriftConstValue.Type.cv_integer:
                self.kind = ThriftConstKind.CV_INT
            elif self.thriftType.type is ThriftConstValue.Type.cv_double:
                self.kind = ThriftConstKind.CV_FLOAT
            else:
                self.kind = ThriftConstKind.CV_STRING
        if self.thriftType.type is ThriftConstValue.Type.cv_struct:
            assert isinstance(self.thriftType.value, ThriftConstStruct)
            self.type = ThriftConstStructProxy(self.thriftType.value)
            self.kind = ThriftConstKind.CV_STRUCT
        if self.thriftType.type is ThriftConstValue.Type.cv_list:
            self.type = tuple(
                ThriftConstValueProxy(ele)
                # pyre-fixme[16]: Undefined attribute [16]: `None` has no attribute `__iter__`
                for ele in self.thriftType.value
            )
            self.kind = ThriftConstKind.CV_LIST
        if self.thriftType.type is ThriftConstValue.Type.cv_map:
            self.type = MappingProxyType(
                {
                    ThriftConstValueProxy(ele.key).type: ThriftConstValueProxy(
                        ele.value
                    )
                    for ele in self.thriftType.value
                }
            )
            self.kind = ThriftConstKind.CV_MAP

    def as_bool(self) -> bool:
        if self.kind == ThriftConstKind.CV_BOOL:
            return cast(bool, self.type)
        raise TypeError("Type is not a boolean")

    def as_int(self) -> int:
        if self.kind == ThriftConstKind.CV_INT:
            return cast(int, self.type)
        raise TypeError("Type is not an integer")

    def as_float(self) -> float:
        if self.kind == ThriftConstKind.CV_FLOAT:
            return cast(float, self.type)
        raise TypeError("Type is not a float")

    def as_string(self) -> str:
        if self.kind == ThriftConstKind.CV_STRING:
            return cast(str, self.type)
        raise TypeError("Type is not a string")

    def as_struct(self) -> Sequence["ThriftConstValueProxy"]:
        if self.kind == ThriftConstKind.CV_STRUCT:
            return cast(Sequence[ThriftConstValueProxy], self.type)
        raise TypeError("Type is not a struct")

    def as_list(self) -> Mapping[ConstType, "ThriftConstValueProxy"]:
        if self.kind == ThriftConstKind.CV_LIST:
            return cast(Mapping[ConstType, ThriftConstValueProxy], self.type)
        raise TypeError("Type is not a list")

    def as_map(self) -> "ThriftConstStructProxy":
        if self.kind == ThriftConstKind.CV_MAP:
            return cast(ThriftConstStructProxy, self.type)
        raise TypeError("Type is not a map")


class ThriftConstStructProxy:
    thriftType: ThriftConstStruct
    name: str
    kind: ThriftKind

    def __init__(self, struct: ThriftConstStruct) -> None:
        self.name = struct.type.name
        self.kind = ThriftKind.STRUCT
        self.thriftType = struct

    @property
    def fields(self) -> Mapping[str, ThriftConstValueProxy]:
        return MappingProxyType(
            {
                key: ThriftConstValueProxy(self.thriftType.fields[key])
                for key in self.thriftType.fields
            }
        )


class ThriftExceptionProxy:
    thriftType: ThriftException
    thriftMeta: ThriftMetadata
    name: str

    def __init__(self, name: str, thriftMeta: ThriftMetadata) -> None:
        self.thriftType = thriftMeta.exceptions[name]
        self.thriftMeta = thriftMeta
        self.name = self.thriftType.name
        self.structuredAnnotations: Tuple[ThriftConstStructProxy] = tuple(
            ThriftConstStructProxy(annotation)
            for annotation in self.thriftType.structured_annotations
        )

    @property
    def fields(self) -> Iterator[ThriftFieldProxy]:
        for field in self.thriftType.fields:
            yield ThriftFieldProxy(field, self.thriftMeta)


class ThriftFunctionProxy:
    name: str
    thriftType: ThriftFunction
    thriftMeta: ThriftMetadata
    return_type: ThriftTypeProxy
    is_oneway: int

    def __init__(self, thriftType: ThriftFunction, thriftMeta: ThriftMetadata) -> None:
        self.name = thriftType.name
        self.thriftType = thriftType
        self.thriftMeta = thriftMeta
        self.return_type = ThriftTypeProxy._fbthrift_create(
            self.thriftType.return_type, self.thriftMeta
        )
        self.is_oneway = self.thriftType.is_oneway
        self.structuredAnnotations: Tuple[ThriftConstStructProxy] = tuple(
            ThriftConstStructProxy(annotation)
            for annotation in self.thriftType.structured_annotations
        )

    @property
    def arguments(self) -> Iterator[ThriftFieldProxy]:
        for argument in self.thriftType.arguments:
            yield ThriftFieldProxy(argument, self.thriftMeta)

    @property
    def exceptions(self) -> Iterator[ThriftFieldProxy]:
        for exception in self.thriftType.exceptions:
            yield ThriftFieldProxy(exception, self.thriftMeta)


class ThriftServiceProxy:
    thriftType: ThriftService
    name: str
    thriftMeta: ThriftMetadata

    def __init__(self, name: str, thriftMeta: ThriftMetadata) -> None:
        self.thriftType = thriftMeta.services[name]
        self.name = self.thriftType.name
        self.thriftMeta = thriftMeta
        self.parent: Optional[ThriftServiceProxy] = (
            None
            if self.thriftType.parent is None
            else ThriftServiceProxy(self.thriftType.parent, self.thriftMeta)
        )
        self.structuredAnnotations: Tuple[ThriftConstStructProxy] = tuple(
            ThriftConstStructProxy(annotation)
            for annotation in self.thriftType.structured_annotations
        )

    @property
    def functions(self) -> Iterator[ThriftFunctionProxy]:
        for function in self.thriftType.functions:
            yield ThriftFunctionProxy(function, self.thriftMeta)


def gen_metadata(
    obj_or_cls: Union[
        StructOrUnion,
        Type[StructOrUnion],
        GeneratedError,
        Type[GeneratedError],
        ServiceInterface,
        Type[ServiceInterface],
    ],
) -> Union[ThriftStructProxy, ThriftExceptionProxy, ThriftServiceProxy, ThriftMetadata]:
    if hasattr(obj_or_cls, "getThriftModuleMetadata"):
        return obj_or_cls.getThriftModuleMetadata()

    cls = obj_or_cls if isinstance(obj_or_cls, type) else type(obj_or_cls)

    if not issubclass(
        cls, (StructOrUnion, GeneratedError, ServiceInterface, Enum, Client)
    ):
        raise TypeError(f"{cls!r} is not a thrift-python type.")

    meta: ThriftMetadata = cls.__get_metadata__()
    name: str = cls.__get_thrift_name__()

    if issubclass(cls, StructOrUnion):
        return ThriftStructProxy(name, meta)
    elif issubclass(cls, GeneratedError):
        return ThriftExceptionProxy(name, meta)
    elif issubclass(cls, ServiceInterface):
        return ThriftServiceProxy(name, meta)
    elif issubclass(cls, Client):
        return ThriftServiceProxy(name, meta)
    elif issubclass(cls, Enum):
        return meta.enums[name]
    else:
        raise TypeError(f"unsupported thrift-python type: {cls!r}.")
