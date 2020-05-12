<?hh

// We don't want to error on special PPL function name calls in __PPL classes
<<__PPL>>
final class TestPPL {
  public function foo(): void {
    sample();
  }
}

// But we do in non __PPL classes
final class TestNonPPL {
  public function foo(): void {
    sample();
  }
}
