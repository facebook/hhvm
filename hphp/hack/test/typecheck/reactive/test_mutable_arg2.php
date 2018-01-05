<?hh // strict

class Foo {
  <<__Rx>>
  public function __construct(public int $value) {}
}
// variadic argument
<<__Rx>>
function foo(<<__Mutable>> Foo $x, <<__Mutable>> Foo ...$ys): void {
  $x->value = 5;
}

<<__Rx>>
function test(): void {
  $y = new Foo(7);
  freeze($y);
  $x = new Foo(8);
  foo($x, $x, new Foo(6), $y, $x);
}
