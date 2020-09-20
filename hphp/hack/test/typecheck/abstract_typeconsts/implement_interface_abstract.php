<?hh

interface I {
  abstract const type T as arraykey;
}
abstract class A {
  <<__Enforceable>>
  abstract const type T as arraykey = string;
}

abstract class C extends A implements I {
  public function f(): this::T {
    return 3 as this::T;
  }
}
