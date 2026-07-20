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

"""
Pure Python module for container typedef base classes.

These are separated from thrift.python.types (a Cython extension module) so
that they can be interned by torch.package, avoiding forward-compatibility
issues when the packaged code is loaded on a host with an older thrift runtime.
"""

from __future__ import annotations

from typing import Any, cast, Protocol

from thrift.python.types import List, Map, Set


class _HasListTypeInfo(Protocol):
    _fbthrift_list_type_info: Any


class _ContainerTypedefMeta(type):
    def __new__(
        mcs, name: str, bases: tuple[type[Any], ...], namespace: dict[str, Any]
    ) -> _ContainerTypedefMeta:
        cls = super().__new__(mcs, name, bases, namespace)
        if "__name__" in namespace:
            cls.__name__ = namespace["__name__"]
        return cls


class _ListTypedefMeta(_ContainerTypedefMeta):
    def __instancecheck__(cls, instance: object) -> bool:
        if type.__instancecheck__(cls, instance):
            return True
        if type.__instancecheck__(List, instance):
            # pyre-ignore[16]: _fbthrift_same_type is a Cython method not visible to Pyre
            return instance._fbthrift_same_type(
                cast(_HasListTypeInfo, cls)._fbthrift_list_type_info
            )
        return False

    # pyre-ignore[24]: `type` is generic due to __future__.annotations
    def __subclasscheck__(cls, subclass: type) -> bool:
        if type.__subclasscheck__(cls, subclass):
            return True
        if type.__subclasscheck__(_ListTypedefBase, subclass):
            cls_info = cast(_HasListTypeInfo, cls)._fbthrift_list_type_info
            return cast(_HasListTypeInfo, subclass)._fbthrift_list_type_info.same_as(
                cls_info
            )
        return False


class _ListTypedefBase(List, metaclass=_ListTypedefMeta):
    _fbthrift_list_type_info: Any = None
    __slots__ = ()

    def __init__(self, values: object = None) -> None:
        if values is None:
            values = ()
        # pyre-ignore[19]: Cython __init__ not visible to Pyre
        super().__init__(self._fbthrift_list_type_info, values)

    @classmethod
    def __get_reflection__(cls) -> Any:
        from thrift.python.reflection.types_reflection import ListSpec

        py_type, thrift_type = _type_and_thrift_type_from_typeinfo(
            cls._fbthrift_list_type_info
        )
        return ListSpec(value=py_type, thrift_type=thrift_type)


class _SetTypedefMeta(_ContainerTypedefMeta):
    def __instancecheck__(cls, instance: object) -> bool:
        if type.__instancecheck__(cls, instance):
            return True
        if type.__instancecheck__(Set, instance):
            # pyre-ignore[16]: Attribute accessed on subclass instances at runtime
            return instance._fbthrift_same_type(cls._fbthrift_set_type_info)
        return False

    # pyre-ignore[24]: `type` is generic due to __future__.annotations
    def __subclasscheck__(cls, subclass: type) -> bool:
        if type.__subclasscheck__(cls, subclass):
            return True
        if type.__subclasscheck__(_SetTypedefBase, subclass):
            # pyre-ignore[16]: Attributes accessed on subclass at runtime
            same = subclass._fbthrift_set_type_info.same_as(cls._fbthrift_set_type_info)
            return same
        return False


class _SetTypedefBase(Set, metaclass=_SetTypedefMeta):
    _fbthrift_set_type_info: Any = None
    __slots__ = ()

    def __init__(self, values: object = None) -> None:
        if values is None:
            values = ()
        # pyre-ignore[19]: Cython __init__ not visible to Pyre
        super().__init__(self._fbthrift_set_type_info, values)

    @classmethod
    def __get_reflection__(cls) -> Any:
        from thrift.python.reflection.types_reflection import SetSpec

        py_type, thrift_type = _type_and_thrift_type_from_typeinfo(
            cls._fbthrift_set_type_info
        )
        return SetSpec(value=py_type, thrift_type=thrift_type)


class _MapTypedefMeta(_ContainerTypedefMeta):
    def __instancecheck__(cls, instance: object) -> bool:
        if type.__instancecheck__(cls, instance):
            return True
        if type.__instancecheck__(Map, instance):
            # pyre-ignore[16]: Attributes accessed on subclass instances at runtime
            cls_key = cls._fbthrift_map_key_type_info
            # pyre-ignore[16]: Attributes accessed on subclass instances at runtime
            cls_val = cls._fbthrift_map_val_type_info
            # pyre-ignore[16]: Cython method not visible to Pyre
            return instance._fbthrift_same_type(cls_key, cls_val)
        return False

    # pyre-ignore[24]: `type` is generic due to __future__.annotations
    def __subclasscheck__(cls, subclass: type) -> bool:
        if type.__subclasscheck__(cls, subclass):
            return True
        if type.__subclasscheck__(_MapTypedefBase, subclass):
            # pyre-ignore[16]: Attributes accessed on subclass at runtime
            cls_key = cls._fbthrift_map_key_type_info
            # pyre-ignore[16]: Attributes accessed on subclass at runtime
            cls_val = cls._fbthrift_map_val_type_info
            key_match = subclass._fbthrift_map_key_type_info.same_as(cls_key)
            val_match = subclass._fbthrift_map_val_type_info.same_as(cls_val)
            return key_match and val_match
        return False


