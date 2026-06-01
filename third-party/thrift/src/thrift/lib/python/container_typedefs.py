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


class _ListTypedefMeta(type):
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


class _SetTypedefMeta(type):
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


class _MapTypedefMeta(type):
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
            self._fbthrift_map_key_type_info, self._fbthrift_map_val_type_info, values
        )


# fallback type aliases when updated Cython module not available
ImmutableList = List
ImmutableSet = Set
ImmutableMap = Map
