<?hh

// Ensure that a loop in trait definitions doesn't cause an infinite
// loop in the trait reuse check.
trait ReusedTrait {
  public final function foo(): void {}
  use OtherTrait;
}

trait OtherTrait {
  use ReusedTrait;
}

class MyParent {
  use ReusedTrait;
}

class BadClass extends MyParent {
  use ReusedTrait;
}
