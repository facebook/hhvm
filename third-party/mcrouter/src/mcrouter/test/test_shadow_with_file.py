#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Mcrouter
from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase
from time import sleep

class TestShadowWithFile(McrouterTestCase):
    config = './mcrouter/test/test_shadow_with_file.json'

    def setUp(self):
        self.mc1 = self.add_server(Memcached())
        self.mc2 = self.add_server(Memcached())
        self.mc_shadow = self.add_server(Memcached())
        self.port_map = {}
        self.extra_args = [
            '--config-params',
            'PORT_1:{},PORT_2:{},PORT_SHADOW:{}'.format(
                self.mc1.getport(), self.mc2.getport(),
                self.mc_shadow.getport())
            ]

    def get_mcrouter(self):
        return self.add_mcrouter(self.config, extra_args=self.extra_args)

    def test_shadow_with_file(self):
        mcr = self.get_mcrouter()
        # SpookyHashV2 will choose these values for 0.0 .. 0.1:
        shadow_list = [5, 7, 13, 33, 43, 46, 58, 71, 83, 85, 89, 91, 93]
        for i in range(100):
            key = 'f' + str(i)
            value = 'value' + str(i)
            mcr.set(key, value)
            self.assertTrue(self.mc1.get(key) == value or
                            self.mc2.get(key) == value)

        # Shadow requrests are async
        sleep(1)

        for i in range(100):
            key = 'f' + str(i)
            value = 'value' + str(i)
            if i in shadow_list:
                self.assertEqual(self.mc_shadow.get(key), value)
            else:
                self.assertEqual(self.mc_shadow.get(key), None)
