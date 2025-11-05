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

from typing import TypeVar

T = TypeVar("T")

def round_thrift_to_float32(val: T, convert_int: bool = False) -> T: ...

# helper function for rolling out float32. Prefer using round_thrift_to_float32
# This function has no effect until 32-bit float is the default for `float` fields
def round_thrift_float32_if_rollout(val: T, convert_int: bool = False) -> T: ...
