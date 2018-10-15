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
  }
}

class C<reified T1, reified T2> extends D<reified (int, (T1, string)), reified T1> {
  public function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T1));
    var_dump(__hhvm_intrinsics\get_reified_type(T2));
    parent::f();
  }
}

$c = new C<reified (int, string), reified string>();

$c->f();
