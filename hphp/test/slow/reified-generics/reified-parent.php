<?hh

class E<reify T1, reify T2> {
  public function f() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T1>());
    var_dump(HH\ReifiedGenerics\get_type_structure<T2>());
  }
}

class D<reify T1, reify T2> extends E<(T1, T1), T2> {
  public function f() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T1>());
    var_dump(HH\ReifiedGenerics\get_type_structure<T2>());
  }
}

class C<reify T1, reify T2> extends D<(int, (T1, string)), T1> {
  public function f() :mixed{
    var_dump(HH\ReifiedGenerics\get_type_structure<T1>());
    var_dump(HH\ReifiedGenerics\get_type_structure<T2>());
    parent::f();
  }
}
<<__EntryPoint>> function main(): void {
$c = new C<(int, string), string>();

$c->f();
}
