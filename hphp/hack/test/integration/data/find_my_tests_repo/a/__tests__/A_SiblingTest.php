<?hh

class A_SiblingTest extends WWWTest {

  public function testTarget(A_Sibling $a): void {
    // We statically know that this will not call A_Mid::target
    $a->target();
  }

}
