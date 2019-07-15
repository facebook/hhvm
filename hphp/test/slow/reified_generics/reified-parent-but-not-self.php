<?hh // strict

class E<reify T1, reify T2> {
  public function f() {
    var_dump(HH\ReifiedGenerics\getTypeStructure<T1>());
    var_dump(HH\ReifiedGenerics\getTypeStructure<T2>());
  }
}

class D<reify T1, reify T2> extends E<(T1, T1), T2> {
  public function f() {
    var_dump(HH\ReifiedGenerics\getTypeStructure<T1>());
    var_dump(HH\ReifiedGenerics\getTypeStructure<T2>());
    parent::f();
  }
}

class C extends D<(int, (int, string)), int> {
  public function f() {
    parent::f();
  }
}
<<__EntryPoint>> function main(): void {
$c = new C();

$c->f();
}
