<?hh

final class CGeneric<T> {
  public function m<Tm>(T $x, Tm $y): void {}
}

function f(): void {
  $c = new CGeneric();
  $c->m(0, "");
//    ^ hover-at-caret
}
