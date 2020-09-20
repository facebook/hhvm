<?hh

interface I {
  <<__Enforceable>>
  abstract const type T as arraykey;
}
abstract class A {
  abstract const type T as arraykey = string;
}

abstract class C extends A implements I {
  public function f(): this::T {
    return 3 as this::T;
  }
}
