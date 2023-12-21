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

import unittest
from unittest import mock

from thrift.async_common import AsyncioRpcConnectionContext


class TestAsyncioRpcConnectionContext(unittest.TestCase):
    def test_getSockName(self):
        test_sock_laddr = ("198.51.100.29", 2929, 0, 0)  # TEST-NET-2
        client_socket = mock.Mock()
        client_socket.getsockname.return_value = test_sock_laddr
        context = AsyncioRpcConnectionContext(client_socket)

        ret = context.getSockName()

        self.assertEqual(ret, test_sock_laddr)
