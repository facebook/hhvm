<?hh // partial

class B {
  <<__Rx>>
  public function __construct() {
  }
}

class A {
  <<__Rx>>
  public function __construct(<<__Mutable>> B $a) {
  }
}

<<__Rx>>
function f(): void {
  $b = new B();
  // ERROR: passing immutable as mutable
  $v1 = new A($b);
}
