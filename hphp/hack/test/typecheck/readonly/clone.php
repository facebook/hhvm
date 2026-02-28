<?hh

class Bar {
  public int $val= 0;
}
class Foo {
  public function __construct(public Bar $prop) {}
}

<<__EntryPoint>>
function test(): void {
  $y = readonly new Foo(new Bar());
  $z = clone $y;
  // should error
  $z->prop->val = 5;
}
