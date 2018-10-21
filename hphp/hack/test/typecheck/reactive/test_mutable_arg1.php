<?hh // strict

class Foo {
  <<__Rx>>
  public function __construct(public int $value) {}
}

<<__Rx>>
function foo(<<__Mutable>>Foo $x): void {
  $x->value = 5;
}

<<__Rx>>
function test(): void {
  $x = \HH\Rx\mutable(new Foo(7));
  foo($x); // no errors
  foo(\HH\Rx\mutable(new Foo(8))); // no problemo
  $y = \HH\Rx\freeze($x);
  foo($y); // error
}
