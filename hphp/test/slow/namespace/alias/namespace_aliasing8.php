<?hh

abstract final class C {
  public static function f(): void {
    // Explicitly testing omitting the namespace declaration and
    // closures.
    self::invoke(() ==> Dict\foo());
  }

  private static function invoke((function(): void) $f): void {
    $f();
  }
}

<<__EntryPoint>>
function f(): void {
  C::f();
}
