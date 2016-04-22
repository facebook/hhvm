<?hh

class C {
  public ?int $foo;
}

function test(C $c) {
  $c->foo;
}
