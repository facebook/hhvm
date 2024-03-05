# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os

from facebook.eden.constants import STATS_MOUNTS_STATS
from facebook.eden.ttypes import GetStatInfoParams
from watchman.integration.lib import WatchmanEdenTestCase


class TestEdenWatchParent(WatchmanEdenTestCase.WatchmanEdenTestCase):
    def test_eden_watch_parent(self) -> None:
        def populate(repo):
            repo.write_file("adir/file", "foo!\n")
            repo.commit("initial commit.")

        def get_loaded_count() -> int:
            with self.eden.get_thrift_client_legacy() as client:
                stats = client.getStatInfo(
                    GetStatInfoParams(statsMask=STATS_MOUNTS_STATS)
                )
            mountPointInfo = stats.mountPointInfo
            if mountPointInfo is None:
                raise Exception("stats.mountPointInfo is not set")
            self.assertEqual(len(mountPointInfo), 1)
            for mountPath in mountPointInfo:
                info = mountPointInfo[mountPath]
                return info.loadedFileCount + info.loadedTreeCount
            return 0

        root = self.makeEdenMount(populate)

        before_loaded = get_loaded_count()
        self.watchmanCommand("watch", os.path.dirname(root))
        after_loaded = get_loaded_count()

        self.assertEqual(before_loaded, after_loaded)
