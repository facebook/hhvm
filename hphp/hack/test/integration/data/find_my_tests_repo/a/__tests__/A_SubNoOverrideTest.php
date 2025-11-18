<?hh

class A_SubNoOverrideTest extends WWWTest {

  public function testTarget(): void {
    // We statically know that this may call A_Mid::target (directly)
    // or an override of it, if A_SubNoOverride had any subclasses.
    A_Factory::makeSubNoOverride()->target();
  }

}
