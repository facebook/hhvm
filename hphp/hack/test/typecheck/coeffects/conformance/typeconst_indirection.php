<?hh

abstract class A {
  abstract const ctx C;
  abstract const type T as A;
  public abstract function f()[this::C, this::T, this::T::C, this::T::T]: void;
}

class C {
  const ctx C = [];
  const type Tc = mixed;
  public function f()[this::C, this::Tc]: void {}
}
