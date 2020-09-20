<?hh

class Foo {}
class Bar {
  const FOO = Foo::class;
  public static function sm() {
    var_dump("sm");
  }
  public static int $sp = 10;
  public function m() {
    var_dump("m");
  }
}

<<__EntryPoint>>
function main() {
  $c = Bar::class;
  var_dump($c);
  $v = vec[Bar::class, Fizz::class]; // Fizz is not a class
  var_dump($v);
  var_dump($c::FOO);
  $c::sm();
  var_dump($c::$sp);
  $o = new $c;
  $o->m();
  $f = Fizz::class;
  $o = new $f;  // should error as Fizz is not a class
}
