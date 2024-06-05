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

from libcpp.memory cimport unique_ptr, shared_ptr
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.unordered_map cimport unordered_map
from thrift.python.client.omni_client cimport cOmniClient, cData, FunctionQualifier, InteractionMethodPosition
from thrift.python.client.request_channel cimport cTProcessorEventHandler


cdef class SyncClient:
    cdef unique_ptr[cOmniClient] _omni_client
    cdef unordered_map[string, string] _persistent_headers
    cdef vector[shared_ptr[cTProcessorEventHandler]] _deferred_event_handlers
    cdef add_event_handler(self, const shared_ptr[cTProcessorEventHandler]& handler)
