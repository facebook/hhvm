<?hh

class E2_Test1 extends WWWTest {
  public function test(): void {
    // This will just be picked up by FindMyTests because we mention E2 from a test file.
    // No special enum handling required here
    foreach (E2::getValues() as $key => $value) {
    }
  }
}
