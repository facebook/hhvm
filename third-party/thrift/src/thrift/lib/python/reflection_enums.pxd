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


cpdef enum NumberType:
    NOT_A_NUMBER = 0
    BYTE = 1
    I08 = 1
    I16 = 2
    I32 = 3
    I64 = 4
    FLOAT = 5
    DOUBLE = 6


cpdef enum Qualifier:
    UNQUALIFIED = 1
    REQUIRED = 2
    OPTIONAL = 3


cpdef enum StructType:
    STRUCT = 1
    UNION = 2
    EXCEPTION = 3
