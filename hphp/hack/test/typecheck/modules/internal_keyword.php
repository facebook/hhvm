//// file1.php
<?hh
<<file:__EnableUnstableFeatures("modules")>>
module foo;
new module foo {}
class Foo {
  internal function bar(): void {}
}

internal class FooInternal extends Foo {
  public function bar(): void {} // ok
}

trait TFoo {
  internal function bar(): void {}
}
internal function foo_internal(): void {}

//// file2.php
<?hh
function test(FooInternal $y): void { // error on typehint
  $x = new Foo(); // ok
  foo_internal(); // error
  $x->bar(); // error
  $y->bar(); // ok
}
