<?hh

interface I1 {
  abstract const type T as arraykey;
}
interface I2 {
  <<__Enforceable>>
  abstract const type T as arraykey;
}

abstract class C implements I1, I2 {
  public function f(): this::T {
    return 3 as this::T;
  }
}

abstract class D implements I2, I1 {
  public function f(): this::T {
    return 3 as this::T;
  }
}
