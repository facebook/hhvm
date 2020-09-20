<?hh // partial
class A {
  public function __construct(public int $a) {}

  <<__Rx, __Mutable>>
  public function f(): void {
    $this->a = 5;
  }

  <<__Rx>>
  public static function g(<<__Mutable>>A $a) {
    $a->a = 5;
  }
}

function f() {
  $a = new A(1);
  // OK - non-rx context
  $a->f();
  // OK - non-rx context
  A::g($a);
}

<<__Rx>>
function g() {
  // non reactive lambda - ok since it is not called
  $a = <<__NonRx>>() ==> {
    $a = new A(1);
    // non-rx context
    $a->f();
  };
  $b = <<__NonRx>>() ==> {
    $a = new A(1);
    // non reactive lambda - ok since it is not called
    A::g($a);
  };
}
