<?hh

class MyClass {
  // I am a property.
  private (function(): void) $foo;

  public function __construct((function(): void) $foo) {
    $this->foo = $foo;
  }

  // I am a method.
  private function foo(): void {
    // This is using the property, not the method.
    ($this->foo)();
    //      ^ hover-at-caret
  }
}
