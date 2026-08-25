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


# Deterministic regression test for the send-path use-after-free in OmniClient.
#
# A thrift-python sync client binds its channel to folly's global IO executor
# EventBase. In prod that EventBase is torn down at interpreter finalization
# (Py_FinalizeEx runs folly's SingletonVault::destroyInstances()) while a
# worker thread is still inside sync_send -- so RequestChannel::sendRequestAsync
# hops onto a freed EventBase via EventBase::runInEventBaseThread -> SIGSEGV.
#
# Relying on the natural shutdown race is flaky (finalization usually frees the
# EventBase outside the send window). Instead we trigger the exact same folly
# teardown at runtime via destroy_all_singletons(), then issue one send -- the
# Python analog of the C++ GlobalIOExecutorDestroyedBeforeClient test. Before
# the fix this crashed with a SIGSEGV; after the fix the send fails fast with a
# clean TransportError.

from __future__ import annotations

import unittest

from thrift.lib.python.client.test.singleton_teardown import destroy_all_singletons
from thrift.lib.python.client.test.test_server import server_in_another_process
from thrift.python.client import get_sync_client
from thrift.python.exceptions import TransportError
from thrift.python.test.thrift_clients import TestService


class SyncClientShutdownTest(unittest.TestCase):
    def test_send_after_eventbase_destroyed(self) -> None:
        with server_in_another_process() as path:
            with get_sync_client(TestService, path=path) as client:
                self.assertEqual(3, client.add(1, 2))  # warm up the connection

                # Free folly's global IO executor + its EventBases out from
                # under the live client's channel -- the Py_FinalizeEx teardown,
                # but at runtime.
                destroy_all_singletons()

                # The channel's EventBase is now freed. Before the fix this
                # hopped onto it via EventBase::runInEventBaseThread ->
                # use-after-free -> SIGSEGV. After the fix the send fails fast
                # with a clean TransportError.
                with self.assertRaises(TransportError):
                    client.add(1, 2)
