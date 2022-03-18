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

// This should complain about reuse:
//   BadClass -> ReusedTrait
// but not about:
//   BadClass -> ReusedTrait -> TraitOne
//
// The latter error is redundant, and these errors are more common in
// large class hierarchies where repetition can be overwhelming.
class BadClass extends MyBase {
  use ReusedTrait;
}
