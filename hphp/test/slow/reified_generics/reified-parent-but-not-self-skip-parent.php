<?hh // strict

class E<reify T1, reify T2> {
  public function f() {
    var_dump(HH\ReifiedGenerics\getType<T1>());
    var_dump(HH\ReifiedGenerics\getType<T2>());
  }
}

class D extends E<(int, int), int> {
  public function f() {
    parent::f();
  }
}

class C extends D {
  public function f() {
    parent::f();
  }
}

$c = new C();

$c->f();
