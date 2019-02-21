<?hh // partial
class P {
  public function foo(): num {
    return 0;
  }
}
trait T {
  public function foo(): int {
    return 0;
  }
}
class C extends P {
  use T; // no error, trait definition is unique
}
