#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


from mcrouter.test.MCProcess import Memcached, McrouterClients
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestLeasePairing(McrouterTestCase):
    config_pairing_basic = './mcrouter/test/test_lease_pairing_basic.json'
    config_pairing_disabled = './mcrouter/test/test_lease_pairing_disabled.json'
    config_pairing_nested = './mcrouter/test/test_lease_pairing_nested.json'

    def create_mcrouter(self, config, num_memcached=2):
        extra_args = ['--proxy-threads', '2']

        self.memcacheds = []
        for _ in range(num_memcached):
            self.memcacheds.append(self.add_server(Memcached()))

        self.mcrouter = self.add_mcrouter(config, extra_args=extra_args)
        self.clients = McrouterClients(self.mcrouter.port, 2)

    def test_lease_pairing_basic(self):
        # The lease-get and it's corresponding lease-set
        # should go to the same server.

        self.create_mcrouter(self.config_pairing_basic)

        # kill memcacheds[0]
        self.memcacheds[0].pause()

        # lease get - should go to memcache[1]
        get_reply = self.clients[0].leaseGet("key")
        self.assertTrue(get_reply is not None)

        # bring memcacheds[0] up
        self.memcacheds[0].resume()

        # lease set should go to the same server as lease get - memcache[1]
        set_reply = self.clients[1].leaseSet("key",
                {"value": "abc", "token": get_reply['token']})
        self.assertTrue(set_reply is not None)
        self.assertTrue(self.memcacheds[0].get("key") is None)
        self.assertTrue(self.memcacheds[1].get("key") is not None)

    def test_lease_pairing_nested_basic(self):
        # The lease-get and it's corresponding lease-set
        # should go to the same server.

        # Nested config has 4 memcached instances
        self.create_mcrouter(self.config_pairing_nested, 4)

        # kill memcacheds[0]
        self.memcacheds[0].pause()

        # lease get - should go to memcache[1]
        get_reply = self.clients[0].leaseGet("key")
        self.assertTrue(get_reply is not None)

        # bring memcacheds[0] up
        self.memcacheds[0].resume()

        # lease set should go to the same server as lease get - memcache[1]
        set_reply = self.clients[1].leaseSet("key",
                {"value": "abc", "token": get_reply['token']})
        self.assertTrue(set_reply is not None)
        self.assertTrue(self.memcacheds[0].get("key") is None)
        self.assertTrue(self.memcacheds[1].get("key") is not None)

    def test_lease_pairing_nested_two_failovers(self):
        # The lease-get and it's corresponding lease-set
        # should go to the same server.

        # Nested config has 4 memcached instances
        self.create_mcrouter(self.config_pairing_nested, 4)

        # Completely kill pool A (memcached 0 and 1)
        # and one box in pool B (memcached[2])
        self.memcacheds[0].pause()
        self.memcacheds[1].pause()
        self.memcacheds[2].pause()

        # lease get - should go to poolB memcache[2]
        get_reply = self.clients[0].leaseGet("key")
        self.assertTrue(get_reply is not None)

        # bring up all memcacheds
        self.memcacheds[0].resume()
        self.memcacheds[1].resume()
        self.memcacheds[2].resume()

        # lease set should go to the same server as lease get - memcache[3]
        set_reply = self.clients[1].leaseSet("key",
                {"value": "abc", "token": get_reply['token']})
        self.assertTrue(set_reply is not None)
        self.assertTrue(self.memcacheds[0].get("key") is None)
        self.assertTrue(self.memcacheds[1].get("key") is None)
        self.assertTrue(self.memcacheds[2].get("key") is None)
        self.assertTrue(self.memcacheds[3].get("key") is not None)
