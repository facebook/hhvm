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

import builtins
import threading
from functools import wraps
from types import MappingProxyType
from typing import Any, Callable, Iterator, overload, Sequence, Type, TypeVar, Union

from thrift.python.exceptions import GeneratedError
from thrift.python.reflection.constants_reflection import (
    _ImmutableMeta,
    ConstantMapSpec,
    ConstantSpec,
    ConstantStructSpec,
    ThriftType,
)
from thrift.python.reflection_enums import NumberType, Qualifier, StructType
from thrift.python.types import (
    ImmutableList,
    ImmutableMap,
    ImmutableSet,
    List,
    Map,
    Set,
    Struct,
    Union as Union_,
)


_THRIFT_TYPE_TO_NUMBER_TYPE: dict[ThriftType, NumberType] = {
    ThriftType.BYTE: NumberType.BYTE,
    ThriftType.I16: NumberType.I16,
    ThriftType.I32: NumberType.I32,
    ThriftType.I64: NumberType.I64,
    ThriftType.FLOAT: NumberType.FLOAT,
    ThriftType.DOUBLE: NumberType.DOUBLE,
}


def _structured_annotations_to_dict(
    structured_annotations: MappingProxyType[str, ConstantSpec],
) -> dict[str, str | dict[str, object]]:
    result: dict[str, str | dict[str, object]] = {}
    for name, const_spec in structured_annotations.items():
        struct_val = const_spec.value
        if not isinstance(struct_val, ConstantStructSpec):
            continue
        if name.endswith("DeprecatedUnvalidatedAnnotations"):
            items_const = struct_val.fields["items"]
            items_map = items_const.value
            assert isinstance(items_map, ConstantMapSpec)
            for key_spec, val_spec in items_map.value.items():
                assert isinstance(key_spec.value, str)
                assert isinstance(val_spec.value, str)
                result[key_spec.value] = val_spec.value
        else:
            result[name] = {
                field_name: field_const.value
                for field_name, field_const in struct_val.fields.items()
            }
    return result


class FieldSpec(metaclass=_ImmutableMeta):
    __slots__ = (
        "id",
        "name",
        "py_name",
        "type",
        "thrift_type",
        "qualifier",
        "default",
        "structured_annotations",
    )
    id: int
    name: str
    py_name: str
    type: builtins.type[Any]
    thrift_type: ThriftType
    qualifier: Qualifier
    default: Any
    structured_annotations: MappingProxyType[str, ConstantSpec]

    def __init__(
        self,
        *,
        id: int,
        name: str,
        py_name: str,
        type: builtins.type[Any],
        thrift_type: ThriftType,
        qualifier: Qualifier,
        default: Any = None,
        structured_annotations: dict[str, ConstantSpec] | None = None,
    ) -> None:
        self.id = id
        self.name = name
        self.py_name = py_name
        self.type = type
        self.thrift_type = thrift_type
        self.qualifier = qualifier
        self.default = default
        self.structured_annotations = MappingProxyType(structured_annotations or {})

    @property
    def kind(self) -> NumberType:
        return _THRIFT_TYPE_TO_NUMBER_TYPE.get(
            self.thrift_type, NumberType.NOT_A_NUMBER
        )

    @property
    def annotations(self) -> dict[str, str | dict[str, object]]:
        return _structured_annotations_to_dict(self.structured_annotations)

    def __iter__(self) -> Iterator[Any]:
        yield self.id
        yield self.name
        yield self.type
        yield self.kind
        yield self.qualifier
        yield self.default
        yield self.annotations

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, FieldSpec):
            return NotImplemented
        return (
            self.id == other.id
            and self.name == other.name
            and self.py_name == other.py_name
            and self.type == other.type
            and self.thrift_type == other.thrift_type
            and self.qualifier == other.qualifier
            and self.default == other.default
            and self.structured_annotations == other.structured_annotations
        )

    def __repr__(self) -> str:
        return (
            f"FieldSpec(id={self.id}, name={self.name!r}, py_name={self.py_name!r}, "
            f"type={self.type!r}, thrift_type={self.thrift_type!r}, "
            f"qualifier={self.qualifier!r}, default={self.default!r})"
        )


