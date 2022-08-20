#!/usr/bin/env python3
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

import re
import signal
import socket
import subprocess
import unittest

from mcrouter.test.config import McrouterGlobals

class OutputCheckerTestCase(unittest.TestCase):
    def setUp(self):
        self.proc = None

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

    def check_for_message(self, good, bad, timeout):
        stderr = ""

        def timeout_handler(signum, frame):
            self.fail("Timed out")

        signal.signal(signal.SIGALRM, timeout_handler)
        signal.alarm(timeout)

        for line in self.proc.stderr:
            line = line.decode()
            stderr += line
            if re.search(bad, line):
                self.fail("bad regex matched '{}'".format(line.strip()))

            if re.search(good, line):
                signal.alarm(0)
                return

        self.fail("no good or bad matches in output: " + stderr)

class TestBadParams(OutputCheckerTestCase):
    def test_bad_config(self):
        listen_sock = socket.socket()
        listen_sock.listen(100)
        args = McrouterGlobals.preprocessArgs([
            McrouterGlobals.binPath('mcrouter'),
            '-f', "/dev/null/doesnotexist",
            '--listen-sock-fd', str(listen_sock.fileno())
        ])

        self.spawn(args)
        self.check_for_message(
                good='Can not read config',
                bad='reconfigured with',
                timeout=10)

        listen_sock.close()
