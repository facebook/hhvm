# vim:ts=4:sw=4:et:
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# pyre-unsafe


import os
import random
import stat
import string
import sys
import time
import unittest

import pywatchman
from watchman.integration.lib import WatchmanInstance


try:
    import grp
except ImportError:
    # Windows
    pass


@unittest.skipIf(
    os.name == "nt" or sys.platform == "darwin" or os.geteuid() == 0,
    "win or root or bad ldap",
)
class TestSockPerms(unittest.TestCase):
    def _new_instance(self, config):
        start_timeout = 20
        return WatchmanInstance.InstanceWithStateDir(
            config=config, start_timeout=start_timeout
        )

    def _get_custom_gid(self):
        # This is a bit hard to do: we need to find a group the user is a member
        # of that's not the effective or real gid. If there are none then we
        # must skip.
        groups = os.getgroups()
        for gid in groups:
            if gid != os.getgid() and gid != os.getegid():
                return gid
        self.skipTest("no usable groups found")

    def _get_non_member_group(self):
        """Get a group tuple that this user is not a member of."""
        user_groups = set(os.getgroups())
        for group in grp.getgrall():
            if group.gr_gid not in user_groups:
                return group
        self.skipTest("no usable groups found")

    def waitFor(self, cond, timeout: float = 20):
        deadline = time.time() + timeout
        res = None
        while time.time() < deadline:
            try:
                res = cond()
                if res:
                    return [True, res]
            except Exception:
                pass
            time.sleep(0.03)
        return [False, res]

    def assertWaitFor(
        self, cond, timeout: int = 60, message=None, get_debug_output=None
    ):
        status, res = self.waitFor(cond, timeout)
        if status:
            return res
        if message is None:
            message = "%s was not met in %s seconds: %s" % (cond, timeout, res)
        if get_debug_output is not None:
            message += "\ndebug output:\n%s\nend debug output\n" % get_debug_output()
        self.fail(message)

    def test_too_open_user_dir(self) -> None:
        instance = self._new_instance({})
        os.makedirs(instance.user_dir)
        os.chmod(instance.user_dir, 0o777)
        with self.assertRaises(pywatchman.SocketConnectError) as ctx:
            instance.start()
        self.assertEqual(ctx.exception.sockpath, instance.getSockPath().unix_domain)

        wanted = "the permissions on %s allow others to write to it" % (
            instance.user_dir
        )
        self.assertWaitFor(
            lambda: wanted in instance.getCLILogContents(),
            get_debug_output=lambda: instance.getCLILogContents(),
        )

    def test_invalid_sock_group(self) -> None:
        # create a random group name
        while True:
            group_name = "".join(
                random.choice(string.ascii_lowercase) for _ in range(8)
            )
            try:
                grp.getgrnam(group_name)
            except KeyError:
                break

        instance = self._new_instance({"sock_group": group_name})
        with self.assertRaises(pywatchman.SocketConnectError) as ctx:
            instance.start()
        self.assertEqual(ctx.exception.sockpath, instance.getSockPath().unix_domain)
        # This is the error we expect to find
        wanted = "group '%s' does not exist" % group_name
        # But if the site uses LDAP or YP/NIS or other similar technology for
        # their password database then we might experience other infra flakeyness
        # so we allow for the alternative error case to be present and consider
        # it a pass.
        we_love_ldap = "getting gid for '%s' failed:" % group_name
        self.assertWaitFor(
            lambda: (wanted in instance.getCLILogContents())
            or (we_love_ldap in instance.getCLILogContents()),
            get_debug_output=lambda: str(ctx.exception)
            + "\n"
            + instance.getCLILogContents(),
        )

    def test_user_not_in_sock_group(self) -> None:
        group = self._get_non_member_group()
        instance = self._new_instance({"sock_group": group.gr_name})
        with self.assertRaises(pywatchman.SocketConnectError) as ctx:
            instance.start()
        self.assertEqual(ctx.exception.sockpath, instance.getSockPath().unix_domain)
        wanted = "setting up group '%s' failed" % group.gr_name
        self.assertWaitFor(
            lambda: wanted in instance.getCLILogContents(),
            get_debug_output=lambda: instance.getCLILogContents(),
        )

    def test_default_sock_group(self) -> None:
        # By default the socket group should be the effective gid of the process
        gid = os.getegid()
        instance = self._new_instance({})
        instance.start()
        instance.stop()

        self.assertFileGID(instance.user_dir, gid)
        self.assertFileGID(instance.sock_file, gid)

    def test_custom_sock_group(self) -> None:
        gid = self._get_custom_gid()
        group = grp.getgrgid(gid)
        instance = self._new_instance({"sock_group": group.gr_name})
        instance.start()
        instance.stop()

        self.assertFileGID(instance.user_dir, gid)
        self.assertFileGID(instance.sock_file, gid)

    def test_user_previously_in_sock_group(self) -> None:
        """This tests the case where a user was previously in sock_group
        (so Watchman created the directory with that group), but no longer is
        (so the socket is created with a different group)."""
        # Since it's hard to drop a group from a process without being
        # superuser, fake it. Use a private testing-only config option to set
        # up separate groups for the directory and the file.
        gid = self._get_custom_gid()
        group = grp.getgrgid(gid)
        non_member_group = self._get_non_member_group()
        # Need to wait for the server to come up here, can't use
        # expect_success=False.
        instance = self._new_instance(
            {"sock_group": group.gr_name, "__sock_file_group": non_member_group.gr_name}
        )
        with self.assertRaises(pywatchman.SocketConnectError):
            instance.start()

        wanted = (
            "for socket '%s', gid %d doesn't match expected gid %d "
            "(group name %s)."
            % (
                instance.getSockPath().unix_domain,
                gid,
                non_member_group.gr_gid,
                non_member_group.gr_name,
            )
        )
        self.assertWaitFor(lambda: wanted in instance.getServerLogContents())

    def test_invalid_sock_access(self) -> None:
        instance = self._new_instance({"sock_access": "bogus"})
        with self.assertRaises(pywatchman.SocketConnectError) as ctx:
            instance.start()
        self.assertEqual(ctx.exception.sockpath, instance.getSockPath().unix_domain)
        wanted = "Expected config value sock_access to be an object"
        self.assertWaitFor(
            lambda: wanted in instance.getCLILogContents(),
            get_debug_output=lambda: instance.getCLILogContents(),
        )

        instance = self._new_instance({"sock_access": {"group": "oui"}})
        with self.assertRaises(pywatchman.SocketConnectError) as ctx:
            instance.start()
        self.assertEqual(ctx.exception.sockpath, instance.getSockPath().unix_domain)
        wanted = "Expected config value sock_access.group to be a boolean"
        self.assertWaitFor(
            lambda: wanted in instance.getCLILogContents(),
            get_debug_output=lambda: instance.getCLILogContents(),
        )

    def test_default_sock_access(self) -> None:
        instance = self._new_instance({})
        instance.start()
        instance.stop()

        self.assertFileMode(instance.user_dir, 0o700 | stat.S_ISGID)
        self.assertFileMode(instance.sock_file, 0o600)

    def test_custom_sock_access_group(self) -> None:
        instance = self._new_instance({"sock_access": {"group": True}})
        instance.start()
        instance.stop()

        self.assertFileMode(instance.user_dir, 0o750 | stat.S_ISGID)
        self.assertFileMode(instance.sock_file, 0o660)

    def test_custom_sock_access_others(self) -> None:
        instance = self._new_instance({"sock_access": {"group": True, "others": True}})
        instance.start()
        instance.stop()

        self.assertFileMode(instance.user_dir, 0o755 | stat.S_ISGID)
        self.assertFileMode(instance.sock_file, 0o666)

    def test_sock_access_upgrade(self) -> None:
        instance = self._new_instance({"sock_access": {"group": True, "others": True}})
        os.makedirs(instance.user_dir)
        os.chmod(instance.user_dir, 0o700)
        instance.start()
        instance.stop()

        self.assertFileMode(instance.user_dir, 0o755 | stat.S_ISGID)
        self.assertFileMode(instance.sock_file, 0o666)

    def test_sock_access_downgrade(self) -> None:
        instance = self._new_instance({"sock_access": {"group": True}})
        os.makedirs(instance.user_dir)
        os.chmod(instance.user_dir, 0o755 | stat.S_ISGID)
        instance.start()
        instance.stop()

        self.assertFileMode(instance.user_dir, 0o750 | stat.S_ISGID)
        self.assertFileMode(instance.sock_file, 0o660)

    def test_sock_access_group_change(self) -> None:
        gid = self._get_custom_gid()
        group = grp.getgrgid(gid)
        instance = self._new_instance({"sock_group": group.gr_name})
        os.makedirs(instance.user_dir)
        # ensure that a different group is set
        os.chown(instance.user_dir, -1, os.getegid())
        instance.start()
        instance.stop()

        self.assertFileGID(instance.user_dir, gid)
        self.assertFileGID(instance.sock_file, gid)

    def assertFileMode(self, f, mode) -> None:
        st = os.lstat(f)
        self.assertEqual(stat.S_IMODE(st.st_mode), mode)

    def assertFileGID(self, f, gid) -> None:
        st = os.lstat(f)
        self.assertEqual(st.st_gid, gid)
