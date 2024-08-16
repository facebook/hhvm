<?hh

class C<T> {
  public function __construct(private T $item) {}
  public function pairwith<Tu>(Tu $x): (T,Tu) {
    return tuple($this->item, $x);
  }
}

function testit():void {
  $c = new C(23);
  $y = $c->pairwith("A");
  //       ^ hover-at-caret
}
