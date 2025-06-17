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

import shutil
import subprocess
import sys
from pathlib import Path
from typing import ClassVar, List, Optional

import hphp.hack.test.integration.common_tests as common_tests
from eden.integration.lib.edenclient import EdenFS
from hphp.hack.test.integration.common_tests import CommonTestDriver
from hphp.hack.test.integration.hh_paths import hh_server

from watchman.integration.lib import WatchmanInstance

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
"""

    config = config + f"watchman_sockname={watchman_socket_path}\n"
    return config


def runAndCheckSetupCommand(args: List[str]) -> None:
    # Not using CommonTestDriver.proc_call here, because it relies on some
    # global state seems to be used for commands denoting the actual test.
    subprocess.run(args, check=True)


def createHgRepo(path: str) -> None:
    "Runs hg init in the given directory and creates a commit containing everything currently in there"

    runAndCheckSetupCommand(["hg", "init", path])
    runAndCheckSetupCommand(["hg", "add", "-R", path])
    runAndCheckSetupCommand(["hg", "commit", "-m", "initial", "-R", path])


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


def assertServerLogContains(driver: CommonTestDriver, needle: str) -> None:
    if DEBUG_EDENFS_WATCHER_TEST_HH_SERVER_FOREGROUND:
        # We don't have access to any logs
        return

    server_log = driver.get_all_logs(driver.repo_dir).current_server_log
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
    assertServerLogContains(driver, "[edenfs_watcher][init] finished init")


def assertServerNotCrashed(driver: CommonTestDriver) -> None:
    if DEBUG_EDENFS_WATCHER_TEST_HH_SERVER_FOREGROUND:
        # We don't have access to any logs
        return
    monitor_log = driver.get_all_logs(driver.repo_dir).all_monitor_logs

    driver.assertFalse("Exit_status.Edenfs_watcher_failed" in monitor_log)


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

        createHgRepo(cls.hg_repo_root)

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

    def commitAllChanges(self, message: str = "test commit") -> str:
        (_, _, retcode) = self.proc_call(["hg", "add", "-R", self.eden_mount_point])
        self.assertEqual(retcode, 0)
        (_, _, retcode) = self.proc_call(
            ["hg", "commit", "--addremove", "-m", message, "-R", self.eden_mount_point]
        )
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

        hhconfig_file = os.path.join(self.test_driver.repo_dir, ".hhconfig")
        with open(hhconfig_file, "a") as f:
            f.write("# a comment to change hhconfig")

        # This will make us wait until the new server has come up.
        # Error message should be the same as before.
        self.test_driver.check_cmd(["No errors!"])

        # Let's make sure that we did indeed restart due to the hhconfig change
        assertMonitorLogContains(
            self.test_driver,
            "Exit_status.Hhconfig_changed",
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
