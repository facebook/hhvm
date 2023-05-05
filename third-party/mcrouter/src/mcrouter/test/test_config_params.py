#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import tempfile
import time

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestConfigParams(McrouterTestCase):
    ROUTING_CONFIG = """
{
    "route": {
        "type": "if",
        "condition": "@equals(%router-info-name%,Memcache)",
        "is_true": {
            "type": "ErrorRoute",
            "response": "Memcache",
            "enable_logging": false
        },
        "is_false": {
            "type": "ErrorRoute",
            "response": "NotMemcache",
            "enable_logging": false
        }
    }
}
"""
    def setUp(self):
        (_, config) = tempfile.mkstemp("")
        with open(config, 'w') as config_file:
            config_file.write(self.ROUTING_CONFIG)
        self.config = config

    def test_client_params(self):
        self.mcrouter = self.add_mcrouter(self.config)
        retries = 10
        while self.mcrouter.is_alive() and self.mcrouter.version() is None and retries > 0:
            time.sleep(1)
            retries -= 1
        res = self.mcrouter.issue_command_and_read_all(
            'get __mcrouter__.route_handles(get,abc)\r\n'
        ).split()[-2].split('|')[-1]
        self.assertEqual(res, 'Memcache')


class TestConstShardHash(McrouterTestCase):
    config = './mcrouter/test/test_config_params.json'

    def test_config_params(self):
        mc = self.add_server(Memcached())
        self.port_map = {}
        extra_args = ['--config-params', 'PORT:{},POOL:A'.format(mc.getport())]
        mcrouter = self.add_mcrouter(self.config, extra_args=extra_args)

        self.assertTrue(mcrouter.set('key', 'value'))
        self.assertEqual(mcrouter.get('key'), 'value')
