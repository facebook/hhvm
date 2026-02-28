<?hh

class Foo {
  static function bar() :mixed{
    $abc = 123;
    $a = function (...$args) use ($abc) {
      var_dump(count($args), $args);
    }
;
    return $a;
  }

  static function baz($obj) :mixed{
    $abc = 456;
    $obj(789);
  }
}

<<__EntryPoint>>
function main_1927() :mixed{
$a = Foo::bar();
Foo::baz($a);
}
