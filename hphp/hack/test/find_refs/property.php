<?hh

class C {
  const int foo = 3;
  public ?string $foo;
  public function foo() {}

  public function test() {
    $this->foo;
  }
}

class D extends C {
  public function test() {
    $this->foo;
  }
}

type E = C;
newtype F = C;

function test(C $c, D $d, E $e, F $f) {
  $c->foo;
  $d->foo;
  $e->foo;
  $f->foo;
  $c->foo();
  C::foo;
}
