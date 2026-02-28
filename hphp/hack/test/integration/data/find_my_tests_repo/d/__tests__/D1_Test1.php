<?hh

class D1_Test1 extends WWWTest {

  public static function test(): void {
    // FindMyTests has no dedicated handling for `BypassVisibility` at the moment.
    //
    // This test is selected because it directly references the class `D1` that was changed.
    BypassVisibility::invokeStaticMethod(D1::class, 'willBeBypassed');
  }

}
