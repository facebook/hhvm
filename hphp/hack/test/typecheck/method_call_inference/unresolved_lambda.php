<?hh

class A {
  public function foo() : void {}
}

class B {}

class C {
  public function foo() : bool {
    return true;
  }
}

function test() : void {
  $lambda = $x ==> $x->foo();
  $func = function ($x) { $x->foo(); };
  $a = new A();
  $b = new B();
  $c = new C();

  $lambda($a);
  $lambda($b); // Error - B doesn't implement method foo
  $lambda($c);
  $func($a);
  $func($b); // Error - B doesn't implement method foo
  $func($c);
}
