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

    # Create multiple nested directories to test the relative_root option
    repo.write_file("foo.c", "1\n")
    repo.write_file("subdir/bar.c", "1\n")
    repo.write_file("subdir/bar.h", "1\n")
    repo.write_file("subdir/subdir2/baz.c", "1\n")
    repo.write_file("subdir/subdir2/baz.h", "1\n")
    repo.write_file("subdir/subdir2/subdir3/baz.c", "1\n")

    repo.commit("initial commit.")


class TestEdenQuery(WatchmanEdenTestCase.WatchmanEdenTestCase):
    def test_simple_suffix(self) -> None:
        root = self.makeEdenMount(populate)
        self.watchmanCommand("watch", root)

        # Test each permutation of relative root
        relative_roots = ["", "subdir", "subdir/subdir2", "subdir/subdir2/subdir3"]
        expected_output = [
            [
                "foo.c",
                "subdir/bar.c",
                "subdir/subdir2/baz.c",
                "subdir/subdir2/subdir3/baz.c",
            ],
            [
                "bar.c",
                "subdir2/baz.c",
                "subdir2/subdir3/baz.c",
            ],
            [
                "baz.c",
                "subdir3/baz.c",
            ],
            [
                "baz.c",
            ],
        ]
        for relative_root, output in zip(relative_roots, expected_output):
            # Test Simple suffix eval
            self.assertFileListsEqual(
                self.watchmanCommand(
                    "query",
                    root,
                    {
                        "relative_root": relative_root,
                        "expression": ["allof", ["type", "f"], ["suffix", ["c"]]],
                        "fields": ["name"],
                    },
                )["files"],
                output,
            )

            # Check that it is the same as normal suffix eval
            self.assertFileListsEqual(
                self.watchmanCommand(
                    "query",
                    root,
                    {
                        "relative_root": relative_root,
                        "expression": [
                            "allof",
                            # Adding a true expression causes watchman to
                            # evaluate this normally instead of as a simple suffix
                            ["true"],
                            ["type", "f"],
                            ["suffix", ["c"]],
                        ],
                        "fields": ["name"],
                    },
                )["files"],
                output,
            )
