<?hh
class C {
  public function getAttribute($name) :mixed{
    var_dump($name);
  }
}

<<__EntryPoint>>
function main_attr_syntax_3() :mixed{
$x = new C();
$x->:foo:bar-baz = 123;
}
