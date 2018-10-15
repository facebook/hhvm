<?hh // strict

class E<reified T1, reified T2> {
  public function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T1));
    var_dump(__hhvm_intrinsics\get_reified_type(T2));
  }
}

class D extends E<reified (int, int), reified int> {
  public function f() {
    parent::f();
  }
}

class C extends D {
  public function f() {
    parent::f();
  }
}

$c = new C();

$c->f();
