#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import json
import os
import time

from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestAsyncFiles(McrouterTestCase):
    default_route = '/././'
    stat_prefix = 'libmcrouter.mcrouter.0.'
    config = './mcrouter/test/mcrouter_test_basic_1_1_1.json'
    config_hash = '837ae7d82f2fe7cb785b941dae505811'
    extra_args = ['--stats-logging-interval', '100', '--use-asynclog-version2']
    mock_smc_config = None
    asynclog_name = 'foo'

    def check_stats(self, stats_dir, retries=20, sleep_interval=1):
        file_stat = os.path.join(stats_dir, self.stat_prefix + 'stats')
        file_startup_options = os.path.join(
            stats_dir, self.stat_prefix + 'startup_options')
        file_config_sources = os.path.join(
            stats_dir, self.stat_prefix + 'config_sources_info')

        fileStatExists = False
        fileStartupExists = False
        fileConfigExists = False
        while retries > 0:
            retries -= 1
            if not fileStatExists:
                fileStatExists = os.path.exists(file_stat)
            if not fileStartupExists:
                fileStartupExists = os.path.exists(file_startup_options)
            if not fileConfigExists:
                fileConfigExists = os.path.exists(file_config_sources)
            if fileStatExists and fileStartupExists and fileConfigExists:
                break
            # Sleep between retry intervals
            time.sleep(sleep_interval)

        self.assertTrue(fileStatExists,
                        "{} doesn't exist".format(file_stat))
        self.assertTrue(fileStartupExists,
                        "{} doesn't exist".format(file_startup_options))
        self.assertTrue(fileConfigExists,
                        "{} doesn't exist".format(file_config_sources))

        return (file_stat, file_startup_options, file_config_sources)

    def test_stats_no_requests(self):
        mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args,
            sr_mock_smc_config=self.mock_smc_config,
        )
        # check will wait for files
        self.check_stats(mcrouter.stats_dir)

    def test_async_files(self):
        mcrouter = self.add_mcrouter(
            self.config,
            extra_args=self.extra_args,
            sr_mock_smc_config=self.mock_smc_config,
        )
        self.assertIsNone(mcrouter.delete('key'))

        # wait for files
        time.sleep(2)

        # check async spool for failed delete
        asynclog_files = []
        for root, _dirs, files in os.walk(mcrouter.get_async_spool_dir()):
            for f in files:
                asynclog_files.append(os.path.join(root, f))

        self.assertEqual(len(asynclog_files), 1)
        foundPool = False

        with open(asynclog_files[0], 'r') as f:
            for line in f.readlines():
                if self.asynclog_name in line:
                    foundPool = True
                    break

        self.assertTrue(foundPool)

        # check stats
        (file_stat, file_startup_options, file_config_sources) = \
            self.check_stats(mcrouter.stats_dir)

        with open(file_stat) as f:
            stat_json = json.load(f)
            self.assertGreaterEqual(stat_json[self.stat_prefix + 'uptime'], 0)

        with open(file_startup_options) as f:
            startup_json = json.load(f)
            self.assertEqual(startup_json['default_route'], self.default_route)

        with open(file_config_sources) as f:
            sources_json = json.load(f)
            self.assertEqual(sources_json['mcrouter_config'], self.config_hash)

        # check stats are up-to-date
        now = time.time()
        time.sleep(2)

        self.assertGreater(os.path.getmtime(file_stat), now)
        self.assertGreater(os.path.getmtime(file_startup_options), now)
        self.assertGreater(os.path.getmtime(file_config_sources), now)


class TestNamedAsyncFiles(TestAsyncFiles):
    config = './mcrouter/test/mcrouter_test_basic_3_1_1.json'
    config_hash = '37ba14d3319c638d970353f6f3861c68'
