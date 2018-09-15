<?hh // strict
class C {
  <<__Rx>>
  public function __construct(public int $val) {}
}

// freeze in multiple scopes
<<__Rx>>
function basic(): void {
  $z = \HH\Rx\mutable(new C(7)); // $z is mutable
  if (true) {
    \HH\Rx\freeze($z);
  } else {
    $z->val = 5;
    \HH\Rx\freeze($z);
  }
  // valid, $z is now immutable
  $y = \HH\Rx\mutable(new C(2));
  if (true) {
    \HH\Rx\freeze($y);
  } else {

  }
  // invalid, $y is frozen in one scope but not another

}
