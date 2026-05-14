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

import enum
from functools import wraps
from types import MappingProxyType
from typing import Any, Callable, Mapping, Sequence

from thrift.python.types import Enum, Struct, Union


class ThriftType(enum.Enum):
    BOOL = enum.auto()
    BYTE = enum.auto()
    I16 = enum.auto()
    I32 = enum.auto()
    I64 = enum.auto()
    FLOAT = enum.auto()
    DOUBLE = enum.auto()
    STRING = enum.auto()
    BINARY = enum.auto()
    LIST = enum.auto()
    SET = enum.auto()
    MAP = enum.auto()
    STRUCT = enum.auto()
    UNION = enum.auto()
    ENUM = enum.auto()
    VOID = enum.auto()


class _ImmutableMeta(type):
    def __new__(
        mcs,
        name: str,
        bases: tuple[type[Any], ...],
        namespace: dict[str, Any],
    ) -> _ImmutableMeta:
        original_init: Callable[..., None] = namespace["__init__"]

        @wraps(original_init)
        def _frozen_init(self: Any, *args: Any, **kwargs: Any) -> None:
            original_init(self, *args, **kwargs)
            object.__setattr__(self, "_fbthrift__frozen", True)

        namespace["__init__"] = _frozen_init
        namespace["__slots__"] = (*namespace.get("__slots__", ()), "_fbthrift__frozen")

        cls = super().__new__(mcs, name, bases, namespace)
        return cls  # type: ignore[return-value]

    @staticmethod
    def _setattr(self: Any, name: str, value: Any) -> None:
        if getattr(self, "_fbthrift__frozen", False):
            raise AttributeError(
                f"cannot set '{name}' on immutable {type(self).__name__}"
            )
        object.__setattr__(self, name, value)

    @staticmethod
    def _delattr(self: Any, name: str) -> None:
        if getattr(self, "_fbthrift__frozen", False):
            raise AttributeError(
                f"cannot delete '{name}' on immutable {type(self).__name__}"
            )
        object.__delattr__(self, name)

    def __init__(
        cls, name: str, bases: tuple[type[Any], ...], namespace: dict[str, Any]
    ) -> None:
        super().__init__(name, bases, namespace)
        cls.__setattr__ = _ImmutableMeta._setattr  # type: ignore[assignment]
        cls.__delattr__ = _ImmutableMeta._delattr  # type: ignore[assignment]


class ConstantStructSpec(metaclass=_ImmutableMeta):
    __slots__ = ("struct_type", "fields")
    struct_type: type[Struct]
    fields: MappingProxyType[str, ConstantSpec]

    def __init__(
        self,
        *,
        struct_type: type[Struct],
        fields: dict[str, ConstantSpec],
    ) -> None:
        self.struct_type = struct_type
        self.fields = MappingProxyType(fields)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, ConstantStructSpec):
            return NotImplemented
        return self.struct_type == other.struct_type and self.fields == other.fields

    def __hash__(self) -> int:
        return hash(
            (
                self.struct_type,
                frozenset(self.fields.items()),
            )
        )

    def __repr__(self) -> str:
        return (
            f"ConstantStructSpec(struct_type={self.struct_type!r}, "
            f"fields={dict(self.fields)!r})"
        )


class ConstantUnionSpec(metaclass=_ImmutableMeta):
    __slots__ = ("union_type", "field", "value")
    union_type: type[Union]
    field: str | None
    value: ConstantSpec | None

    def __init__(
        self,
        *,
        union_type: type[Union],
        field: str | None = None,
        value: ConstantSpec | None = None,
    ) -> None:
        if (field is None) != (value is None):
            raise ValueError("field and value must both be None or both be non-None")
        self.union_type = union_type
        self.field = field
        self.value = value

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, ConstantUnionSpec):
            return NotImplemented
        return (
            self.union_type == other.union_type
            and self.field == other.field
            and self.value == other.value
        )

    def __hash__(self) -> int:
        return hash((self.union_type, self.field, self.value))

    def __repr__(self) -> str:
        return (
            f"ConstantUnionSpec(union_type={self.union_type!r}, "
            f"field={self.field!r}, value={self.value!r})"
        )


class ConstantEnumSpec(metaclass=_ImmutableMeta):
    __slots__ = ("enum_type", "value")
    enum_type: type[Enum]
    value: Enum

    def __init__(
        self,
        *,
        enum_type: type[Enum],
        value: Enum,
    ) -> None:
        self.enum_type = enum_type
        self.value = value

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, ConstantEnumSpec):
            return NotImplemented
        return self.enum_type == other.enum_type and self.value == other.value

    def __hash__(self) -> int:
        return hash((self.enum_type, self.value))

    def __repr__(self) -> str:
        return f"ConstantEnumSpec(enum_type={self.enum_type!r}, value={self.value!r})"


class ConstantListSpec(metaclass=_ImmutableMeta):
    __slots__ = ("value",)
    value: tuple[ConstantSpec, ...]

    def __init__(self, *, value: Sequence[ConstantSpec]) -> None:
        self.value = tuple(value)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, ConstantListSpec):
            return NotImplemented
        return self.value == other.value

    def __hash__(self) -> int:
        return hash(self.value)

    def __repr__(self) -> str:
        return f"ConstantListSpec(value={self.value!r})"


class ConstantSetSpec(metaclass=_ImmutableMeta):
    __slots__ = ("value",)
    value: frozenset[ConstantSpec]

    def __init__(self, *, value: Sequence[ConstantSpec]) -> None:
        self.value = frozenset(value)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, ConstantSetSpec):
            return NotImplemented
        return self.value == other.value

    def __hash__(self) -> int:
        return hash(self.value)

    def __repr__(self) -> str:
        return f"ConstantSetSpec(value={self.value!r})"


class ConstantMapSpec(metaclass=_ImmutableMeta):
    __slots__ = ("value",)
    value: MappingProxyType[ConstantSpec, ConstantSpec]

    def __init__(self, *, value: Mapping[ConstantSpec, ConstantSpec]) -> None:
        self.value = MappingProxyType(value)

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, ConstantMapSpec):
            return NotImplemented
        return self.value == other.value

    def __hash__(self) -> int:
        return hash(frozenset(self.value.items()))

    def __repr__(self) -> str:
        return f"ConstantMapSpec(value={dict(self.value)!r})"


ConstantValue = (
    bool
    | int
    | float
    | str
    | bytes
    | ConstantStructSpec
    | ConstantUnionSpec
    | ConstantEnumSpec
    | ConstantListSpec
    | ConstantSetSpec
    | ConstantMapSpec
)


class ConstantSpec(metaclass=_ImmutableMeta):
    __slots__ = ("value", "thrift_type")
    value: ConstantValue
    thrift_type: ThriftType

    def __init__(
        self,
        *,
        value: ConstantValue,
        thrift_type: ThriftType,
    ) -> None:
        self.value = value
        self.thrift_type = thrift_type

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, ConstantSpec):
            return NotImplemented
        return self.value == other.value and self.thrift_type == other.thrift_type

    def __hash__(self) -> int:
        return hash((self.value, self.thrift_type))

    def __repr__(self) -> str:
        return f"ConstantSpec(value={self.value!r}, thrift_type={self.thrift_type!r})"
