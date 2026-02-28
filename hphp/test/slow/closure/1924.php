<?hh

class Foo {
  static function bar() :mixed{
    $abc = 123;
    $a = function ($abc) use ($abc) {
      var_dump($abc);
    }
;
    return $a;
  }
}

<<__EntryPoint>>
function main_1924() :mixed{
$a = Foo::bar();
$a(456);
}
