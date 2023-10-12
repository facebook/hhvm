<?hh // strict

trait FooTrait {
  // We should report the redundant type parameter here.
  public function bar<T>(T $_t): void {}
}

// But not here.
class UsesTrait {
  use FooTrait;
}
