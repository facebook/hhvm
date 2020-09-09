<?hh // strict
class Something {
  public function __construct(public int $val) {}

  <<__Rx, __Mutable>>
  public function setVal(int $x): void {
    $this->val = $x;
  }
}
class Test {
  <<__Rx, __MutableReturn>>
  public function get(): Something {
    // UNSAFE
  }
}
<<__Rx>>
function foo(Test $x): void {
  // $x->get returns mutable
  $z = \HH\Rx\mutable($x->get());
  // this is allowed
  $z->val = 5;
  // so is this
  $z1 = \HH\Rx\freeze($z);
  hh_show($z1);
  // not allowed, $z1 is immutable
  $z1->setVal(7);
}
