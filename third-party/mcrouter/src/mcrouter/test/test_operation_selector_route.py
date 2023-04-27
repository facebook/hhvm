#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestOperationSelectorRoute(McrouterTestCase):
    config = './mcrouter/test/test_operation_selector_route.json'
    extra_args = []

    def setUp(self):
        self.memcached_get = self.add_server(Memcached())
        self.memcached_set = self.add_server(Memcached())
        self.memcached_delete = self.add_server(Memcached())

        self.mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args)

    def test_get(self):
        self.assertTrue(self.memcached_get.set('key_get', 'val_get'))
        self.assertEqual('val_get', self.mcrouter.get('key_get'))

    def test_set(self):
        self.assertTrue(self.mcrouter.set('key_set', 'val_set'))
        self.assertEqual('val_set', self.memcached_set.get('key_set'))

    def test_delete(self):
        self.assertTrue(self.memcached_delete.set('key_del', 'val_del'))
        self.assertTrue(self.mcrouter.delete('key_del'))
        self.assertFalse(self.memcached_delete.get('key_del'))
