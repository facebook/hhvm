#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import unittest
from types import SimpleNamespace
from unittest.mock import Mock, patch

from carbon.carbon_result.thrift_types import Result
from mcrouter.test.MCProcess import MCProcess, McrouterGlobals


class MCProcessEnsureConnectedTest(unittest.TestCase):
    @patch.object(McrouterGlobals, "useThriftClient", return_value=False)
    def test_thrift_only_requires_enabled_thrift_client(self, use_thrift_client):
        with self.assertRaisesRegex(ValueError, "Thrift test client must be enabled"):
            MCProcess(
                cmd=None,
                addr=12345,
                thriftPort=12346,
                connectLegacySocket=False,
            )

        use_thrift_client.assert_called_once_with()

    def make_process(self, thrift_client=None):
        # Bypass __init__ to isolate ensure_connected: construct a bare MCProcess
        # and set only the attributes the readiness path reads. If __init__ later
        # adds attributes that path depends on, they must be added here too.
        process = MCProcess.__new__(MCProcess)
        process.connect = Mock()
        process.disconnect = Mock()
        process.is_alive = Mock(return_value=True)
        process.terminate = Mock()
        process.versionPing = False
        process.connectLegacySocket = True
        process.thrift_client = thrift_client
        process.max_retries = 4
        return process

    @patch("mcrouter.test.MCProcess.time.sleep")
    def test_thrift_readiness_retries_until_ok(self, sleep):
        thrift_client = Mock()
        thrift_client.mcVersion.side_effect = [
            ConnectionError("listener unavailable"),
            SimpleNamespace(result=Result.UNKNOWN),
            SimpleNamespace(result=Result.OK),
        ]
        process = self.make_process(thrift_client)

        process.ensure_connected()

        self.assertEqual(3, thrift_client.mcVersion.call_count)
        self.assertEqual(2, sleep.call_count)

    def test_thrift_only_readiness_skips_legacy_connection(self):
        thrift_client = Mock()
        thrift_client.mcVersion.return_value = SimpleNamespace(result=Result.OK)
        process = self.make_process(thrift_client)
        process.connectLegacySocket = False

        process.ensure_connected()

        process.connect.assert_not_called()
        thrift_client.mcVersion.assert_called_once_with()

    @patch("mcrouter.test.MCProcess.time.sleep")
    def test_child_exit_during_thrift_readiness_fails_promptly(self, sleep):
        error = ConnectionError("listener unavailable")
        thrift_client = Mock()
        thrift_client.mcVersion.side_effect = error
        process = self.make_process(thrift_client)
        process.is_alive.return_value = False

        with self.assertRaisesRegex(
            RuntimeError, "exited while waiting for Thrift readiness"
        ) as context:
            process.ensure_connected()

        self.assertIs(error, context.exception.__cause__)
        process.terminate.assert_called_once_with()
        sleep.assert_not_called()

    @patch("mcrouter.test.MCProcess.time.sleep")
    def test_thrift_readiness_raises_when_retries_exhausted(self, sleep):
        thrift_client = Mock()
        thrift_client.mcVersion.return_value = SimpleNamespace(result=Result.UNKNOWN)
        process = self.make_process(thrift_client)

        with self.assertRaisesRegex(RuntimeError, "did not respond to mcVersion"):
            process.ensure_connected()

        self.assertEqual(process.max_retries, thrift_client.mcVersion.call_count)
        self.assertEqual(process.max_retries - 1, sleep.call_count)

    def test_process_without_thrift_client_only_connects(self):
        process = self.make_process()

        process.ensure_connected()

        process.connect.assert_called_once_with()
        process.is_alive.assert_not_called()
