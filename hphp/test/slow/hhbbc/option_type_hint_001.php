<?hh

class Foo {
  public function __construct(?Foo $x) {
    var_dump($x);
  }
}


<<__EntryPoint>>
function main_option_type_hint_001() :mixed{
new Foo(1);
}
