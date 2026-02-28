<?hh

class C1_SupTest extends WWWTest {

  public static function test(): void {
    // Should not be selected
    C1_Factory::makeSup();
  }

}
