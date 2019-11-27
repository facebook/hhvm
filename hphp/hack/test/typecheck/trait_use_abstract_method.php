<?hh

trait MyTrait {
  abstract static public function foo(): void;

  public static function bar(): void {
    self::foo();
  }
}

abstract class MyExampleParent {
  use MyTrait;
  // MyExampleParent::bar() will call self::foo(),
  // which resolves to the current class. We end up
  // calling MyExampleParent::foo() and erroring.
}

class MyExample extends MyExampleParent {
  // This method is ignored.
  static public function foo(): void {}
}

<<__EntryPoint>>
function main(): void {
  MyExample::bar();
}
