<?hh

class C {
  public function takes_int(int $x): int {
    return $x;
  }
}

function test(C $c): void {
  $y = 42;
  $c->takes_int($y);
//               ^ enforcement-at-caret
}
