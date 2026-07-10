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

import threading
from functools import wraps
from typing import Any, Callable, overload, Type, TypeVar, Union

from thrift.python.client.client_wrapper import Client
from thrift.python.exceptions import GeneratedError
from thrift.python.reflection.services_reflection import ServiceSpec
from thrift.python.reflection.types_reflection import (
    ListSpec,
    MapSpec,
    SetSpec,
    StructSpec,
)
from thrift.python.types import (
    ImmutableList,
    ImmutableMap,
    ImmutableSet,
    List,
    Map,
    ServiceInterface,
    Set,
    Struct,
    Union as Union_,
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
def _inspect_impl(cls: type[Any]) -> Any:
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


@overload
def inspect(
    cls_or_instance: Union[ServiceInterface, Type[ServiceInterface]],
) -> ServiceSpec | None: ...


@overload
def inspect(
    cls_or_instance: Union[Client[Any, Any], Type[Client[Any, Any]]],
) -> ServiceSpec | None: ...


def inspect(cls_or_instance: Any) -> Any:
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
