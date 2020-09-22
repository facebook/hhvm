<?hh

trait TraitOne {
  public final function foo(): void {}
}
trait ReusedTrait {
  public final function bar(): void {}
  use TraitOne;
}

class MyBase {
  use ReusedTrait;
}

// This should complain about reuse of ReusedTrait, but not
// TraitOne. There's nothing wrong with TraitOne usage.
class BadClass extends MyBase {
  use ReusedTrait;
}
