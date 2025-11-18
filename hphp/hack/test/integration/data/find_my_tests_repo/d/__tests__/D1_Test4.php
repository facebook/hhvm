<?hh

class D1_Test4 extends WWWTest {

  <<DataProvider('makeData')>>
  public static function test(D1 $d1): void {
    // FindMyTests has no dedicated handling for `BypassVisibility` or `DataProvider` at the moment.

    // Currently, this test is only being selected because it has a direct
    // reference to the type D1 (in its parameter type)

    BypassVisibility::invokeInstanceMethod($d1, 'willBeBypassed');
  }

  public static function makeData(): dict<string, shape('d1' => D1)> {
    return dict['testy test' => shape('d1' => D1_Factory::makeD1())];
  }

}
