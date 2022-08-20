#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Mcrouter
from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.mock_servers import SleepServer

import time


class TestRendezvousFailoverNoFailure(McrouterTestCase):
    config = "./mcrouter/test/test_rendezvous_failover.json"
    null_route_config = "./mcrouter/test/test_nullroute.json"
    mcrouter_server_extra_args = []
    extra_args = ["--timeouts-until-tko=1", "--num-proxies=4"]

    def setUp(self):
        self.mc = []
        for _i in range(5):
            self.mc.append(
                Mcrouter(
                    self.null_route_config, extra_args=self.mcrouter_server_extra_args
                )
            )
            self.add_server(self.mc[_i])

        self.mcrouter = self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_rendezvous_failover(self):
        for i in range(0, 0):
            key = "key_{}".format(i)
            self.mcrouter.get(key)
        time.sleep(5)
        stats = self.mcrouter.stats("all")
        self.assertEqual(int(stats["result_error_count"]), 0)


class TestRendezvousFailoverAllSleepServers(McrouterTestCase):
    config = "./mcrouter/test/test_rendezvous_failover.json"
    null_route_config = "./mcrouter/test/test_nullroute.json"
    mcrouter_server_extra_args = []
    extra_args = [
        "--timeouts-until-tko=1",
        "--disable-miss-on-get-errors",
        "--num-proxies=1",
    ]

    def setUp(self):
        self.mc = []
        # configure SleepServer for all servers
        for _i in range(5):
            self.mc.append(SleepServer())
            self.add_server(self.mc[_i])

        self.mcrouter = self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_rendezvous_failover(self):
        for i in range(0, 10):
            key = "key_{}_abc_{}".format(i, 17 * i)
            self.mcrouter.get(key)
            time.sleep(1)
            stats = self.mcrouter.stats("all")
            # The following stats include both the normal route and failover
            # route responses when all servers are not present (ie expected to
            # timeout and be declared TKO after the first failure)
            # The progression of result errors and tko errors show how well the
            # hash function is working
            expected_values = [
                (4, 0),
                (4, 4),
                (4, 8),
                (4, 12),
                (4, 16),
                (4, 20),
                (4, 24),
                (4, 28),
                (4, 32),
                (4, 36),
            ]

            self.assertEqual(
                int(stats["failover_policy_result_error"]), expected_values[i][0]
            )
            self.assertEqual(
                int(stats["failover_policy_tko_error"]), expected_values[i][1]
            )
