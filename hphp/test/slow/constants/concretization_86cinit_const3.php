<?hh

<<file:__EnableUnstableFeatures('class_const_default')>>

class S {
  const int K = 4;
  const int J = 5;
}

interface I1 {
  abstract const int X = S::K;
}

interface I2 {
  abstract const int Y = S::J;
}

// C does not have an 86cinit, so it will copy I1's
// then I2's will be merged into the copy
class C implements I1, I2 {}

<<__EntryPoint>>
function main(): void {
  var_dump(C::X);
  var_dump(C::Y);
}
