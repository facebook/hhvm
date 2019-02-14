<?hh // strict

class E<reify T1, reify T2> {
  public function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T1));
    var_dump(__hhvm_intrinsics\get_reified_type(T2));
  }
}

class D<reify T1, reify T2> extends E<reify (T1, T1), reify T2> {
  public function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T1));
    var_dump(__hhvm_intrinsics\get_reified_type(T2));
  }
}

class C<reify T1, reify T2> extends D<reify (int, (T1, string)), reify T1> {
  public function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T1));
    var_dump(__hhvm_intrinsics\get_reified_type(T2));
    parent::f();
  }
}

$c = new C<(int, string), string>();

$c->f();
