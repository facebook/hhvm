# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe

from watchman.integration.lib import WatchmanEdenTestCase


def populate(repo) -> None:
    # We ignore ".hg" here just so some of the tests that list files don't have to
    # explicitly filter out the contents of this directory.  However, in most situations
    # the .hg directory normally should not be ignored.
    repo.write_file(".watchmanconfig", '{"ignore_dirs":[".hg"]}')
    repo.write_file("hello", "hola\n")
    repo.write_file("world", "sekai\n")
    repo.write_file("adir/file", "foo!\n")
    repo.write_file("bdir/test.sh", "#!/bin/bash\necho test\n", mode=0o755)
    repo.write_file("bdir/noexec.sh", "#!/bin/bash\necho test\n")
    repo.symlink("slink", "hello")
    repo.commit("initial commit.")


class TestEdenQuery(WatchmanEdenTestCase.WatchmanEdenTestCase):
    def test_eden_path_query(self) -> None:
        root = self.makeEdenMount(populate)
        res = self.watchmanCommand("watch", root)
        self.assertEqual("eden", res["watcher"])

        # It should return `hello`
        res = self.watchmanCommand(
            "query",
            root,
            {
                "path": ["hello"],
                "fields": ["name"],
            },
        )
        self.assertFileListsEqual(res["files"], ["hello"])

        # It should return files under bdir
        res = self.watchmanCommand(
            "query",
            root,
            {
                "path": ["bdir"],
                "fields": ["name"],
            },
        )
        self.assertFileListsEqual(res["files"], ["bdir/test.sh", "bdir/noexec.sh"])
