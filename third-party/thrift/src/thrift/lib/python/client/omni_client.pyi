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

from enum import Enum

class RpcKind(Enum):
    # pyrefly: ignore [invalid-annotation]
    SINGLE_REQUEST_SINGLE_RESPONSE: RpcKind = ...
    # pyrefly: ignore [invalid-annotation]
    SINGLE_REQUEST_NO_RESPONSE: RpcKind = ...
    # pyrefly: ignore [invalid-annotation]
    SINGLE_REQUEST_STREAMING_RESPONSE: RpcKind = ...
    # pyrefly: ignore [invalid-annotation]
    SINK: RpcKind = ...
    # pyrefly: ignore [invalid-annotation]
    BIDIRECTIONAL_STREAM: RpcKind = ...

class FunctionQualifier(Enum):
    # pyrefly: ignore [invalid-annotation]
    Unspecified: FunctionQualifier = ...
    # pyrefly: ignore [invalid-annotation]
    OneWay: FunctionQualifier = ...
    # pyrefly: ignore [invalid-annotation]
    Idempotent: FunctionQualifier = ...
    # pyrefly: ignore [invalid-annotation]
    ReadOnly: FunctionQualifier = ...

class InteractionMethodPosition(Enum):
    # None: InteractionMethodPosition = ...
    # pyrefly: ignore [invalid-annotation]
    Factory: InteractionMethodPosition = ...
    # pyrefly: ignore [invalid-annotation]
    Member: InteractionMethodPosition = ...
