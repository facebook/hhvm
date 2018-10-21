<?hh // strict
class C {
  <<__Rx>>
  public function __construct(public int $val) {}
}

<<__Rx>>
function basic(): void {
  $z = \HH\Rx\mutable(new C(7)); // $z is mutable
  $z1 = \HH\Rx\freeze($z); // $z1 is immutable
  // error
  $z1->val = 5;
}
