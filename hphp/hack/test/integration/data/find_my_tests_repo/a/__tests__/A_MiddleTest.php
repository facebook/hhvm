<?hh

class A_MiddleTest extends WWWTest {

  public function testTarget(): void {
    A_Factory::makeMiddle()->target();
  }
}
