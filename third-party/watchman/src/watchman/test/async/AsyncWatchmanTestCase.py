# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.


import asyncio
import errno
import os
import os.path
import unittest

import WatchmanInstance
from pywatchman_aio import AIOClient as WatchmanClient


class AsyncWatchmanTestCase(unittest.TestCase):
    def setUp(self):
        self.loop = asyncio.get_event_loop()
        sockpath = WatchmanInstance.getSharedInstance().getSockPath()
        self.client = self.loop.run_until_complete(WatchmanClient.from_socket(sockpath))

    def tearDown(self):
        self.client.close()

    def run(self, result):
        assert result
        super(AsyncWatchmanTestCase, self).run(result)
        return result

    def touch(self, fname, times=None):
        try:
            os.utime(fname, times)
        except OSError as e:
            if e.errno == errno.ENOENT:
                with open(fname, "a"):
                    os.utime(fname, times)
            else:
                raise

    def touch_relative(self, base, *fname):
        fname = os.path.join(base, *fname)
        self.touch(fname, None)

    def watchman_command(self, *args):
        task = asyncio.wait_for(self.client.query(*args), 10)
        return self.loop.run_until_complete(task)

    def get_file_list(self, root):
        expr = {"expression": ["exists"], "fields": ["name"]}
        res = self.watchman_command("query", root, expr)["files"]
        return res

    def assert_sub_contains_all(self, sub, what):
        files = set(sub["files"])
        for obj in what:
            assert obj in files, str(obj) + " was not in subscription " + repr(sub)

    def assert_file_sets_equal(self, iter1, iter2, message=None):
        set1 = set(iter1)
        set2 = set(iter2)
        self.assertEqual(set1, set2, message)

    # Wait for the file list to match the input set
    def assert_root_file_set(self, root, files):
        self.assert_file_sets_equal(self.get_file_list(root), files)

    def wait_for_sub(self, name, root, timeout=10):
        client = self.client
        task = asyncio.wait_for(client.get_subscription(name, root), timeout)
        return self.loop.run_until_complete(task)
