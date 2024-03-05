# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os
import subprocess

import pywatchman

from . import WatchmanTestCase


STRING_TYPES = (str, bytes)


class HgMixin:
    def hg(self, args, cwd=None):
        env = dict(os.environ)
        env["HGPLAIN"] = "1"
        env["HGUSER"] = "John Smith <smith@example.com>"
        env["NOSCMLOG"] = "1"  # disable some instrumentation at FB
        sockpath = self.watchmanInstance().getSockPath()
        env["WATCHMAN_SOCK"] = sockpath.legacy_sockpath()
        p = subprocess.Popen(
            [
                env.get("EDEN_HG_BINARY", "hg"),
                # we force the extension on.  This is a soft error for
                # mercurial if it is not available, so we also employ
                # the skipIfNoFSMonitor() test above to make sure the
                # environment is sane.
                "--config",
                "extensions.fsmonitor=",
                # Deployed versions of mercurial regressed and stopped
                # respecting the WATCHMAN_SOCK environment override, so
                # we have to reach in and force their hardcoded sockpath here.
                "--config",
                "fsmonitor.sockpath=%s" % sockpath.legacy_sockpath(),
            ]
            + args,
            env=env,
            cwd=cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
        )
        out, err = p.communicate()
        if p.returncode != 0:
            raise Exception("hg %r failed: %s, %s" % (args, out, err))

        return out, err


class WatchmanSCMTestCase(WatchmanTestCase.WatchmanTestCase, HgMixin):
    def __init__(self, methodName: str = "run") -> None:
        super(WatchmanSCMTestCase, self).__init__(methodName)

    def requiresPersistentSession(self) -> bool:
        return True

    def skipIfNoFSMonitor(self) -> None:
        """cause the test to skip if fsmonitor is not available.
        We don't call this via unittest.skip because we want
        to have the skip message show the context"""
        try:
            out, err = self.hg(["help", "--extension", "fsmonitor"])
        except Exception as e:
            self.skipTest("fsmonitor is not available: %s" % str(e))
        else:
            out = out.decode("utf-8")
            err = err.decode("utf-8")
            fail_str = "failed to import extension"
            if (fail_str in out) or (fail_str in err):
                self.skipTest("hg configuration is broken: %s %s" % (out, err))

    def checkOSApplicability(self) -> None:
        if os.name == "nt":
            self.skipTest("The order of events on Windows is funky")

    def resolveCommitHash(self, revset, cwd=None) -> str:
        return self.hg(args=["log", "-T", "{node}", "-r", revset], cwd=cwd)[0].decode(
            "utf-8"
        )

    def waitForStatesToVacate(self, root) -> None:
        # Wait for all states to vacate (check repeatedly)
        def checkAssertedStates():
            result = self.getClient().query("debug-get-asserted-states", root)
            return result["states"]

        self.assertWaitForEqual([], checkAssertedStates)

    def getConsolidatedFileList(self, dat):
        fset = set()
        for _ in dat:
            fset.update(_.get("files", []))
        return fset

    def getSubFatClocksOnly(self, subname, root):
        dat = self.waitForSub(subname, root=root)
        return [item for item in dat if not isinstance(item["clock"], STRING_TYPES)]
