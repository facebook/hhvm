<?hh

class D1_Test5 extends WWWTest {

  use D1_Test5_DataProvider;

  <<DataProvider('makeData')>>
  public static function test(D1 $d1): void {
    // This is similar to `D1_Test4`, but this time the `makeData` function is coming from a trait.
    //
    // As for `D1_Test5`, the reason this test is currently selected is the usage of `D1` in the parameter type.
    BypassVisibility::invokeInstanceMethod($d1, 'willBeBypassed');
  }

}
