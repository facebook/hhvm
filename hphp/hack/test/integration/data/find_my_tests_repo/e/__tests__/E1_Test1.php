<?hh

class E1_Test1 extends WWWTest {
  public function test(): void {
    // This will just be picked up by FindMyTests because we mention E1 from a test file.
    // No special enum handling required here
    foreach (E1::getNames() as $key => $value) {
    }
  }
}
