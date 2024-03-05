# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import binascii
import json
import os
import os.path
import subprocess
import unittest

from pywatchman import bser, encoding
from watchman.integration.lib import WatchmanInstance


class TestDashJCliOption(unittest.TestCase):
    def getSockPath(self):
        return WatchmanInstance.getSharedInstance().getSockPath()

    def doJson(self, addNewLine, pretty: bool = False) -> None:
        sockpath = self.getSockPath()
        if pretty:
            watchman_cmd = b'[\n"get-sockname"\n]'
        else:
            watchman_cmd = json.dumps(["get-sockname"])
            watchman_cmd = watchman_cmd.encode("ascii")
        if addNewLine:
            watchman_cmd = watchman_cmd + b"\n"

        cli_cmd = [
            os.environ.get("WATCHMAN_BINARY", "watchman"),
            "--unix-listener-path={0}".format(sockpath.unix_domain),
            "--named-pipe-path={0}".format(sockpath.named_pipe),
            "--logfile=/BOGUS",
            "--statefile=/BOGUS",
            "--no-spawn",
            "--no-local",
            "-j",
        ]
        proc = subprocess.Popen(
            cli_cmd,
            stdin=subprocess.PIPE,
            stderr=subprocess.PIPE,
            stdout=subprocess.PIPE,
        )

        stdout, stderr = proc.communicate(input=watchman_cmd)
        self.assertEqual(proc.poll(), 0, stderr.decode(errors="replace"))
        # the response should be json because that is the default
        result = json.loads(stdout.decode("utf-8"))
        self.assertEqual(result["unix_domain"], sockpath.unix_domain)

    def test_jsonInputNoNewLine(self) -> None:
        self.doJson(False)

    def test_jsonInputNewLine(self) -> None:
        self.doJson(True)

    def test_jsonInputPretty(self) -> None:
        self.doJson(True, True)

    def test_bserInput(self) -> None:
        sockpath = self.getSockPath()
        # pyre-fixme[16]: Module `pywatchman` has no attribute `bser`.
        watchman_cmd = bser.dumps(["get-sockname"])
        cli_cmd = [
            os.environ.get("WATCHMAN_BINARY", "watchman"),
            "--unix-listener-path={0}".format(sockpath.unix_domain),
            "--named-pipe-path={0}".format(sockpath.named_pipe),
            "--logfile=/BOGUS",
            "--statefile=/BOGUS",
            "--no-spawn",
            "--no-local",
            "-j",
        ]
        proc = subprocess.Popen(
            cli_cmd,
            stdin=subprocess.PIPE,
            stderr=subprocess.PIPE,
            stdout=subprocess.PIPE,
        )

        stdout, stderr = proc.communicate(input=watchman_cmd)
        self.assertEqual(proc.poll(), 0, stderr)
        # the response should be bser to match our input
        # pyre-fixme[16]: Module `pywatchman` has no attribute `bser`.
        result = bser.loads(stdout)
        result_sockname = result["unix_domain"]
        result_sockname = encoding.decode_local(result_sockname)
        self.assertEqual(
            result_sockname,
            sockpath.unix_domain,
            binascii.hexlify(stdout).decode("ascii"),
        )
