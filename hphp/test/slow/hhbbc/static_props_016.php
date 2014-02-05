<?hh

class Foo {
  private static $a = "hehehehehehe";

  public static function main() {
    unset(self::$a[2]['bar']);
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
