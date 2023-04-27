#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import time

from mcrouter.test.MCProcess import BaseDirectory, Memcached, Mcpiper
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestMcpiper(McrouterTestCase):
    mcrouter_ascii_config = './mcrouter/test/mcrouter_test_basic_1_1_1.json'
    mcrouter_ascii_extra_args = ['--debug-fifo-root',
                                 BaseDirectory('mcrouter_ascii').path]

    mcrouter_caret_config = './mcrouter/test/test_caret_server.json'
    mcrouter_caret_extra_args = ['--debug-fifo-root',
                                 BaseDirectory('mcrouter_caret').path]

    def setUp(self):
        self.memcached = self.add_server(Memcached())
        self.mcrouter_ascii = self.add_mcrouter(
            self.mcrouter_ascii_config,
            extra_args=self.mcrouter_ascii_extra_args,
            bg_mcrouter=True)
        self.mcrouter_caret = self.add_mcrouter(
            self.mcrouter_caret_config,
            extra_args=self.mcrouter_caret_extra_args)

    def get_mcpiper(self, mcrouter, raw):
        args = []
        if raw:
            args.append('--raw')
        mcpiper = Mcpiper(mcrouter.debug_fifo_root, args)

        # Make sure mcrouter creates fifos and start replicating data to them.
        mcrouter.set('abc', '123')
        mcrouter.delete('abc')
        time.sleep(3)

        return mcpiper

    def do_get_test(self, mcrouter, raw, special_symbol):
        # Prepare data
        self.assertTrue(self.memcached.set('key_hit', 'value_hit'))

        mcpiper = self.get_mcpiper(mcrouter, raw)

        self.assertEqual('value_hit', mcrouter.get('key_hit'))
        self.assertFalse(mcrouter.get('key_miss'))

        # wait for data to arrive in mcpiper
        time.sleep(2)

        self.assertTrue(mcpiper.contains('value_hit'))
        if raw:
            self.assertTrue(mcpiper.contains(special_symbol))
            self.assertTrue(mcpiper.contains('key_miss'))
            self.assertTrue(mcpiper.contains('key_hit'))
        else:
            self.assertTrue(mcpiper.contains('mc_res_found'))
            self.assertTrue(mcpiper.contains('mc_res_notfound'))
            self.assertTrue(mcpiper.contains('get key_miss'))
            self.assertTrue(mcpiper.contains('get key_hit'))

    def do_set_test(self, mcrouter, raw, special_symbol):

        mcpiper = self.get_mcpiper(mcrouter, raw)

        self.assertTrue(mcrouter.set('key', 'value2'))

        # wait for data to arrive in mcpiper
        time.sleep(2)
        self.assertTrue(mcpiper.contains('value2'))
        if raw:
            self.assertTrue(mcpiper.contains(special_symbol))
            self.assertTrue(mcpiper.contains('key'))
        else:
            self.assertTrue(mcpiper.contains('set key'))

    def do_delete_test(self, mcrouter, raw, special_symbol):
        # Prepare data
        self.assertTrue(self.memcached.set('key_del', 'value_to_del'))

        mcpiper = self.get_mcpiper(mcrouter, raw)

        self.assertTrue(mcrouter.delete('key_del'))
        self.assertFalse(mcrouter.delete('key_not_found'))

        # wait for data to arrive in mcpiper
        time.sleep(2)

        if raw:
            self.assertTrue(mcpiper.contains(special_symbol))
            self.assertTrue(mcpiper.contains('key_del'))
            self.assertTrue(mcpiper.contains('key_not_found'))
        else:
            self.assertTrue(mcpiper.contains('mc_res_notfound'))
            self.assertTrue(mcpiper.contains('deleted'))
            self.assertTrue(mcpiper.contains('delete key_del'))
            self.assertTrue(mcpiper.contains('delete key_not_found'))

    def test_get_ascii(self):
        self.do_get_test(self.mcrouter_ascii, False, '')

    def test_set_ascii(self):
        self.do_set_test(self.mcrouter_ascii, False, '')

    def test_delete_ascii(self):
        self.do_delete_test(self.mcrouter_ascii, False, '')

    def test_get_caret(self):
        self.do_get_test(self.mcrouter_caret, False, '^')

    def test_set_caret(self):
        self.do_set_test(self.mcrouter_caret, False, '^')

    def test_delete_caret(self):
        self.do_delete_test(self.mcrouter_caret, False, '^')

    def test_get_caret_raw(self):
        self.do_get_test(self.mcrouter_caret, True, '^')

    def test_set_caret_raw(self):
        self.do_set_test(self.mcrouter_caret, True, '^')

    def test_delete_caret_raw(self):
        self.do_delete_test(self.mcrouter_caret, True, '^')
