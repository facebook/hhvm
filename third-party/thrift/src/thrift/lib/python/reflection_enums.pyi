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

class NumberType(Enum):
    # pyrefly: ignore [invalid-annotation]
    NOT_A_NUMBER: NumberType = ...
    # pyrefly: ignore [invalid-annotation]
    BYTE: NumberType = ...
    # pyrefly: ignore [invalid-annotation]
    I08: NumberType = ...
    # pyrefly: ignore [invalid-annotation]
    I16: NumberType = ...
    # pyrefly: ignore [invalid-annotation]
    I32: NumberType = ...
    # pyrefly: ignore [invalid-annotation]
    I64: NumberType = ...
    # pyrefly: ignore [invalid-annotation]
    FLOAT: NumberType = ...
    # pyrefly: ignore [invalid-annotation]
    DOUBLE: NumberType = ...

class Qualifier(Enum):
    # pyrefly: ignore [invalid-annotation]
    UNQUALIFIED: Qualifier = ...
    # pyrefly: ignore [invalid-annotation]
    REQUIRED: Qualifier = ...
    # pyrefly: ignore [invalid-annotation]
    OPTIONAL: Qualifier = ...

class StructType(Enum):
    # pyrefly: ignore [invalid-annotation]
    STRUCT: StructType = ...
    # pyrefly: ignore [invalid-annotation]
    UNION: StructType = ...
    # pyrefly: ignore [invalid-annotation]
    EXCEPTION: StructType = ...

class FunctionQualifier(Enum):
    # pyrefly: ignore [invalid-annotation]
    UNSPECIFIED: FunctionQualifier = ...
    # pyrefly: ignore [invalid-annotation]
    ONE_WAY: FunctionQualifier = ...
    # pyrefly: ignore [invalid-annotation]
    IDEMPOTENT: FunctionQualifier = ...
    # pyrefly: ignore [invalid-annotation]
    READ_ONLY: FunctionQualifier = ...
