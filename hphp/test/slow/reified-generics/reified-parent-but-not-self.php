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
    parent::f();
  }
}

class C extends D<(int, (int, string)), int> {
  public function f() :mixed{
    parent::f();
  }
}
<<__EntryPoint>> function main(): void {
$c = new C();

$c->f();
}
