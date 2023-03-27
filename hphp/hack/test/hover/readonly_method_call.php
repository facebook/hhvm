<?hh

class Foo {
  // This is a property.
  public (function(): void) $foo;

  public function __construct() {
    $this->foo = () ==> {};
  }

  // This is a method.
  public readonly function foo(): void {}

  public readonly function callReadonlyMethod(): void {
    $this->foo();
    //     ^ hover-at-caret
  }
}
