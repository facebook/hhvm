<?hh

class C {
  public function foo() {}
}

class D extends C {}

function test(D $d) {
  $d->foo();
}
