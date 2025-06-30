# pyre-strict

"""

This file tests the Edenfs_watcher module.

Note that like all the other tests in this directory, it is run *without* the
run_as_bundle configuration flag on its python_unittest definition in TARGETS.
This means that the distinction beween setUp vs setUpClass (and tearDown vs
tearDownClass) is insignificant. Therefore, we just follow CommonTestDriver to
decide what setup and teardown code goes where.
"""

import os
import re

import shutil
import subprocess
import sys
import time
from pathlib import Path
from typing import ClassVar, List, Optional

import hphp.hack.test.integration.common_tests as common_tests
from eden.integration.lib.edenclient import EdenFS
from hphp.hack.test.integration.common_tests import CommonTestDriver
from hphp.hack.test.integration.hh_paths import hh_server

from watchman.integration.lib import WatchmanInstance

# Matches the message that is logged by ServerNotifier.get_changes_sync and
# get_changes_async when it saw a non-zero number of changes. We look at the server log
# to test if the server did or didn't pick up some changes.
#
# While this approach is very brittle, we do have test_server_notifier_re in place to
# check that the RE still works.
SERVER_NOTIFIER_RE = r"ServerNotifier\.get_changes_(sync|async) got (\d+) changes"

# Used to debug this test suite.
# If enabled, we launch hh_server in the foreground and
# redirect its stdout and stderr into this process'.
# Make sure that this is never enabled by default, it disabled some testing
# assertions!
DEBUG_EDENFS_WATCHER_TEST_HH_SERVER_FOREGROUND: bool = (
    os.environ.get("DEBUG_EDENFS_WATCHER_TEST_HH_SERVER_FOREGROUND") is not None
)


# Contents of hh.conf used in these tests Currently, enabling the EdenFS file
# watcher actually runs the old Watchman code under the hood. Thus, we configure
# Watchman the same way it is in /etc/hh.conf these days.
def makeWatchmanEdenHhConf(watchman_socket_path: str) -> str:
    config = """
use_watchman = true
watchman_debug_logging = false
watchman_subscribe_v2 = true
watchman_sync_directory = .hg
edenfs_file_watcher_enabled=true
min_log_level=Debug
"""

    config = config + f"watchman_sockname={watchman_socket_path}\n"
    return config


def runAndCheckSetupCommand(args: List[str]) -> str:
    # Not using CommonTestDriver.proc_call here, because it relies on some
    # global state seems to be used for commands denoting the actual test.
    proc = subprocess.run(args, capture_output=True, text=True)
    if proc.returncode != 0:
        print("Failed setup command stdout", proc.stdout)
        print("Failed setup command stderr", proc.stderr)
        proc.check_returncode()
    return proc.stdout


def createEdenInstance(eden_base_dir: str) -> EdenFS:
    """Creates an EdenFS instance, and starts it.

    The instance is independent from the one powering for example
    ~/fbsource, and stores all of its state and metadata in eden_base_dir.
    """

    instance = EdenFS(Path(eden_base_dir))
    instance.start()
    return instance


def createWatchmanInstance() -> WatchmanInstance.Instance:
    """Creates a Watchman instance, and starts it.

    The instance is independent from the one powering for example
    ~/fbsource, and stores all of its state and metadata in a temp dir
    that it manages.
    """

    instance = WatchmanInstance.Instance()
    instance.start()
    return instance


def mountEden(eden_instance: EdenFS, hg_repo: str, eden_mount_point: str) -> None:
    eden_instance.clone(hg_repo, eden_mount_point)


def unmountEden(eden_instance: EdenFS, eden_mount_point: str) -> None:
    eden_instance.remove(eden_mount_point)


def assertCurrentServerLogContains(driver: CommonTestDriver, needle: str) -> None:
    if DEBUG_EDENFS_WATCHER_TEST_HH_SERVER_FOREGROUND:
        # We don't have access to any logs
        return

    server_log = driver.get_all_logs(driver.repo_dir).current_server_log
    contains = needle in server_log
    if not contains:
        print("Server log:")
        print(server_log)
    driver.assertTrue(contains)


