<?hh

class Foo {

  public static int $i = 0;

  public function __construct()[] : void {
    self::$i = 4; // error
  }

}
