<?hh // strict
class C {
  <<__Rx>>
  public function __construct(public int $val) {}
}

// freeze in multiple scopes
<<__Rx>>
function basic(): void {
  $z = new C(7); // $z is mutable
  if (true) {
    freeze($z);
  } else {
    $z->val = 5;
    freeze($z);
  }
  // valid, $z is now immutable
  $y = new C(2);
  if (true) {
    freeze($y);
  } else {

  }
  // invalid, $y is frozen in one scope but not another

}
