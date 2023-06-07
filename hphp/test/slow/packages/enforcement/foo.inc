<?hh

module x.y; // outside the active deployment

function foo() {
    echo "in foo\n";
}

class Foo {
  public static string $x = 'foo';

  function __construct() {
    echo "in Foo constructor\n";
  }

  static function foo() {
    var_dump($x);
  }
}

enum EnumFoo: string as string {
  FOO = 'foo';
}

enum class EnumClsFoo: Foo {
  Foo FOO = new Foo();
}
