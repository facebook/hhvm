<?hh // strict
class C {
  <<__Rx>>
  public function __construct(public int $val) {}
}

<<__Rx>>
function basic(): void {
  $z = \HH\Rx\mutable(new C(7)); // $z is mutable
  $z->val = 5; // okay
  $z = 7; // error, cannot change mutability flavor of the local
  $b = $z; // can reassign an immutable object

  // $x is mutable(mutably owned)
  $x = \HH\Rx\mutable(returnsMut());
  // error, cannot reassign a mutable object
  $y = $x;
}

<<__Rx, __MutableReturn>>
function returnsMut(): C {
  // UNSAFE
}
