<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

class S {
  const int K = 4;
  const int J = 5;
}

abstract class A {
  abstract const int X = S::K;
}

class C extends A {
  // C has an 86cinit, A's will get merged
  const int Y = S::J;
}

<<__EntryPoint>>
function main(): void {
  var_dump(C::X);

}
