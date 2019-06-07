<?hh

class Foo {
  static function bar() {
    $abc = 123;
    $a = function (...$args) use ($abc) {
      var_dump(count($args), $args);
    }
;
    return $a;
  }

  static function baz($obj) {
    $abc = 456;
    $obj(789);
  }
}

<<__EntryPoint>>
function main_1927() {
$a = Foo::bar();
Foo::baz($a);
}
