<?hh

trait Too {
  static function bar() :mixed{
    $abc = 123;
    $a = function ($abc) use ($abc, $abc) {
      var_dump($abc);
    }
;
    return $a;
  }
}
class Foo {
 use Too;
 }

<<__EntryPoint>>
function main_2083() :mixed{
$a = Foo::bar();
$a(456);
}
