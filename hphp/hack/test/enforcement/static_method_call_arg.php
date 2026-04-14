<?hh

class C {
  public static function takes_int(int $x): void {}
}

function test(): void {
  $y = 42;
  C::takes_int($y);
//             ^ enforcement-at-caret
}
