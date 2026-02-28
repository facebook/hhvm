<?hh
class C {
  public function getAttribute($name) :mixed{
    var_dump($name);
  }
}

<<__EntryPoint>>
function main_attr_syntax_1() :mixed{
$x = new C();
$y = $x->:foo:bar-baz;
}
