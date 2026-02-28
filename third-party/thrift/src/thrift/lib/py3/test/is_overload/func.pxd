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

from libcpp.string cimport string
from libcpp.map cimport map
from libcpp cimport bool as cbool
from libcpp.memory cimport shared_ptr
from thrift.py3.server cimport cThriftServer

ctypedef map[string, string] map_str_str

cdef extern from "thrift/lib/py3/test/is_overload/func.h" namespace "thrift::py3::test":
    cbool checkOverload(shared_ptr[cThriftServer] server, string method)
    cbool isOverloaded(const map_str_str* headers, const string* method)
