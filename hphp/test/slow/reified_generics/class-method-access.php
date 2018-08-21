<?hh

class C {
  public function f<reified T>() {
    var_dump(__hhvm_intrinsics\get_reified_type(T));
    echo "yep\n";
  }
}

class D<reified T1> {
  public function f<reified T2>() {
    $f = 'f';

    T1::f<reified C>();
    T1::$f<reified C>();

    T2::f<reified C>();
    T2::$f<reified C>();
  }
}

$c = new D<reified C>();
$c->f<reified C>();
