<?hh

trait ReusedTrait {
  public final function foo(): void {}
}

trait XOne {
  use ReusedTrait;
}

trait YOne { use ReusedTrait; }
trait YTwo { use YOne; }

// There are multiple routes from MyParent -> ReusedTrait, and we
// should display the longer route (via YOne), because it's the least
// obvious to the reader.
class MyParent {
  use YTwo;
  use XOne;
}