class StructSpec(metaclass=_ImmutableMeta):
    __slots__ = ("name", "_fields", "_fields_by_name", "kind", "structured_annotations")
    name: str
    _fields: list[FieldSpec]
    _fields_by_name: MappingProxyType[str, FieldSpec] | None
    kind: StructType
    structured_annotations: MappingProxyType[str, ConstantSpec]

    def __init__(
        self,
        *,
        name: str,
        kind: StructType,
        fields: Sequence[FieldSpec] | None = None,
        structured_annotations: dict[str, ConstantSpec] | None = None,
    ) -> None:
        self.name = name
        self.kind = kind
        self._fields = list(fields) if fields else []
        self._fields_by_name = None
        self.structured_annotations = MappingProxyType(structured_annotations or {})

    @property
    def fields(self) -> tuple[FieldSpec, ...]:
        return tuple(self._fields)

    @property
    def fields_by_name(self) -> MappingProxyType[str, FieldSpec]:
        result = self._fields_by_name
        if result is None:
            result = MappingProxyType({f.py_name: f for f in self._fields})
            object.__setattr__(self, "_fields_by_name", result)
        return result

    def get_field(self, name: str) -> FieldSpec | None:
        if (field := self.fields_by_name.get(name)) is not None:
            return field
        return next((f for f in self._fields if f.name == name), None)

    def add_field(self, field: FieldSpec) -> None:
        self._fields.append(field)
        object.__setattr__(self, "_fields_by_name", None)

    @property
    def annotations(self) -> dict[str, str | dict[str, object]]:
        return _structured_annotations_to_dict(self.structured_annotations)

    def __iter__(self) -> Iterator[Any]:
        yield self.name
        yield self.fields
        yield self.kind
        yield self.annotations

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, StructSpec):
            return NotImplemented
        return (
            self.name == other.name
            and self.kind == other.kind
            and self.fields == other.fields
            and self.structured_annotations == other.structured_annotations
        )

    def __repr__(self) -> str:
        return (
            f"StructSpec(name={self.name!r}, kind={self.kind!r}, "
            f"fields={self.fields!r})"
        )


class ListSpec(metaclass=_ImmutableMeta):
    __slots__ = ("value", "thrift_type")
    value: type[Any]
    thrift_type: ThriftType

    def __init__(self, *, value: type[Any], thrift_type: ThriftType) -> None:
        self.value = value
        self.thrift_type = thrift_type

    @property
    def kind(self) -> NumberType:
        return _THRIFT_TYPE_TO_NUMBER_TYPE.get(
            self.thrift_type, NumberType.NOT_A_NUMBER
        )

    def __iter__(self) -> Iterator[Any]:
        yield self.value
        yield self.kind

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, ListSpec):
            return NotImplemented
        return self.value == other.value and self.thrift_type == other.thrift_type

    def __repr__(self) -> str:
        return f"ListSpec(value={self.value!r}, thrift_type={self.thrift_type!r})"


class SetSpec(metaclass=_ImmutableMeta):
    __slots__ = ("value", "thrift_type")
    value: type[Any]
    thrift_type: ThriftType

    def __init__(self, *, value: type[Any], thrift_type: ThriftType) -> None:
        self.value = value
        self.thrift_type = thrift_type

    @property
    def kind(self) -> NumberType:
        return _THRIFT_TYPE_TO_NUMBER_TYPE.get(
            self.thrift_type, NumberType.NOT_A_NUMBER
        )

    def __iter__(self) -> Iterator[Any]:
        yield self.value
        yield self.kind

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, SetSpec):
            return NotImplemented
        return self.value == other.value and self.thrift_type == other.thrift_type

    def __repr__(self) -> str:
        return f"SetSpec(value={self.value!r}, thrift_type={self.thrift_type!r})"


