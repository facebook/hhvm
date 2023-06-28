<?hh

class X {

  private static $genX = 0;
  public static function gen() :AsyncGenerator<mixed,mixed,void>{
    yield ++self::$genX;
    yield 2;
    yield ++self::$genX;
  }
}
class Y extends X {
}

<<__EntryPoint>>
function main_2178() :mixed{
$g = X::gen();
foreach ($g as $i) var_dump($i);
}
