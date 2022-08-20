#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Mcrouter
from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.mock_servers import SleepServer

import time


class TestDeterministicFailoverNoFailure(McrouterTestCase):
    config = './mcrouter/test/test_deterministic_failover_big_retries.json'
    null_route_config = './mcrouter/test/test_nullroute.json'
    mcrouter_server_extra_args = []
    extra_args = [
        '--timeouts-until-tko=1',
        '--num-proxies=4']

    def setUp(self):
        self.mc = []
        # configure SleepServer for the east and wc pools
        for _i in range(5):
            self.mc.append(Mcrouter(self.null_route_config,
                           extra_args=self.mcrouter_server_extra_args))
            self.add_server(self.mc[_i])

        for _i in range(12):
            self.mc.append(Mcrouter(self.null_route_config,
                           extra_args=self.mcrouter_server_extra_args))
            self.add_server(self.mc[5 + _i])

        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_deterministic_failover(self):
        for i in range(0, 0):
            key = 'key_{}'.format(i)
            self.mcrouter.get(key)
        time.sleep(5)
        stats = self.mcrouter.stats('all')
        self.assertEqual(int(stats['result_error_count']), 0)


class TestDeterministicFailoverAllSleepServers(McrouterTestCase):
    config = './mcrouter/test/test_deterministic_failover.json'
    null_route_config = './mcrouter/test/test_nullroute.json'
    mcrouter_server_extra_args = []
    extra_args = [
        '--timeouts-until-tko=1',
        '--disable-miss-on-get-errors',
        '--num-proxies=1']

    def setUp(self):
        self.mc = []
        # configure SleepServer for all servers
        for _i in range(17):
            self.mc.append(SleepServer())
            self.add_server(self.mc[_i])

        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_deterministic_failover(self):
        for i in range(0, 10):
            key = 'key_{}_abc_{}'.format(i, 17 * i)
            self.mcrouter.get(key)
            time.sleep(1)
            stats = self.mcrouter.stats('all')
            # The following stats include both the normal route and failover
            # route responses when all servers are not present (ie expected to
            # timeout and be declared TKO after the first failure)
            # The progression of result errors and tko errors show how well the
            # hash function is working
            expected_values = [(3, 0), (6, 0), (9, 3), (12, 9), (13, 17),
                               (14, 25), (14, 34), (15, 42), (15, 51), (16, 59)]

            self.assertEqual(int(stats["failover_policy_result_error"]),
                    expected_values[i][0])
            self.assertEqual(int(stats["failover_policy_tko_error"]),
                    expected_values[i][1])


class TestDeterministicFailoverAllSleepServersSamePool(McrouterTestCase):
    config = './mcrouter/test/test_deterministic_failover2.json'
    null_route_config = './mcrouter/test/test_nullroute.json'
    mcrouter_server_extra_args = []
    extra_args = [
        '--timeouts-until-tko=1',
        '--disable-miss-on-get-errors',
        '--num-proxies=1']

    def setUp(self):
        self.mc = []
        # configure SleepServer for all servers
        for _i in range(23):
            self.mc.append(SleepServer())
            self.add_server(self.mc[_i])

        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_deterministic_failover(self):
        for i in range(0, 10):
            key = 'key_{}_hYgGEs_{}_kBVq9Z_{}'.format(13 * i, i, 71 * i, i)
            self.mcrouter.get(key)
            time.sleep(1)
            stats = self.mcrouter.stats('all')
            # The following stats include both the normal route and failover
            # route responses when all servers are not present (ie expected to
            # timeout and be declared TKO after the first failure)
            # The progression of result errors and tko errors show how well the
            expected_values = [(3, 0, 1), (6, 0, 3), (9, 2, 4), (12, 3, 5),
                               (15, 5, 6), (17, 14, 10), (19, 23, 12),
                               (21, 32, 21), (21, 43, 26), (23, 52, 30)]

            self.assertEqual(int(stats["failover_policy_result_error"]),
                    expected_values[i][0])
            self.assertEqual(int(stats["failover_policy_tko_error"]),
                    expected_values[i][1])
            self.assertEqual(int(stats["failover_num_collisions"]),
                    expected_values[i][2])
            if i < 5 :
                self.assertEqual(int(stats["failover_all_failed_count"]), 0)
            else:
                self.assertEqual(int(stats["failover_all_failed_count"]), i - 4)

