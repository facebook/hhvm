<?hh

// Ensure that a loop in class definitions doesn't cause an infinite
// loop in the trait reuse check.
trait ReusedTrait {
  public final function foo(): void {}
}

class MyParent extends BadClass {
  use ReusedTrait;
}

class BadClass extends MyParent {
  use ReusedTrait;
}
