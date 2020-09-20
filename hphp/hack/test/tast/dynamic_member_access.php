<?hh // partial

function returnsTheStringFoo(): string { return 'foo'; }

function test(dynamic $c): void {
  $foo = 'foo';
  $c->foo;
  $c->$foo;
  $c->{returnsTheStringFoo()};
}
