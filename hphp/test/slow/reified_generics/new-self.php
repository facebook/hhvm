<?hh

class C<reify T> {
  function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T));
  }
  function a() {
    $c = new self();
    $c->f();
  }
}

class D<reify T> extends C<bool> {
  function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T));
  }
  function h() {
    $this->a();
  }
}


$d = new D<int>();
$d->h();
