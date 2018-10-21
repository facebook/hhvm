<?hh // strict

class Foo {
  <<__Rx>>
  public function __construct(public int $value) {}
}
// variadic argument
<<__Rx>>
function foo(<<__Mutable>>Foo $x, <<__Mutable>>Foo ...$ys): void {
  $x->value = 5;
}

<<__Rx>>
function test(): void {
  $y = \HH\Rx\mutable(new Foo(7));
  $y1 = \HH\Rx\freeze($y);
  $x = \HH\Rx\mutable(new Foo(8));
  foo($x, $x, \HH\Rx\mutable(new Foo(6)), $y1, $x);
}
