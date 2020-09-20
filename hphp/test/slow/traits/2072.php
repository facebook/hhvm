<?hh

trait T {

  private static $genX;
  public static function gen() {
    yield ++self::$genX;
    yield 2;
    yield ++self::$genX;
  }
}
class X {
 use T;
 }
class Y extends X {
}

<<__EntryPoint>>
function main_2072() {
$g = X::gen();
foreach ($g as $i) var_dump($i);
}
