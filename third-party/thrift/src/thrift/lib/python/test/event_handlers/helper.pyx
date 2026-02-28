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
from enum import Enum

from thrift.python.client.request_channel cimport cTProcessorEventHandler, cTProcessorBase
from thrift.lib.python.test.event_handlers.handler cimport cThrowPreReadClientEventHandler, cThrowOnReadClientEventHandler, cThrowPostReadClientEventHandler

class ThrowHelperHandler(Enum):
    PRE_READ = "pre_read"
    ON_READ = "on_read"
    POST_READ = "post_read"


cdef class ThrowHelper:
    cdef shared_ptr[cThrowPreReadClientEventHandler] pre_read_handler
    cdef shared_ptr[cThrowOnReadClientEventHandler] on_read_handler
    cdef shared_ptr[cThrowPostReadClientEventHandler] post_read_handler
    cdef object handler_choice

    def __init__(self, handler: ThrowHelperHandler):
        self.handler_choice = handler

    def __enter__(self):
        cTProcessorBase.addProcessorEventHandler_deprecated(self.get_handler())

    def __exit__(self, exc_type, exc_val, exc_tb):
        cTProcessorBase.removeProcessorEventHandler(self.get_handler())

    def __cinit__(self):
        self.pre_read_handler = make_shared[cThrowPreReadClientEventHandler]()
        self.on_read_handler = make_shared[cThrowOnReadClientEventHandler]()
        self.post_read_handler = make_shared[cThrowPostReadClientEventHandler]()

    cdef shared_ptr[cTProcessorEventHandler] get_handler(self):
        if self.handler_choice == ThrowHelperHandler.PRE_READ:
            return static_pointer_cast[cTProcessorEventHandler, cThrowPreReadClientEventHandler](self.pre_read_handler)
        elif self.handler_choice == ThrowHelperHandler.ON_READ:
            return static_pointer_cast[cTProcessorEventHandler, cThrowOnReadClientEventHandler](self.on_read_handler)
        elif self.handler_choice == ThrowHelperHandler.POST_READ:
            return static_pointer_cast[cTProcessorEventHandler, cThrowPostReadClientEventHandler](self.post_read_handler)
        else:
            raise Exception("Unknown choice for handler")
