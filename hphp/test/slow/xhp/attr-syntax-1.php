<?hh
class C {
  public function getAttribute($name) {
    var_dump($name);
  }
}

<<__EntryPoint>>
function main_attr_syntax_1() {
$x = new C();
$y = $x->:foo:bar-baz;
}