class _MapTypedefBase(Map, metaclass=_MapTypedefMeta):
    _fbthrift_map_key_type_info: Any = None
    _fbthrift_map_val_type_info: Any = None
    __slots__ = ()

    def __init__(self, values: object = None) -> None:
        if values is None:
            values = {}
        # pyre-ignore[19]: Cython __init__ not visible to Pyre
        super().__init__(
            # pyrefly: ignore [bad-argument-count]
            self._fbthrift_map_key_type_info,
            # pyrefly: ignore [bad-argument-count]
            self._fbthrift_map_val_type_info,
            values,
        )

    @classmethod
    def __get_reflection__(cls) -> Any:
        from thrift.python.reflection.types_reflection import MapSpec

        key_type, key_thrift_type = _type_and_thrift_type_from_typeinfo(
            cls._fbthrift_map_key_type_info
        )
        val_type, val_thrift_type = _type_and_thrift_type_from_typeinfo(
            cls._fbthrift_map_val_type_info
        )
        return MapSpec(
            key=key_type,
            key_thrift_type=key_thrift_type,
            value=val_type,
            value_thrift_type=val_thrift_type,
        )


def _type_and_thrift_type_from_typeinfo(
    typeinfo: Any,
) -> tuple[type[Any], Any]:
    from thrift.python.reflection.types_reflection import ThriftType
    from thrift.python.types import (
        EnumTypeInfo,
        ListTypeInfo,
        MapTypeInfo,
        SetTypeInfo,
        StructTypeInfo,
        typeinfo_binary,
        typeinfo_bool,
        typeinfo_byte,
        typeinfo_double,
        typeinfo_float,
        typeinfo_i16,
        typeinfo_i32,
        typeinfo_i64,
        typeinfo_string,
    )

    _PRIMITIVES: list[tuple[Any, type[Any], Any]] = [
        (typeinfo_bool, bool, ThriftType.BOOL),
        (typeinfo_byte, int, ThriftType.BYTE),
        (typeinfo_i16, int, ThriftType.I16),
        (typeinfo_i32, int, ThriftType.I32),
        (typeinfo_i64, int, ThriftType.I64),
        (typeinfo_float, float, ThriftType.FLOAT),
        (typeinfo_double, float, ThriftType.DOUBLE),
        (typeinfo_string, str, ThriftType.STRING),
        (typeinfo_binary, bytes, ThriftType.BINARY),
    ]
    for prim_info, py_type, tt in _PRIMITIVES:
        if typeinfo is prim_info:
            return (py_type, tt)
    if isinstance(typeinfo, StructTypeInfo):
        # pyre-ignore[16]: Cython attribute
        return (typeinfo._class, ThriftType.STRUCT)
    if isinstance(typeinfo, EnumTypeInfo):
        # pyre-ignore[16]: Cython attribute
        return (typeinfo._class, ThriftType.ENUM)
    if isinstance(typeinfo, ListTypeInfo):
        return (_make_anonymous_list_typedef(typeinfo), ThriftType.LIST)
    if isinstance(typeinfo, SetTypeInfo):
        return (_make_anonymous_set_typedef(typeinfo), ThriftType.SET)
    if isinstance(typeinfo, MapTypeInfo):
        return (_make_anonymous_map_typedef(typeinfo), ThriftType.MAP)
    raise TypeError(f"Unsupported typeinfo: {type(typeinfo).__name__}")


def _make_anonymous_list_typedef(typeinfo: Any) -> type[Any]:
    cls = type.__new__(
        _ListTypedefMeta,
        "_AnonymousListTypedef",
        (_ListTypedefBase,),
        {"__slots__": (), "_fbthrift_list_type_info": typeinfo.get_val_info()},
    )
    return cls


def _make_anonymous_set_typedef(typeinfo: Any) -> type[Any]:
    cls = type.__new__(
        _SetTypedefMeta,
        "_AnonymousSetTypedef",
        (_SetTypedefBase,),
        {"__slots__": (), "_fbthrift_set_type_info": typeinfo.get_val_info()},
    )
    return cls


def _make_anonymous_map_typedef(typeinfo: Any) -> type[Any]:
    cls = type.__new__(
        _MapTypedefMeta,
        "_AnonymousMapTypedef",
        (_MapTypedefBase,),
        {
            "__slots__": (),
            "_fbthrift_map_key_type_info": typeinfo.get_key_info(),
            "_fbthrift_map_val_type_info": typeinfo.get_val_info(),
        },
    )
    return cls


# fallback type aliases when updated Cython module not available
ImmutableList = List
ImmutableSet = Set
ImmutableMap = Map
