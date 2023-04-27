#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestSlowWarmUp(McrouterTestCase):
    config = './mcrouter/test/test_slow_warmup.json'

    def setUp(self):
        self.memcached = self.add_server(Memcached())
        self.mcrouter = self.add_mcrouter(self.config)

    def test_basic(self):
        hitKey = "abc"
        missKey = "ghi"
        self.assertTrue(self.memcached.set(hitKey, "123"))

        # Should be warm and return just hits.
        hits = misses = 0
        for _ in range(0, 10):
            if self.mcrouter.get(hitKey) is None:
                misses += 1
            else:
                hits += 1
        self.assertEqual(0, misses)
        self.assertEqual(10, hits)

        # Let's make this box cold
        for _ in range(0, 100):
            self.mcrouter.get(missKey)

        # Now we should have some misses as mcrouter will actively
        # return a miss to speed up failover.
        hits = misses = 0
        for _ in range(0, 30):
            if self.mcrouter.get(hitKey) is None:
                misses += 1
            else:
                hits += 1
        self.assertTrue(misses > 0)
        self.assertTrue(hits > 0)
        self.assertEqual(30, hits + misses)
