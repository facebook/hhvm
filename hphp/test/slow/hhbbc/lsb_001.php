<?hh

class Foo {
  public static function ok() {
    return static::bar();
  }

  public static function bar() {
    return "heh";
  }
}

function main() {
  $y = Foo::ok();
  echo $y;
  echo "\n";
}



<<__EntryPoint>>
function main_lsb_001() {
main();
}
