<?hh

class D1_Test6 extends WWWTest {

  public static function test(): void {
    // FindMyTests has no dedicated handling for `BypassVisibility` at the moment.
    //
    // This test is selected because of the call to, which in turn uses
    // `D1_TestHelper::getClass()` uses `D1::class`.
    //
    // Note that the current file has no direct reference to the type `D1`!
    $class = D1_TestHelper::getClass();
    BypassVisibility::invokeStaticMethod($class, 'willBeBypassed');
  }

}
