<?hh // strict

class Test {
  <<__Rx, __Mutable>>
  public function __construct(public int $val) {}
}

class Foo {
  <<__Rx>>
  public function bar(<<__Mutable>>Test $x, <<__Mutable>>Test $y): void {
    $x->val = 5;
  }
}
<<__Rx>>
function test(): void {
  $x = new Test(4);
  $y = new Test(7);
  $z = new Foo();
  $z->bar($x, $y);
  \HH\Rx\freeze($x);
  $z->bar($x, $y);
}
