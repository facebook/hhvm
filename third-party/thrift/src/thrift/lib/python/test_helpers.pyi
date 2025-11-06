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

from typing import TypeVar

T = TypeVar("T")

# test assertion for better comparisons between thrift concepts
# including almost equal for float and double fields
# See docblock for full details.
def assert_thrift_almost_equal(
    unittest: object,  # typically class with unittest.TestCase base
    result: T,
    expected: T,
    field_context: str | None = None,
    **almost_equal_kwargs: object,
) -> None: ...
def round_thrift_to_float32(val: T, convert_int: bool = False) -> T: ...

# helper function for rolling out float32. Prefer using round_thrift_to_float32
# This function has no effect until 32-bit float is the default for `float` fields
def round_thrift_float32_if_rollout(val: T, convert_int: bool = False) -> T: ...
