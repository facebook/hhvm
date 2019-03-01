<?hh // strict

class E<reify T1, reify T2> {
  public function f() {
    var_dump(HH\ReifiedGenerics\getType<T1>());
    var_dump(HH\ReifiedGenerics\getType<T2>());
  }
}

class D<reify T1, reify T2> extends E<(T1, T1), T2> {
  public function f() {
    var_dump(HH\ReifiedGenerics\getType<T1>());
    var_dump(HH\ReifiedGenerics\getType<T2>());
    parent::f();
  }
}

class C extends D<(int, (int, string)), int> {
  public function f() {
    parent::f();
  }
}

$c = new C();

$c->f();
