<?hh

class Foo {
  private static $a = dict['foo' => 'a', 'bar' => 'b'];

  public static function main() :mixed{
    unset(self::$a['foo']);
  }
  public static function get() :mixed{
    return self::$a;
  }
}

function main() :mixed{
  Foo::main();
  var_dump(Foo::get());
}

<<__EntryPoint>>
function main_static_props_017() :mixed{
main();
}
