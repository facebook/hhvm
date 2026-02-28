<?hh

class Foo {
  private static $a = "hehehehehehe";

  public static function main() :mixed{
    unset(self::$a[2]['bar']);
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
function main_static_props_016() :mixed{
main();
}
