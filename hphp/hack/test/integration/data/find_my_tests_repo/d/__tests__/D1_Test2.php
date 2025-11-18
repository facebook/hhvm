<?hh

class D1_Test2 extends WWWTest {

  public static function test(): void {
    // FindMyTests has no dedicated handling for `BypassVisibility` at the moment.
    //
    // This test is selected because of the path to `D1` via `new D1` in `D1_Factory`
    $d = D1_Factory::makeD1();
    BypassVisibility::invokeInstanceMethod($d, 'willBeBypassed');
  }

}
