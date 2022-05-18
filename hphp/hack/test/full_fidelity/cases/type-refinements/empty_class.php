<?hh

<<file:__EnableUnstableFeatures('type_refinements')>>

abstract class A {
  abstract const type T;
}

class G<T as A with {}> {}

class G2<T2 as G<A with {}>> {}
