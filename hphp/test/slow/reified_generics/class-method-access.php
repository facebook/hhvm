<?hh

class C {
  public function f<reify T>() {
    var_dump(__hhvm_intrinsics\get_reified_type(T));
    echo "yep\n";
  }
}

class D<reify T1> {
  public function f<reify T2>() {
    $f = 'f';

    T1::f<reify C>();
    T1::$f<reify C>();

    T2::f<reify C>();
    T2::$f<reify C>();
  }
}

$c = new D<reify C>();
$c->f<reify C>();
