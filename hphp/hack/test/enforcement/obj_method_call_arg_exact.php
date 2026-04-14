<?hh

class C {
  public function takes_int(int $x): int {
    return $x;
  }
}

function test(): void {
  $c = new C();
  $y = 42;
  $c->takes_int($y);
//               ^ enforcement-at-caret
}
