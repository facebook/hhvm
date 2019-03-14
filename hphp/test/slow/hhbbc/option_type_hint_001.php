<?hh

class Foo {
  public function foo(?Foo $x) {
    var_dump($x);
  }
}


<<__EntryPoint>>
function main_option_type_hint_001() {
new Foo(1);
}
