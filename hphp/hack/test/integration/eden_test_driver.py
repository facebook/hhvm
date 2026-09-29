# (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

from __future__ import annotations

import os
import shutil
import subprocess
from pathlib import Path
from typing import ClassVar

from eden.integration.lib.edenclient import EdenFS
from hphp.hack.test.integration.common_tests import CommonTestDriver


def runAndCheckSetupCommand(args: list[str]) -> str:
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


def mountEden(eden_instance: EdenFS, hg_repo: str, eden_mount_point: str) -> None:
    eden_instance.clone(hg_repo, eden_mount_point)


def unmountEden(eden_instance: EdenFS, eden_mount_point: str) -> None:
    eden_instance.remove(eden_mount_point)


class EdenTestDriver(CommonTestDriver):
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

    # This is a commit in the testing repo at which point we only added the files returned by prepare_initial_commit
    clean_slate_commit: ClassVar[str]

    @classmethod
    def setUpClassImpl(
        cls, template_repo: str, repo_subdirectory_path: str | None
    ) -> None:
        # We need to call CommonTestDriver.setUpClass, but make the class
        # variable changes visible to our cls object
        super().setUpClass(template_repo)

        # This is where the Eden testing instance will put all of its state and files
        eden_base_dir = os.path.join(cls.base_tmp_dir, "eden_base")
        os.mkdir(eden_base_dir)
        cls.eden_instance = createEdenInstance(eden_base_dir)

        cls.hg_repo_root = os.path.join(cls.base_tmp_dir, "hg_repo")

        # We will mount the hg repo here ...
        cls.eden_mount_point = os.path.join(cls.base_tmp_dir, "repo")

        # Subfolder inside hg_repo where we actually put testing files:
        template_repo_destination = (
            os.path.join(cls.hg_repo_root, repo_subdirectory_path)
            if repo_subdirectory_path
            else cls.hg_repo_root
        )

        # This is the main testing folder, which we point the hh binary to test at.
        # Note that we keep the name `repo_dir` in order to be consistent with how
        # CommonTestDriver uses the term. However, this is not necessarily the same
        # as the root of the repository! If repo_subdirectory_path is set, then the
        # "repo" we test on is a subfolder of the Eden mount point.
        cls.repo_dir = (
            os.path.join(cls.eden_mount_point, repo_subdirectory_path)
            if repo_subdirectory_path
            else cls.eden_mount_point
        )

        shutil.copytree(template_repo, template_repo_destination)

        # CommonTestDriver.setUpClass already did this, but we changed the value of repo_dir
        cls.test_env["HH_LOCALCONF_PATH"] = cls.repo_dir
        cls._commit_initial_repository(template_repo_destination)

    @classmethod
    def prepare_initial_commit(cls, output_folder: str) -> list[str]:
        """Prepare config files and return their paths relative to output_folder.

        output_folder is the backing-repository directory that will be mounted at
        repo_dir. Setup points HH_LOCALCONF_PATH at repo_dir, so subclasses providing
        hh.conf should write it into output_folder.
        """
        return [".hhconfig"]

    @classmethod
    def _commit_initial_repository(cls, template_repo_destination: str) -> None:
        initial_files = cls.prepare_initial_commit(template_repo_destination)
        runAndCheckSetupCommand(["hg", "init", cls.hg_repo_root])
        runAndCheckSetupCommand(
            ["hg", "add", "-R", cls.hg_repo_root]
            + [os.path.join(template_repo_destination, path) for path in initial_files]
        )
        # Create the clean slate commit for those tests that don't want the full template repo
        runAndCheckSetupCommand(
            ["hg", "commit", "--message", "clean slate commit", "-R", cls.hg_repo_root]
        )
        cls.clean_slate_commit = runAndCheckSetupCommand(
            ["hg", "whereami", "-R", cls.hg_repo_root]
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
        cls.eden_instance.cleanup()
        super().tearDownClass()

    def setUp(self) -> None:
        # For hygiene, we (re-)mount our Eden repo for each individual test
        mountEden(self.eden_instance, self.hg_repo_root, self.eden_mount_point)

    def tearDown(self) -> None:
        # CommonTestDriver.tearDown recursively deletes repo_dir, which is an
        # Eden mount here. Consumers must stop their processes before unmounting.
        unmountEden(self.eden_instance, self.eden_mount_point)

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
