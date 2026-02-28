# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import pywatchman
from watchman.integration.lib import WatchmanInstance, WatchmanTestCase


@WatchmanTestCase.expand_matrix
class TestIllegalFSType(WatchmanTestCase.WatchmanTestCase):
    def test_Illegal(self) -> None:
        config = {
            "illegal_fstypes": [
                # This should include any/all fs types. If this test fails on
                # your platform, look in /tmp/watchman-test.log for a line like:
                # "path /var/tmp/a3osdzvzqnco0sok is on filesystem type zfs"
                # then add the type name to this list, in sorted order
                "NTFS",
                "apfs",
                "btrfs",
                "cifs",
                "ext2",
                "ext3",
                "ext4",
                "fuse",
                "hfs",
                "msdos",
                "nfs",
                "smb",
                "tmpfs",
                "ufs",
                "unknown",
                "xfs",
                "zfs",
            ],
            "illegal_fstypes_advice": "just cos",
        }

        with WatchmanInstance.Instance(config=config) as inst:
            inst.start()
            client = self.getClient(inst)

            d = self.mkdtemp()
            with self.assertRaises(pywatchman.WatchmanError) as ctx:
                client.query("watch", d)
            self.assertIn(
                (
                    "filesystem and is disallowed by global config"
                    + " illegal_fstypes: just cos"
                ),
                str(ctx.exception),
            )
