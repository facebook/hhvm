# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

# no unicode literals

import os
import sys
from typing import Optional

from . import WatchmanTestCase


TestParent = object
try:
    from eden.integration.lib import edenclient, hgrepo

    def is_sandcastle():
        return "SANDCASTLE" in os.environ

    if edenclient.can_run_eden():
        TestParent = WatchmanTestCase.WatchmanTestCase

    can_run_eden = edenclient.can_run_eden

except ImportError:

    def is_buck_build():
        return "BUCK_BUILD_ID" in os.environ

    # We want import failures to hard fail the build when using buck internally
    # because it means we overlooked something, but we want it to be a soft
    # fail when we run our opensource build
    if is_buck_build():
        raise

    def can_run_eden():
        return False

    class WatchmanEdenTestCase:
        pass

else:

    class WatchmanEdenTestCase(WatchmanTestCase.WatchmanTestCase):
        # The contents of the .eden directory
        # This is used by several tests when checking reported file lists
        eden_dir_entries = [
            ".eden/root",
            ".eden/socket",
            ".eden/client",
            ".eden/this-dir",
        ]

        eden: edenclient.EdenFS

        def setUp(self) -> None:
            super(WatchmanEdenTestCase, self).setUp()

            # The test EdenFS instance.
            # We let it create and manage its own temporary test directory, rather than
            # using the one scoped to the test because we have very real length limits on
            # the socket path name that we're likely to hit otherwise.
            self.eden = edenclient.EdenFS()
            self.addCleanup(lambda: self.cleanUpEden())

            # where we'll mount the eden client(s)
            self.mounts_dir = self.mkdtemp(prefix="eden_mounts")

            # Watchman needs to start up with the same HOME as eden, otherwise
            # it won't be able to locate the eden socket
            self.save_home = os.environ["HOME"]
            os.environ["HOME"] = str(self.eden.home_dir)
            self.addCleanup(lambda: self._restoreHome())

            self.system_hgrc = None

            self.eden_watchman = self.watchmanInstance()
            self.addCleanup(self.cleanUpWatchman)

            self.client = self.getClient(self.eden_watchman)

            # chg can interfere with eden, so disable it up front
            os.environ["CHGDISABLE"] = "1"

            # Start the EdenFS instance
            self.eden.start()

        def _restoreHome(self) -> None:
            assert self.save_home is not None
            os.environ["HOME"] = self.save_home

        def cleanUpEden(self) -> None:
            self.eden.cleanup()

        def cleanUpWatchman(self):
            roots = self.watchmanCommand("watch-list")["roots"]
            self.watchmanCommand("watch-del-all")
            for root in roots:
                try:
                    self.eden.unmount(root)
                except Exception:
                    pass

            self.eden_watchman.stop()
            self.eden_watchman = None

        def makeEdenMount(self, populate_fn=None):
            """populate_fn is a function that accepts a repo object and
            that is expected to populate it as a pre-requisite to
            starting up the eden mount for it.
            """

            repo_path = self.mkdtemp(prefix="eden_repo_")
            repo_name = os.path.basename(repo_path)
            repo = self.repoForPath(repo_path)
            repo.init()

            if populate_fn:
                populate_fn(repo)

            mount_path = os.path.join(self.mounts_dir, repo_name)

            self.eden.clone(repo_path, mount_path)
            return mount_path

        def repoForPath(self, path):
            if self.system_hgrc is None:
                system_hgrc_path = self.mktemp("hgrc")
                with open(system_hgrc_path, "w") as f:
                    f.write(hgrepo.HgRepository.get_system_hgrc_contents())
                self.system_hgrc = system_hgrc_path

            return hgrepo.HgRepository(path, system_hgrc=self.system_hgrc)

        def setDefaultConfiguration(self):
            self.setConfiguration("local", "bser", False, False)

        def isCaseSensitiveMount(self, path) -> bool:
            # Ideally we'd ask Eden somehow, but this is close enough for use in
            # tests. See eden/fs/cli/main.py.
            return sys.platform == "linux"
