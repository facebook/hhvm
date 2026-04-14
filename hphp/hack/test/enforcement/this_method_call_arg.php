<?hh

class C {
  public function takes_int(int $x): int {
    return $x;
  }

  public function test(): void {
    $y = 42;
    $this->takes_int($y);
//                    ^ enforcement-at-caret
  }
}
