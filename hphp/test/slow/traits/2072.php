<?hh

trait T {

  private static $genX =0;
  public static function gen() :AsyncGenerator<mixed,mixed,void>{
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
function main_2072() :mixed{
$g = X::gen();
foreach ($g as $i) var_dump($i);
}
