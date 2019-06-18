<?hh // strict

class Test {
  <<__Rx>>
  public function __construct(public int $val) {}
}

class Foo {
  <<__Rx, __Mutable>>
  public function bar(<<__Mutable>>Test $x, <<__Mutable>>Test $y): void {
    $x->val = 5;
  }
}
<<__Rx>>
function test(): void {
  $x = \HH\Rx\mutable(new Test(4));
  $y = \HH\Rx\mutable(new Test(7));
  $z = \HH\Rx\mutable(new Foo());
  $z->bar($x, $y);
  $x1 = \HH\Rx\freeze($x);
  $z->bar($x1, $y);
}
