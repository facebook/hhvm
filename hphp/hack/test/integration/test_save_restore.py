from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
from __future__ import unicode_literals
import glob
import os
import shutil
import subprocess
import tempfile
import time
import unittest

from utils import touch, write_files, proc_call, ensure_output_contains

def write_load_config(repo_dir, saved_state_path, changed_files=[]):
    """
    Writes a .hhconfig that allows hh_client to launch hh_server from a saved
    state archive.

    repo_dir: Repository to run hh_server on
    saved_state_path: Path to file containing saved server state
    changed_files: list of strings
    """
    with open(os.path.join(repo_dir, 'server_options.sh'), 'w') as f:
        f.write(r"""
#! /bin/sh
echo %s
""" % saved_state_path)
        for fn in changed_files:
            f.write("echo %s\n" % fn)
        os.fchmod(f.fileno(), 0o700)

    with open(os.path.join(repo_dir, '.hhconfig'), 'w') as f:
        # we can't just write 'echo ...' inline because Hack server will
        # be passing this command some command-line options
        f.write(r"""
# some comment
load_script = %s
        """ % os.path.join(repo_dir, 'server_options.sh'))

class TestSaveRestore(unittest.TestCase):
    @classmethod
    def init_class(cls, hh_server, hh_client):
        """
        Should be called before tests are run. This is an awkward hack to allow
        us to pass command-line options to unittests, but Python doesn't
        provide any nicer way.
        """
        cls.hh_server = hh_server
        cls.hh_client = hh_client

    @classmethod
    def setUpClass(cls):
        cls.maxDiff = 2000
        # we create the state in a different dir from the one we run our tests
        # on, to verify that the saved state does not depend on any absolute
        # paths
        init_dir = tempfile.mkdtemp()
        cls.repo_dir = tempfile.mkdtemp()
        cls.saved_state_dir = tempfile.mkdtemp()

        touch(os.path.join(init_dir, '.hhconfig'))

        cls.files = {}

        cls.files['foo_1.php'] = """
        <?hh
        function f() {
            return g() + 1;
        }
        """

        cls.files['foo_2.php'] = """
        <?hh
        function g(): string {
            return "a";
        }
        """

        cls.files['foo_3.php'] = """
        <?hh
        function h(): string {
            return 1;
        }

        class Foo {}

        function some_long_function_name() {
            new Foo();
            h();
        }
        """

        cls.initial_errors = [
            '{root}foo_1.php:4:20,22: Typing error (Typing[4110])',
            '  {root}foo_1.php:4:20,22: This is a num (int/float) because this is used in an arithmetic operation',
            '  {root}foo_2.php:3:23,28: It is incompatible with a string',
            '{root}foo_3.php:4:20,20: Invalid return type (Typing[4110])',
            '  {root}foo_3.php:3:23,28: This is a string',
            '  {root}foo_3.php:4:20,20: It is incompatible with an int',
        ]

        write_files(cls.files, init_dir)
        write_files(cls.files, cls.repo_dir)

        subprocess.call([
            cls.hh_server,
            '--check', init_dir,
            '--save', os.path.join(cls.saved_state_dir, 'foo'),
        ])

        shutil.rmtree(init_dir)

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(cls.repo_dir)
        shutil.rmtree(cls.saved_state_dir)

    @classmethod
    def start_hh_server(cls):
        return subprocess.Popen(
                [cls.hh_server, cls.repo_dir],
                stderr=subprocess.PIPE)

    def setUp(self):
        write_files(self.files, self.repo_dir)

    def tearDown(self):
        subprocess.call([
            self.hh_client,
            'stop',
            self.repo_dir
        ])
        for p in glob.glob(os.path.join(self.repo_dir, '*')):
            os.remove(p)

    def check_cmd(self, expected_output, stdin=None, options=[]):
        root = self.repo_dir + os.path.sep
        output = proc_call([
            self.hh_client,
            'check',
            '--retries',
            '5',
            self.repo_dir
            ] + list(map(lambda x: x.format(root=root), options)),
            stdin=stdin)
        self.assertCountEqual(
            map(lambda x: x.format(root=root), expected_output),
            output.splitlines())

    def test_mtime_update(self):
        """
        Update mtimes of files and check that errors remain unchanged.
        """
        state_fn = os.path.join(self.saved_state_dir, 'foo')
        write_load_config(
            self.repo_dir,
            state_fn)
        server_proc = self.start_hh_server()
        ensure_output_contains(server_proc.stderr,
                'Load state found at %s.' % state_fn)

        self.check_cmd(self.initial_errors)
        touch(os.path.join(self.repo_dir, 'foo_1.php'))
        self.check_cmd(self.initial_errors)
        touch(os.path.join(self.repo_dir, 'foo_2.php'))
        self.check_cmd(self.initial_errors)

    def test_new_error(self):
        """
        Replace an error in an existing file with a new one.
        """
        with open(os.path.join(self.repo_dir, 'foo_2.php'), 'w') as f:
            f.write("""
            <?hh
            function g(): int {
                return 'a';
            }
            """)

        write_load_config(
            self.repo_dir,
            os.path.join(self.saved_state_dir, 'foo'),
            ['foo_2.php']
        )

        self.check_cmd([
            '{root}foo_2.php:4:24,26: Invalid return type (Typing[4110])',
            '  {root}foo_2.php:3:27,29: This is an int',
            '  {root}foo_2.php:4:24,26: It is incompatible with a string',
            '{root}foo_3.php:4:20,20: Invalid return type (Typing[4110])',
            '  {root}foo_3.php:3:23,28: This is a string',
            '  {root}foo_3.php:4:20,20: It is incompatible with an int',
        ])

    def test_new_error_after_load(self):
        """
        Replace an error in an existing file with a new one after restoring
        from saved state.
        """

        write_load_config(
            self.repo_dir,
            os.path.join(self.saved_state_dir, 'foo'),
            ['foo_2.php']
        )

        with open(os.path.join(self.repo_dir, 'foo_2.php'), 'w') as f:
            f.write("""
            <?hh
            function g(): int {
                return 'a';
            }
            """)

        self.check_cmd([
            '{root}foo_2.php:4:24,26: Invalid return type (Typing[4110])',
            '  {root}foo_2.php:3:27,29: This is an int',
            '  {root}foo_2.php:4:24,26: It is incompatible with a string',
            '{root}foo_3.php:4:20,20: Invalid return type (Typing[4110])',
            '  {root}foo_3.php:3:23,28: This is a string',
            '  {root}foo_3.php:4:20,20: It is incompatible with an int',
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

        write_load_config(
            self.repo_dir,
            os.path.join(self.saved_state_dir, 'foo'),
            ['foo_4.php']
        )

        self.check_cmd(self.initial_errors + [
            '{root}foo_4.php:4:24,26: Invalid return type (Typing[4110])',
            '  {root}foo_4.php:3:27,29: This is an int',
            '  {root}foo_4.php:4:24,26: It is incompatible with a string',
        ])

    def test_deleted_file(self):
        """
        Delete a file containing an error.
        """
        os.remove(os.path.join(self.repo_dir, 'foo_3.php'))

        write_load_config(
            self.repo_dir,
            os.path.join(self.saved_state_dir, 'foo'),
            ['foo_3.php']
        )

        self.check_cmd([
            '{root}foo_1.php:4:20,22: Typing error (Typing[4110])',
            '  {root}foo_1.php:4:20,22: This is a num (int/float) because this is used in an arithmetic operation',
            '  {root}foo_2.php:3:23,28: It is incompatible with a string',
        ])

    def test_deleted_file_after_load(self):
        """
        Delete a file containing an error after restoring from a saved state.
        """
        write_load_config(
            self.repo_dir,
            os.path.join(self.saved_state_dir, 'foo'),
            ['foo_3.php']
        )

        os.remove(os.path.join(self.repo_dir, 'foo_3.php'))

        self.check_cmd([
            '{root}foo_1.php:4:20,22: Typing error (Typing[4110])',
            '  {root}foo_1.php:4:20,22: This is a num (int/float) because this is used in an arithmetic operation',
            '  {root}foo_2.php:3:23,28: It is incompatible with a string',
        ])

    def test_moved_files(self):
        """
        Move a file containing errors + a file referenced from an error
        originating in another file.
        """
        os.rename(
            os.path.join(self.repo_dir, 'foo_2.php'),
            os.path.join(self.repo_dir, 'bar_2.php'),
        )
        os.rename(
            os.path.join(self.repo_dir, 'foo_3.php'),
            os.path.join(self.repo_dir, 'bar_3.php'),
        )

        write_load_config(
            self.repo_dir,
            os.path.join(self.saved_state_dir, 'foo'),
            ['foo_2.php', 'bar_2.php', 'foo_3.php', 'bar_3.php']
        )

        try:
            self.check_cmd([
                '{root}foo_1.php:4:20,22: Typing error (Typing[4110])',
                '  {root}foo_1.php:4:20,22: This is a num (int/float) because this is used in an arithmetic operation',
                '  {root}bar_2.php:3:23,28: It is incompatible with a string',
                '{root}bar_3.php:4:20,20: Invalid return type (Typing[4110])',
                '  {root}bar_3.php:3:23,28: This is a string',
                '  {root}bar_3.php:4:20,20: It is incompatible with an int',
            ])
        except AssertionError:
            # FIXME: figure out why moving a file sometimes duplicates
            # errors. this occurs regardless of whether we are restoring from
            # a saved state or are running from a fresh initialized state,
            # and seems to be due to a race condition
            self.check_cmd([
                '{root}foo_1.php:4:20,22: Typing error (Typing[4110])',
                '  {root}foo_1.php:4:20,22: This is a num (int/float) because this is used in an arithmetic operation',
                '  {root}bar_2.php:3:23,28: It is incompatible with a string',
                '{root}bar_3.php:4:20,20: Invalid return type (Typing[4110])',
                '  {root}bar_3.php:3:23,28: This is a string',
                '  {root}bar_3.php:4:20,20: It is incompatible with an int',
                '{root}bar_3.php:4:20,20: Invalid return type (Typing[4110])',
                '  {root}bar_3.php:3:23,28: This is a string',
                '  {root}bar_3.php:4:20,20: It is incompatible with an int',
            ])

    def test_ide_tools(self):
        """
        Test hh_client --search, --find-refs, --find-class-refs, --type-at-pos,
        and --list-files

        We *could* break this up into multiple tests, but starting the server
        takes time and this test is slow enough already
        """

        write_load_config(
            self.repo_dir,
            os.path.join(self.saved_state_dir, 'foo'))

        # adding flags to hh_client check disables the autostart behavior, so
        # we start up hh_server manually
        proc_call([
            self.hh_client,
            'start',
            self.repo_dir
        ])

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
            '{root}foo_1.php',
            '{root}foo_3.php',
            ], options=['--list-files'])

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

    def test_options_cmd(self):
        """
        Make sure we are invoking the server_options_cmd with the right flags
        """
        args_file = os.path.join(self.saved_state_dir, 'cmd_args')
        with open(os.path.join(self.repo_dir, 'server_options.sh'), 'w') as f:
            f.write(r"""
#! /bin/sh
echo "$1" > {out}
echo "$2" >> {out}
            """.format(out=args_file))
            os.fchmod(f.fileno(), 0o700)

        with open(os.path.join(self.repo_dir, '.hhconfig'), 'w') as f:
            f.write(r"""
# some comment
load_script = %s
            """ % os.path.join(self.repo_dir, 'server_options.sh'))

        proc_call([
            self.hh_client,
            'start',
            self.repo_dir
        ])

        version = proc_call([
            self.hh_server,
            '--version'
        ])

        with open(args_file) as f:
            self.assertEqual(f.read().splitlines(), [self.repo_dir, version])
