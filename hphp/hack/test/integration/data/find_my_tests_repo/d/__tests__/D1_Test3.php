<?hh

class D1_Test3 extends WWWTest {

  public static function test(): void {
    // FindMyTests has no dedicated handling for `BypassVisibility` at the moment.

    // Note that this test is selected because it directly references `D1`.
    $d1 = new D1();
    BypassVisibility::invokeInstanceMethod($d1, 'willBeBypassed');
  }

}
