<?hh

class C {
  public function f<reify T>() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T>());
    echo "yep\n";
  }
}

class D<reify T1> {
  public function f<reify T2>() :mixed{
    $f = 'f';

    T1::f<C>();
    T1::$f<reify C>();

    T2::f<C>();
    T2::$f<reify C>();
  }
}
<<__EntryPoint>> function main(): void {
$c = new D<C>();
$c->f<C>();
}
