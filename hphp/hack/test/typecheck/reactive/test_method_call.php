<?hh // strict
class Something {
  public function __construct(public int $val) {}

  <<__Rx, __Mutable>>
  public function setVal(int $x) {
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
  $z = $x->get();
  // this is allowed
  $z->val = 5;
  // so is this
  \HH\Rx\freeze($z);
  // not allowed, $z is immutable
  $z->setVal(7);
}
