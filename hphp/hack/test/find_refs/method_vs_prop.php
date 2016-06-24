<?hh

class C {
  const int foo = 3;
  public ?string $foo;
  public function foo() {}
}

function test(C $c) {
  $c->foo;
  $c->foo();
  C::foo;
}
