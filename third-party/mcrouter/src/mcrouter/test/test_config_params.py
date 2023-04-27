#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestConstShardHash(McrouterTestCase):
    config = './mcrouter/test/test_config_params.json'

    def test_config_params(self):
        mc = self.add_server(Memcached())
        self.port_map = {}
        extra_args = ['--config-params', 'PORT:{},POOL:A'.format(mc.getport())]
        mcrouter = self.add_mcrouter(self.config, extra_args=extra_args)

        self.assertTrue(mcrouter.set('key', 'value'))
        self.assertEqual(mcrouter.get('key'), 'value')
