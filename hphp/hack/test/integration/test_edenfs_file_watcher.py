# pyre-strict

import os

import shutil
import subprocess
from typing import List

import hphp.hack.test.integration.common_tests as common_tests
from hphp.hack.test.integration.common_tests import CommonTestDriver


# Contents of hh.conf used in these tests Currently, enabling the EdenFS file
# watcher actually runs the old Watchman code under the hood. Thus, we configure
# Watchman the same way it is in /etc/hh.conf these days.
WATCHMAN_AND_EDENFS_HH_CONF = """
use_watchman = true
watchman_debug_logging = false
watchman_subscribe_v2 = true
watchman_sync_directory = .hg
edenfs_file_watcher_enabled=true
"""


def runAndCheckSetupCommand(args: List[str]) -> None:
    # Not using CommonTestDriver.proc_call here, because it relies on some
    # global state seems to be used for commands denoting the actual test.
    subprocess.run(args, check=True)


def createHgRepo(path: str) -> None:
    "Runs hg init in the given directory and creates a commit containing everything currently in there"

    runAndCheckSetupCommand(["hg", "init", path])
    runAndCheckSetupCommand(["hg", "add", "-R", path])
    runAndCheckSetupCommand(["hg", "commit", "-m", "initial", "-R", path])


def mountEden(hg_repo: str, eden_mount_point: str) -> None:
    runAndCheckSetupCommand(["edenfsctl", "clone", hg_repo, eden_mount_point])


def unmountEden(eden_mount_point: str) -> None:
    runAndCheckSetupCommand(["edenfsctl", "remove", "--no-prompt", eden_mount_point])


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

        @classmethod
        def setUpClass(cls, template_repo: str) -> None:
            print("running CommonTestsOnEden.Driver.setUpClass")

            # We need to call CommonTestDriver.setUpClass, but make the class
            # variable changes visible to our cls object.
            super(CommonTestsOnEden._Driver, cls).setUpClass(template_repo)

            # The call above set cls.repo_dir, but does not create that directory. Good!
            # Let's create an hg repo once for all tests, and then mount it for
            # each test individually.
            cls.hg_repo_dir = os.path.join(cls.base_tmp_dir, "hg_repo")
            shutil.copytree(template_repo, cls.hg_repo_dir)

            with open(os.path.join(cls.hg_repo_dir, "hh.conf"), "w") as f:
                f.write(WATCHMAN_AND_EDENFS_HH_CONF)

            createHgRepo(cls.hg_repo_dir)

        def setUp(self) -> None:
            print("running CommonTestsOnEden._Driver.setUp")

            # For hygiene, we (re-)mount our Eden repo for each individual test
            mountEden(self.hg_repo_dir, self.repo_dir)

        def tearDown(self) -> None:
            print("running CommonTestsOnEden._Driver.tearDown")

            # It's ugly to do this here, since this isn't really tear-down code,
            # but core testing logic, but we need to get this into every test
            assertEdenFsWatcherInitialized(self)

            # For hygiene, we (re-)mount our Eden repo for each individual test
            unmountEden(self.repo_dir)

    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return CommonTestsOnEden._Driver()

    # Nothing else to see here: We inherit all tests from CommonTests, but run
    # them using CommonTestsOnEden._Driver
