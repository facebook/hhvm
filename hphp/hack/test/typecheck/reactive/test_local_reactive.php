<?hh // strict
class Foo {
  public function __construct(public int $val) {}
}
<<__RxLocal>>
function test(Foo $x): void {
  // Calling non reactive functions is okay
  test2();
  // Setting properties is not
  $x->val = 5;
}

function test2(): void {}
