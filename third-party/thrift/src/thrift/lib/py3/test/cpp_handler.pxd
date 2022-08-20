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

from libcpp.memory cimport shared_ptr, unique_ptr

from thrift.py3.server cimport cServerInterface, cAsyncProcessorFactory, cTransportRoutingHandler

cdef extern from "thrift/lib/py3/test/cpp_handler.h" namespace "::thrift::py3::test":
    cdef cppclass cTestingServiceInterfaceService "::thrift::py3::test::TestingService"(cServerInterface):
        @staticmethod
        shared_ptr[cAsyncProcessorFactory] createInstance()
