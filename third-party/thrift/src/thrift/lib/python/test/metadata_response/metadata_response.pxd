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
from folly cimport cFollyTry, cFollySemiFuture
from folly.iobuf cimport cIOBuf
from thrift.py3.server cimport cAsyncProcessorFactory

cdef extern from "thrift/lib/python/test/metadata_response/metadata_response.h" namespace "::thrift::python::test":
    cdef cFollySemiFuture[unique_ptr[cIOBuf]] get_serialized_metadata(shared_ptr [cAsyncProcessorFactory] factory)
