#! /usr/bin/env python3

import os

class HierarchyTests(object):

    template_repo = 'hphp/hack/test/integration/data/hierarchy'

    def test_inheritance(self):
        """
        Test --inheritance-ancestors and --inheritance-children
        """
        self.write_load_config()
        self.check_cmd([
            'File "{root}foo.php", line 3, characters 7-9: Foo',

            '    inherited by File "{root}bar.php", line 3, characters 7-9: Bar',
            'File "{root}foo.php", line 4, characters 19-19: Foo::f',
            '    inherited by File "{root}bar.php", line 4, characters 19-19: Bar::f',
            'File "{root}foo.php", line 3, characters 7-9: Foo',
            '    inherited by File "{root}baz.php", line 3, characters 7-9: Baz',
            'File "{root}foo.php", line 5, characters 19-19: Foo::g',
            '    inherited by File "{root}baz.php", line 4, characters 19-19: Baz::g',
        ],
            options=['--inheritance-children', 'Foo'])
        self.check_cmd([
            'File "{root}baz.php", line 3, characters 7-9: Baz',
            '    inherited from File "{root}foo.php", line 3, characters 7-9: Foo',
            'File "{root}baz.php", line 4, characters 19-19: Baz::g',
            '    inherited from File "{root}foo.php", line 5, characters 19-19: Foo::g',
            'File "{root}baz.php", line 3, characters 7-9: Baz',
            '    inherited from File "{root}bar.php", line 3, characters 7-9: Bar',
        ],
            options=['--inheritance-ancestors', 'Baz'])

    def test_method_signature_change(self):
        with open(os.path.join(self.repo_dir, 'qux.php'), 'w') as f:
            f.write('''<?hh
class Qux {
  public function f() {
    $x = new Foo();
    $x->f();
  }
}
''')
        self.write_load_config('qux.php')
        self.check_cmd(['No errors!'])
        debug_sub = self.subscribe_debug()
        with open(os.path.join(self.repo_dir, 'foo.php'), 'w') as f:
            f.write('''<?hh
class Foo {
  public function f(): void {}
  public final function g() {}
}
''')
        msgs = debug_sub.get_incremental_logs()
        self.assertEqual(set(msgs['to_redecl_phase1']['files']),
                         set(['foo.php']))
        # FIXME: redeclaring qux.php is unnecessary
        self.assertEqual(set(msgs['to_redecl_phase2']['files']),
                         set(['foo.php', 'bar.php', 'baz.php', 'qux.php']))
        self.assertEqual(set(msgs['to_recheck']['files']),
                         set(['foo.php', 'bar.php', 'baz.php', 'qux.php']))

    def test_failed_decl(self):
        """
        When a file generates errors in the decl phase, we make the very
        conservative assumption that we know nothing about its dependencies.
        So we redeclare it every time a file changes, even if the changed file
        is wholly unrelated to the contents of the erroneous file.

        I can't actually think of any example where this is necessary, but
        until we have a good argument for why it isn't, it's probably
        reasonable to ensure this behavior is not accidentally broken.
        """

        self.write_load_config('foo.php')
        with open(os.path.join(self.repo_dir, 'foo.php'), 'w') as f:
            f.write('''<?hh
class Foo {
  public function f() {}
  public final function g() {}
}
''')
        # If we ever change things so this error is raised from somewhere
        # other than the decl phase, remember to update this test...
        self.check_cmd([
            '{root}baz.php:4:19,19: You cannot override this method '
            '(Typing[4070])',
            '  {root}foo.php:4:25,25: It was declared as final',
        ])

        debug_sub = self.subscribe_debug()
        # now let's create a wholly unrelated file
        with open(os.path.join(self.repo_dir, 'qux.php'), 'w') as f:
            f.write('<?hh class Qux {}')

        msgs = debug_sub.get_incremental_logs()
        # baz.php still gets rechecked
        self.assertEqual(set(msgs['to_redecl_phase1']['files']),
                         set(['baz.php', 'qux.php']))
        self.assertEqual(set(msgs['to_redecl_phase2']['files']),
                         set(['baz.php', 'qux.php']))
        self.assertEqual(set(msgs['to_recheck']['files']),
                         set(['baz.php', 'qux.php']))
