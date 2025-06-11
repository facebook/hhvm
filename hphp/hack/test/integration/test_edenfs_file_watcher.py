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
from pathlib import Path
from typing import ClassVar, List, Optional, Tuple

import hphp.hack.test.integration.common_tests as common_tests
import hphp.hack.test.integration.test_case as test_case
from eden.integration.lib.edenclient import EdenFS
from hphp.hack.test.integration.common_tests import CommonTestDriver

from watchman.integration.lib import WatchmanInstance


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
    server_log = driver.get_all_logs(driver.repo_dir).current_server_log
    contains = needle in server_log
    if not contains:
        print("Server log:")
        print(server_log)
    driver.assertTrue(contains)


def assertEdenFsWatcherInitialized(driver: CommonTestDriver) -> None:
    """Checks that an Edenfs_watcher instance was initialized

    Given that we may fall back to using Watchman if Edenfs_watcher
    initialization fails, this prevents tests from passing even though the
    EdenFS watcher wasn't actually used.
    """
    assertServerLogContains(driver, "[edenfs_watcher][init] finished init")


class CommonTestsOnEden(common_tests.CommonTests):
    "Runs CommonTests, but on an Eden-backed testing repo with edenfs_file_watcher_enabled set"

    class _Driver(common_tests.CommonTestDriver):
        """Driver compatible with CommonTestDriver, but creating an Eden-backed repo

        Concretely, this means that all the helpers in CommonTestDriver must
        still work. All we need to do is making sure that the repo pointed at by
        the path in `repo_dir` gets initialized in a different way, by mounting an hg
        commit using Eden.
        """

        eden_instance: ClassVar[EdenFS]
        watchman_instance: ClassVar[WatchmanInstance.Instance]

        @classmethod
        def setUpClass(cls, template_repo: str) -> None:
            print("running CommonTestsOnEden._Driver.setUpClass")

            # We need to call CommonTestDriver.setUpClass, but make the class
            # variable changes visible to our cls object.
            super(CommonTestsOnEden._Driver, cls).setUpClass(template_repo)

            # This is where the Eden testing intsance will put all of its state and files
            eden_base_dir = os.path.join(cls.base_tmp_dir, "eden_base")
            os.mkdir(eden_base_dir)
            cls.eden_instance = createEdenInstance(eden_base_dir)

            cls.watchman_instance = createWatchmanInstance()
            watchman_socket_path = cls.watchman_instance.getUnixSockPath()

            # The call above set cls.repo_dir, but does not create that directory. Good!
            # Let's create an hg repo once for all tests, and then mount it for
            # each test individually.
            cls.hg_repo_dir = os.path.join(cls.base_tmp_dir, "hg_repo")
            shutil.copytree(template_repo, cls.hg_repo_dir)

            with open(os.path.join(cls.hg_repo_dir, "hh.conf"), "w") as f:
                f.write(makeWatchmanEdenHhConf(watchman_socket_path))

            createHgRepo(cls.hg_repo_dir)

        @classmethod
        def tearDownClass(cls) -> None:
            print("running CommonTestsOnEden._Driver.tearDownClass")

            cls.eden_instance.cleanup()
            cls.watchman_instance.stop()
            super(CommonTestsOnEden._Driver, cls).tearDownClass()

        def setUp(self) -> None:
            print("running CommonTestsOnEden._Driver.setUp")

            # For hygiene, we (re-)mount our Eden repo for each individual test
            mountEden(self.eden_instance, self.hg_repo_dir, self.repo_dir)

        def tearDown(self) -> None:
            print("running CommonTestsOnEden._Driver.tearDown")

            # It's ugly to do this here, since this isn't really tear-down code,
            # but core testing logic, but we need to get this into every test
            assertEdenFsWatcherInitialized(self)

            # For hygiene, we (re-)mount our Eden repo for each individual test.
            # Note that we must stop the server before unmounting the Eden repo that it works on.
            # 3 retries is the value used in CommonTestDriver.tearDown
            self.stop_hh_server(retries=3)
            unmountEden(self.eden_instance, self.repo_dir)

        def run_check(
            self, stdin: Optional[str] = None, options: Optional[List[str]] = None
        ) -> Tuple[str, str, int]:
            """Just a wrapper overriding of CommonTestDriver.run_check such that we get the server log if hh_client check failed."""

            (stdout, stderr, retcode) = super().run_check(stdin, options)
            if retcode != 0:
                print(
                    f"hh_client check failed with exit code {retcode} for options {options}"
                )
                print(
                    f"Server log:\n{self.get_all_logs(self.repo_dir).current_server_log}"
                )
            return (stdout, stderr, retcode)

        def commitAllChanges(self, message: str = "test commit") -> str:
            (_, _, retcode) = self.proc_call(["hg", "add", "-R", self.repo_dir])
            self.assertEqual(retcode, 0)
            (_, _, retcode) = self.proc_call(
                ["hg", "commit", "--addremove", "-m", message, "-R", self.repo_dir]
            )
            self.assertEqual(retcode, 0)

            # Get the revision hash of the commit we just created
            (stdout, _, retcode) = self.proc_call(
                ["hg", "whereami", "-R", self.repo_dir]
            )
            self.assertEqual(retcode, 0)
            rev_hash = stdout.strip()
            return rev_hash

        def gotoRev(self, rev: str, merge: bool = False) -> None:
            args = ["hg", "goto", rev, "-R", self.repo_dir]
            if merge:
                args.append("--merge")
            (_, _, retcode) = self.proc_call(args)
            self.assertEqual(retcode, 0)

    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return CommonTestsOnEden._Driver()

    # Need to duplicate this here from TestCase to make pyre happy
    _test_driver: Optional[_Driver] = None

    # Need to duplicate this here from TestCase to make pyre happy
    @property
    def test_driver(self) -> _Driver:
        test_driver = self._test_driver
        assert test_driver is not None
        return test_driver

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

    # Note that we inherit all tests from CommonTests and run them,
    # but using CommonTestsOnEden._Driver


