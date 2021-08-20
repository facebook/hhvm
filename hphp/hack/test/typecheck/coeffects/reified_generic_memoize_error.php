<?hh

class A {
  const ctx C = [];
}
type T = A;

class C<reify T as A> {
  <<__Memoize>> public function m()[T::C]: void {} // not ok

  <<__Memoize>> public function n<reify Tm as A>()[Tm::C]: void {} // not ok
}

<<__Memoize>> function f()[T::C]: void {} // ok, T is an alias

<<__Memoize>> function g<reify T as A>()[T::C]: void {} // not ok
