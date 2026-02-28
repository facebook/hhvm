<?hh

trait Too {
  static function bar() :mixed{
    $abc = 123;
    $a = function (...$args) use ($abc) {
      $n = count($args);
      var_dump($n, $args);
    }
;
    return $a;
  }

  static function baz($obj) :mixed{
    $abc = 456;
    $obj(789);
  }
}
class Foo {
 use Too;
 }

<<__EntryPoint>>
function main_2085() :mixed{
$a = Foo::bar();
Foo::baz($a);
}
