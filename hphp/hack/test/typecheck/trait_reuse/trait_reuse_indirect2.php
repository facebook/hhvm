<?hh

trait ReusedTrait {
  public final function foo(): void {}
}
trait TraitTwo { use ReusedTrait; }

class MyBase {
  use TraitTwo;
}

class BadClass extends MyBase {
  use TraitTwo;
}
