<?hh

interface IFoo {

  <<__AutocompleteSortText("2getFoo")>>
  public static function getFoo() : string;

  <<__AutocompleteSortText("1getBar")>>
  public static function getBar() : string;

}

interface IBaz {

  public static function getFoo() : string;

  public static function getBar() : string;

}

abstract class ABC implements IFoo, IBaz {

  public static function getFoo() : string {
    return 'foo';
  }

  public static function getBar() : string {
    return 'bar';
  }

}

abstract class DEF implements IBaz, IFoo {

  public static function getFoo() : string {
    return 'foo';
  }

  public static function getBar() : string {
    return 'barrrrr';
  }

  public function three(): void {
  }

}
