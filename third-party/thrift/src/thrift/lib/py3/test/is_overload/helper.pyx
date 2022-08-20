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

from thrift.py3.server cimport ThriftServer, cIsOverloadedFunc
from thrift.py3.test.is_overload.func cimport isOverloaded, checkOverload, map_str_str

cdef class OverloadTestHelper:
    cdef ThriftServer server

    def __cinit__(self, handler, port):
        self.server = ThriftServer(handler, port=port)

    def set_is_overload(self):
        self.server.set_is_overloaded(<cIsOverloadedFunc>isOverloaded)

    def check_overload(self, str method):
        return checkOverload(self.server.server, method.encode("utf8"))
