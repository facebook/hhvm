<?hh

class C1_SubTest extends WWWTest {

  public static function test(): void {
    // Should not be selected
    C1_Factory::makeSub();
  }

}
