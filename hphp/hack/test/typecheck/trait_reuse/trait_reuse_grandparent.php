<?hh

trait ReusedTrait {
  public final function foo(): void {}
}

class One {
  use ReusedTrait;
}

class Two extends One {
}

class BadClass extends Two {
  use ReusedTrait;
}
