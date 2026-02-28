<?hh

<<file:__EnableUnstableFeatures('type_const_super_bound')>>

interface I {
  abstract const type T super int;
}

abstract class A {
  const type T super int = arraykey;  // ERROR: no `abstract`
}
