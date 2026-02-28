<?hh

class C2_SupTest extends WWWTest {

  public static function test(): void {
    // Should not be selected
    C2_Factory::makeSup();
  }

}
