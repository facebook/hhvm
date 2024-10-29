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

import typing

def define_int_flag(name: str, default_value: int) -> None: ...
def get_int_flag(name: str) -> int: ...
def mock_int_flag(name: str, value: int) -> typing.ContextManager[None]: ...
def define_bool_flag(name: str, default_value: bool) -> None: ...
def get_bool_flag(name: str) -> bool: ...
def mock_bool_flag(name: str, value: bool) -> typing.ContextManager[None]: ...
