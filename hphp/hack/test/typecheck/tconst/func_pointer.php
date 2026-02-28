<?hh

class C {
  const type T = int;

  public static function foo(this::T $x): void {}
}

class Test {
  private static function f1((function(int): void) $x): void {}

  public function f2(): void {
    $y = C::foo<>;
    self::f1($y);
  }

}
