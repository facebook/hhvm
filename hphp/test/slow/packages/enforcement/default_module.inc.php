<?hh // in the active deployment

function default_foo() {
    echo "in default_foo\n";
}

class DefaultFoo {
  public static string $x = 'in DefaultFoo::static_foo';

  function __construct() {
    echo "in DefaultFoo constructor\n";
  }

  static function static_foo() {
    var_dump(self::$x);
  }
}
