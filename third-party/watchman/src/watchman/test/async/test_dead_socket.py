#!/usr/bin/env python3
# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import asyncio
import os
import unittest
from unittest import IsolatedAsyncioTestCase

import pywatchman_aio
import WatchmanInstance


# Note this does not extend AsyncWatchmanTestCase as it wants to start its
# own Watchman server instances per test.
class TestDeadSocket(IsolatedAsyncioTestCase):
    @unittest.skipIf(os.name == "nt", "not supported on windows")
    def test_query_dead_socket(self):
        async def test_core(wminst):
            with await pywatchman_aio.AIOClient.from_socket(
                sockname=wminst.getSockPath()
            ) as client:
                wminst.stop()
                with self.assertRaises(ConnectionResetError):
                    await client.query("version")

        self._async_runner(test_core)

    @unittest.skipIf(os.name == "nt", "not supported on windows")
    def test_subscription_dead_socket(self):
        async def test_core(wminst):
            with await pywatchman_aio.AIOClient.from_socket(
                sockname=wminst.getSockPath()
            ) as client:
                root = f"{wminst.base_dir}/work"
                os.makedirs(root)
                await client.query("watch", root)
                await client.query("subscribe", root, "sub", {"expression": ["exists"]})
                wminst.stop()
                with self.assertRaises(ConnectionResetError):
                    await client.get_subscription("sub", root)

        self._async_runner(test_core)

    def _async_runner(self, test_core):
        wminst = WatchmanInstance.Instance()
        wminst.start()
        try:
            return asyncio.new_event_loop().run_until_complete(test_core(wminst))
        finally:
            wminst.stop()
