# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os
import os.path

from watchman.integration.lib import WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestBigFile(WatchmanTestCase.WatchmanTestCase):
    def test_big_file(self) -> None:
        root = self.mkdtemp()

        def check(file_size):
            with open(os.path.join(root, "big"), "w") as f:
                f.truncate(file_size)

            self.watchmanCommand("watch", root)

            result = self.watchmanCommand(
                "query",
                root,
                {
                    "fields": [
                        "name",
                        "size",
                    ],
                },
            )

            self.assertEqual(len(result["files"]), 1)

            file = result["files"][0]
            self.assertEqual(file["name"], "big")
            self.assertEqual(file["size"], file_size)

        # This size overflows int32
        check(2**31)

        # This size overflows uint32
        check(2**32)
