#!/usr/bin/env python3
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


from __future__ import annotations

import asyncio
import time
import unittest
from pathlib import Path
from typing import Optional, Sequence

from cpython.ref cimport PyObject
from folly.iobuf cimport IOBuf, cIOBuf, from_unique_ptr
from folly.executor cimport get_executor
from folly.futures cimport bridgeSemiFutureWith
from libcpp.memory cimport unique_ptr
from libcpp.utility cimport move as cmove
from thrift.python.server cimport ThriftServer

cdef void serialized_metadata_callback(cFollyTry[unique_ptr[cIOBuf]]&& result, PyObject* userdata) noexcept:
    pyfuture = <object> userdata
    pyfuture.set_result(from_unique_ptr(cmove(result.value())))


def get_serialized_cpp_metadata(ThriftServer server):
    loop = asyncio.get_event_loop()
    future = loop.create_future()
    bridgeSemiFutureWith[unique_ptr[cIOBuf]](
        get_executor(),
        get_serialized_metadata(server.factory._cpp_obj),
        serialized_metadata_callback,
        <PyObject *>future,
    )

    return future
