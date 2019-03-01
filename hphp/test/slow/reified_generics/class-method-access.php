<?hh

class C {
  public function f<reify T>() {
    var_dump(HH\ReifiedGenerics\getType<T>());
    echo "yep\n";
  }
}

class D<reify T1> {
  public function f<reify T2>() {
    $f = 'f';

    T1::f<C>();
    T1::$f<reify C>();

    T2::f<C>();
    T2::$f<reify C>();
  }
}

$c = new D<C>();
$c->f<C>();
