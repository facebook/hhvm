<?hh
class C {
  public function getAttribute($name) {
    var_dump($name);
  }
}
$x = new C();
$x->:foo:bar-baz = 123;
