<?hh

class A_SubNoOverrideTest extends WWWTest {

  public function testTarget(A_SubNoOverride $a): void {
    // We statically know that this may call A_Mid::target (directly)
    // or an override of it, if A_SubNoOverride had any subclasses.
    $a->target();
  }
}