def assertAnyServerLogContains(driver: CommonTestDriver, needle: str) -> None:
    server_log = driver.get_all_logs(driver.repo_dir).all_server_logs
    contains = needle in server_log
    if not contains:
        print("Server log:")
        print(server_log)
    driver.assertTrue(contains)


def assertMonitorLogContains(driver: CommonTestDriver, needle: str) -> None:
    if DEBUG_EDENFS_WATCHER_TEST_HH_SERVER_FOREGROUND:
        # We don't have access to any logs
        return
    monitor_log = driver.get_all_logs(driver.repo_dir).all_monitor_logs

    driver.assertTrue(needle in monitor_log)


def assertEdenFsWatcherInitialized(driver: CommonTestDriver) -> None:
    """Checks that an Edenfs_watcher instance was initialized

    Given that we may fall back to using Watchman if Edenfs_watcher
    initialization fails, this prevents tests from passing even though the
    EdenFS watcher wasn't actually used.
    """
    assertCurrentServerLogContains(driver, "[edenfs_watcher][init] finished init")


def assertServerNotCrashed(driver: CommonTestDriver) -> None:
    if DEBUG_EDENFS_WATCHER_TEST_HH_SERVER_FOREGROUND:
        # We don't have access to any logs
        return
    monitor_log = driver.get_all_logs(driver.repo_dir).all_monitor_logs

    driver.assertFalse("Exit_status.Edenfs_watcher_failed" in monitor_log)


def assertServerNotifierChangesYes(driver: CommonTestDriver) -> None:
    if DEBUG_EDENFS_WATCHER_TEST_HH_SERVER_FOREGROUND:
        # We don't have access to any logs
        return
    server_log = driver.get_all_logs(driver.repo_dir).current_server_log
    matches = re.search(SERVER_NOTIFIER_RE, server_log)
    driver.assertTrue(matches is not None)


def assertServerNotifierChangesNo(driver: CommonTestDriver) -> None:
    if DEBUG_EDENFS_WATCHER_TEST_HH_SERVER_FOREGROUND:
        # We don't have access to any logs
        return
    # Not that in the "No" version, we check all logs
    server_log = driver.get_all_logs(driver.repo_dir).all_server_logs
    matches = re.search(SERVER_NOTIFIER_RE, server_log)
    driver.assertTrue(matches is None)


