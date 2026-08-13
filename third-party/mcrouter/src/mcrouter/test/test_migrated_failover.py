#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

import time

from carbon.carbon_result.thrift_types import Result
from mcrouter.test.MCProcess import MockMemcached
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestMigratedFailover(McrouterTestCase):
    config = "./mcrouter/test/test_migrated_failover.json"
    extra_args = ["--probe-timeout-initial=100", "--probe-timeout-max=100"]

    def get_mcrouter(self):
        return self.add_mcrouter(
            self.config, extra_args=self.extra_args, enable_thrift=True
        )

    def test_migrated_failover(self):
        self.add_server(MockMemcached())  # "old" pool, ignored
        mc_a = self.add_server(MockMemcached())
        self.add_server(MockMemcached())  # "old" pool, ignored
        mc_b = self.add_server(MockMemcached())

        mc_a.set("key", "a")
        mc_b.set("key", "b")

        client = self.get_mcrouter().get_thrift_client()

        reply = client.mcGet(b"key")
        self.assertEqual(Result.FOUND, reply.result)
        self.assertEqual(b"a", bytes(reply.value))

        mc_a.terminate()

        reply = None
        for _ in range(20):
            try:
                reply = client.mcGet(b"key")
            except Exception:
                reply = None
            if reply is not None and reply.result == Result.FOUND:
                break
            time.sleep(0.5)

        self.assertIsNotNone(reply)
        self.assertEqual(Result.FOUND, reply.result)
        self.assertEqual(b"b", bytes(reply.value))
