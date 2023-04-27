#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import os
import shutil
import time

from mcrouter.test.MCProcess import BaseDirectory
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestDumpConfig(McrouterTestCase):
    valid_config = './mcrouter/test/mcrouter_test_basic_1_1_1.json'
    invalid_config = './mcrouter/test/test_dump_config_invalid.json'

    def setUp(self):
        self.config_dump_root = BaseDirectory('config_dump')

    def get_mcrouter(self, config, mc_extra_args=None):
        extra_args = mc_extra_args or []
        # we don't want to create Memcache instances here to avoid
        # creating a new config file with port substitution.
        extra_args = extra_args + ['--proxy-threads', '1',
                                   '--config-dump-root',
                                   self.config_dump_root.path]
        return self.add_mcrouter(config, extra_args=extra_args)

    def _get_valid_config(self, base_dir):
        config_path = os.path.join(base_dir.path, 'config.json')
        shutil.copyfile(self.valid_config, config_path)
        return config_path

    def _replace_with_invalid_config(self, config_path):
        os.remove(config_path)
        shutil.copyfile(self.invalid_config, config_path)

    def _get_dumped_config_root(self):
        return '{}/mcrouter/0'.format(self.config_dump_root.path)

    def test_dump_config(self):
        """
        Ensures that config files are successfully dumped to disk.
        """
        mcrouter = self.get_mcrouter(self.valid_config)
        self.assertTrue(self._is_mcrouter_running(mcrouter))

        dumped_config_root = self._get_dumped_config_root()

        # check if dir exists
        self.assertTrue(os.path.isdir(dumped_config_root))
        self.assertTrue(os.path.exists(dumped_config_root))

        # check config files
        dumped_config_files = os.listdir(dumped_config_root)
        self.assertEqual(1, len(dumped_config_files))

        dumped_config_file = '{}/{}'.format(
            dumped_config_root, dumped_config_files[0])
        with open(mcrouter.config) as original_file:
            with open(dumped_config_file) as dumped_file:
                self.assertEqual(original_file.read(), dumped_file.read())

    def test_dont_dump_bad_config(self):
        """
        Ensures that mcrouter do not dump invalid configs.
        """
        # first, copy valid config to a temp file, as we need to modify it later
        base_dir = BaseDirectory('McConfigs')
        config_path = self._get_valid_config(base_dir)

        # create mcrouter with a valid config (mcrouter will dump it to disk).
        mcrouter = self.get_mcrouter(config_path)
        self.assertTrue(self._is_mcrouter_running(mcrouter))

        # replace config file with an invalid one.
        # Mcrouter should not dump config this time, because it's invalid.
        self._replace_with_invalid_config(config_path)

        # shutdown mcrouter
        mcrouter.shutdown()
        mcrouter.terminate()

        # check if the last config dumped is the valid config.
        dumped_config_root = self._get_dumped_config_root()
        dumped_config_files = os.listdir(dumped_config_root)
        self.assertEqual(1, len(dumped_config_files))
        dumped_config_file = '{}/{}'.format(
            dumped_config_root, dumped_config_files[0])
        with open(self.valid_config) as original_file:
            with open(dumped_config_file) as dumped_file:
                self.assertEqual(original_file.read(), dumped_file.read())

    def test_use_dumped_config(self):
        """
        Test if mcrouter can successfully reconfigure from dumped configs
        when a bad config file if pushed.
        """
        # first, copy valid config to a temp file, as we need to modify it later
        base_dir = BaseDirectory('McConfigs')
        config_path = self._get_valid_config(base_dir)

        # create mcrouter with a valid config (mcrouter will dump it to disk).
        mcrouter = self.get_mcrouter(config_path)
        self.assertTrue(self._is_mcrouter_running(mcrouter))

        # shutdown mcrouter
        mcrouter.shutdown()
        mcrouter.terminate()

        # replace config file with an invalid one
        self._replace_with_invalid_config(config_path)

        # check if mcrouter configured correctly.
        mcrouter = self.get_mcrouter(config_path)
        self.assertTrue(self._is_mcrouter_running(mcrouter))

    def test_dumped_config_too_old(self):
        """
        Test if mcrouter fails to configure if dumped config is too old
        to be trusted.
        """
        # first, copy valid config to a temp file, as we need to modify it later
        base_dir = BaseDirectory('McConfigs')
        config_path = self._get_valid_config(base_dir)

        # create mcrouter with a valid config (mcrouter will dump it to disk).
        mcrouter = self.get_mcrouter(config_path)
        self.assertTrue(self._is_mcrouter_running(mcrouter))

        # shutdown mcrouter
        mcrouter.shutdown()
        mcrouter.terminate()

        # replace config file with an invalid one
        self._replace_with_invalid_config(config_path)

        # sleep for 2 seconds and forbid mcrouter from using backup configs
        # that were dumped more than 1 second ago.
        time.sleep(2)
        mcrouter = self.get_mcrouter(config_path,
                                     ['--max-dumped-config-age', '1'])

        # Mcrouter should not be running this time
        self.assertFalse(self._is_mcrouter_running(mcrouter))
