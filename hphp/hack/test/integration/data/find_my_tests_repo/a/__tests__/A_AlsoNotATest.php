<?hh


// Notably, this does not extend WWWTest
class A_AlsoNotATest {
  public function testTarget(A_Sub $a): void {
    $a->target();
  }
}
