#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import signal
import subprocess
import unittest
import tempfile

from mcrouter.test.config import McrouterGlobals


class OutputCheckerTestCase(unittest.TestCase):
    config_str = '''{
          "route": {
            "type": "XYZRoute",
            "pool": {
              "name" : "A",
              "servers": [
                "localhost:5001",
                "localhost:5002",
                "localhost:5003"
              ],
            },
          }
        }
    '''

    def setUp(self):
        self.proc = None
        (_, config) = tempfile.mkstemp("")
        with open(config, 'w') as config_file:
            config_file.write(self.config_str)
        self.config = config

    def tearDown(self):
        if self.proc:
            try:
                self.proc.terminate()
            except OSError:
                pass
            self.proc.wait()
        signal.alarm(0)

    def spawn(self, cmd):
        self.proc = subprocess.Popen(cmd, stderr=subprocess.PIPE)

    def check_for_error_line_number(self):
        for line in self.proc.stderr:
            line = line.decode()
            if "Unknown RouteHandle: XYZRoute line: 3" in line:
                    return True
        return False


class TestLineNumbers(OutputCheckerTestCase):
    def test_linenumbers(self):
        args = McrouterGlobals.preprocessArgs([
            McrouterGlobals.binPath('mcrouter'),
            '--config', 'file:' + self.config,
            '-p', str(5721)
        ])

        self.spawn(args)
        self.assertTrue(self.check_for_error_line_number())