class TestDeterministicFailoverAllSleepServersSharedConfig(McrouterTestCase):
    config = './mcrouter/test/test_deterministic_failover3.json'
    null_route_config = './mcrouter/test/test_nullroute.json'
    mcrouter_server_extra_args = []
    extra_args = [
        '--timeouts-until-tko=1',
        '--disable-miss-on-get-errors',
        '--route-prefix=/Route/A/',
        '--num-proxies=1']

    def setUp(self):
        self.mc = []
        # configure SleepServer for all servers
        for _i in range(23):
            self.mc.append(SleepServer())
            self.add_server(self.mc[_i])

        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_deterministic_failover(self):
        for i in range(0, 8):
            key = 'key_{}_hYgGEs_{}_kBVq9Z_{}'.format(13 * i, i, 71 * i, i)
            self.mcrouter.get(key)
            time.sleep(1)
            stats = self.mcrouter.stats('all')
            # The following stats include both the normal route and failover
            # route responses when all servers are not present (ie expected to
            # timeout and be declared TKO after the first failure)
            # The progression of result errors and tko errors show how well the
            expected_values = [(3, 0, 1), (6, 0, 3), (9, 2, 4), (12, 3, 5),
                               (15, 5, 6), (18, 9, 7), (20, 18, 9),
                               (20, 29, 18)]

            self.assertEqual(int(stats["failover_policy_result_error"]),
                    expected_values[i][0])
            self.assertEqual(int(stats["failover_policy_tko_error"]),
                    expected_values[i][1])
            self.assertEqual(int(stats["failover_num_collisions"]),
                    expected_values[i][2])
            if i < 6 :
                self.assertEqual(int(stats["failover_all_failed_count"]), 0)
            else:
                self.assertEqual(int(stats["failover_all_failed_count"]), i-5)


class TestDeterministicFailoverAllSleepServersFailureDomains(McrouterTestCase):
    config = './mcrouter/test/test_deterministic_failover4.json'
    null_route_config = './mcrouter/test/test_nullroute.json'
    mcrouter_server_extra_args = []
    extra_args = [
        '--timeouts-until-tko=1',
        '--disable-miss-on-get-errors',
        '--route-prefix=/Route/A/',
        '--num-proxies=1']

    def setUp(self):
        self.mc = []
        # configure SleepServer for all servers
        for _i in range(23):
            self.mc.append(SleepServer())
            self.add_server(self.mc[_i])

        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_deterministic_failover(self):
        for i in range(0, 8):
            key = 'key_{}_hYgGEs_{}_kBVq9Z_{}'.format(13 * i, i, 71 * i, i)
            self.mcrouter.get(key)
            time.sleep(1)
            stats = self.mcrouter.stats('all')
            # The following stats include both the normal route and failover
            # route responses when all servers are not present (ie expected to
            # timeout and be declared TKO after the first failure)
            # The progression of result errors and tko errors show how well the
            expected_values = [(3, 0, 1, 0), (6, 0, 3, 0), (9, 2, 3, 1),
                               (12, 3, 4, 1), (15, 5, 4, 2), (18, 13, 5, 14),
                               (20, 22, 5, 21), (21, 32, 5, 81)]

            self.assertEqual(int(stats["failover_policy_result_error"]),
                    expected_values[i][0])
            self.assertEqual(int(stats["failover_policy_tko_error"]),
                    expected_values[i][1])
            self.assertEqual(int(stats["failover_num_collisions"]),
                    expected_values[i][2])
            self.assertEqual(int(stats["failover_num_failed_domain_collisions"]),
                    expected_values[i][3])
            if i < 6 :
                self.assertEqual(int(stats["failover_all_failed_count"]), 0)
            else:
                self.assertEqual(int(stats["failover_all_failed_count"]), i-5)


class TestDeterministicFailoverSmallerFailoverPool(McrouterTestCase):
    config = './mcrouter/test/test_deterministic_failover5.json'
    null_route_config = './mcrouter/test/test_nullroute.json'
    mcrouter_server_extra_args = []
    extra_args = [
        '--timeouts-until-tko=1',
        '--disable-miss-on-get-errors',
        '--num-proxies=1']

    def setUp(self):
        self.mc = []
        # configure SleepServer for all servers
        for _i in range(11):
            self.mc.append(SleepServer())
            self.add_server(self.mc[_i])

        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_deterministic_failover(self):
        for i in range(0, 10):
            key = 'key_{}_abc_{}'.format(i, 11 * i)
            self.mcrouter.get(key)
            time.sleep(1)
            stats = self.mcrouter.stats('all')
            self.assertEqual(int(stats["failover_num_failed_domain_collisions"]),0)
