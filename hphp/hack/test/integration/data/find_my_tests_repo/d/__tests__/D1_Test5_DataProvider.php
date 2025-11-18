<?hh

trait D1_Test5_DataProvider {

  public static function makeData(): dict<string, shape('d1' => D1)> {
    return dict['testy test' => shape('d1' => D1_Factory::makeD1())];
  }

}
