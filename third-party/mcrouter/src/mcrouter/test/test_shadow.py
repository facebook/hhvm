#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import time

from mcrouter.test.MCProcess import Memcached
from mcrouter.test.McrouterTestCase import McrouterTestCase

class TestShadow(McrouterTestCase):
    config = './mcrouter/test/test_shadow.json'
    extra_args = []

    def setUp(self):
        # The order here must corresponds to the order of hosts in the .json
        self.mc_foo_0 = self.add_server(Memcached())
        self.mc_foo_1 = self.add_server(Memcached())
        self.mc_bar_0 = self.add_server(Memcached())
        self.mc_bar_1 = self.add_server(Memcached())
        self.mc_foo_shadow = self.add_server(Memcached())
        self.mc_foo_shadow_specific_keys = self.add_server(Memcached())
        self.mc_bar_shadow = self.add_server(Memcached())

    def get_mcrouter(self, more_extra_args=()):
        extra_args = list(more_extra_args)
        extra_args.extend(self.extra_args)
        return self.add_mcrouter(
            self.config,
            extra_args=extra_args)

    def test_normal_shadow(self):
        mcrouter = self.get_mcrouter()
        # SpookyHashV2 will choose these values for 0.0 .. 0.1:
        shadow_list = [5, 7, 13, 33, 43, 46, 58, 71, 83, 85, 89, 91, 93]
        kv = [('f' + str(i), 'value' + str(i)) for i in range(100)]
        shadow_keys = [kv[i][0] for i in shadow_list]

        for key, value in kv:
            mcrouter.set(key, value)

        # shadow is async, so wait until all sets complete
        time.sleep(1)

        for key, value in kv:
            self.assertTrue(self.mc_foo_0.get(key) == value or
                            self.mc_foo_1.get(key) == value)
            if key in shadow_keys:
                self.assertEqual(self.mc_foo_shadow.get(key), value)
                self.assertEqual(self.mc_bar_shadow.get(key), value)
            else:
                self.assertIsNone(self.mc_foo_shadow.get(key))
                self.assertIsNone(self.mc_bar_shadow.get(key))

    def test_normal_shadow_specific_keys(self):
        mcrouter = self.get_mcrouter()
        # hashstop and after should be ignored as far as routing goes
        kv = [('f' + str(i) + "|#|ignored", 'value' + str(i))
              for i in range(101, 111)]
        shadow_list = range(101, 107)
        shadow_keys = [kv[i - 101][0] for i in shadow_list]

        for key, value in kv:
            mcrouter.set(key, value)

        # shadow is async, so wait until all sets complete
        time.sleep(1)

        for key, value in kv:
            self.assertTrue(self.mc_foo_0.get(key) == value or
                            self.mc_foo_1.get(key) == value)
            if key in shadow_keys:
                self.assertEqual(self.mc_foo_shadow_specific_keys.get(key),
                                 value)
            else:
                self.assertIsNone(self.mc_foo_shadow_specific_keys.get(key))

    def test_regional_shadow(self):
        mcrouter = self.get_mcrouter()
        # SpookyHashV2 will choose these values for 0.0 .. 0.1:
        shadow_list = [20, 30, 39, 57, 69, 71]
        kv = [('reg_f' + str(i), 'value' + str(i)) for i in range(100)]
        shadow_keys = [kv[i][0] for i in shadow_list]

        for key, value in kv:
            mcrouter.set(key, value)

        # shadow is async, so wait until all sets complete
        time.sleep(1)

        for key, value in kv:
            self.assertTrue(self.mc_foo_0.get(key) == value or
                            self.mc_foo_1.get(key) == value)
            if key in shadow_keys:
                self.assertEqual(self.mc_foo_shadow.get(key), value)
            else:
                self.assertIsNone(self.mc_foo_shadow.get(key))

    def test_migrated_shadow(self):
        mcrouter = self.get_mcrouter()
        # SpookyHashV2 will choose these values for 0.0 .. 0.1:
        shadow_list = [26, 30, 39, 63]
        kv = [('b_migrate' + str(i), 'value' + str(i)) for i in range(100)]
        shadow_keys = [kv[i][0] for i in shadow_list]

        for key, value in kv:
            mcrouter.set(key, value)

        # shadow is async, so wait until all sets complete
        time.sleep(1)

        for key, value in kv:
            self.assertIsNone(self.mc_bar_0.get(key))
            self.assertEqual(self.mc_bar_1.get(key), value)
            if key in shadow_keys:
                self.assertEqual(self.mc_bar_shadow.get(key), value)
            else:
                self.assertIsNone(self.mc_bar_shadow.get(key))

    def test_migrated_old_shadow(self):
        mcrouter = self.get_mcrouter()
        # SpookyHashV2 will choose these values for 0.0 .. 0.1:
        shadow_list = [10, 33, 35, 43, 45, 46, 47, 62, 85]
        kv = [('b_old' + str(i), 'value' + str(i)) for i in range(100)]
        shadow_keys = [kv[i][0] for i in shadow_list]

        for key, value in kv:
            mcrouter.set(key, value)

        # shadow is async, so wait until all sets complete
        time.sleep(1)

        for key, value in kv:
            self.assertEqual(self.mc_bar_0.get(key), value)
            self.assertIsNone(self.mc_bar_1.get(key))
            if key in shadow_keys:
                self.assertEqual(self.mc_bar_shadow.get(key), value)
            else:
                self.assertIsNone(self.mc_bar_shadow.get(key))

    def test_migrated_new_shadow(self):
        mcrouter = self.get_mcrouter()
        # SpookyHashV2 will choose these values for 0.0 .. 0.1:
        shadow_list = [10, 11, 15, 34, 40, 43, 47, 71, 93]
        kv = [('b_new' + str(i), 'value' + str(i)) for i in range(100)]
        shadow_keys = [kv[i][0] for i in shadow_list]

        for key, value in kv:
            mcrouter.set(key, value)

        # shadow is async, so wait until all sets complete
        time.sleep(1)

        for key, value in kv:
            self.assertIsNone(self.mc_bar_0.get(key))
            self.assertEqual(self.mc_bar_1.get(key), value)
            if key in shadow_keys:
                self.assertEqual(self.mc_bar_shadow.get(key), value)
            else:
                self.assertIsNone(self.mc_bar_shadow.get(key))

    def test_runtime_variables_override_key_fraction(self):
        mcrouter = self.get_mcrouter(
            ['--runtime-vars-file=mcrouter/'
             'test/runtime_vars_file.json'])
        # SpookyHashV2 will choose these values for 0.4 .. 0.5:
        shadow_list = [9, 10, 20, 26, 32, 34, 42, 47, 54, 63, 64, 98]
        kv = [('f' + str(i), 'value' + str(i)) for i in range(100)]
        shadow_keys = [kv[i][0] for i in shadow_list]

        for key, value in kv:
            mcrouter.set(key, value)

        # shadow is async, so wait until all sets complete
        time.sleep(1)

        for key, value in kv:
            self.assertTrue(self.mc_foo_0.get(key) == value or
                            self.mc_foo_1.get(key) == value)
            if key in shadow_keys:
                self.assertEqual(self.mc_foo_shadow.get(key), value)
                self.assertEqual(self.mc_bar_shadow.get(key), value)
            else:
                self.assertIsNone(self.mc_foo_shadow.get(key))
                self.assertIsNone(self.mc_bar_shadow.get(key))

    def test_runtime_variables_override_range(self):
        mcrouter = self.get_mcrouter(
            ['--runtime-vars-file=mcrouter/'
             'test/runtime_vars_file.json'])
        # SpookyHashV2 will choose these values for 0.0 .. 0.1:
        shadow_list = [10, 11, 15, 34, 40, 43, 47, 71, 93]
        kv = [('b_new' + str(i), 'value' + str(i)) for i in range(100)]
        shadow_keys = [kv[i][0] for i in shadow_list]

        for key, value in kv:
            mcrouter.set(key, value)

        # shadow is async, so wait until all sets complete
        time.sleep(1)

        for key, value in kv:
            self.assertIsNone(self.mc_bar_0.get(key))
            self.assertEqual(self.mc_bar_1.get(key), value)
            if key in shadow_keys:
                self.assertEqual(self.mc_bar_shadow.get(key), value)
            else:
                self.assertIsNone(self.mc_bar_shadow.get(key))
