from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import glob
import os
import shutil
import subprocess
import sys
import tempfile

from hh_paths import hh_server, hh_client
from utils import write_files

class CommonSaveStateTests(object):

    @classmethod
    def setUpClass(cls):
        cls.maxDiff = 2000
        # we create the state in a different dir from the one we run our tests
        # on, to verify that the saved state does not depend on any absolute
        # paths
        init_dir = tempfile.mkdtemp()
        cls.repo_dir = tempfile.mkdtemp()
        cls.config_path = os.path.join(cls.repo_dir, '.hhconfig')
        cls.tmp_dir = tempfile.mkdtemp()
        cls.hh_tmp_dir = tempfile.mkdtemp()
        cls.saved_state_name = 'foo'
        cls.test_env = dict(os.environ, **{
            'HH_TEST_MODE': '1',
            'HH_TMPDIR': cls.hh_tmp_dir,
            'PATH': '%s:/bin:/usr/bin' % cls.tmp_dir,
            })

        with open(os.path.join(init_dir, '.hhconfig'), 'w') as f:
            f.write(r"""
# some comment
assume_php = false""")

        cls.files = {}

        cls.files['foo_1.php'] = """
        <?hh
        function f() {
            return g() + 1;
        }
        """

        cls.files['foo_2.php'] = """
        <?hh
        function g(): int {
            return 0;
        }
        """

        cls.files['foo_3.php'] = """
        <?hh
        function h(): string {
            return "a";
        }

        class Foo {}

        function some_long_function_name() {
            new Foo();
            h();
        }
        """

        write_files(cls.files, init_dir)
        write_files(cls.files, cls.repo_dir)

        cls.save_command(init_dir)

        shutil.rmtree(init_dir)

    @classmethod
    def save_command(cls):
        raise NotImplementedError()

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.repo_dir)
        shutil.rmtree(cls.tmp_dir)
        shutil.rmtree(cls.hh_tmp_dir)

    @classmethod
    def saved_state_path(cls):
        return os.path.join(cls.tmp_dir, cls.saved_state_name)

    def write_load_config(self, *changed_files):
        raise NotImplementedError()

    @classmethod
    def start_hh_server(cls):
        cmd = [hh_server, cls.repo_dir]
        print(" ".join(cmd), file=sys.stderr)
        return subprocess.Popen(
                cmd,
                stderr=subprocess.PIPE,
                env=cls.test_env)

    @classmethod
    def get_server_logs(cls):
        log_file = cls.proc_call([
            hh_client, '--logname', cls.repo_dir]).strip()
        with open(log_file) as f:
            return f.read()

    def setUp(self):
        write_files(self.files, self.repo_dir)

    def tearDown(self):
        self.proc_call([
            hh_client,
            'stop',
            self.repo_dir
        ])
        for p in glob.glob(os.path.join(self.repo_dir, '*')):
            os.remove(p)

    @classmethod
    def proc_call(cls, args, env=None, stdin=None):
        """
        Invoke a subprocess, return stdout, send stderr to our stderr (for
        debugging)
        """
        env = {} if env is None else env
        print(" ".join(args), file=sys.stderr)
        proc = subprocess.Popen(
                args,
                stdin=subprocess.PIPE,
                stdout=subprocess.PIPE,
                stderr=subprocess.PIPE,
                env=dict(cls.test_env, **env),
                universal_newlines=True)
        (stdout_data, stderr_data) = proc.communicate(stdin)
        sys.stderr.write(stderr_data)
        sys.stderr.flush()
        return stdout_data

    def check_cmd(self, expected_output, stdin=None, options=None):
        raise NotImplementedError()

    def test_modify_file(self):
        """
        Add an error to a file that previously had none.
        """
        with open(os.path.join(self.repo_dir, 'foo_2.php'), 'w') as f:
            f.write("""
            <?hh
            function g(): int {
                return 'a';
            }
            """)

        self.write_load_config('foo_2.php')

        self.check_cmd([
            '{root}foo_2.php:4:24,26: Invalid return type (Typing[4110])',
            '  {root}foo_2.php:3:27,29: This is an int',
            '  {root}foo_2.php:4:24,26: It is incompatible with a string',
        ])

    def test_new_file(self):
        """
        Add a new file that contains an error.
        """
        with open(os.path.join(self.repo_dir, 'foo_4.php'), 'w') as f:
            f.write("""
            <?hh
            function k(): int {
                return 'a';
            }
            """)

        self.write_load_config('foo_4.php')

        self.check_cmd([
            '{root}foo_4.php:4:24,26: Invalid return type (Typing[4110])',
            '  {root}foo_4.php:3:27,29: This is an int',
            '  {root}foo_4.php:4:24,26: It is incompatible with a string',
        ])

    def test_deleted_file(self):
        """
        Delete a file that still has dangling references after restoring from
        a saved state.
        """
        os.remove(os.path.join(self.repo_dir, 'foo_2.php'))

        self.write_load_config('foo_2.php')

        self.check_cmd([
            '{root}foo_1.php:4:20,20: Unbound name: g (a global function) (Naming[2049])',
            '{root}foo_1.php:4:20,20: Unbound name: g (a global constant) (Naming[2049])',
            ])

    def test_duplicated_file(self):
        self.write_load_config('foo_2.php')
        self.check_cmd(['No errors!'])

        shutil.copyfile(
                os.path.join(self.repo_dir, 'foo_2.php'),
                os.path.join(self.repo_dir, 'foo_2_dup.php'))

        self.check_cmd([
            '{root}foo_2_dup.php:3:18,18: Name already bound: g (Naming[2012])',
            '  {root}foo_2.php:3:18,18: Previous definition is here'])

        os.remove(os.path.join(self.repo_dir, 'foo_2.php'))
        self.check_cmd(['No errors!'])

    def test_moved_file(self):
        """
        Move a file, then create an error that references a definition in it.
        Check that the new file name is displayed in the error.
        """

        self.write_load_config(
            'foo_1.php', 'foo_2.php', 'bar_2.php',
        )

        os.rename(
            os.path.join(self.repo_dir, 'foo_2.php'),
            os.path.join(self.repo_dir, 'bar_2.php'),
        )

        with open(os.path.join(self.repo_dir, 'foo_1.php'), 'w') as f:
            f.write("""
            <?hh
            function f(): string {
                return g();
            }
            """)

        self.check_cmd([
            '{root}foo_1.php:4:24,26: Invalid return type (Typing[4110])',
            '  {root}foo_1.php:3:27,32: This is a string',
            '  {root}bar_2.php:3:23,25: It is incompatible with an int',

            ])

    def test_ide_tools(self):
        """
        Test hh_client --search, --find-refs, --find-class-refs, --type-at-pos,
        and --list-files

        We *could* break this up into multiple tests, but starting the server
        takes time and this test is slow enough already
        """

        self.write_load_config()

        self.check_cmd([
            'File "{root}foo_3.php", line 9, characters 18-40: some_long_function_name, function'
            ], options=['--search', 'some_lo'])

        self.check_cmd([
            'File "{root}foo_3.php", line 11, characters 13-13: h',
            '1 total results'
            ], options=['--find-refs', 'h'])

        self.check_cmd([
            'File "{root}foo_3.php", line 10, characters 13-21: Foo::__construct',
            '1 total results'
            ], options=['--find-refs', 'Foo::__construct'])

        self.check_cmd([
            'File "{root}foo_3.php", line 10, characters 17-19: Foo::__construct',
            '1 total results'
            ], options=['--find-class-refs', 'Foo'])

        self.check_cmd([
            'string'
            ], options=['--type-at-pos', '{root}foo_3.php:11:13'])

        self.check_cmd([
            # the doubled curly braces are because this string gets passed
            # through format()
            '[{{"name":"some_long_function_name",'
            '"type":"(function(): _)",'
            '"pos":{{"filename":"{root}foo_3.php",'
            '"line":9,"char_start":18,"char_end":40}},'
            '"func_details":{{"min_arity":0,"return_type":"_","params":[]}},'
            '"expected_ty":false}}]'
            ],
            # test the --json output because the non-json one doesn't contain
            # the filename, and we are especially interested in testing file
            # paths
            options=['--auto-complete', '--json'],
            stdin='<?hh function f() { some_AUTO332\n')

        self.check_cmd([
            'Foo::bar'
            ],
            options=['--identify-function', '1:51'],
            stdin='<?hh class Foo { private function bar() { $this->bar() }}')

        os.remove(os.path.join(self.repo_dir, 'foo_2.php'))
        self.check_cmd([
            '{root}foo_1.php',
            ], options=['--list-files'])
