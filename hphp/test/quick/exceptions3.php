<?php
function newobj() {
  return new C;
}
class C {
  public function __destruct() {
    echo "C::__destruct\n";
  }
  public function g($x, $y, $z) {
    return $y;
  }
  public function foo($z) {
    $w2 = 2;
    newobj()->g(newobj(), 7, 8, $w2 = $this->foo(newobj()));
  }
}
function bar() {
  $obj = new C;
  newobj()->foo(123);
}
bar();
