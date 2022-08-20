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

import enum
from typing import List, NamedTuple, Sequence, Tuple

class CompileRetcode(enum.Enum):
    success: CompileRetcode = ...
    failure: CompileRetcode = ...

class DiagnosticLevel(enum.Enum):
    error: DiagnosticLevel = ...
    warning: DiagnosticLevel = ...
    info: DiagnosticLevel = ...
    debug: DiagnosticLevel = ...

class DiagnosticMessage(NamedTuple):
    level: DiagnosticLevel
    filename: str
    lineno: int
    message: str

def thrift_compile(
    args: Sequence[str],
) -> Tuple[CompileRetcode, List[DiagnosticMessage]]: ...
