<?hh

interface A {
  public function f(&$x);
}

class B implements A {
  public function f($x) {}
}
