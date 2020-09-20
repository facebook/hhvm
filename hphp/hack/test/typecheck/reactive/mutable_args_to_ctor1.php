<?hh // partial

class B {
  <<__Rx>>
  public function __construct() {
  }
}

class A {
  <<__Rx>>
  public function __construct(B $a) {
  }
}

<<__Rx>>
function f(): void {
  $b = \HH\Rx\mutable(new B());
  // ERROR: passing mutable as immutable
  $v1 = new A($b);
}
