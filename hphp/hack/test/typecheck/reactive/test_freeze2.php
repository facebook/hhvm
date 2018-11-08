<?hh // strict
class C {
  <<__Rx>>
  public function __construct(public int $val) {}
}

// freeze in multiple scopes
<<__Rx>>
function basic(bool $b): void {
  $z = \HH\Rx\mutable(new C(7)); // $z is mutable
  if ($b) {
    $z1 = \HH\Rx\freeze($z);
  } else {
    $z->val = 5;
    $z1 = \HH\Rx\freeze($z);
  }
  // valid, $z1 is now immutable
  $y = \HH\Rx\mutable(new C(2));
  if ($b) {
    $y1 = \HH\Rx\freeze($y);
  } else {

  }
  $y;
  // invalid, $y is frozen in one scope but not another

}
