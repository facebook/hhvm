<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

class S {
  const int K = 4;
}

abstract class A {
  abstract const int X = S::K;
}

// C does not have 86cinit, will copy it from A
class C extends A {}

<<__EntryPoint>>
function main(): void {
  var_dump(C::X);
}
