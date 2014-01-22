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

main();

