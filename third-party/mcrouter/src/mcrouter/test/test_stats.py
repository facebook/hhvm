#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.mock_servers import CustomErrorServer
from mcrouter.test.mock_servers import SleepServer


class TestServerStatsRequests(McrouterTestCase):
    config = './mcrouter/test/test_server_stats_pending.json'
    extra_args = ['--server-timeout', '5000']

    def setUp(self):
        # The order here must corresponds to the order of hosts in the .json
        self.add_server(SleepServer())
        self.mcrouter = self.add_mcrouter(
            self.config, extra_args=self.extra_args
        )

    def test_server_stats(self):
        self.mcrouter.set('test:foo', 'bar')
        stats = self.mcrouter.stats('servers')
        num_stats = 0
        for _, stat_value in stats.items():
            num_stats += 1
            num_outstanding_reqs = 0
            value_parts = stat_value.split(' ')
            pending_reqs = value_parts[1].split(':')
            inflight_reqs = value_parts[2].split(':')
            num_outstanding_reqs += int(pending_reqs[1])
            num_outstanding_reqs += int(inflight_reqs[1])
            self.assertEqual('avg_latency_us:0.000', value_parts[0])
            self.assertEqual('pending_reqs', pending_reqs[0])
            self.assertEqual('inflight_reqs', inflight_reqs[0])
            self.assertEqual(1, num_outstanding_reqs)
        self.assertEqual(1, num_stats)


class TestErrorStatsRequests(McrouterTestCase):
    config = './mcrouter/test/mcrouter_test_basic_1_1_1.json'
    extra_args = []
    cmd = 'set test_key 0 0 3\r\nabc\r\n'

    def setUp(self):
        self.server = self.add_server(CustomErrorServer())
        self.mcrouter = self.add_mcrouter(
            self.config, extra_args=self.extra_args
        )

    def test_client_error(self):
        # send command
        self.server.setExpectedBytes(len(self.cmd))
        expected_reply = 'CLIENT_ERROR'
        self.server.setError(expected_reply)
        reply = self.mcrouter.issue_command(self.cmd)
        self.assertEqual(reply, expected_reply + '\r\n')

        # check stats
        stats = self.mcrouter.stats('all')
        self.assertEqual(1, int(stats['result_client_error_count']))
        self.assertEqual(1, int(stats['result_error_count']))

    def test_remote_error(self):
        # send command
        self.server.setExpectedBytes(len(self.cmd))
        expected_reply = 'SERVER_ERROR remote error'
        self.server.setError(expected_reply)
        reply = self.mcrouter.issue_command(self.cmd)
        self.assertEqual(reply, expected_reply + '\r\n')

        # check stats
        stats = self.mcrouter.stats('all')
        self.assertEqual(1, int(stats['result_remote_error_count']))
        self.assertEqual(1, int(stats['result_error_count']))