class NonMountPointRepoTest(test_case.TestCase[common_tests.CommonTestDriver]):
    "Tests that we can run hh_server on a path that isn't the root of a hg repository/mount point of an Eden file system."

    class _Driver(common_tests.CommonTestDriver):
        "Similar to CommonTestsOnEden._Driver, but sets up a testing repo that is a subfolder of the hg repo"

        eden_mount_point: ClassVar[str]
        hg_repo_dir: ClassVar[str]

        @classmethod
        def setUpClass(cls, template_repo: str) -> None:
            print("running NonMountPointRepoTest._Driver.setUpClass")

            # We need to call CommonTestDriver.setUpClass, but make the class
            # variable changes visible to our cls object
            super(NonMountPointRepoTest._Driver, cls).setUpClass(template_repo)
            print(f"base_tmp_dir is {cls.base_tmp_dir}")

            # This is where the Eden testing intsance will put all of its state and files
            eden_base_dir = os.path.join(cls.base_tmp_dir, "eden_base")
            os.mkdir(eden_base_dir)
            cls.eden_instance = createEdenInstance(eden_base_dir)

            cls.watchman_instance = createWatchmanInstance()
            watchman_socket_path = cls.watchman_instance.getUnixSockPath()

            # The call to setUpClass in CommonTests above defined cls.repo_dir,
            # but we want to redefine it to point to a nested folder.
            # Root of the hg repo we will create
            cls.hg_repo_dir = os.path.join(cls.base_tmp_dir, "hg_repo")

            # Subfolder inside hg_repo where we actually put testing files:
            hg_repo_sub_dir = os.path.join(cls.hg_repo_dir, "some", "sub", "folder")

            shutil.copytree(template_repo, hg_repo_sub_dir)

            # This file can go wherever, as long as HH_LOCALCONF_PATH points to
            # it. CommonTestDriver.setUpClass sets that variable to cls.repo_dir
            # / hh.conf, so let's put the file at the toplevel of the repo (not
            # with the other files)
            with open(os.path.join(cls.hg_repo_dir, "hh.conf"), "w") as f:
                f.write(makeWatchmanEdenHhConf(watchman_socket_path))

            createHgRepo(cls.hg_repo_dir)

            # We will mount the hg repo here ...
            cls.eden_mount_point = cls.repo_dir
            # ... which means that hh will find the testing files here instead:
            cls.repo_dir = os.path.join(cls.repo_dir, "some", "sub", "folder")

        @classmethod
        def tearDownClass(cls) -> None:
            print("running NonMountPointRepoTest._Driver.tearDownClass")

            cls.eden_instance.cleanup()
            cls.watchman_instance.stop()
            super(NonMountPointRepoTest._Driver, cls).tearDownClass()

        def setUp(self) -> None:
            print("running NonMountPointRepoTest._Driver.setUp")

            # For hygiene, we (re-)mount our Eden repo for each individual test
            mountEden(self.eden_instance, self.hg_repo_dir, self.eden_mount_point)

        def tearDown(self) -> None:
            print("running NonMountPointRepoTest._Driver.tearDown")

            server_log = self.get_all_logs(self.repo_dir).current_server_log
            print(f"server log:\n{server_log}")

            # It's ugly to do this here, since this isn't really tear-down code,
            # but core testing logic, but we need to get this into every test
            assertEdenFsWatcherInitialized(self)

            # For hygiene, we (re-)mount our Eden repo for each individual test.
            # Note that we must stop the server before unmounting the Eden repo that it works on.
            # 3 retries is the value used in CommonTestDriver.tearDown
            self.stop_hh_server(retries=3)
            unmountEden(self.eden_instance, self.eden_mount_point)

    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return NonMountPointRepoTest._Driver()

    def test_non_mount_point_repo(self) -> None:
        # This is mostly just a copy-paste of
        # CommonTests.test_file_delete_after_load but with an additional check
        # that the EdenFS watcher was initialized correctly.

        self.test_driver.start_hh_server()

        self.test_driver.check_cmd(["No errors!"])

        os.remove(os.path.join(self.test_driver.repo_dir, "foo_2.php"))
        self.test_driver.check_cmd(
            [
                "ERROR: {root}foo_1.php:4:20,20: Unbound name: `g` (a global function) (Naming[2049])",
                "ERROR: {root}foo_1.php:4:20,20: Unbound name (typing): `g` (Typing[4107])",
            ]
        )

        assertEdenFsWatcherInitialized(self.test_driver)
