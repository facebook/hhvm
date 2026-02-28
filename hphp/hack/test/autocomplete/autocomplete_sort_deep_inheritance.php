<?hh

abstract class Bar {

  <<__AutocompleteSortText("1getFoo")>>
  public static function getFoo() : string;

  <<__AutocompleteSortText("2getBar")>>
  public static function getBar() : string;

}

abstract class Foo extends Bar {

}

final class ABC extends Foo {

  public function test(): void {
    self::gAUTO332
  }

}
