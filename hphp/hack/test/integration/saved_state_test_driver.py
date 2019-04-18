from __future__ import absolute_import, division, print_function, unicode_literals

import json
import os
import shlex
import shutil
import tempfile
from typing import List, NamedTuple, Optional, Union

import common_tests
from hh_paths import hh_client, hh_server


def write_echo_json(f, obj):
    f.write("echo %s\n" % shlex.quote(json.dumps(obj)))


class SaveStateCommandResult(NamedTuple):
    retcode: int
    edges_added: Union[int, None]


class SaveStateResult(NamedTuple):
    path: str
    returned_values: SaveStateCommandResult


class SavedStateTestDriver(common_tests.CommonTestDriver):

    repo_dir: str

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
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
    def tearDownClass(cls):
        super().tearDownClass()
        shutil.rmtree(cls.saved_state_dir)

    @classmethod
    def saved_state_path(cls):
        return os.path.join(cls.saved_state_dir, "foo")

    @classmethod
    def exec_save_command(
        cls,
        hh_command: List[str],
        assert_edges_added: bool = False,
        ignore_errors: bool = False,
    ) -> SaveStateCommandResult:
        stdout, stderr, retcode = cls.proc_call(hh_command)
        if retcode != 0 and not ignore_errors:
            raise Exception(
                'Failed to save! stdout: "%s" stderr: "%s"' % (stdout, stderr)
            )

        edges_added = None
        if assert_edges_added is not None:
            obj = json.loads(stdout)

            if obj.get("saved_state_result", None) is not None:
                saved_state_result = obj["saved_state_result"]
                edges_added = saved_state_result.get("edges_added", None)
            else:
                edges_added = obj.get("result", None)

            if edges_added is None:
                raise Exception(
                    'Failed. Missing result field: "%s" stderr: "%s"' % (stdout, stderr)
                )

        return SaveStateCommandResult(retcode, edges_added)

    @classmethod
    def save_command(
        cls,
        init_dir: str,
        saved_state_path: str = None,
        assert_edges_added: bool = False,
        ignore_errors: bool = False,
        replace_state_after_saving: bool = False,
    ) -> SaveStateCommandResult:

        actual_saved_state_path: str = (
            saved_state_path if saved_state_path is not None else cls.saved_state_path()
        )

        hh_command: List[str] = [
            hh_client,
            "--json",
            "--save-state",
            actual_saved_state_path,
            init_dir,
        ]

        if ignore_errors:
            hh_command.append("--gen-saved-ignore-type-errors")

        if replace_state_after_saving:
            hh_command.append("--replace-state-after-saving")

        return cls.exec_save_command(hh_command, assert_edges_added, ignore_errors)

    @classmethod
    def dump_saved_state(
        cls,
        assert_edges_added: bool = False,
        ignore_errors: bool = False,
        replace_state_after_saving: bool = False,
    ) -> SaveStateResult:
        # Dump a saved state to a temporary directory.
        # Return the path to the saved state.
        saved_state_path = os.path.join(tempfile.mkdtemp(), "new_saved_state")
        result: SaveStateCommandResult = cls.save_command(
            cls.repo_dir,
            saved_state_path,
            assert_edges_added,
            ignore_errors,
            replace_state_after_saving,
        )

        return SaveStateResult(saved_state_path, result)

    def write_local_conf(self):
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
"""
            )

    def write_hhconfig(self):
        with open(os.path.join(self.repo_dir, ".hhconfig"), "w") as f:
            f.write(
                r"""
# some comment
assume_php = false
auto_namespace_map = {"Herp": "Derp\\Lib\\Herp"}
"""
            )

    def write_watchman_config(self):
        with open(os.path.join(self.repo_dir, ".watchmanconfig"), "w") as f:
            f.write("{}")

        os.mkdir(os.path.join(self.repo_dir, ".hg"))

    def setUp(self):
        super(SavedStateTestDriver, self).setUp()
        self.write_local_conf()
        self.write_hhconfig()
        self.write_watchman_config()

    def start_hh_server(self, changed_files=None, saved_state_path=None):
        if changed_files is None:
            changed_files = []

        # Yeah, gross again. This function's default value for a parameter
        # is from the object's state.
        if saved_state_path is None:
            saved_state_path = self.saved_state_path()

        state = {
            "state": saved_state_path,
            "corresponding_base_revision": "1",
            "is_cached": True,
            "deptable": saved_state_path + ".sql",
            "changes": changed_files,
        }

        with_state_arg = {"data_dump": state}

        cmd = [
            hh_server,
            "--daemon",
            "--with-mini-state",
            json.dumps(with_state_arg),
            self.repo_dir,
        ]

        self.proc_call(cmd)
        self.wait_until_server_ready()

    def check_cmd(
        self,
        expected_output: Optional[List[str]],
        stdin=None,
        options=None,
        assert_loaded_saved_state=True,
    ):
        result = super(SavedStateTestDriver, self).check_cmd(
            expected_output, stdin, options
        )
        logs = self.get_server_logs()
        self.assertIn("Using watchman", logs)
        if assert_loaded_saved_state:
            self.assertIn(
                "loading saved state succeeded",
                logs,
                msg=("***Logs contain an init with no saved state. Did your "
                     "hh_server die and get restarted by the monitor?***"))
        return result

    def assertEqualString(self, first, second, msg=None):
        root = self.repo_dir + os.path.sep
        second = second.format(root=root)
        self.assertEqual(first, second, msg)


# Like SavedStateTestDriver except saves the state by invoking hh_server
# directly instead of over RPC via hh_client
class SavedStateClassicTestDriver(SavedStateTestDriver):
    @classmethod
    def save_command(cls, init_dir):
        stdout, stderr, retcode = cls.proc_call(
            [hh_server, "--check", init_dir, "--save-state", cls.saved_state_path()]
        )
        if retcode != 0:
            raise Exception(
                'Failed to save! stdout: "%s" stderr: "%s"' % (stdout, stderr)
            )
