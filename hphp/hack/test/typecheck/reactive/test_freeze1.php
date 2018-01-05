<?hh // strict
class C {
  <<__Rx>>
  public function __construct(public int $val) {}
}

<<__Rx>>
function basic(): void {
  $z = new C(7); // $z is mutable
  freeze($z); // $z is immutable
  // error
  $z->val = 5;
}
