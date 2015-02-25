<?hh


class A {
  public static function __callStatic($fn, $args) {
    var_dump($fn, $args);
  }
}

class Test {
  public static function main() {
    $cls = 'A';
    $cls::foo(1, 2, 3);
  }
}

Test::main();