class MapSpec(metaclass=_ImmutableMeta):
    __slots__ = ("key", "key_thrift_type", "value", "value_thrift_type")
    key: type[Any]
    key_thrift_type: ThriftType
    value: type[Any]
    value_thrift_type: ThriftType

    def __init__(
        self,
        *,
        key: type[Any],
        key_thrift_type: ThriftType,
        value: type[Any],
        value_thrift_type: ThriftType,
    ) -> None:
        self.key = key
        self.key_thrift_type = key_thrift_type
        self.value = value
        self.value_thrift_type = value_thrift_type

    @property
    def key_kind(self) -> NumberType:
        return _THRIFT_TYPE_TO_NUMBER_TYPE.get(
            self.key_thrift_type, NumberType.NOT_A_NUMBER
        )

    @property
    def value_kind(self) -> NumberType:
        return _THRIFT_TYPE_TO_NUMBER_TYPE.get(
            self.value_thrift_type, NumberType.NOT_A_NUMBER
        )

    def __iter__(self) -> Iterator[Any]:
        yield self.key
        yield self.key_kind
        yield self.value
        yield self.value_kind

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, MapSpec):
            return NotImplemented
        return (
            self.key == other.key
            and self.key_thrift_type == other.key_thrift_type
            and self.value == other.value
            and self.value_thrift_type == other.value_thrift_type
        )

    def __repr__(self) -> str:
        return (
            f"MapSpec(key={self.key!r}, key_thrift_type={self.key_thrift_type!r}, "
            f"value={self.value!r}, value_thrift_type={self.value_thrift_type!r})"
        )


_F = TypeVar("_F", bound=Callable[..., Any])


def _threadsafe_cached(fn: _F) -> _F:
    cache: dict[type[Any], Any] = {}
    lock: threading.Lock = threading.Lock()

    @wraps(fn)
    def wrapper(key: type[Any]) -> Any:
        if key in cache:
            return cache[key]
        with lock:
            if key in cache:
                return cache[key]
            result = cache[key] = fn(key)
            return result

    return wrapper  # type: ignore[return-value]


@_threadsafe_cached
def _inspect_impl(cls: type[Any]) -> StructSpec | ListSpec | SetSpec | MapSpec:
    return cls.__get_reflection__()  # type: ignore[attr-defined]


@overload
def inspect(
    cls_or_instance: Union[ImmutableList[Any], Type[ImmutableList[Any]]],
) -> ListSpec: ...


@overload
def inspect(
    cls_or_instance: Union[ImmutableSet[Any], Type[ImmutableSet[Any]]],
) -> SetSpec: ...


@overload
def inspect(
    cls_or_instance: Union[ImmutableMap[Any, Any], Type[ImmutableMap[Any, Any]]],
) -> MapSpec: ...


@overload
def inspect(
    cls_or_instance: Union[
        Struct, Type[Struct], Union_, Type[Union_], GeneratedError, Type[GeneratedError]
    ],
) -> StructSpec: ...


def inspect(
    cls_or_instance: Any,
) -> StructSpec | ListSpec | SetSpec | MapSpec:
    if isinstance(cls_or_instance, (List, Set, Map)):
        return cls_or_instance.__get_reflection__()  # type: ignore[attr-defined]
    klass = (
        cls_or_instance if isinstance(cls_or_instance, type) else type(cls_or_instance)
    )
    if hasattr(klass, "__get_reflection__"):
        return _inspect_impl(klass)
    raise TypeError(
        f"No reflection information found: '{klass.__module__}.{klass.__name__}'"
    )


def inspectable(cls_or_instance: Any) -> bool:
    if not isinstance(cls_or_instance, type):
        if isinstance(cls_or_instance, (List, Set, Map)):
            return True
        cls_or_instance = type(cls_or_instance)
    return hasattr(cls_or_instance, "__get_reflection__")
