# pyre-strict

from __future__ import absolute_import, division, print_function, unicode_literals

import json
import os
import shlex
import shutil
import sys
import tempfile
from typing import (
    Iterable,
    List,
    Mapping,
    NamedTuple,
    Optional,
    TextIO,
    Tuple,
    TypeVar,
    Union,
)

import common_tests
from hh_paths import hh_client, hh_server


T = TypeVar("T")
ChangedFile = Union[str, Mapping[str, str]]


def write_echo_json(f: TextIO, obj: T) -> None:
    f.write("echo %s\n" % shlex.quote(json.dumps(obj)))


class SaveStateCommandResult(NamedTuple):
    retcode: int


class SaveStateResult(NamedTuple):
    path: str
    returned_values: SaveStateCommandResult


class SavedStateTestDriver(common_tests.CommonTestDriver):

    repo_dir: str
    enable_naming_table_fallback = False
    saved_state_dir: str

    @classmethod
    def setUpClass(cls, template_repo: str) -> None:
        super().setUpClass(template_repo)
        # we create the state in a different dir from the one we run our tests
        # on, to verify that the saved state does not depend on any absolute
        # paths
        init_dir = os.path.join(cls.base_tmp_dir, "init")
        shutil.copytree(cls.template_repo, init_dir)
        cls.saved_state_dir = tempfile.mkdtemp()
        cls.save_command(init_dir)
        cls.proc_call([hh_client, "stop", init_dir])
        shutil.rmtree(init_dir)

    @classmethod
    def tearDownClass(cls) -> None:
        super().tearDownClass()
        shutil.rmtree(cls.saved_state_dir)

    @classmethod
    def saved_state_path(cls) -> str:
        return os.path.join(cls.saved_state_dir, "foo")

    @classmethod
    def naming_saved_state_path(cls, base_path: Optional[str]) -> str:
        if base_path is None:
            return os.path.join(cls.saved_state_dir, "naming.sql")
        else:
            return base_path + "_naming.sql"

    @classmethod
    def save_command(
        cls,
        init_dir: str,
        saved_state_path: Optional[str] = None,
        ignore_errors: bool = False,
    ) -> SaveStateCommandResult:

        actual_saved_state_path: str = (
            saved_state_path if saved_state_path is not None else cls.saved_state_path()
        )
        edges_dir = tempfile.mkdtemp()
        hhdg_path = actual_saved_state_path + ".hhdg"

        # call hh
        hh_command: List[str] = [
            hh_client,
            "--json",
            "--save-64bit",
            edges_dir,
            "--save-state",
            actual_saved_state_path,
            init_dir,
            "--config",
            "max_workers=2",
        ]

        if ignore_errors:
            hh_command.append("--gen-saved-ignore-type-errors")

        stdout, stderr, retcode = cls.proc_call(hh_command)
        if not (retcode == 0 or (ignore_errors and retcode == 2)):
            logs = cls.get_all_logs(init_dir)
            print("STDOUT:\n%s\n\n" % stdout, file=sys.stderr)
            print("STDERR:\n%s\n\n" % stderr, file=sys.stderr)
            print("SERVER-LOG:\n%s\n\n" % logs.all_server_logs, file=sys.stderr)
            print("MONITOR-LOG:\n%s\n\n" % logs.all_monitor_logs, file=sys.stderr)
            raise Exception("Failed to save!")
        result = SaveStateCommandResult(retcode)

        if cls.enable_naming_table_fallback:
            cls.dump_naming_saved_state(init_dir, saved_state_path)

        assert retcode == 0

        return result

    @classmethod
    def dump_saved_state(
        cls,
        ignore_errors: bool = False,
    ) -> SaveStateResult:
        # Dump a saved state to a temporary directory.
        # Return the path to the saved state.
        saved_state_path = os.path.join(tempfile.mkdtemp(), "new_saved_state")
        result: SaveStateCommandResult = cls.save_command(
            cls.repo_dir,
            saved_state_path,
            ignore_errors,
        )

        return SaveStateResult(saved_state_path, result)

    @classmethod
    def save_command_incr(
        cls,
        init_dir: str,
        saved_state_path: str,
        original_saved_state_path: str,
        ignore_errors: bool = False,
    ) -> SaveStateCommandResult:
        delta_file = saved_state_path + "_64bit_dep_graph.delta"
        hhdg_path = saved_state_path + ".hhdg"
        original_hhdg_path = original_saved_state_path + ".hhdg"

        # call hh
        hh_command: List[str] = [
            hh_client,
            "--json",
            "--save-state",
            saved_state_path,
            init_dir,
            "--config",
            "max_workers=2",
        ]

        if ignore_errors:
            hh_command.append("--gen-saved-ignore-type-errors")

        stdout, stderr, retcode = cls.proc_call(hh_command)
        if not (retcode == 0 or (ignore_errors and retcode == 2)):
            raise Exception(
                'Failed to save! stdout: "%s" stderr: "%s"' % (stdout, stderr)
            )
        result = SaveStateCommandResult(retcode)

        if cls.enable_naming_table_fallback:
            cls.dump_naming_saved_state(init_dir, saved_state_path)

        assert retcode == 0

        return result

    @classmethod
    def dump_saved_state_incr(
        cls,
        original_saved_state_result: SaveStateResult,
        ignore_errors: bool = False,
    ) -> SaveStateResult:
        # Dump a saved state to a temporary directory.
        # Return the path to the saved state.
        saved_state_path = os.path.join(tempfile.mkdtemp(), "new_saved_state")
        original_saved_state_path = original_saved_state_result.path
        result: SaveStateCommandResult = cls.save_command_incr(
            cls.repo_dir,
            saved_state_path,
            original_saved_state_path,
            ignore_errors,
        )

        return SaveStateResult(saved_state_path, result)

    @classmethod
    def dump_naming_saved_state(
        cls, init_dir: str, saved_state_path: Optional[str] = None
    ) -> str:
        naming_saved_state_path = cls.naming_saved_state_path(saved_state_path)
        cls.proc_call(
            [
                hh_client,
                "--json",
                "--save-naming",
                naming_saved_state_path,
                init_dir,
                "--config",
                "max_workers=2",
            ]
        )
        return naming_saved_state_path

    def write_local_conf(self) -> None:
        with open(os.path.join(self.repo_dir, "hh.conf"), "w") as f:
            f.write(
                r"""
# some comment
use_mini_state = true
use_watchman = true
watchman_subscribe_v2 = true
lazy_decl = true
lazy_parse = true
lazy_init2 = true
fetch_remote_old_decls = false
"""
            )

    def write_hhconfig(self) -> None:
        with open(os.path.join(self.repo_dir, ".hhconfig"), "w") as f:
            f.write(
                r"""
# some comment
assume_php = false
auto_namespace_map = {"Herp": "Derp\\Lib\\Herp"}
"""
            )

    def write_watchman_config(self) -> None:
        with open(os.path.join(self.repo_dir, ".watchmanconfig"), "w") as f:
            f.write("{}")

        os.mkdir(os.path.join(self.repo_dir, ".hg"))

    def setUp(self) -> None:
        super(SavedStateTestDriver, self).setUp()
        self.write_local_conf()
        self.write_hhconfig()
        self.write_watchman_config()

    def start_hh_server(
        self,
        changed_files: Optional[Iterable[ChangedFile]] = None,
        changed_naming_files: Optional[Iterable[str]] = None,
        saved_state_path: Optional[str] = None,
        naming_saved_state_path: Optional[str] = None,
        args: Optional[List[str]] = None,
    ) -> None:
        if changed_files is None:
            changed_files = []

        if changed_naming_files is None:
            changed_naming_files = []

        if naming_saved_state_path is None:
            naming_saved_state_path = self.naming_saved_state_path(saved_state_path)

        # Yeah, gross again. This function's default value for a parameter
        # is from the object's state.
        if saved_state_path is None:
            saved_state_path = self.saved_state_path()

        if args is None:
            args = []

        state = {
            "state": saved_state_path,
            "corresponding_base_revision": "1",
            "is_cached": True,
            "deptable": saved_state_path + ".hhdg",
            "changes": changed_files,
            "naming_changes": changed_naming_files,
        }

        with_state_arg = {"data_dump": state}

        cmd = [
            hh_server,
            "--daemon",
            "--with-mini-state",
            json.dumps(with_state_arg),
            self.repo_dir,
            "--max-procs",
            "2",
        ] + args

        if self.enable_naming_table_fallback:
            cmd += [
                "--config",
                "enable_naming_table_fallback=true",
                "--config",
                "naming_sqlite_path={nt}".format(nt=naming_saved_state_path),
            ]

        self.proc_call(cmd)
        self.wait_until_server_ready()

    def check_cmd(
        self,
        expected_output: Optional[List[str]],
        stdin: Optional[str] = None,
        options: Optional[List[str]] = None,
        assert_loaded_saved_state: bool = False,
    ) -> Tuple[str, str]:
        result = super(SavedStateTestDriver, self).check_cmd(
            expected_output, stdin, options
        )
        logs = self.get_all_logs(self.repo_dir)
        try:
            self.assertTrue("Using watchman" in logs.current_server_log)
            if assert_loaded_saved_state:
                self.assertTrue(
                    "loading saved state succeeded" in logs.current_server_log,
                    "***Logs contain an init with no saved state. Did your "
                    "hh_server die and get restarted by the monitor?***",
                )
        except AssertionError:
            print("SERVER-LOG:\n%s\n\n" % logs.all_server_logs, file=sys.stderr)
            print("MONITOR_LOG:\n%s\n\n" % logs.all_monitor_logs, file=sys.stderr)
            raise
        return result

    def assertEqualString(
        self, first: str, second: str, msg: Optional[str] = None
    ) -> None:
        root = self.repo_dir + os.path.sep
        second = second.format(root=root)
        self.assertEqual(first, second, msg)
