<?hh // strict

class E<reified T1, reified T2> {
  public function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T1));
    var_dump(__hhvm_intrinsics\get_reified_type(T2));
  }
}

class D<reified T1, reified T2> extends E<reified (T1, T1), reified T2> {
  public function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T1));
    var_dump(__hhvm_intrinsics\get_reified_type(T2));
    parent::f();
  }
}

class C extends D<reified (int, (int, string)), reified int> {
  public function f() {
    parent::f();
  }
}

$c = new C();

$c->f();
