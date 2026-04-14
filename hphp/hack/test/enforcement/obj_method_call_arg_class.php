<?hh

class C {
  public function takes_int(int $x): void {}

  public function test(C $c): void {
    $y = 42;
    $c->takes_int($y);
//                 ^ enforcement-at-caret
  }
}
