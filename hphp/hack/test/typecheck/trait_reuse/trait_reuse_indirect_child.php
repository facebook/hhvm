<?hh

trait ReusedTrait {
  public final function foo(): void {}
}
trait TraitTwo { use ReusedTrait; }

class MyBase {
  use ReusedTrait;
}

class BadClass extends MyBase {
  // This ends up reusing ReusedTrait. We should display both routes
  // to ReusedTrait.
  use TraitTwo;
}
