from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals

import json
import os
import shlex
import shutil
import tempfile

import common_tests

from hh_paths import hh_server, hh_client


def write_echo_json(f, obj):
    f.write("echo %s\n" % shlex.quote(json.dumps(obj)))


class MiniStateTestDriver(common_tests.CommonTestDriver):

    @classmethod
    def setUpClass(cls):
        super().setUpClass()
        # we create the state in a different dir from the one we run our tests
        # on, to verify that the saved state does not depend on any absolute
        # paths
        init_dir = os.path.join(cls.base_tmp_dir, 'init')
        shutil.copytree(cls.template_repo, init_dir)
        cls.saved_state_dir = tempfile.mkdtemp()
        cls.save_command(init_dir)
        cls.proc_call([hh_client, 'stop', init_dir])
        shutil.rmtree(init_dir)

    @classmethod
    def tearDownClass(cls):
        super().tearDownClass()
        shutil.rmtree(cls.saved_state_dir)

    @classmethod
    def saved_state_path(cls):
        return os.path.join(cls.saved_state_dir, 'foo')

    @classmethod
    def save_command(cls, init_dir, saved_state_path=None, assert_edges_added=False):
        if saved_state_path is None:
            saved_state_path = cls.saved_state_path()
        stdout, stderr, retcode = cls.proc_call([
            hh_client,
            '--json',
            '--save-state', saved_state_path,
            init_dir,
        ])
        if retcode != 0:
            raise Exception('Failed to save! stdout: "%s" stderr: "%s"' %
                            (stdout, stderr))
        if assert_edges_added:
            obj = json.loads(stdout)
            if obj['result'] is None:
                raise Exception('Failed. Missing result field: "%s" stderr: "%s"' %
                                (stdout, stderr))
            if obj['result'] <= 0:
                raise Exception('Failed. Expected some edges added: "%s" stderr: "%s"' %
                                (stdout, stderr))


    @classmethod
    def dump_saved_state(cls, assert_edges_added=False):
        # Dump a saved state to a temporary directory.
        # Return the path to the saved state.
        saved_state_path = os.path.join(tempfile.mkdtemp(), 'new_saved_state')
        cls.save_command(
            cls.repo_dir,
            saved_state_path=saved_state_path,
            assert_edges_added=assert_edges_added)
        return saved_state_path

    def write_local_conf(self):
        with open(os.path.join(self.repo_dir, 'hh.conf'), 'w') as f:
            f.write(r"""
# some comment
use_mini_state = true
use_watchman = true
watchman_subscribe_v2 = true
lazy_decl = true
lazy_parse = true
lazy_init2 = true
""")

    def write_hhconfig(self):
        with open(os.path.join(self.repo_dir, '.hhconfig'), 'w') as f:
            f.write(r"""
# some comment
assume_php = false
auto_namespace_map = {"Herp": "Derp\\Lib\\Herp"}
""")

    def write_watchman_config(self):
        with open(os.path.join(self.repo_dir, '.watchmanconfig'), 'w') as f:
            f.write('{}')

        os.mkdir(os.path.join(self.repo_dir, '.hg'))

    def setUp(self):
        super(MiniStateTestDriver, self).setUp()
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
            'state': saved_state_path,
            'corresponding_base_revision': '1',
            'is_cached': True,
            'deptable': saved_state_path + '.sql',
        }
        if changed_files:
            state['changes'] = changed_files
        else:
            state['changes'] = []
        with_state_arg = {
            'data_dump': state
        }
        cmd = [
            hh_server,
            '--daemon',
            '--with-mini-state',
            json.dumps(with_state_arg),
            self.repo_dir
        ]
        self.proc_call(cmd)
        self.wait_until_server_ready()

    def check_cmd(
        self,
        expected_output,
        stdin=None,
        options=None,
        assert_loaded_mini_state=True
    ):
        result = super(MiniStateTestDriver, self).check_cmd(
            expected_output,
            stdin,
            options
        )
        logs = self.get_server_logs()
        self.assertIn('Using watchman', logs)
        if assert_loaded_mini_state:
            self.assertIn('Successfully loaded mini-state', logs)
        return result

    def assertEqualString(self, first, second, msg=None):
        root = self.repo_dir + os.path.sep
        second = second.format(root=root)
        self.assertEqual(first, second, msg)


# Like MiniStateTestDriver except saves the mini state by invoking hh_server
# directly instead of over RPC via hh_client
class MiniStateClassicTestDriver(MiniStateTestDriver):

    @classmethod
    def save_command(cls, init_dir):
        stdout, stderr, retcode = cls.proc_call([
            hh_server,
            '--check', init_dir,
            '--save-mini', cls.saved_state_path()
        ])
        if retcode != 0:
            raise Exception('Failed to save! stdout: "%s" stderr: "%s"' %
                            (stdout, stderr))
