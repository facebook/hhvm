<?hh

class C2_SubTest extends WWWTest {

  public static function test(): void {
    // Should not be selected
    C2_Factory::makeSub();
  }

}
