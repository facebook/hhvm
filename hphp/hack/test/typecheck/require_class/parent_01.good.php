<?hh

trait T {
  require class D;

  public function foo(): int {
    return parent::bar();
  }
}

class C {
  public static function bar(): int {
    return 42;
  }
}

final class D extends C {
  use T;
}
