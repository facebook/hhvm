<?hh

class C {
  public int $prop = 0;
}

function test(): void {
  $c = new C();
  $x = 42;
  $c->prop = $x;
//           ^ enforcement-at-caret
}
