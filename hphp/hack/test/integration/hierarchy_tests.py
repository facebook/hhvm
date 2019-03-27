#! /usr/bin/env python3

import os

from typing import Optional


class HierarchyTests(object):

    template_repo: Optional[str] = 'hphp/hack/test/integration/data/hierarchy'

    def test_inheritance(self):
        """
        Test --inheritance-ancestors and --inheritance-children
        """
        self.start_hh_server()
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

    def test_inheritance_filter(self):
        self.start_hh_server()
        self.check_cmd([
            'File "{root}filter.php", line 15, characters 7-12: Filter',
            '    inherited from File "{root}filter.php", line 3, characters 7-13: CFilter',
            'File "{root}filter.php", line 18, characters 19-31: Filter::cfilterMethod',
            '    inherited from File "{root}filter.php", line 4, characters 19-31: CFilter::cfilterMethod',
        ], options=['--inheritance-ancestor-classes', 'Filter'])
        self.check_cmd([
            'File "{root}filter.php", line 15, characters 7-12: Filter',
            '    inherited from File "{root}filter.php", line 7, characters 11-17: IFilter',
            'File "{root}filter.php", line 19, characters 19-31: Filter::ifilterMethod',
            '    inherited from File "{root}filter.php", line 8, characters 19-31: IFilter::ifilterMethod',
        ], options=['--inheritance-ancestor-interfaces', 'Filter'])
        self.check_cmd([
            'File "{root}filter.php", line 15, characters 7-12: Filter',
            '    inherited from File "{root}filter.php", line 11, characters 7-13: TFilter',
            'File "{root}filter.php", line 20, characters 19-31: Filter::tfilterMethod',
            '    inherited from File "{root}filter.php", line 12, characters 19-31: TFilter::tfilterMethod',
        ], options=['--inheritance-ancestor-traits', 'Filter'])

    def test_method_signature_change(self):
        with open(os.path.join(self.repo_dir, 'qux.php'), 'w') as f:
            f.write('''<?hh //partial
class Qux {
  public function f() {
    $x = new Foo();
    $x->f();
  }
}
''')
        self.start_hh_server(changed_files=['qux.php'])
        self.check_cmd(['No errors!'])
        debug_sub = self.subscribe_debug()
        with open(os.path.join(self.repo_dir, 'foo.php'), 'w') as f:
            f.write('''<?hh //partial
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
