# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-strict

from eden.integration.lib import hgrepo
from watchman.integration.lib import WatchmanEdenTestCase, WatchmanInstance


class TestEdenShutdown(WatchmanEdenTestCase.WatchmanEdenTestCase):
    def test_shutdown_and_restart(self) -> None:
        def populate(repo: hgrepo.HgRepository) -> None:
            repo.write_file("hello", "hola\n")
            repo.commit("initial commit.")

        root = self.makeEdenMount(populate)

        inst = WatchmanInstance.Instance()
        _, stderr = inst.commandViaCLI(["watch", root])
        self.assertEqual(stderr, b"")
        _, stderr = inst.commandViaCLI(["shutdown-server"])
        self.assertEqual(stderr, b"")
        _, stderr = inst.commandViaCLI(["get-sockname"])
        self.assertEqual(stderr, b"")
