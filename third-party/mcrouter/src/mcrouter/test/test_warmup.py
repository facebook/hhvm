#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import time

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestWarmup(McrouterTestCase):
    config = './mcrouter/test/test_warmup.json'
    extra_args = []

    def setUp(self):
        self.mc1 = self.add_server(Memcached())
        self.mc2 = self.add_server(Memcached())
        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_sanity(self):
        key = 'foo'
        value = 'value'
        self.mc1.set(key, value)
        self.assertEqual(self.mcrouter.get(key), value)
        # warmup request is async
        time.sleep(1)
        self.assertEqual(self.mc2.get(key), value)

    def test_expiration(self):
        key = 'foo2'
        value = 'value'
        self.mc1.set(key, value)
        self.assertEqual(self.mcrouter.get(key), value)
        min_expected_exptime = int(time.time() + 20000)
        # warmup request is async
        time.sleep(1)
        self.assertEqual(self.mc2.get(key), value)
        mc2_foo2_exptime = int(self.mc2.metaget(key)['exptime'])
        # Prevent random failures due to delays when setting the key on mc2
        self.assertGreaterEqual(mc2_foo2_exptime, min_expected_exptime - 4)
        self.assertLessEqual(mc2_foo2_exptime, min_expected_exptime + 4)
