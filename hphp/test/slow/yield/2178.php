<?hh

class X {

  private static $genX;
  public static function gen() {
    yield ++self::$genX;
    yield 2;
    yield ++self::$genX;
  }
}
class Y extends X {
}

<<__EntryPoint>>
function main_2178() {
$g = X::gen();
foreach ($g as $i) var_dump($i);
}
