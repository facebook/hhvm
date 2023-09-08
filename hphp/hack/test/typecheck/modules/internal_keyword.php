//// modules.php
<?hh


new module foo {}

//// test.php
<?hh

module foo;
class Foo {
  internal function bar(): void {}
}
public interface IFoo {
  public function bar(): void;
}

internal class FooInternal extends Foo implements IFoo {
  public function bar(): void {} // ok
}

internal trait TFoo {
  internal function bar(): void {}
}
internal function foo_internal(): void {}

//// file2.php
<?hh
function test(FooInternal $y): void { // error on typehint
  $x = new Foo(); // ok
  foo_internal(); // error
  $x->bar(); // error
  // The following two are both fine at runtime
  $y->bar(); // error when $y is a FooInternal: $y is opaque
  ($y as IFoo)->bar(); // ok since $y is now a known type
}
