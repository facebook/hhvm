<?hh

<<__PPL>>
final class TestPPL {
  public function foo(): void {
    // We want to error on the inner sample as an unbound global id
    sample(sample);
  }
}
