<?hh

class C {
  public int $prop = 0;
}

function test(C $c): void {
  $x = 42;
  $c->prop = $x;
//           ^ enforcement-at-caret
}
