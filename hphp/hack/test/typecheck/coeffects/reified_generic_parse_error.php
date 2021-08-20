<?hh

abstract class A {
  abstract const ctx C;
  abstract const type Tself as A;
}

function f<T as A>()[T::Tself::C]: void {}

class C<T as A> {
  public function m<Tm as A>()[
    T::C,
    Tm::C
  ]: void {}
}
