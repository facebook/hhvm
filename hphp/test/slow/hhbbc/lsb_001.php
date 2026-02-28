<?hh

class Foo {
  public static function ok() :mixed{
    return static::bar();
  }

  public static function bar() :mixed{
    return "heh";
  }
}

function main() :mixed{
  $y = Foo::ok();
  echo $y;
  echo "\n";
}



<<__EntryPoint>>
function main_lsb_001() :mixed{
main();
}
