#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

import json
import os
import tempfile
import time

from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.mock_servers import SleepServer


class TestMcrouterProcessingTime(McrouterTestCase):
    (_, debug_file) = tempfile.mkstemp()
    config_processing_time = "./mcrouter/test/test_mcrouter_processing_time.json"
    default_route = "/a/a/"
    extra_args = [
        "--enable-scuba-samples",
        "--scuba-sample-period=1",
        "--scuba-samples-debug-file=" + debug_file,
    ]

    def setUp(self) -> None:
        self.mc_latency_injection = self.add_server(SleepServer())
        self.mc_shadow1 = self.add_server(SleepServer())
        self.mc_shadow2 = self.add_server(SleepServer())
        self.mcrouter = self.add_mcrouter(
            self.config_processing_time, self.default_route, self.extra_args
        )

    def tearDown(self) -> None:
        os.remove(self.debug_file)

    def test_mcrouter_processing_time(self) -> None:
        lease_get_cmd = "lease-get key1\r\n"
        response = self.mcrouter.issue_command(lease_get_cmd)
        self.assertEqual(response, "SERVER_ERROR Reply timeout\r\n")
        time.sleep(0.5)
        stat = self.mcrouter.stats()
        self.mcrouter.terminate()
        scuba_samples = open(self.debug_file).readlines()
        # One error sampler plus three main sampler.
        self.assertEqual(len(scuba_samples), 4)
        self.assertGreaterEqual(int(stat["processing_time_us"]), 200000)
        for sample in scuba_samples:
            sample_parsed = json.loads(sample)
            self.assertEqual(
                sample_parsed["normal"]["sampler"],
                "error" if scuba_samples.index(sample) == 0 else "main",
            )
            self.assertEqual(sample_parsed["normal"]["reply_message"], "Reply timeout")
            # Inject latency 200 ms, server timeout 1 sec.
            if scuba_samples.index(sample) < 2:
                self.assertEqual(
                    sample_parsed["int"]["relative_start_time_us"],
                    sample_parsed["int"]["processing_time_us"],
                )
                self.assertGreaterEqual(
                    sample_parsed["int"]["processing_time_us"], 200000
                )
            else:
                self.assertGreaterEqual(
                    sample_parsed["int"]["relative_start_time_us"], 1200000
                )
                self.assertGreater(sample_parsed["int"]["processing_time_us"], 200000)
                self.assertLess(sample_parsed["int"]["processing_time_us"], 1000000)
