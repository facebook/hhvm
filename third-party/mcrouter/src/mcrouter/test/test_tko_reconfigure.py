#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import tempfile
import shutil
import time

from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestTkoReconfigure(McrouterTestCase):
    config1 = './mcrouter/test/test_tko_reconfigure1.json'
    config2 = './mcrouter/test/test_tko_reconfigure2.json'
    extra_args = ['--timeouts-until-tko', '1', '-v', '3']

    def tearDown(self):
        super().tearDown()

    def setUp(self):
        self.config = tempfile.mktemp()
        shutil.copyfile(self.config1, self.config)
        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_tko_reconfigure(self):
        # server a is in 'fast' state
        self.assertIsNone(self.mcrouter.get('hit'))
        self.assertEqual(self.mcrouter.stats('suspect_servers'), {
            '127.0.0.1:12345': 'status:tko num_failures:2'
        })

        self.mcrouter.change_config(self.config2)
        # wait for mcrouter to reconfigure
        time.sleep(4)
        # no servers should be marked as TKO
        self.assertEqual(self.mcrouter.stats('suspect_servers'), {})
        # one was removed from config
        self.assertTrue(self.mcrouter.check_in_log(
            '127.0.0.1:12345 was TKO, removed from config'))