class EdenfsWatcherTestDriver(common_tests.CommonTestDriver):
    """Driver compatible with CommonTestDriver, but creating an Eden-backed repo.

    Concretely, this means that all the helpers in CommonTestDriver must
    still work. All we need to do is making sure that the repo pointed at by
    the path in `repo_dir` gets initialized in a different way, by mounting an hg
    commit using Eden.
    """

    # This is the root of the hg repo that we will create
    hg_repo_root: ClassVar[str]

    # This is where we will mount hg_repo_root to. cls.repo_dir will be the same or a subfolder of this.
    eden_mount_point: ClassVar[str]

    eden_instance: ClassVar[EdenFS]
    watchman_instance: ClassVar[WatchmanInstance.Instance]

    # This is a commit in the testing repo at which point we only added the .hhconfig and hh.conf files
    clean_slate_commit: ClassVar[str]

    @classmethod
    def setUpClassImpl(
        cls, template_repo: str, repo_subdirectory_path: Optional[str]
    ) -> None:
        print("running EdenfsWatcherTestDriver.setUpClassImpl")

        # We need to call CommonTestDriver.setUpClass, but make the class
        # variable changes visible to our cls object
        super(EdenfsWatcherTestDriver, cls).setUpClass(template_repo)

        # This is where the Eden testing instance will put all of its state and files
        eden_base_dir = os.path.join(cls.base_tmp_dir, "eden_base")
        os.mkdir(eden_base_dir)
        cls.eden_instance = createEdenInstance(eden_base_dir)

        cls.watchman_instance = createWatchmanInstance()
        watchman_socket_path = cls.watchman_instance.getUnixSockPath()

        cls.hg_repo_root = os.path.join(cls.base_tmp_dir, "hg_repo")

        # We will mount the hg repo here ...
        cls.eden_mount_point = os.path.join(cls.base_tmp_dir, "repo")

        # Subfolder inside hg_repo where we actually put testing files:
        template_repo_destination = (
            os.path.join(cls.hg_repo_root, repo_subdirectory_path)
            if repo_subdirectory_path
            else cls.hg_repo_root
        )

        # This is the main folder testing folder, which we point hh at. Note that we
        # keep the name `repo_dir` in order to be consistent with how CommonTestDriver
        # uses the term. However, this is not necessarily the same as the root of the
        # repository! If repo_subdirectory_path is set, then the "repo" we test on is a
        # subfolder of the Eden mount point.
        cls.repo_dir = (
            os.path.join(cls.eden_mount_point, repo_subdirectory_path)
            if repo_subdirectory_path
            else cls.eden_mount_point
        )

        shutil.copytree(template_repo, template_repo_destination)

        # This file can go wherever, as long as HH_LOCALCONF_PATH points to
        # it. Some other tests expect it to be inside the folder cls.repo_dir.
        # So let's put it in the place that after mounting will end up at cls.repo_dir
        with open(os.path.join(template_repo_destination, "hh.conf"), "w") as f:
            f.write(makeWatchmanEdenHhConf(watchman_socket_path))

        # CommonTestDriver.setUpClass already did this, but we changed the value of repo_dir
        cls.test_env["HH_LOCALCONF_PATH"] = cls.repo_dir

        runAndCheckSetupCommand(["hg", "init", cls.hg_repo_root])
        runAndCheckSetupCommand(
            [
                "hg",
                "add",
                "-R",
                cls.hg_repo_root,
                os.path.join(template_repo_destination, ".hhconfig"),
                os.path.join(template_repo_destination, "hh.conf"),
            ]
        )
        # Create the clean slate commit for those tests that don't want the full template repo
        runAndCheckSetupCommand(
            [
                "hg",
                "commit",
                "--message",
                "clean slate commit",
                "-R",
                cls.hg_repo_root,
            ]
        )

        cls.clean_slate_commit = runAndCheckSetupCommand(
            [
                "hg",
                "whereami",
                "-R",
                cls.hg_repo_root,
            ]
        )

        # Commit everything else. This is the commit that all tests start on.
        runAndCheckSetupCommand(
            [
                "hg",
                "commit",
                "--addremove",
                "--message",
                "test repo finished",
                "-R",
                cls.hg_repo_root,
            ]
        )

    @classmethod
    def setUpClass(cls, template_repo: str) -> None:
        # This driver creates a test setup where we run hh on the Eden mount point
        # directly.
        cls.setUpClassImpl(template_repo, None)

    @classmethod
    def tearDownClass(cls) -> None:
        print("running EdenfsWatcherDriver.tearDownClass")

        cls.eden_instance.cleanup()
        cls.watchman_instance.stop()
        super(EdenfsWatcherTestDriver, cls).tearDownClass()

    def setUp(self) -> None:
        print("running EdenfsWatcherDriver.setUp")

        # For hygiene, we (re-)mount our Eden repo for each individual test
        mountEden(self.eden_instance, self.hg_repo_root, self.eden_mount_point)

    def tearDown(self) -> None:
        print("running EdenfsWatcherDriver.tearDown")

        # It's ugly to do this here, since this isn't really tear-down code,
        # but core testing logic, but we need to get this into every test
        assertEdenFsWatcherInitialized(self)

        # Similarly, another check: hh_server's resiliance is sometimes hiding bugs:
        # We may accidentally crash the Edenfs_watcher, which can take the server down.
        # But then the monitor restarts it and it re-checks the testing repo.
        # From the outside, that looks very similar to the server correctly
        # picking up a change we've made to the testing repo!
        assertServerNotCrashed(self)

        # For hygiene, we (re-)mount our Eden repo for each individual test.
        # Note that we must stop the server before unmounting the Eden repo that it works on.
        # 3 retries is the value used in CommonTestDriver.tearDown
        self.stop_hh_server(retries=3)
        unmountEden(self.eden_instance, self.eden_mount_point)

    def start_hh_server(
        self,
        changed_files: Optional[List[str]] = None,
        saved_state_path: Optional[str] = None,
        args: Optional[List[str]] = None,
    ) -> None:
        args = args or []
        if DEBUG_EDENFS_WATCHER_TEST_HH_SERVER_FOREGROUND:
            cmd = [hh_server, "--max-procs", "2", self.repo_dir] + args
            subprocess.Popen(
                cmd,
                env=self.test_env,
                stdout=sys.stderr,
                stderr=sys.stderr,
                universal_newlines=True,
            )
            self.wait_until_server_ready()
        else:
            super().start_hh_server(changed_files, saved_state_path, args)

    def commitAllChanges(
        self, message: str = "test commit", allow_empty: bool = True
    ) -> str:
        (_, _, retcode) = self.proc_call(["hg", "add", "-R", self.eden_mount_point])
        self.assertEqual(retcode, 0)
        (_, _, retcode) = self.proc_call(
            ["hg", "commit", "--addremove", "-m", message, "-R", self.eden_mount_point]
        )
        if allow_empty:
            # hg commit --help states that 1 is the exit code used when nothing changed
            self.assertTrue(retcode == 0 or retcode == 1)
        else:
            self.assertEqual(retcode, 0)

        # Get the revision hash of the commit we just created
        (stdout, _, retcode) = self.proc_call(
            ["hg", "whereami", "-R", self.eden_mount_point]
        )
        self.assertEqual(retcode, 0)
        rev_hash = stdout.strip()
        return rev_hash

    def gotoRev(self, rev: str, merge: bool = False) -> None:
        args = ["hg", "goto", rev, "-R", self.eden_mount_point]
        if merge:
            args.append("--merge")
        (_, _, retcode) = self.proc_call(args)
        self.assertEqual(retcode, 0)

    @classmethod
    def isMountPointIgnored(cls) -> bool:
        """Returns True if cls.eden_mount_point is not watched directly

        We always have one of the following:
        1. cls.repo_dir is identical to cls.eden_mount_point or
        2. cls.repo_dir is different from, but an ancestor of, cls.eden_mount_point

        In the second case, changes that happen within cls.eden_mount_point, but outside
        cls.repo_dir, are ignored.

        Note that this check is identical to asking if a test is running with
        EdenfsWatcherTestDriver or EdenfsWatcherNonMountPointRepoTests._Driver
        """

        return Path(cls.eden_mount_point).absolute() != Path(cls.repo_dir).absolute()

    @classmethod
    def createNonHackFile(cls, path: str, base: Optional[str] = None) -> None:
        """Creates a file that won't type-check"""

        base = base or cls.repo_dir
        full_path = os.path.join(base, path)
        os.makedirs(os.path.dirname(full_path), exist_ok=True)

        with open(full_path, "w") as f:
            f.write("Not a valid hack file")

    @classmethod
    def createIgnoredFiles(cls) -> None:
        cls.createNonHackFile("randomfileweshouldignore")
        cls.createNonHackFile("almost.hhconfig")
        cls.createNonHackFile(".php")
        cls.createNonHackFile("folder.php/ignored_file.txt")
        cls.createNonHackFile(".hg/ignored.php", base=cls.eden_mount_point)
        if cls.isMountPointIgnored():
            cls.createNonHackFile("ignored.php", base=cls.eden_mount_point)


