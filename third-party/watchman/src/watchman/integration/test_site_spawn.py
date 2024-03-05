# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import json
import os
import sys
import unittest

import pywatchman
from watchman.integration.lib import HELPER_ROOT, WatchmanInstance

SITE_SPAWN = os.path.join(HELPER_ROOT, "site_spawn.py")
SITE_SPAWN_FAIL = os.path.join(HELPER_ROOT, "site_spawn_fail.py")


@unittest.skipIf(os.name == "nt", "not supported on windows")
class TestSiteSpawn(unittest.TestCase):
    def test_failingSpawner(self) -> None:
        config = {"spawn_watchman_service": SITE_SPAWN_FAIL}

        inst = WatchmanInstance.Instance(config=config)
        stdout, stderr = inst.commandViaCLI(["version"])
        sys.stdout.buffer.write(b"stdout:\n")
        sys.stdout.buffer.write(stdout)
        sys.stdout.buffer.write(b"stderr:\n")
        sys.stdout.buffer.write(stderr)
        stderr = stderr.decode("ascii")
        self.assertEqual(b"", stdout)
        self.assertRegex(stderr, "failed to start\n")
        self.assertRegex(stderr, "site_spawn_fail.py: exited with status 1")

    def test_no_site_spawner(self) -> None:
        """With a site spawner configured to otherwise fail, pass
        `--no-site-spawner` and ensure that a failure didn't occur."""
        config = {"spawn_watchman_service": SITE_SPAWN_FAIL}

        inst = WatchmanInstance.Instance(config=config)
        stdout, stderr = inst.commandViaCLI(["version", "--no-site-spawner"])

        print(stdout, stderr.decode("ascii"))
        parsed = json.loads(stdout.decode("ascii"))
        self.assertTrue("version" in parsed)

        inst.commandViaCLI(["--no-spawn", "--no-local", "shutdown-server"])

    def test_spawner(self) -> None:
        config = {"spawn_watchman_service": SITE_SPAWN}

        inst = WatchmanInstance.Instance(config=config)
        stdout, stderr = inst.commandViaCLI(["version"])

        parsed = json.loads(stdout.decode("ascii"))
        self.assertTrue("version" in parsed)

        # Shut down that process, as we have no automatic way to deal with it
        inst.commandViaCLI(["--no-spawn", "--no-local", "shutdown-server"])
