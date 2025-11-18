<?hh

class A_SubTest extends WWWTest {

  public function testTarget(): void {
    // We statically know that this isn't calling A_Mid::target (directly),
    // because A_Sub overrides it.
    A_Factory::makeSub()->target();
  }

}
