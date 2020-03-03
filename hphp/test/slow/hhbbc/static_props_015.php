<?hh

class Foo {
  private static $a = darray['foo' => varray['bar'], 0 => 2, 1 => 3];

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

<<__EntryPoint>>
function main_static_props_015() {
main();
}
