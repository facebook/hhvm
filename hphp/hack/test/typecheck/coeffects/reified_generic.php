<?hh

abstract class A {
  abstract const ctx C;
}

class C extends A {
  const ctx C = [write_props];
}

function f<reify T as A>()[T::C]: void {}

function g()[]: void {
  f<C>();
}
