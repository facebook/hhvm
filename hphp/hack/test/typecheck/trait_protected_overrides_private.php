<?hh

trait T1 {
  abstract protected function foo(): void;
}

trait T2 {
  private function foo(): void {}
}

// Ensure that C::foo overrides T2::foo (by emitting no errors here).
// A bug in Decl_inheritance caused us to inherit T2::foo here in the past.
class C {
  use T1, T2;

  final protected function foo(): void {}
}
