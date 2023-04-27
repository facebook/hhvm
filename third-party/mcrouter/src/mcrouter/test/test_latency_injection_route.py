#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from datetime import datetime

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestLatencyInjectionRoute(McrouterTestCase):
    config_latency_before = './mcrouter/test/test_latency_injection_before.json'
    config_latency_after = './mcrouter/test/test_latency_injection_after.json'
    config_latency_total = './mcrouter/test/test_latency_injection_total.json'

    def setUp(self) -> None:
        self.mc = self.add_server(Memcached())

        self.mcrouter_latency_before =\
            self.add_mcrouter(self.config_latency_before)
        self.mcrouter_latency_after =\
            self.add_mcrouter(self.config_latency_after)
        self.mcrouter_latency_total =\
            self.add_mcrouter(self.config_latency_total)

    def test_latency_before(self) -> None:
        self.mc.set("key1", "value1")

        t_start = datetime.now()
        self.assertEqual("value1", self.mcrouter_latency_before.get("key1"))
        t_end = datetime.now()

        duration = t_end - t_start
        self.assertGreaterEqual(duration.total_seconds(), 2)

    def test_latency_after(self) -> None:
        self.mc.set("key2", "value2")

        t_start = datetime.now()
        self.assertTrue("value2", self.mcrouter_latency_after.get("key2"))
        t_end = datetime.now()

        duration = t_end - t_start
        self.assertGreaterEqual(duration.total_seconds(), 1)

    def test_latency_total(self) -> None:
        self.mc.set("key3", "value3")

        t_start = datetime.now()
        self.assertTrue("value3", self.mcrouter_latency_total.get("key3"))
        t_end = datetime.now()

        duration = t_end - t_start
        self.assertGreaterEqual(duration.total_seconds(), 1)
