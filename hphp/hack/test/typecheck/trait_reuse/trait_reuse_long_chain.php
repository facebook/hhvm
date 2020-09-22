<?hh

trait ReusedTrait {
  public final function foo(): void {}
}

trait TraitTwo {
  use ReusedTrait;
}

trait TraitThree {
  use TraitTwo;
}

class GrandParent {
  use TraitThree;
}

class MyParent extends GrandParent {
}

class BadClass extends MyParent {
  use ReusedTrait;
}
