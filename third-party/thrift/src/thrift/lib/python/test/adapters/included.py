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

"""
Adapter for testing that adapter imports from included thrift files are generated correctly.
This adapter is ONLY used via dependency.thrift -> adapter.thrift to test import generation.
"""

from typing import Generic, TypeVar

T = TypeVar("T")


class IncludedAdapterConfig(Generic[T]):
    """A wrapper type used as typeHint for the IncludedAdapter annotation."""

    def __init__(self, value: T) -> None:
        self.value = value

    def __repr__(self) -> str:
        return f"IncludedAdapterConfig({self.value!r})"


class IncludedAdapterImpl:
    """Adapter implementation for IncludedAdapter annotation."""

    @staticmethod
    def from_thrift(value: T, **kwargs) -> IncludedAdapterConfig[T]:
        return IncludedAdapterConfig(value)

    @staticmethod
    def to_thrift(adapted: IncludedAdapterConfig[T], **kwargs) -> T:
        return adapted.value
