<?hh

module z.z; // soft deployed in the active deployment

function soft_foo() {
    echo "in soft_foo\n";
}

class SoftFoo {
  public static string $static_foo = 'foo';

  function __construct() {
    echo "in SoftFoo constructor\n";
  }

  static function static_foo() {
    var_dump(self::$static_foo);
  }
}

enum SoftEnumFoo: string as string {
  FOO = 'foo';
}

enum class SoftEnumClsFoo: SoftFoo {
  SoftFoo FOO = new SoftFoo();
}
