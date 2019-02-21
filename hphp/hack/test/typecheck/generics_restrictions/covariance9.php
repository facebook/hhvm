<?hh // partial

interface Y<+T1, -T2> {}

class X<+T> {
  public function test((function(Y<int, T>): void) $f): void {
  }
}
