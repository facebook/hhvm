<?hh

class C<reify T> {
  function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T));
  }
}

class D<reify T> extends C<bool> {
  function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T));
  }
  function h() {
    $c = new parent();
    $c->f();
  }
}


$d = new D<int>();
$d->h();
