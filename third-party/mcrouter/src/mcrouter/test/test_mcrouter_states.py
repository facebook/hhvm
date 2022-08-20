#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import time

from mcrouter.test.MCProcess import McrouterClient, Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestMcrouterStates(McrouterTestCase):
    config = './mcrouter/test/mcrouter_test_basic_1_1_1.json'
    config2 = './mcrouter/test/mcrouter_test_basic_2_1_1.json'

    # 2 proxy threads with initial probe delay time 0.1s
    extra_args = ['--num-proxies', '2', '-r', '100']

    def setUp(self):
        # The order here corresponds to the order of hosts in the .json
        self.mc = self.add_server(Memcached())

    def get_mcrouter(self, additional_args=()):
        extra_args = list(additional_args)
        extra_args.extend(self.extra_args)
        return self.add_mcrouter(self.config, extra_args=extra_args)

    def test_mcrouter_states(self):
        mcr = self.get_mcrouter()
        self.assertTrue(mcr.is_alive())

        # another client
        c2 = McrouterClient(mcr.port)
        c2.connect()

        def check_all_up():
            self.assertTrue(mcr.set('key', 'value'))
            self.assertTrue(c2.set('key', 'value'))
            self.assertEqual(mcr.get('key'), 'value')
            self.assertEqual(c2.get('key'), 'value')
            stat = mcr.stats()
            self.assertEqual(stat['num_servers'], '2')
            self.assertEqual(stat['num_servers_up'], '2')
            self.assertEqual(stat['num_servers_down'], '0')

        def check_invariant():
            stat = mcr.stats()
            self.assertEqual(int(stat['num_servers']),
                             int(stat['num_servers_new'])
                             + int(stat['num_servers_up'])
                             + int(stat['num_servers_down'])
                             + int(stat['num_servers_closed']))

        check_invariant()
        check_all_up()

        # down aka hard tko
        self.mc.terminate()
        self.assertEqual(mcr.get('key'), None)
        self.assertEqual(c2.get('key'), None)
        stat = mcr.stats()
        self.assertEqual(stat['num_servers_up'], '0')
        self.assertEqual(stat['num_servers_down'], '2')
        check_invariant()

        # change config
        mcr.change_config(self.config2)
        # wait for mcrouter to reconfigure
        time.sleep(2)
        check_invariant()

        # make sure we dont crash
        self.assertTrue(mcr.is_alive())
