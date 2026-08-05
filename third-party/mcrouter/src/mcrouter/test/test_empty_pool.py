#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

from carbon.carbon_result.thrift_types import Result
from mcrouter.test.McrouterTestCase import McrouterTestCase


class TestEmptyPool(McrouterTestCase):
    config = "./mcrouter/test/test_empty_pool.json"
    extra_args = []

    def get_mcrouter(self):
        return self.add_mcrouter(
            self.config, extra_args=self.extra_args, enable_thrift=True
        ).get_thrift_client()

    def test_empty_pool(self):
        # Start a mcrouter without route handles
        key = "foo"
        mcrouter_w_rh = self.get_mcrouter()
        reply = mcrouter_w_rh.mcGet(key.encode())
        self.assertEqual(Result.LOCAL_ERROR, reply.result)
        self.assertIn("HashRoute with empty children", reply.message)
