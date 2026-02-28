<?hh
<<__AutocompleteSortText('XYZ')>>
class ABC {

  private static function getFoo() : string {
    return 'foo';
  }

  private static function getBar() : string {
    return 'bar';
  }

}

final class DEF extends ABC {

  private function three(): void {}

}
