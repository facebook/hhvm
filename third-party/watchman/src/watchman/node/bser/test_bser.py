#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

import distutils.spawn
import os
import shutil
import subprocess
import tempfile
import unittest

from watchman.integration.lib.node import node_bin, yarn_bin


WATCHMAN_SRC_DIR = os.environ.get("WATCHMAN_SRC_DIR", os.getcwd())
THIS_DIR = os.path.join(WATCHMAN_SRC_DIR, "tests")


class BserTestCase(unittest.TestCase):
    @unittest.skipIf(
        yarn_bin is None or node_bin is None, "yarn/node not correctly installed"
    )
    def runTest(self):
        with tempfile.TemporaryDirectory() as tempdir:
            env = os.environ.copy()
            env["TMPDIR"] = tempdir

            # build the node module with yarn
            node_dir = os.path.join(env["TMPDIR"], "fb-watchman")
            shutil.copytree(os.path.join(WATCHMAN_SRC_DIR, "node"), node_dir)
            bser_dir = os.path.join(node_dir, "bser")

            # install pre-reqs
            install_args = [yarn_bin, "install"]
            if "YARN_OFFLINE" in env:
                install_args.append("--offline")
            print("Installing yarn deps with ", install_args)
            subprocess.check_call(install_args, cwd=bser_dir, env=env)

            env["TMP"] = env["TMPDIR"]
            env["TEMP"] = env["TMPDIR"]
            env["NODE_PATH"] = "%s:%s" % (env["TMPDIR"], env.get("NODE_PATH", ""))
            subprocess.check_call([yarn_bin, "test"], cwd=bser_dir, env=env)
            self.assertTrue(True, "test completed")
