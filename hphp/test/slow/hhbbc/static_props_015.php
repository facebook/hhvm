<?hh

class Foo {
  private static $a = array('foo' => array('bar'), 2, 3);

  public static function main() {
    unset(self::$a['foo']['bar']);
  }
  public static function get() {
    return self::$a;
  }
}

function main() {
  Foo::main();
  var_dump(Foo::get());
}
main();
