#! /usr/bin/env python3

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
