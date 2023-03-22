#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

from mcrouter.test.McrouterTestCase import McrouterTestCase
from mcrouter.test.mock_servers import DeadServer

class TestAxonLogBase(McrouterTestCase):
    config = "./mcrouter/test/test_axon_log.json"
    extra_args = ["--enable-axonlog"]

    def setUp(self):
        self.mr = self.add_mcrouter(self.config,
            extra_args=self.extra_args)

class TestAxonProxyFailedDelete(TestAxonLogBase):

    def setUp(self):
        self.mc1 = self.add_server(DeadServer(5000))
        self.mr = self.add_mcrouter(self.config,
            extra_args=self.extra_args)

    def test_failed_del(self):
        for i in range(1000):
            self.mr.delete(f"key_del{i}")

        def condition():
            stats = self.mr.stats()
            return (
                float(
                    stats["axon_proxy_request_success_rate"]
                    + stats["axon_proxy_request_fail_rate"]
                )
                > 0
            )

        self.assert_eventually_true(condition)



class TestAxonLogAllDelete(TestAxonLogBase):
    config = "./mcrouter/test/test_axon_log_alldelete.json"

    def setUp(self):
        self.mr = self.add_mcrouter(self.config,
            extra_args=self.extra_args)
    def test_all_delete(self):
        for i in range(1000):
            self.mr.delete(f"key_del{i}")

        def condition():
            stats = self.mr.stats()
            return (
                float(
                    stats["axon_proxy_request_success_rate"]
                    + stats["axon_proxy_request_fail_rate"]
                )
                > 0
            )

        self.assert_eventually_true(condition)
