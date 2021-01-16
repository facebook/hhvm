<?hh // strict
class C {
  <<__Rx>>
  public function __construct(public int $val) {}
}

// freeze in multiple scopes
<<__Rx>>
function basic(): void {
  $z = \HH\Rx\mutable(new C(7)); // $z is mutable
  $z1 = \HH\Rx\freeze($z);
  // error, $z is moved and $z1 is immutable
  $z2 = \HH\Rx\freeze($z1);
}
