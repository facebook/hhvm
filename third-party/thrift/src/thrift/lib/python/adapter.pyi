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

import typing

TAdaptFrom = typing.TypeVar("TAdaptFrom")
TAdaptTo = typing.TypeVar("TAdaptTo")

class Adapter(typing.Generic[TAdaptFrom, TAdaptTo]):
    """
    Base class of Python adapter.

    For Type Adapter, override (from|to)_thrift.
    """

    @classmethod
    def from_thrift(
        cls,
        original: TAdaptFrom,
        *,
        transitive_annotation: "typing.Optional[thrift.python.types.Struct]" = None,
        constant_uri: typing.Optional[str] = None,
    ) -> TAdaptTo: ...
    @classmethod
    def to_thrift(
        cls,
        adapted: TAdaptTo,
        *,
        transitive_annotation: "typing.Optional[thrift.python.types.Struct]" = None,
        constant_uri: typing.Optional[str] = None,
    ) -> TAdaptFrom: ...

    """
    For Field Adapter, override (from|to)_thrift_field.
    """

    @classmethod
    def from_thrift_field(
        cls,
        original: TAdaptFrom,
        field_id: int,
        strct: "thrift.python.types.StructOrUnion",
        *,
        transitive_annotation: "typing.Optional[thrift.python.types.Struct]" = None,
    ) -> TAdaptTo: ...
    @classmethod
    def to_thrift_field(
        cls,
        adapted: TAdaptTo,
        field_id: int,
        strct: "thrift.python.types.StructOrUnion",
        *,
        transitive_annotation: "typing.Optional[thrift.python.types.Struct]" = None,
    ) -> TAdaptFrom: ...