class EdenfsWatcherTests(common_tests.CommonTests):
    "Runs integration tests on an Eden-backed testing repo with edenfs_file_watcher_enabled set"

    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return EdenfsWatcherTestDriver()

    # Need to duplicate this here from TestCase to make pyre happy
    _test_driver: Optional[EdenfsWatcherTestDriver] = None

    # Need to duplicate this here from TestCase to make pyre happy
    @property
    def test_driver(self) -> EdenfsWatcherTestDriver:
        test_driver = self._test_driver
        assert test_driver is not None
        return test_driver

    def test_server_notifier_re(self) -> None:
        "This just makes sure that SERVER_NOTIFIER_RE still matches the messages that ServerNotifier logs on file changes"

        self.test_driver.start_hh_server()

        with open(os.path.join(self.test_driver.repo_dir, "test_file.php"), "w") as f:
            f.write("<?hh")

        self.test_driver.check_cmd(["No errors!"])
        assertServerNotifierChangesYes(self.test_driver)

    def test_interrupt(self) -> None:
        # Unlike the other users of CommonTests, we set up our own Watchman instance. We
        # don't want to mess up the root detectiong by placing an empty .watchmanconfig
        # in cls.repo_dir. Thus, this test is like CommonTests.test_interrupt, but does
        # not to create a .watchmanconfig file.

        with open(os.path.join(self.test_driver.repo_dir, "hh.conf"), "a") as f:
            f.write("interrupt_on_watchman = true\n" + "interrupt_on_client = true\n")

        self.test_driver.start_hh_server()
        self.test_driver.start_hh_loop_forever_assert_timeout()
        self.test_driver.check_cmd(
            ["string"], options=["--type-at-pos", "{root}foo_3.php:11:14"]
        )
        self.test_driver.stop_hh_loop_forever()

    def test_sync_queries(self) -> None:
        iterations = 50

        self.test_driver.start_hh_server()

        # We make some changes, followed by a type-at-pos query (which is sync, in
        # contrast to ordinary check queries). Note that we always make two changes, to
        # make sure that we would catch the following, bad scenario: The type-checker
        # may pick up the first change and perform a re-check (independent from the
        # following query sent by the client) If the query from the client then comes in
        # before the file watching service picks up the second change, we may answer the
        # client's query without having processed the second change at all.
        # This should not happen with a sync query.
        for _ in range(iterations):
            with open(os.path.join(self.test_driver.repo_dir, "sync_1.php"), "w") as f:
                f.write(
                    """<?hh

            function sync_g(): int {
                $res = sync_f();
                return $res;
            }
            """
                )
            with open(os.path.join(self.test_driver.repo_dir, "sync_2.php"), "w") as f:
                f.write(
                    """<?hh

            function sync_f(): int {
                return 3;
            }
            """
                )

            self.test_driver.check_cmd(
                ["int"], options=["--type-at-pos", "{root}sync_1.php:5:24"]
            )
            print("finished iteration")

            with open(os.path.join(self.test_driver.repo_dir, "sync_1.php"), "w") as f:
                f.write(
                    """<?hh

            function sync_g(): string {
                $res = sync_f();
                return $res;
            }
            """
                )
            with open(os.path.join(self.test_driver.repo_dir, "sync_2.php"), "w") as f:
                f.write(
                    """<?hh

            function sync_f(): string {
                return "123";
            }
            """
                )
            self.test_driver.check_cmd(
                ["string"], options=["--type-at-pos", "{root}sync_1.php:5:24"]
            )

    def test_hg_update_basic(self) -> None:
        self.test_driver.start_hh_server()

        # Create a file that uses hg_test_fun1
        hg_update2 = os.path.join(self.test_driver.repo_dir, "hg_update2.php")
        with open(hg_update2, "w") as f:
            f.write(
                """<?hh

function hg_test_fun2(): string {
    return hg_test_fun1();
}
"""
            )

        only_hg_update2_rev = self.test_driver.commitAllChanges()

        # Create a file that defines hg_test_fun1
        hg_update1 = os.path.join(self.test_driver.repo_dir, "hg_update1.php")
        with open(hg_update1, "w") as f:
            f.write(
                """<?hh

function hg_test_fun1(): string {
    return "Hello";
}
"""
            )

        self.test_driver.check_cmd(["No errors!"])

        all_present_rev = self.test_driver.commitAllChanges()

        # this effectively deletes hg_update1, so hg_test_fun1 is gone
        self.test_driver.gotoRev(only_hg_update2_rev)
        self.test_driver.check_cmd(
            [
                "ERROR: {root}hg_update2.php:4:12,23: Unbound name (typing): `hg_test_fun1` (Typing[4107])",
                "ERROR: {root}hg_update2.php:4:12,23: Unbound name: `hg_test_fun1` (a global function) (Naming[2049])",
            ]
        )

        # Let's bring the file back
        self.test_driver.gotoRev(all_present_rev)
        self.test_driver.check_cmd(["No errors!"])

    def test_hg_update_dirty(self) -> None:
        """Tests that we handle dirty repos correctly.

        When we switch commits from rev1 to rev2, the changes that Edenfs_watcher
        reports to hh_server are just the difference between the two revisions. However,
        if we have uncommitted changes, then this may not actually reflect how the
        working copy changes.
        """

        # Create a file with a type error
        hg_update_bad_file = os.path.join(
            self.test_driver.repo_dir, "hg_update_bad_file.php"
        )
        with open(hg_update_bad_file, "w") as f:
            f.write(
                """<?hh

function hg_test_fun(): string {
    return 3;
}
"""
            )

        added_bad_file = self.test_driver.commitAllChanges()

        os.remove(hg_update_bad_file)
        self.test_driver.start_hh_server()

        # Until T226510404 is fixed, this will actually cause a recheck.
        removed_bad_file = self.test_driver.commitAllChanges()
        self.test_driver.check_cmd(["No errors!"])

        # go back one commit, where the file was still there
        self.test_driver.gotoRev(added_bad_file)
        self.test_driver.check_cmd(
            [
                "ERROR: {root}hg_update_bad_file.php:4:12,12: Invalid return type (Typing[4110])",
                "  {root}hg_update_bad_file.php:3:25,30: Expected `string`",
                "  {root}hg_update_bad_file.php:4:12,12: But got `int`",
            ]
        )

        # let's remove the file ourselves ...
        os.remove(hg_update_bad_file)
        self.test_driver.check_cmd(["No errors!"])

        # ... and now remove it "again" by changing to the commit where it was removed.
        # In other words, we now report a change to hh_server about a file that was
        # already gone, and is still gone afterwards.
        # Need to set "--merge" here so that hg let's us do this.

        self.test_driver.gotoRev(removed_bad_file, merge=True)

        self.test_driver.check_cmd(["No errors!"])

    def test_folder_rename(self) -> None:
        self.test_driver.start_hh_server()

        folder_path = os.path.join(self.test_driver.repo_dir, "old_folder")
        os.makedirs(os.path.join(folder_path, "sub"), exist_ok=True)

        with open(os.path.join(folder_path, "sub", "rename_test.php"), "w") as f:
            f.write(
                """<?hh

function rename_sub_test_fun(): int {
    return "ill-typed";
}
"""
            )

        with open(os.path.join(folder_path, "rename_test.php"), "w") as f:
            f.write(
                """<?hh

function rename_test_fun(): int {
    return "ill-typed";
}
"""
            )

        self.test_driver.check_cmd(
            [
                "ERROR: {root}old_folder/rename_test.php:4:12,22: Invalid return type (Typing[4110])",
                "  {root}old_folder/rename_test.php:3:29,31: Expected `int`",
                "  {root}old_folder/rename_test.php:4:12,22: But got `string`",
                "ERROR: {root}old_folder/sub/rename_test.php:4:12,22: Invalid return type (Typing[4110])",
                "  {root}old_folder/sub/rename_test.php:3:33,35: Expected `int`",
                "  {root}old_folder/sub/rename_test.php:4:12,22: But got `string`",
            ]
        )

        # Rename the folder
        new_folder_path = os.path.join(self.test_driver.repo_dir, "new_folder")
        os.rename(folder_path, new_folder_path)

        self.test_driver.check_cmd(
            [
                "ERROR: {root}new_folder/rename_test.php:4:12,22: Invalid return type (Typing[4110])",
                "  {root}new_folder/rename_test.php:3:29,31: Expected `int`",
                "  {root}new_folder/rename_test.php:4:12,22: But got `string`",
                "ERROR: {root}new_folder/sub/rename_test.php:4:12,22: Invalid return type (Typing[4110])",
                "  {root}new_folder/sub/rename_test.php:3:33,35: Expected `int`",
                "  {root}new_folder/sub/rename_test.php:4:12,22: But got `string`",
            ]
        )

    def test_hhconfig_change(self) -> None:
        self.test_driver.start_hh_server()

        # Let's create a file that calls a deprecated function and tries to HH_FIXME
        # that away. With the current .hhconfig, that's not actually allowed.
        deprecated = os.path.join(self.test_driver.repo_dir, "deprecated.php")
        with open(deprecated, "a") as f:
            f.write("""<?hh
<<__Deprecated("deprecated")>>
function deprecated() : void {
}

function test_deprecated() : void {
    /* HH_FIXME[4128] */
    deprecated();
}
""")

        self.test_driver.check_cmd(
            [
                "ERROR: {root}deprecated.php:7:5,24: You cannot use `HH_FIXME` or `HH_IGNORE_ERROR` comments to suppress error 4128 (Typing[4128])",
                "ERROR: {root}deprecated.php:8:5,14: The function deprecated is deprecated: deprecated (Typing[4128])",
                "  {root}deprecated.php:3:10,19: Definition is here",
            ]
        )

        # Let's make HH_FIXME[4128] legal. There's an `allowed_fixme_codes_strict` line
        # in the mini repo's .hhconfig already, so we need to patch it :(
        hhconfig_path = os.path.join(self.test_driver.repo_dir, ".hhconfig")
        with open(hhconfig_path, "r") as f:
            hhconfig_content = f.read()

        lines = hhconfig_content.split("\n")
        found_allowed_fixme_line = False

        for i, line in enumerate(lines):
            if line.strip().startswith("allowed_fixme_codes_strict"):
                lines[i] = line + ", 4128"
                found_allowed_fixme_line = True
                break

        if not found_allowed_fixme_line:
            self.fail(
                "No line starting with 'allowed_fixme_codes_strict' found in .hhconfig"
            )

        with open(hhconfig_path, "w") as f:
            f.write("\n".join(lines))

        # We need to wait until the server is back up.
        # There is one annoyance: If we run `hh check` JUST in the moment that the
        # server notices the .hhconfig check and restarts, then `hh check` spits out an
        # error message complaining that the server disconnected unexpectedly (instead
        # of waiting until it's back).
        while True:
            time.sleep(1)
            (_, _, exit_code) = self.test_driver.run_check()
            if exit_code == 0:
                break

        # We've just made the HH_FIXME[4128] legal.
        self.test_driver.check_cmd(["No errors!"])

        # Let's double-check that we did indeed restart due to the .hhconfig change
        assertMonitorLogContains(
            self.test_driver,
            "Exit_status.Hhconfig_changed",
        )

    def test_filter_file_changes(self) -> None:
        self.test_driver.start_hh_server()

        # Let's check that changes to random files are not picked up:
        self.test_driver.createIgnoredFiles()

        assertServerNotifierChangesNo(self.test_driver)
        self.test_driver.check_cmd(["No errors!"])

        # Let's check that extensions other than php are included
        hhi_file = os.path.join(self.test_driver.repo_dir, "invalid_file.hhi")

        with open(hhi_file, "w") as f:
            f.write("not a valid file")

        self.test_driver.check_cmd(
            [
                "ERROR: {root}invalid_file.hhi:1:5,5: A semicolon `;` is expected here. (Parsing[1002])"
            ]
        )

    def test_hg_update_filter(self) -> None:
        """Tests that we correctly filter out changes we get due to changing commits"""

        # We use three commits:
        # rev0: baseline, state of testing repo set up by test harness
        # rev1: contains files that should be ignored
        # rev2: contains .hhconfig change
        # all three commits are built on top of each other

        rev0 = self.test_driver.commitAllChanges(allow_empty=True)
        self.test_driver.createIgnoredFiles()
        rev1 = self.test_driver.commitAllChanges()

        hhconfig_file = os.path.join(self.test_driver.repo_dir, ".hhconfig")
        with open(hhconfig_file, "a") as f:
            f.write("# a comment to change hhconfig")

        rev2 = self.test_driver.commitAllChanges()

        # We start the server on rev0, effectively hiding all changes we've made so far
        self.test_driver.gotoRev(rev0)
        self.test_driver.start_hh_server()

        # We go to rev1. The server shouldn't see any of these changes
        self.test_driver.gotoRev(rev1)
        self.test_driver.check_cmd(["No errors!"])
        assertServerNotifierChangesNo(self.test_driver)

        # We go to rev2. The server should see the hhconfig change and restart
        self.test_driver.gotoRev(rev2)
        time.sleep(2)  # give server some time to initiate restart
        self.test_driver.check_cmd(["No errors!"])  # wait until server ready again
        assertMonitorLogContains(
            self.test_driver,
            "Exit_status.Hhconfig_changed",
        )

    def test_filter_folder_rename(self) -> None:
        self.test_driver.start_hh_server()

        folder_path = os.path.join(self.test_driver.repo_dir, "old_folder")

        # create some ignored files
        self.test_driver.createNonHackFile("old_folder/randomfileweshouldignore")
        self.test_driver.createNonHackFile("old_folder/.php")
        self.test_driver.createNonHackFile("old_folder/folder.php/ignored_file.txt")

        self.test_driver.check_cmd(["No errors!"])
        assertServerNotifierChangesNo(self.test_driver)

        # Rename the folder
        new_folder_path = os.path.join(self.test_driver.repo_dir, "new_folder")
        os.rename(folder_path, new_folder_path)

        self.test_driver.check_cmd(["No errors!"])
        assertServerNotifierChangesNo(self.test_driver)

    def test_filter_alternation(self) -> None:
        """Alternates between making filtered and non-filtered changes

        This is a regression test for a bug where this pattern caused the notification mechanism
        to (errorenously) fail a consistency check.
        """

        self.test_driver.start_hh_server()

        for _ in range(5):
            self.test_driver.createNonHackFile("not_filtered_out.php")
            self.test_driver.createNonHackFile("filtered_out.txt")

            self.test_driver.check_cmd(
                [
                    "ERROR: {root}not_filtered_out.php:1:1,1: A .php file must begin with `<?hh`. (Parsing[1002])"
                ]
            )

    def test_get_all_files(self) -> None:
        # Let's go to the commit where only hh.conf and .hhconfig existed
        self.test_driver.gotoRev(self.test_driver.clean_slate_commit)

        # Let's double-check that we really just have the files we are expecting:
        hhconfig_count = 0
        hh_conf_count = 0
        for _root, dirs, files in os.walk(self.test_driver.repo_dir):
            # Modifying dirs is the documented way to prevent os.walk from traversing into a subdirectory
            if ".hg" in dirs:
                dirs.remove(".hg")
            if ".eden" in dirs:
                dirs.remove(".eden")

            for file in files:
                if file == ".hhconfig":
                    hhconfig_count += 1
                elif file == "hh.conf":
                    hh_conf_count += 1
                else:
                    self.fail("unexpected file in repo at clean slate commit")

        if hhconfig_count != 1 or hh_conf_count != 1:
            self.fail("unexpected file in repo at clean slate commit")

        # Let's add a bunch of files, all of which should be ignored
        self.test_driver.createIgnoredFiles()

        # Let's create some files that should be picked up:
        self.test_driver.createNonHackFile("file.php")
        self.test_driver.createNonHackFile("subfolder/file.php")
        self.test_driver.createNonHackFile("subfolder/.hg/file.php")

        # Let's check that extensions other than .php are included
        hhi_file = os.path.join(self.test_driver.repo_dir, "invalid_file.hhi")
        with open(hhi_file, "w") as f:
            f.write("not a valid hhi file")

        # The .hhconfig file + the 4 non-ignored files we created above, none of the files added
        # by createIgnoredFiles
        exected_file_count = 5
        self.test_driver.start_hh_server()
        self.test_driver.check_cmd(
            [
                "ERROR: {root}file.php:1:1,1: A .php file must begin with `<?hh`. (Parsing[1002])",
                "ERROR: {root}invalid_file.hhi:1:5,5: A semicolon `;` is expected here. (Parsing[1002])",
                "ERROR: {root}subfolder/.hg/file.php:1:1,1: A .php file must begin with `<?hh`. (Parsing[1002])",
                "ERROR: {root}subfolder/file.php:1:1,1: A .php file must begin with `<?hh`. (Parsing[1002])",
            ]
        )
        assertCurrentServerLogContains(
            self.test_driver,
            f"Edenfs_watcher.get_all_files returned {exected_file_count} files",
        )

    # Note that we inherit all tests from CommonTests and run them,
    # but using EdenfsWatcherTestDriver


class EdenfsWatcherNonMountPointRepoTests(EdenfsWatcherTests):
    "Runs the same tests as EdenfsWatcherTests, but with a testing repo that's not the mount point of the Eden mount"

    class _Driver(EdenfsWatcherTestDriver):
        @classmethod
        def setUpClass(cls, template_repo: str) -> None:
            cls.setUpClassImpl(template_repo, "some/sub/folder")

    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return EdenfsWatcherNonMountPointRepoTests._Driver()

    # Don't add any tests here, add them to EdenfsWatcherTests!
