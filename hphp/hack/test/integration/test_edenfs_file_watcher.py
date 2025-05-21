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

    @classmethod
    def get_test_driver(cls) -> common_tests.CommonTestDriver:
        return CommonTestsOnEden._Driver()

    # Nothing else to see here: We inherit all tests from CommonTests, but run
    # them using CommonTestsOnEden._Driver
