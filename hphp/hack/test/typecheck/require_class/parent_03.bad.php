<?hh

trait T {
  require class D;

  public function foo(): int {
    return parent::bar();
  }
}


abstract class C {
  public static abstract function bar(): int;
}

final class D extends C {
  use T;

  public static function bar(): int {
    return 42;
  }
}
