<?hh // partial

class C {
  public ?int $foo;
}

function returnsTheStringFoo(): string { return 'foo'; }

function test(C $c): void {
  $foo = 'foo';
  $c->foo;
  $c->$foo;
  $c->{returnsTheStringFoo()};
}
