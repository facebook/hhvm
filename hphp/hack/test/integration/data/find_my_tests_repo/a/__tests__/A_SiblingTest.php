<?hh

class A_SiblingTest extends WWWTest {

  public function testTarget(): void {
    // We statically know that this will not call A_Mid::target
    A_Factory::makeSibling()->target();
  }

}
