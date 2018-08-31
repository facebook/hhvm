<?hh
class C {
  public function getAttribute($name) {
    var_dump($name);
  }
}

<<__EntryPoint>>
function main_attr_syntax_3() {
$x = new C();
$x->:foo:bar-baz = 123;
}
