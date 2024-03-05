# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


from watchman.integration.lib import WatchmanInstance, WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestBulkStat(WatchmanTestCase.WatchmanTestCase):
    def test_bulkstat_on(self) -> None:
        config = {"_use_bulkstat": True}
        with WatchmanInstance.Instance(config=config) as inst:
            inst.start()
            self.getClient(inst, replace_cached=True)

            root = self.mkdtemp()
            # pyre-fixme[16]: `TestBulkStat` has no attribute `client`.
            self.client.query("watch", root)

            self.touchRelative(root, "foo")
            self.touchRelative(root, "bar")
            self.assertFileList(root, ["foo", "bar"])

    def test_bulkstat_off(self) -> None:
        config = {"_use_bulkstat": False}
        with WatchmanInstance.Instance(config=config) as inst:
            inst.start()
            self.getClient(inst, replace_cached=True)

            root = self.mkdtemp()
            # pyre-fixme[16]: `TestBulkStat` has no attribute `client`.
            self.client.query("watch", root)

            self.touchRelative(root, "foo")
            self.touchRelative(root, "bar")
            self.assertFileList(root, ["foo", "bar"])
