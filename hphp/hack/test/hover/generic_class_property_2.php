<?hh

final class CGeneric<T> {
  public ?Vector<T> $v;
  public function m<Tm>(T $x, Tm $y): void {}
}

function f(): void {
  $c = new CGeneric();
  $c->m(0, "");
  $c->v;
//    ^ hover-at-caret
}
