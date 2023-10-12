<?hh // strict

class C {
  public function foo(): void {}
}

class D extends C {}

class E {
  public function foo(): void {}
}

function test(C $c, D $d, E $e): void {
  $c->foo();
  $d->foo();
  $e->foo();
}
