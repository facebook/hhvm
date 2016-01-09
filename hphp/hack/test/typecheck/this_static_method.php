<?hh

/* Check that we correctly handle static method lookups on abstract types
 * constrained by Tvars */
final class Foo {
  public static function f(): int {}
  public function g(): string {
    if (true) {
    }
    hh_show($this);
    return $this::f();
  }
}
