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

from convertible.types import Nested, Simple, Union
from testing.types import NonCopyable

def echo_simple(strucc: Simple) -> Simple: ...
def echo_nested(strucc: Nested) -> Nested: ...
def echo_union(strucc: Union) -> Union: ...
def echo_noncopyable(strucc: NonCopyable) -> NonCopyable: ...
def echo_simple_corrupted(strucc: Simple, bad: bytes) -> Simple: ...
def try_mutate_simple(strucc: Simple) -> str: ...
