<?hh // strict
class C {
  <<__Rx>>
  public function __construct(public int $val) {}
}

<<__Rx>>
function basic(): void {
  $z = \HH\Rx\mutable(new C(7)); // $z is mutable
  \HH\Rx\freeze($z); // $z is immutable
  // error
  $z->val = 5;
}
