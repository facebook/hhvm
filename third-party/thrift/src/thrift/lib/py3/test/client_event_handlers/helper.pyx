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

from libcpp.memory cimport make_shared, static_pointer_cast, shared_ptr

from thrift.py3.client cimport Client, cTProcessorEventHandler
from thrift.py3.client import get_client as _get_client
from thrift.py3.test.client_event_handler.handler cimport cTestClientEventHandler

cdef class TestHelper:
    cdef shared_ptr[cTestClientEventHandler] handler

    def __cinit__(self):
        self.handler = make_shared[cTestClientEventHandler]()

    def get_client(
        self,
        clientKlass,
        host='::1',
        int port=-1,
    ):
        cdef Client client = _get_client(clientKlass, host=host, port=port)
        client.add_event_handler(static_pointer_cast[cTProcessorEventHandler, cTestClientEventHandler](self.handler))
        return client

    def is_handler_called(self):
        return self.handler.get().preWriteCalled()
