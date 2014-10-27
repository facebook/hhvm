<?hh
class C {
  public function getAttribute($name) {
    var_dump($name);
  }
}
$x = new C();
$y = $x->:foo:bar-baz;
