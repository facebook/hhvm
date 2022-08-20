#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import shutil
import time

from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.mock_servers import SleepServer

class TestProbeTimeout(McrouterTestCase):
    config = './mcrouter/test/test_probe_timeout.json'
    config_noservers = './mcrouter/test/test_probe_timeout_reconfig.json'
    extra_args = ['-t', '2000',
                  '--timeouts-until-tko', '1',
                  '-r', '100']

    def setUp(self):
        # The order here must corresponds to the order of hosts in the .json
        self.mc = self.add_server(SleepServer())

    def get_mcrouter(self):
        return self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_probe_timeout(self):
        mcrouter = self.get_mcrouter()

        # send first request and initiate tko
        resp = mcrouter.get("key")
        self.assertEqual(resp, None)

        # wait for the probe to be sent
        time.sleep(0.15)

        # initiate reconfigure
        shutil.copyfile(self.config_noservers, mcrouter.config)

        time.sleep(3)

        # check that mcrouter is still alive
        self.assertTrue(mcrouter.is_alive())

class TestRequestTimeout(TestProbeTimeout):
    extra_args = ['-t', '2000', '--timeouts-until-tko', '2', '-r', '100']
